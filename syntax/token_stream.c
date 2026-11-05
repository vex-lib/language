#include "syntax/token_stream.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
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
    uint64_t compilationContextId;
    Token *tokens;
    size_t count;
    size_t capacity;
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

static bool append(TokenStream *self, LanguageLexResult result,
                   uint64_t previousEnd) {
    SyntaxSpan span = result.span;
    SourceRange range = span.range;
    uint64_t length = TextSnapshot_getLength((*self).snapshot);
    if (range.documentId != TextSnapshot_getDocumentId((*self).snapshot)
        || range.startByte < previousEnd || range.startByte >= range.endByte
        || range.endByte > length || span.role < SYNTAX_ROLE_NONE
        || span.role > SYNTAX_ROLE_IDENTIFIER) {
        THROW("TokenStream: rejected invalid or unordered lexical span");
        return false;
    }
    if ((*self).count == (*self).capacity) {
        size_t next = (*self).capacity == 0u ? 16u : (*self).capacity * 2u;
        if (next < (*self).capacity || next > SIZE_MAX / sizeof(Token)) {
            THROW("TokenStream: token capacity overflow");
            return false;
        }
        Token *grown = realloc((*self).tokens, next * sizeof(Token));
        if (grown == nullptr) {
            THROW("TokenStream: allocation failed for %zu tokens", next);
            return false;
        }
        (*self).tokens = grown;
        (*self).capacity = next;
    }
    (*self).tokens[(*self).count] = Token(tokenKindForRole(span.role), range);
    ++(*self).count;
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
    (*self).snapshot = snapshot;
    uint64_t previousEnd = 0u;
    for (;;) {
        LanguageLexResult result = LanguageEngine_next(engine);
        if (result.documentRevision != TextSnapshot_getRevision(snapshot)
            || result.compilationContextId == 0u
            || ((*self).compilationContextId != 0u
                && (*self).compilationContextId != result.compilationContextId)) {
            THROW("TokenStream: lexer result revision/context mismatch");
            TokenStream_free(self);
            return nullptr;
        }
        if ((*self).compilationContextId == 0u)
            (*self).compilationContextId = result.compilationContextId;
        if (!result.hasSpan) {
            (*self).complete = true;
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

void TokenStream_free(TokenStream *self) {
    if (self == nullptr)
        return;
    TextSnapshot_release((*self).snapshot);
    (*self).snapshot = nullptr;
    free((*self).tokens);
    (*self).tokens = nullptr;
    free(self);
}

uint32_t TokenStream_getDocumentId(const TokenStream *self) {
    return self == nullptr ? 0u : TextSnapshot_getDocumentId((*self).snapshot);
}
uint64_t TokenStream_getDocumentRevision(const TokenStream *self) {
    return self == nullptr ? 0u : TextSnapshot_getRevision((*self).snapshot);
}
uint64_t TokenStream_getCompilationContextId(const TokenStream *self) {
    return self == nullptr ? 0u : (*self).compilationContextId;
}
size_t TokenStream_getCount(const TokenStream *self) {
    return self == nullptr ? 0u : (*self).count;
}
Token TokenStream_getAt(const TokenStream *self, size_t index) {
    return self == nullptr || index >= (*self).count ? Token_zero() : (*self).tokens[index];
}
bool TokenStream_isComplete(const TokenStream *self) {
    return self != nullptr && (*self).complete;
}
BoundedText TokenStream_toString(const TokenStream *self) {
    return self == nullptr ? BoundedText_zero()
        : BoundedText_format("TokenStream(document=%u, revision=%llu, tokens=%zu)",
            TokenStream_getDocumentId(self),
            (unsigned long long) TokenStream_getDocumentRevision(self), (*self).count);
}
BoundedText TokenStream_toStringStruct(const TokenStream *self) {
    return self == nullptr ? BoundedText_zero()
        : BoundedText_format("TokenStream{documentId=%u, revision=%llu, context=%llu, count=%zu, complete=%s}",
            TokenStream_getDocumentId(self),
            (unsigned long long) TokenStream_getDocumentRevision(self),
            (unsigned long long) (*self).compilationContextId, (*self).count,
            (*self).complete ? "true" : "false");
}
