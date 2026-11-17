#include "vex.h"
#include "syntax/token_stream.h"
#include <stdlib.h>

;;DEFINITION
/** TokenStream owns a checked, coarse-token projection of a LanguageEngine's
 * lexical spans and retains the immutable source snapshot they refer to. */

;;OVERVIEW
/** CLASS: TokenStream (syntax/token_stream.c)
 * STRUCT FIELDS: retained snapshot, compilation context id, owned token array,
 * count/capacity and complete marker.
 * PRIVATE HELPERS: tokenKindForRole maps language-neutral syntax roles to
 * shared token categories; append validates byte identity and ordering.
 * FUNCTION REGISTRY: collect, free, identity/context/token accessors, status
 * and bounded projections.
 * CONTRACT: collection consumes to EOF; every span must match snapshot doc,
 * revision and context and be ordered, nonempty and within source bounds. A
 * failed collection frees all partial state and returns null. */

struct TokenStream {
    TextSnapshot *snapshot;
    u64 compilationContextId;
    Token *tokens;
    bytes count;
    bytes capacity;
    bool complete;
};

static TokenKind tokenKindForRole(SyntaxRole role) {
    switch (role) {
        case SYNTAX_ROLE_KEYWORD: return TOKEN_KIND_KEYWORD;
        case SYNTAX_ROLE_STRING: return TOKEN_KIND_STRING;
        case SYNTAX_ROLE_CHARACTER: return TOKEN_KIND_CHARACTER;
        case SYNTAX_ROLE_NUMBER: return TOKEN_KIND_NUMBER;
        case SYNTAX_ROLE_COMMENT:
        case SYNTAX_ROLE_DOC_COMMENT: return TOKEN_KIND_COMMENT;
        case SYNTAX_ROLE_OPERATOR:
        case SYNTAX_ROLE_POINTER:
        case SYNTAX_ROLE_CAST_POINTER:
        case SYNTAX_ROLE_DEREFERENCE:
        case SYNTAX_ROLE_ADDRESS: return TOKEN_KIND_OPERATOR;
        case SYNTAX_ROLE_PUNCTUATION:
        case SYNTAX_ROLE_BRACKET: return TOKEN_KIND_PUNCTUATION;
        case SYNTAX_ROLE_DIRECTIVE:
        case SYNTAX_ROLE_MACRO:
        case SYNTAX_ROLE_MACRO_PARAMETER:
        case SYNTAX_ROLE_MACRO_INVOCATION: return TOKEN_KIND_PREPROCESSOR;
        case SYNTAX_ROLE_TYPE:
        case SYNTAX_ROLE_TYPE_PARAMETER:
        case SYNTAX_ROLE_TYPE_ARGUMENT:
        case SYNTAX_ROLE_FUNCTION:
        case SYNTAX_ROLE_METHOD:
        case SYNTAX_ROLE_CONSTRUCTOR:
        case SYNTAX_ROLE_PARAMETER:
        case SYNTAX_ROLE_FIELD:
        case SYNTAX_ROLE_VARIABLE:
        case SYNTAX_ROLE_CONSTANT:
        case SYNTAX_ROLE_NAMESPACE:
        case SYNTAX_ROLE_LABEL:
        case SYNTAX_ROLE_ANNOTATION:
        case SYNTAX_ROLE_ANNOTATION_SIGIL:
        case SYNTAX_ROLE_ANNOTATION_ARGUMENT:
        case SYNTAX_ROLE_ATTRIBUTE:
        case SYNTAX_ROLE_STRING_ESCAPE:
        case SYNTAX_ROLE_STRING_INTERPOLATION:
        case SYNTAX_ROLE_REGEX:
        case SYNTAX_ROLE_BOOLEAN:
        case SYNTAX_ROLE_NULL:
        case SYNTAX_ROLE_MEMBER:
        case SYNTAX_ROLE_IDENTIFIER: return TOKEN_KIND_IDENTIFIER;
        case SYNTAX_ROLE_NONE:
        case SYNTAX_ROLE_INACTIVE:
        case SYNTAX_ROLE_INVALID: return TOKEN_KIND_UNKNOWN;
    }
    return TOKEN_KIND_UNKNOWN;
}

;;GETTER
static inline TextSnapshot *TokenStream_snapshot(const TokenStream *self) { return (*self).snapshot; }
;;SETTER
static inline fn TokenStream_setSnapshot(TokenStream *self, TextSnapshot *v) { (*self).snapshot = v; }
;;GETTER
static inline u64 TokenStream_compilationContextId(const TokenStream *self) { return (*self).compilationContextId; }
;;SETTER
static inline fn TokenStream_setCompilationContextId(TokenStream *self, u64 v) { (*self).compilationContextId = v; }
;;GETTER
static inline Token *TokenStream_tokens(const TokenStream *self) { return (*self).tokens; }
;;SETTER
static inline fn TokenStream_setTokens(TokenStream *self, Token *v) { (*self).tokens = v; }
;;GETTER
static inline bytes TokenStream_count(const TokenStream *self) { return (*self).count; }
;;SETTER
static inline fn TokenStream_setCount(TokenStream *self, bytes v) { (*self).count = v; }
;;GETTER
static inline bytes TokenStream_capacity(const TokenStream *self) { return (*self).capacity; }
;;SETTER
static inline fn TokenStream_setCapacity(TokenStream *self, bytes v) { (*self).capacity = v; }
;;GETTER
static inline bool TokenStream_complete(const TokenStream *self) { return (*self).complete; }
;;SETTER
static inline fn TokenStream_setComplete(TokenStream *self, bool v) { (*self).complete = v; }

static bool append(TokenStream *self, LanguageLexResult result,
                   u64 previousEnd) {
    SyntaxSpan span = result.span;
    SourceRange range = span.range;
    u64 length = TextSnapshot_getLength(TokenStream_snapshot(self));
    if (range.documentId != TextSnapshot_getDocumentId(TokenStream_snapshot(self))
        || range.startByte < previousEnd || range.startByte >= range.endByte
        || range.endByte > length || span.role < SYNTAX_ROLE_NONE
        || span.role > SYNTAX_ROLE_IDENTIFIER) {
        THROW("TokenStream: rejected invalid or unordered lexical span");
        return false;
    }
    if (TokenStream_count(self) == TokenStream_capacity(self)) {
        bytes next = TokenStream_capacity(self) == 0u ? 16u : TokenStream_capacity(self) * 2u;
        if (next < TokenStream_capacity(self) || next > SIZE_MAX / sizeof(Token)) {
            THROW("TokenStream: token capacity overflow");
            return false;
        }
        Token *grown = realloc(TokenStream_tokens(self), next * sizeof(Token));
        if (grown == nullptr) {
            THROW("TokenStream: allocation failed for %zu tokens", next);
            return false;
        }
        TokenStream_setTokens(self, grown);
        TokenStream_setCapacity(self, next);
    }
    TokenStream_tokens(self)[TokenStream_count(self)] = Token(tokenKindForRole(span.role), range);
    TokenStream_setCount(self, TokenStream_count(self) + 1u);
    return true;
}

TokenStream *TokenStream_collect(LanguageEngine *engine, TextSnapshot *snapshot) {
    if (engine == nullptr || snapshot == nullptr
        || LanguageEngine_getLanguageId(engine) == nullptr
        || TextSnapshot_getDocumentId(snapshot) == 0u
        || TextSnapshot_getRevision(snapshot) == 0u
        || LanguageEngine_getDocumentId(engine) != TextSnapshot_getDocumentId(snapshot)
        || LanguageEngine_getDocumentRevision(engine) != TextSnapshot_getRevision(snapshot)
        || LanguageEngine_getCompilationContextId(engine) == 0u) {
        THROW("TokenStream: rejected unopened engine or invalid snapshot");
        return nullptr;
    }
    TokenStream *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("TokenStream: allocation failed");
        return nullptr;
    }
    if (!TextSnapshot_retain(snapshot)) {
        free(self);
        return nullptr;
    }
    TokenStream_setSnapshot(self, snapshot);
    u64 previousEnd = 0u;
    for (;;) {
        LanguageLexResult result = LanguageEngine_next(engine);
        if (result.documentRevision != TextSnapshot_getRevision(snapshot)
            || result.compilationContextId == 0u
            || (TokenStream_compilationContextId(self) != 0u
                && TokenStream_compilationContextId(self) != result.compilationContextId)) {
            THROW("TokenStream: lexer result revision/context mismatch");
            TokenStream_free(self);
            return nullptr;
        }
        if (TokenStream_compilationContextId(self) == 0u)
            TokenStream_setCompilationContextId(self, result.compilationContextId);
        if (!result.hasSpan) {
            TokenStream_setComplete(self, true);
            return self;
        }
        if (!append(self, result, previousEnd)) {
            TokenStream_free(self);
            return nullptr;
        }
        SourceRange range = result.span.range;
        previousEnd = range.endByte;
    }
}

fn TokenStream_free(TokenStream *self) {
    if (self == nullptr)
        return;
    TextSnapshot_release(TokenStream_snapshot(self));
    TokenStream_setSnapshot(self, nullptr);
    free(TokenStream_tokens(self));
    TokenStream_setTokens(self, nullptr);
    free(self);
}

u32 TokenStream_getDocumentId(const TokenStream *self) {
    return self == nullptr ? 0u : TextSnapshot_getDocumentId(TokenStream_snapshot(self));
}
u64 TokenStream_getDocumentRevision(const TokenStream *self) {
    return self == nullptr ? 0u : TextSnapshot_getRevision(TokenStream_snapshot(self));
}
;;GETTER
u64 TokenStream_getCompilationContextId(const TokenStream *self) {
    return self == nullptr ? 0u : TokenStream_compilationContextId(self);
}
;;GETTER
bytes TokenStream_getCount(const TokenStream *self) {
    return self == nullptr ? 0u : TokenStream_count(self);
}
Token TokenStream_getAt(const TokenStream *self, bytes index) {
    return self == nullptr || index >= TokenStream_count(self) ? Token_zero() : TokenStream_tokens(self)[index];
}
;;GETTER
bool TokenStream_isComplete(const TokenStream *self) {
    return self != nullptr && TokenStream_complete(self);
}
BoundedText TokenStream_toString(const TokenStream *self) {
    return self == nullptr ? BoundedText_zero()
        : BoundedText_format("TokenStream(document=%u, revision=%llu, tokens=%zu)",
            TokenStream_getDocumentId(self),
            (u64) TokenStream_getDocumentRevision(self), TokenStream_count(self));
}
BoundedText TokenStream_toStringStruct(const TokenStream *self) {
    return self == nullptr ? BoundedText_zero()
        : BoundedText_format("TokenStream{documentId=%u, revision=%llu, context=%llu, count=%zu, complete=%s}",
            TokenStream_getDocumentId(self),
            (u64) TokenStream_getDocumentRevision(self),
            (u64) TokenStream_compilationContextId(self), TokenStream_count(self),
            TokenStream_complete(self) ? "true" : "false");
}
