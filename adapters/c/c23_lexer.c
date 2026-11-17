#include "vex.h"
#include "adapters/c/c23_lexer.h"
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/** C23Lexer is a borrowed-snapshot lexical span producer. It gives an editor or
 * renderer byte-accurate SyntaxSpan values without owning language semantics,
 * preprocessing, parsing, symbol resolution or presentation colors. The strict
 * profile recognizes the documented C23 lexical subset; a separately selected
 * profile may style a leading vex `;;` annotation sigil, without validating the
 * annotation. The caller retains immutable source bytes through reset/free.
 *
 * The pull cursor allocates only at construction. next() skips ASCII whitespace,
 * emits one token value, and advances within the admitted byte length. UTF-8
 * validity is checked deterministically: malformed maximal subparts and valid
 * non-ASCII scalars (outside the initial ASCII identifier subset) become
 * invalid spans; NUL is one invalid byte. No locale-dependent ctype behavior is
 * used. */

;;OVERVIEW
/** ============================================================================
 * CLASS: C23Lexer (adapters/c/c23_lexer.c)
 * STRUCT FIELDS (Mirroring adapters/c/c23_lexer.c)
 *   u32 documentId       host-assigned immutable snapshot identity
 *   const char *source        borrowed immutable source bytes
 *   bytes length             admitted source byte count
 *   bytes offset             next unread byte position
 *   C23LexerProfile profile   strict C23 or explicit annotation presentation
 *   bool stopped              cursor is stopped by an impossible span-extent guard
 *   u8 mode               normal, comment, string or character continuation
 *   char quote                active string/character delimiter
 *   bool docComment            current comment token has documentation prefix
 * PRIVATE HELPER RECORDS
 *   Utf8ScalarResult { bool valid; bytes length; } // UTF-8 result and extent
 *   QuotePrefixResult { bool matched; bytes quoteAt; char quote; } // literal prefix
 * PRIVATE HELPERS
 *   identifierStart/Continue(byte) ASCII identifier predicates
 *   continuation(byte)                UTF-8 continuation-byte predicate
 *   utf8SubpartLength(bytes,len,at)   deterministic maximal-subpart extent
 *   validUtf8Scalar(bytes,len,at)     returns validity and scalar extent
 *   startsWith/wordIs                 bounded byte-word comparisons
 *   keywordRole(word,len)             reserved C23 keyword table
 *   skipWhitespace(self,offset)        ASCII whitespace skip
 *   numberEnd(source,len,start)        preprocessing-number-shaped extent
 *   punctuatorEnd(source,len,start)    longest-match punctuator extent
 *   tokenIs(source,start,end,text)     exact token-byte equality
 *   punctuatorRole(source,start,end)   shared role for recognized punctuators
 *   makeSpan(self,start,end,role,key)  document-bound SyntaxSpan construction
 *   quotePrefix(bytes,len,at)          prefixed literal result
 *   scanQuotedPart                     literal admission and continuation
 *   scanComment/invalidSpan            comment continuation and invalid range result
 * FUNCTION REGISTRY
 *   C23Lexer *C23Lexer_0(), C23Lexer *C23Lexer_3(u32,const char*,bytes)
 *   C23Lexer *C23Lexer_zero()
 *   fn C23Lexer_free(C23Lexer*)
 *   bool C23Lexer_setSource(C23Lexer*,u32,const char*,bytes)
 *   bool C23Lexer_setProfile(C23Lexer*,C23LexerProfile)
 *   u32 C23Lexer_getDocumentId(const C23Lexer*)
 *   const char *C23Lexer_getSource(const C23Lexer*)
 *   bytes C23Lexer_getLength/getOffset(const C23Lexer*)
 *   C23LexerProfile C23Lexer_getProfile(const C23Lexer*)
 *   bool C23Lexer_isDone(const C23Lexer*)
 *   C23LexStep C23Lexer_next(C23Lexer*)
 *   BoundedText C23Lexer_toString/toStringStruct(const C23Lexer*)
 * CONTRACT
 *   - Source is borrowed, immutable during use, and retained by caller.
 *   - Rejected source/profile changes preserve the complete prior state.
 *   - next returns a fully initialized step; no span means a zero SyntaxSpan.
 *   - Spans use constructor documentId and valid half-open byte offsets.
 *   - No semantic naming heuristics, preprocessor, parser, or hint generation.
 * ============================================================================ */

struct C23Lexer {
    u32 documentId;
    const char *source;
    bytes length;
    bytes offset;
    C23LexerProfile profile;
    bool stopped;
    u8 mode;
    char quote;
    bool docComment;
};

;;GETTER
static inline u32 C23Lexer_documentId(const C23Lexer *self) { return (*self).documentId; }
;;SETTER
static inline fn C23Lexer_setDocumentId(C23Lexer *self, u32 v) { (*self).documentId = v; }
;;GETTER
static inline const char *C23Lexer_source(const C23Lexer *self) { return (*self).source; }
;;SETTER
static inline fn C23Lexer_setSourcePtr(C23Lexer *self, const char *v) { (*self).source = v; }
;;GETTER
static inline bytes C23Lexer_length(const C23Lexer *self) { return (*self).length; }
;;SETTER
static inline fn C23Lexer_setLength(C23Lexer *self, bytes v) { (*self).length = v; }
;;GETTER
static inline bytes C23Lexer_offset(const C23Lexer *self) { return (*self).offset; }
;;SETTER
static inline fn C23Lexer_setOffset(C23Lexer *self, bytes v) { (*self).offset = v; }
;;GETTER
static inline C23LexerProfile C23Lexer_profile(const C23Lexer *self) { return (*self).profile; }
;;SETTER
static inline fn C23Lexer_setProfileField(C23Lexer *self, C23LexerProfile v) { (*self).profile = v; }
;;GETTER
static inline bool C23Lexer_stopped(const C23Lexer *self) { return (*self).stopped; }
;;SETTER
static inline fn C23Lexer_setStopped(C23Lexer *self, bool v) { (*self).stopped = v; }
;;GETTER
static inline u8 C23Lexer_mode(const C23Lexer *self) { return (*self).mode; }
;;SETTER
static inline fn C23Lexer_setMode(C23Lexer *self, u8 v) { (*self).mode = v; }
;;GETTER
static inline char C23Lexer_quote(const C23Lexer *self) { return (*self).quote; }
;;SETTER
static inline fn C23Lexer_setQuote(C23Lexer *self, char v) { (*self).quote = v; }
;;GETTER
static inline bool C23Lexer_docComment(const C23Lexer *self) { return (*self).docComment; }
;;SETTER
static inline fn C23Lexer_setDocComment(C23Lexer *self, bool v) { (*self).docComment = v; }

enum {
    C23_LEXER_MODE_NORMAL = 0u,
    C23_LEXER_MODE_LINE_COMMENT,
    C23_LEXER_MODE_BLOCK_COMMENT,
    C23_LEXER_MODE_STRING,
    C23_LEXER_MODE_CHARACTER
};

static bool identifierStart(u8 byte) {
    return (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') || byte == '_';
}

static bool identifierContinue(u8 byte) {
    return identifierStart(byte) || (byte >= '0' && byte <= '9');
}

static bool continuation(u8 byte) {
    return byte >= 0x80u && byte <= 0xBFu;
}

static bytes utf8SubpartLength(const u8 *data, bytes length, bytes offset) {
    u8 lead = data[offset];
    if (lead < 0x80u)
        return 1u;
    if (lead >= 0xC2u && lead <= 0xDFu) {
        if (offset + 1u < length && continuation(data[offset + 1u])) return 2u;
        return 1u;
    }
    if (lead >= 0xE0u && lead <= 0xEFu) {
        if (offset + 1u >= length) return 1u;
        u8 second = data[offset + 1u];
        bool secondValid = continuation(second)
            && !(lead == 0xE0u && second < 0xA0u)
            && !(lead == 0xEDu && second > 0x9Fu);
        if (!secondValid) return 1u;
        if (offset + 2u < length && continuation(data[offset + 2u])) return 3u;
        return 2u;
    }
    if (lead >= 0xF0u && lead <= 0xF4u) {
        if (offset + 1u >= length) return 1u;
        u8 second = data[offset + 1u];
        bool secondValid = continuation(second)
            && !(lead == 0xF0u && second < 0x90u)
            && !(lead == 0xF4u && second > 0x8Fu);
        if (!secondValid) return 1u;
        if (offset + 2u >= length || !continuation(data[offset + 2u])) return 2u;
        if (offset + 3u < length && continuation(data[offset + 3u])) return 4u;
        return 3u;
    }
    return 1u;
}

typedef struct Utf8ScalarResult {
    bool valid;
    bytes length;
} Utf8ScalarResult;

typedef struct QuotePrefixResult {
    bool matched;
    bytes quoteAt;
    char quote;
} QuotePrefixResult;

static Utf8ScalarResult validUtf8Scalar(const u8 *data, bytes length,
                                        bytes offset) {
    bytes span = utf8SubpartLength(data, length, offset);
    bool valid = data[offset] >= 0xC2u && data[offset] <= 0xF4u
        && ((data[offset] <= 0xDFu && span == 2u)
            || (data[offset] <= 0xEFu && span == 3u)
            || (data[offset] >= 0xF0u && span == 4u));
    return (Utf8ScalarResult) { .valid = valid, .length = span };
}

static bool startsWith(const char *source, bytes length, bytes offset, const char *text) {
    bytes textLength = strlen(text);
    return offset <= length && textLength <= length - offset
        && memcmp(source + offset, text, textLength) == 0;
}

static bool wordIs(const char *source, bytes length, const char *text) {
    return strlen(text) == length && memcmp(source, text, length) == 0;
}

static SyntaxRole keywordRole(const char *word, bytes length) {
    static const char *const keywords[] = {
        "alignas", "alignof", "auto", "bool", "break", "case", "char", "char8_t",
        "const", "constexpr", "continue", "default", "do", "d64", "else", "enum",
        "extern", "false", "f32", "for", "goto", "if", "inline", "int", "long",
        "nullptr", "register", "restrict", "return", "short", "signed", "sizeof", "static",
        "static_assert", "struct", "switch", "thread_local", "true", "typedef", "typeof",
        "typeof_unqual", "union", "unsigned", "void", "volatile", "while", "_Alignas",
        "_Alignof", "_Atomic", "_BitInt", "_Bool", "_Complex", "_Decimal32", "_Decimal64",
        "_Decimal128", "_Generic", "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
    };
    for (bytes index = 0u; index < sizeof keywords / sizeof keywords[0]; ++index)
        if (wordIs(word, length, keywords[index]))
            return SYNTAX_ROLE_KEYWORD;
    return SYNTAX_ROLE_IDENTIFIER;
}

static bytes skipWhitespace(const C23Lexer *self, bytes offset) {
    while (offset < C23Lexer_length(self)) {
        u8 byte = (u8) C23Lexer_source(self)[offset];
        if (byte != ' ' && byte != '\t' && byte != '\r' && byte != '\n'
            && byte != '\f' && byte != '\v')
            break;
        ++offset;
    }
    return offset;
}

static bytes numberEnd(const char *source, bytes length, bytes start) {
    bytes offset = start;
    while (offset < length) {
        u8 byte = (u8) source[offset];
        if (identifierContinue(byte) || byte == '.' ) {
            ++offset;
            continue;
        }
        if (byte == '\'' && offset + 1u < length
            && (identifierStart((u8) source[offset + 1u])
                || (source[offset + 1u] >= '0' && source[offset + 1u] <= '9'))) {
            offset += 2u;
            continue;
        }
        if ((byte == '+' || byte == '-') && offset > start
            && (source[offset - 1u] == 'e' || source[offset - 1u] == 'E'
                || source[offset - 1u] == 'p' || source[offset - 1u] == 'P')) {
            ++offset;
            continue;
        }
        break;
    }
    return offset;
}

static bytes punctuatorEnd(const char *source, bytes length, bytes start) {
    static const char *const punctuators[] = {
        "%:%:", "<<=", ">>=", "...", "->", "++", "--", "&&", "||",
        "<=", ">=", "==", "!=", "*=", "/=", "%=", "+=", "-=", "<<", ">>",
        "&=", "^=", "|=", "##", "<:", ":>", "<%", "%>", "%:",
        "[", "]", "(", ")", "{", "}", ".", "&", "*", "+", "-",
        "~", "!", "/", "%", "<", ">", "^", "|", "?", ":", ";", "=", ",", "#"
    };
    for (bytes index = 0u; index < sizeof punctuators / sizeof punctuators[0]; ++index)
        if (startsWith(source, length, start, punctuators[index]))
            return start + strlen(punctuators[index]);
    return start;
}

static bool tokenIs(const char *source, bytes start, bytes end, const char *token) {
    bytes tokenLength = strlen(token);
    return end - start == tokenLength && memcmp(source + start, token, tokenLength) == 0;
}

static SyntaxRole punctuatorRole(const char *source, bytes start, bytes end) {
    static const char *const brackets[] = {"<:", ":>", "<%", "%>"};
    static const char *const punctuation[] = {"...", "#", "##", "%:", "%:%:"};
    if ((end - start == 1u && strchr("()[]{}", source[start]) != nullptr))
        return SYNTAX_ROLE_BRACKET;
    for (bytes index = 0u; index < sizeof brackets / sizeof brackets[0]; ++index)
        if (tokenIs(source, start, end, brackets[index]))
            return SYNTAX_ROLE_BRACKET;
    if ((end - start == 1u && strchr(".,:;", source[start]) != nullptr))
        return SYNTAX_ROLE_PUNCTUATION;
    for (bytes index = 0u; index < sizeof punctuation / sizeof punctuation[0]; ++index)
        if (tokenIs(source, start, end, punctuation[index]))
            return SYNTAX_ROLE_PUNCTUATION;
    return SYNTAX_ROLE_OPERATOR;
}

static SyntaxSpan makeSpan(const C23Lexer *self, bytes start, bytes end,
                           SyntaxRole role, const char *styleKey) {
    return SyntaxSpan_3(SourceRange_3(C23Lexer_documentId(self), start, end), role, styleKey);
}

static QuotePrefixResult quotePrefix(const char *source, bytes length, bytes start) {
    static const char *const prefixes[] = {"u8", "u", "U", "L"};
    if (source[start] == '"' || source[start] == '\'') {
        return (QuotePrefixResult) {
            .matched = true, .quoteAt = start, .quote = source[start]
        };
    }
    for (bytes index = 0u; index < sizeof prefixes / sizeof prefixes[0]; ++index) {
        bytes prefixLength = strlen(prefixes[index]);
        bytes at = start + prefixLength;
        if (startsWith(source, length, start, prefixes[index]) && at < length
            && (source[at] == '"' || source[at] == '\'')) {
            return (QuotePrefixResult) {
                .matched = true, .quoteAt = at, .quote = source[at]
            };
        }
    }
    return (QuotePrefixResult) {0};
}

static C23LexStep invalidSpan(C23Lexer *self, bytes start, bytes end) {
    C23Lexer_setOffset(self, end);
    return C23LexStep_2(true, makeSpan(self, start, end, SYNTAX_ROLE_INVALID, nullptr));
}

static C23LexStep scanComment(C23Lexer *self, bytes start) {
    const char *source = C23Lexer_source(self);
    bool blockComment = C23Lexer_mode(self) == C23_LEXER_MODE_BLOCK_COMMENT;
    bytes end = C23Lexer_offset(self) > start ? C23Lexer_offset(self) : start;
    while (end < C23Lexer_length(self)) {
        u8 byte = (u8) source[end];
        if (C23Lexer_mode(self) == C23_LEXER_MODE_LINE_COMMENT && (byte == '\n' || byte == '\r')) {
            C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
            C23Lexer_setOffset(self, end);
            if (end == start)
                return C23Lexer_next(self);
            return C23LexStep_2(true, makeSpan(self, start, end,
                C23Lexer_docComment(self) ? SYNTAX_ROLE_DOC_COMMENT : SYNTAX_ROLE_COMMENT, nullptr));
        }
        if (C23Lexer_mode(self) == C23_LEXER_MODE_BLOCK_COMMENT && byte == '*' && end + 1u < C23Lexer_length(self)
            && source[end + 1u] == '/') {
            end += 2u;
            C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
            C23Lexer_setOffset(self, end);
            return C23LexStep_2(true, makeSpan(self, start, end,
                C23Lexer_docComment(self) ? SYNTAX_ROLE_DOC_COMMENT : SYNTAX_ROLE_COMMENT, nullptr));
        }
        if (byte >= 0x80u) {
            Utf8ScalarResult scalar = validUtf8Scalar((const u8*) source,
                                                       C23Lexer_length(self), end);
            if (!scalar.valid) {
                if (end != start) {
                    C23Lexer_setOffset(self, end);
                    return C23LexStep_2(true, makeSpan(self, start, end,
                        C23Lexer_docComment(self) ? SYNTAX_ROLE_DOC_COMMENT : SYNTAX_ROLE_COMMENT, nullptr));
                }
                return invalidSpan(self, end, end + scalar.length);
            }
            end += scalar.length;
        } else if (byte == 0u) {
            if (end != start) {
                C23Lexer_setOffset(self, end);
                return C23LexStep_2(true, makeSpan(self, start, end,
                    C23Lexer_docComment(self) ? SYNTAX_ROLE_DOC_COMMENT : SYNTAX_ROLE_COMMENT, nullptr));
            }
            return invalidSpan(self, end, end + 1u);
        } else {
            ++end;
        }
    }
    C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
    if (blockComment) {
        C23Lexer_setOffset(self, end);
        return C23LexStep_2(true, makeSpan(self, start, end, SYNTAX_ROLE_INVALID, nullptr));
    }
    C23Lexer_setOffset(self, end);
    return C23LexStep_2(true, makeSpan(self, start, end,
        C23Lexer_docComment(self) ? SYNTAX_ROLE_DOC_COMMENT : SYNTAX_ROLE_COMMENT, nullptr));
}

static C23LexStep scanQuotedPart(C23Lexer *self, bytes start, bool opening) {
    const char *source = C23Lexer_source(self);
    bytes end = start;
    if (opening) {
        QuotePrefixResult prefix = quotePrefix(source, C23Lexer_length(self), start);
        C23Lexer_setQuote(self, prefix.quote);
        C23Lexer_setMode(self, prefix.quote == '"'
            ? C23_LEXER_MODE_STRING : C23_LEXER_MODE_CHARACTER);
        end = prefix.quoteAt + 1u;
    }
    while (end < C23Lexer_length(self)) {
        u8 byte = (u8) source[end];
        if (byte == 0u) {
            C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
            C23Lexer_setQuote(self, 0);
            return invalidSpan(self, start, end + 1u);
        }
        if (byte == '\n' || byte == '\r') {
            C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
            if (end == start)
                return C23Lexer_next(self);
            C23Lexer_setOffset(self, end);
            return C23LexStep_2(true, makeSpan(self, start, end, SYNTAX_ROLE_INVALID, nullptr));
        }
        if (byte == '\\') {
            if (end + 1u >= C23Lexer_length(self)) {
                C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
                return invalidSpan(self, start, C23Lexer_length(self));
            }
            u8 escaped = (u8) source[end + 1u];
            if (escaped == '\n' || escaped == '\r') {
                C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
                C23Lexer_setQuote(self, 0);
                return invalidSpan(self, start, end + 1u);
            }
            if (escaped == 0u || escaped >= 0x80u) {
                ++end;
                continue;
            }
            end += 2u;
            continue;
        }
        if (byte == (u8) C23Lexer_quote(self)) {
            ++end;
            C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
            C23Lexer_setOffset(self, end);
            SyntaxRole role = C23Lexer_quote(self) == '"' ? SYNTAX_ROLE_STRING : SYNTAX_ROLE_CHARACTER;
            return C23LexStep_2(true, makeSpan(self, start, end, role, nullptr));
        }
        if (byte >= 0x80u) {
            Utf8ScalarResult scalar = validUtf8Scalar((const u8*) source,
                                                       C23Lexer_length(self), end);
            if (!scalar.valid) {
                if (end != start) {
                    C23Lexer_setOffset(self, end);
                    return C23LexStep_2(true, makeSpan(self, start, end,
                        C23Lexer_quote(self) == '"' ? SYNTAX_ROLE_STRING : SYNTAX_ROLE_CHARACTER, nullptr));
                }
                return invalidSpan(self, end, end + scalar.length);
            }
            end += scalar.length;
        } else {
            ++end;
        }
    }
    C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
    return invalidSpan(self, start, C23Lexer_length(self));
}

C23Lexer *C23Lexer_0() {
    C23Lexer *self = calloc(1u, sizeof *self);
    if (self == nullptr)
        THROW("C23Lexer could not allocate empty lexer");
    return self;
}

C23Lexer *C23Lexer_3(u32 documentId, const char *source, bytes length) {
    C23Lexer *self = C23Lexer_0();
    if (self == nullptr)
        return nullptr;
    if (!C23Lexer_setSource(self, documentId, source, length)) {
        C23Lexer_free(self);
        return nullptr;
    }
    return self;
}

C23Lexer *C23Lexer_zero() {
    return C23Lexer_0();
}

fn C23Lexer_free(C23Lexer *self) {
    free(self);
}

bool C23Lexer_setSource(C23Lexer *self, u32 documentId,
                        const char *source, bytes length) {
    if (self == nullptr || documentId == 0u || (source == nullptr && length != 0u)) {
        THROW("C23Lexer rejected source admission: document id/source invalid for %zu bytes", length);
        return false;
    }
    C23Lexer_setDocumentId(self, documentId);
    C23Lexer_setSourcePtr(self, source == nullptr ? "" : source);
    C23Lexer_setLength(self, length);
    C23Lexer_setOffset(self, 0u);
    C23Lexer_setStopped(self, false);
    C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
    C23Lexer_setQuote(self, 0);
    C23Lexer_setDocComment(self, false);
    return true;
}

bool C23Lexer_setProfile(C23Lexer *self, C23LexerProfile profile) {
    if (self == nullptr || (profile != C23_LEXER_PROFILE_STRICT_C23
                            && profile != C23_LEXER_PROFILE_VEX_ANNOTATION)) {
        THROW("C23Lexer rejected unsupported lexical profile %u", (u32) profile);
        return false;
    }
    C23Lexer_setProfileField(self, profile);
    return true;
}

u32 C23Lexer_getDocumentId(const C23Lexer *self) {
    return self == nullptr ? 0u : C23Lexer_documentId(self);
}
const char *C23Lexer_getSource(const C23Lexer *self) {
    return self == nullptr ? nullptr : C23Lexer_source(self);
}
bytes C23Lexer_getLength(const C23Lexer *self) {
    return self == nullptr ? 0u : C23Lexer_length(self);
}
bytes C23Lexer_getOffset(const C23Lexer *self) {
    return self == nullptr ? 0u : C23Lexer_offset(self);
}
C23LexerProfile C23Lexer_getProfile(const C23Lexer *self) {
    return self == nullptr ? C23_LEXER_PROFILE_STRICT_C23 : C23Lexer_profile(self);
}
bool C23Lexer_isDone(const C23Lexer *self) {
    return self == nullptr || C23Lexer_stopped(self)
        || (C23Lexer_mode(self) == C23_LEXER_MODE_NORMAL
            && skipWhitespace(self, C23Lexer_offset(self)) >= C23Lexer_length(self));
}

C23LexStep C23Lexer_next(C23Lexer *self) {
    if (self == nullptr || C23Lexer_stopped(self))
        return C23LexStep_0();
    if (C23Lexer_mode(self) != C23_LEXER_MODE_NORMAL && C23Lexer_offset(self) >= C23Lexer_length(self)) {
        C23Lexer_setMode(self, C23_LEXER_MODE_NORMAL);
        return C23LexStep_0();
    }
    bytes start = C23Lexer_mode(self) == C23_LEXER_MODE_NORMAL
        ? skipWhitespace(self, C23Lexer_offset(self)) : C23Lexer_offset(self);
    if (start >= C23Lexer_length(self)) {
        C23Lexer_setOffset(self, C23Lexer_length(self));
        return C23LexStep_0();
    }
    const char *source = C23Lexer_source(self);
    bytes length = C23Lexer_length(self);
    if (C23Lexer_mode(self) == C23_LEXER_MODE_LINE_COMMENT
        || C23Lexer_mode(self) == C23_LEXER_MODE_BLOCK_COMMENT)
        return scanComment(self, start);
    if (C23Lexer_mode(self) == C23_LEXER_MODE_STRING || C23Lexer_mode(self) == C23_LEXER_MODE_CHARACTER)
        return scanQuotedPart(self, start, false);
    bytes end = start;
    SyntaxRole role = SYNTAX_ROLE_PUNCTUATION;
    const char *styleKey = nullptr;
    u8 byte = (u8) source[start];

    if (byte == 0u) {
        end = start + 1u;
        role = SYNTAX_ROLE_INVALID;
    } else if (byte >= 0x80u) {
        Utf8ScalarResult scalar = validUtf8Scalar((const u8*) source,
                                                   length, start);
        end = start + scalar.length;
        role = SYNTAX_ROLE_INVALID;
    } else if (startsWith(source, length, start, "//") || startsWith(source, length, start, "/*")) {
        bool lineComment = startsWith(source, length, start, "//");
        C23Lexer_setMode(self, lineComment ? C23_LEXER_MODE_LINE_COMMENT : C23_LEXER_MODE_BLOCK_COMMENT);
        C23Lexer_setDocComment(self, lineComment
            ? startsWith(source, length, start, "///") || startsWith(source, length, start, "//!")
            : startsWith(source, length, start, "/**") || startsWith(source, length, start, "/*!"));
        C23Lexer_setOffset(self, start + 2u);
        return scanComment(self, start);
    } else {
        QuotePrefixResult prefix = quotePrefix(source, length, start);
        if (prefix.matched) {
            return scanQuotedPart(self, start, true);
        } else if (identifierStart(byte)) {
            end = start + 1u;
            while (end < length && identifierContinue((u8) source[end]))
                ++end;
            role = keywordRole(source + start, end - start);
        } else if ((byte >= '0' && byte <= '9')
                   || (byte == '.' && start + 1u < length
                       && source[start + 1u] >= '0' && source[start + 1u] <= '9')) {
            end = numberEnd(source, length, start);
            role = SYNTAX_ROLE_NUMBER;
        } else if (C23Lexer_profile(self) == C23_LEXER_PROFILE_VEX_ANNOTATION
                   && startsWith(source, length, start, ";;")
                   && start + 2u < length
                   && identifierStart((u8) source[start + 2u])) {
            bytes lineStart = start;
            while (lineStart > 0u && source[lineStart - 1u] != '\n' && source[lineStart - 1u] != '\r')
                --lineStart;
            bool onlyIndent = true;
            for (bytes at = lineStart; at < start; ++at)
                if (source[at] != ' ' && source[at] != '\t') onlyIndent = false;
            if (onlyIndent) {
                end = start + 2u;
                role = SYNTAX_ROLE_ANNOTATION_SIGIL;
                styleKey = "annotation.sigil";
            }
        }
        if (end == start) {
            end = punctuatorEnd(source, length, start);
            if (end == start) {
                end = start + 1u;
                role = SYNTAX_ROLE_INVALID;
            } else {
                role = punctuatorRole(source, start, end);
            }
        }
    }

    if (end <= start || end > length) {
        C23Lexer_setStopped(self, true);
        return C23LexStep_2(true, makeSpan(self, start, start + 1u, SYNTAX_ROLE_INVALID, nullptr));
    }
    C23Lexer_setOffset(self, end);
    return C23LexStep_2(true, makeSpan(self, start, end, role, styleKey));
}

BoundedText C23Lexer_toString(const C23Lexer *self) {
    if (self == nullptr) return BoundedText_1("nullptr");
    return BoundedText_format("C23Lexer(document=%u, length=%zu, offset=%zu)",
                              C23Lexer_documentId(self), C23Lexer_length(self), C23Lexer_offset(self));
}

BoundedText C23Lexer_toStringStruct(const C23Lexer *self) {
    if (self == nullptr) return BoundedText_1("nullptr");
    return BoundedText_format("C23Lexer { documentId=%u, source=%p, length=%zu, offset=%zu, profile=%u, stopped=%s, mode=%u, quote=%u, docComment=%s }",
                              C23Lexer_documentId(self), (any) C23Lexer_source(self), C23Lexer_length(self),
                              C23Lexer_offset(self), (u32) C23Lexer_profile(self),
                              C23Lexer_stopped(self) ? "true" : "false", (u32) C23Lexer_mode(self),
                              (u8) C23Lexer_quote(self),
                              C23Lexer_docComment(self) ? "true" : "false");
}
