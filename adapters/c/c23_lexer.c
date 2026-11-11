#include "adapters/c/c23_lexer.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
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
 *   C23Lexer *C23Lexer_0(void), C23Lexer *C23Lexer_3(u32,const char*,bytes)
 *   C23Lexer *C23Lexer_zero(void)
 *   void C23Lexer_free(C23Lexer*)
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

enum {
    C23_LEXER_MODE_NORMAL = 0u,
    C23_LEXER_MODE_LINE_COMMENT,
    C23_LEXER_MODE_BLOCK_COMMENT,
    C23_LEXER_MODE_STRING,
    C23_LEXER_MODE_CHARACTER
};

static bool identifierStart(unsigned char byte) {
    return (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') || byte == '_';
}

static bool identifierContinue(unsigned char byte) {
    return identifierStart(byte) || (byte >= '0' && byte <= '9');
}

static bool continuation(unsigned char byte) {
    return byte >= 0x80u && byte <= 0xBFu;
}

static bytes utf8SubpartLength(const unsigned char *bytes, bytes length, bytes offset) {
    unsigned char lead = bytes[offset];
    if (lead < 0x80u)
        return 1u;
    if (lead >= 0xC2u && lead <= 0xDFu) {
        if (offset + 1u < length && continuation(bytes[offset + 1u])) return 2u;
        return 1u;
    }
    if (lead >= 0xE0u && lead <= 0xEFu) {
        if (offset + 1u >= length) return 1u;
        unsigned char second = bytes[offset + 1u];
        bool secondValid = continuation(second)
            && !(lead == 0xE0u && second < 0xA0u)
            && !(lead == 0xEDu && second > 0x9Fu);
        if (!secondValid) return 1u;
        if (offset + 2u < length && continuation(bytes[offset + 2u])) return 3u;
        return 2u;
    }
    if (lead >= 0xF0u && lead <= 0xF4u) {
        if (offset + 1u >= length) return 1u;
        unsigned char second = bytes[offset + 1u];
        bool secondValid = continuation(second)
            && !(lead == 0xF0u && second < 0x90u)
            && !(lead == 0xF4u && second > 0x8Fu);
        if (!secondValid) return 1u;
        if (offset + 2u >= length || !continuation(bytes[offset + 2u])) return 2u;
        if (offset + 3u < length && continuation(bytes[offset + 3u])) return 4u;
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

static Utf8ScalarResult validUtf8Scalar(const unsigned char *bytes, bytes length,
                                        bytes offset) {
    bytes span = utf8SubpartLength(bytes, length, offset);
    bool valid = bytes[offset] >= 0xC2u && bytes[offset] <= 0xF4u
        && ((bytes[offset] <= 0xDFu && span == 2u)
            || (bytes[offset] <= 0xEFu && span == 3u)
            || (bytes[offset] >= 0xF0u && span == 4u));
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
    while (offset < (*self).length) {
        unsigned char byte = (unsigned char) (*self).source[offset];
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
        unsigned char byte = (unsigned char) source[offset];
        if (identifierContinue(byte) || byte == '.' ) {
            ++offset;
            continue;
        }
        if (byte == '\'' && offset + 1u < length
            && (identifierStart((unsigned char) source[offset + 1u])
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
    return SyntaxSpan_3(SourceRange_3((*self).documentId, start, end), role, styleKey);
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
    (*self).offset = end;
    return C23LexStep_2(true, makeSpan(self, start, end, SYNTAX_ROLE_INVALID, nullptr));
}

static C23LexStep scanComment(C23Lexer *self, bytes start) {
    const char *source = (*self).source;
    bool blockComment = (*self).mode == C23_LEXER_MODE_BLOCK_COMMENT;
    bytes end = (*self).offset > start ? (*self).offset : start;
    while (end < (*self).length) {
        unsigned char byte = (unsigned char) source[end];
        if ((*self).mode == C23_LEXER_MODE_LINE_COMMENT && (byte == '\n' || byte == '\r')) {
            (*self).mode = C23_LEXER_MODE_NORMAL;
            (*self).offset = end;
            if (end == start)
                return C23Lexer_next(self);
            return C23LexStep_2(true, makeSpan(self, start, end,
                (*self).docComment ? SYNTAX_ROLE_DOC_COMMENT : SYNTAX_ROLE_COMMENT, nullptr));
        }
        if ((*self).mode == C23_LEXER_MODE_BLOCK_COMMENT && byte == '*' && end + 1u < (*self).length
            && source[end + 1u] == '/') {
            end += 2u;
            (*self).mode = C23_LEXER_MODE_NORMAL;
            (*self).offset = end;
            return C23LexStep_2(true, makeSpan(self, start, end,
                (*self).docComment ? SYNTAX_ROLE_DOC_COMMENT : SYNTAX_ROLE_COMMENT, nullptr));
        }
        if (byte >= 0x80u) {
            Utf8ScalarResult scalar = validUtf8Scalar((const unsigned char *) source,
                                                       (*self).length, end);
            if (!scalar.valid) {
                if (end != start) {
                    (*self).offset = end;
                    return C23LexStep_2(true, makeSpan(self, start, end,
                        (*self).docComment ? SYNTAX_ROLE_DOC_COMMENT : SYNTAX_ROLE_COMMENT, nullptr));
                }
                return invalidSpan(self, end, end + scalar.length);
            }
            end += scalar.length;
        } else if (byte == 0u) {
            if (end != start) {
                (*self).offset = end;
                return C23LexStep_2(true, makeSpan(self, start, end,
                    (*self).docComment ? SYNTAX_ROLE_DOC_COMMENT : SYNTAX_ROLE_COMMENT, nullptr));
            }
            return invalidSpan(self, end, end + 1u);
        } else {
            ++end;
        }
    }
    (*self).mode = C23_LEXER_MODE_NORMAL;
    if (blockComment) {
        (*self).offset = end;
        return C23LexStep_2(true, makeSpan(self, start, end, SYNTAX_ROLE_INVALID, nullptr));
    }
    (*self).offset = end;
    return C23LexStep_2(true, makeSpan(self, start, end,
        (*self).docComment ? SYNTAX_ROLE_DOC_COMMENT : SYNTAX_ROLE_COMMENT, nullptr));
}

static C23LexStep scanQuotedPart(C23Lexer *self, bytes start, bool opening) {
    const char *source = (*self).source;
    bytes end = start;
    if (opening) {
        QuotePrefixResult prefix = quotePrefix(source, (*self).length, start);
        (*self).quote = prefix.quote;
        (*self).mode = prefix.quote == '"'
            ? C23_LEXER_MODE_STRING : C23_LEXER_MODE_CHARACTER;
        end = prefix.quoteAt + 1u;
    }
    while (end < (*self).length) {
        unsigned char byte = (unsigned char) source[end];
        if (byte == 0u) {
            (*self).mode = C23_LEXER_MODE_NORMAL;
            (*self).quote = 0;
            return invalidSpan(self, start, end + 1u);
        }
        if (byte == '\n' || byte == '\r') {
            (*self).mode = C23_LEXER_MODE_NORMAL;
            if (end == start)
                return C23Lexer_next(self);
            (*self).offset = end;
            return C23LexStep_2(true, makeSpan(self, start, end, SYNTAX_ROLE_INVALID, nullptr));
        }
        if (byte == '\\') {
            if (end + 1u >= (*self).length) {
                (*self).mode = C23_LEXER_MODE_NORMAL;
                return invalidSpan(self, start, (*self).length);
            }
            unsigned char escaped = (unsigned char) source[end + 1u];
            if (escaped == '\n' || escaped == '\r') {
                (*self).mode = C23_LEXER_MODE_NORMAL;
                (*self).quote = 0;
                return invalidSpan(self, start, end + 1u);
            }
            if (escaped == 0u || escaped >= 0x80u) {
                ++end;
                continue;
            }
            end += 2u;
            continue;
        }
        if (byte == (unsigned char) (*self).quote) {
            ++end;
            (*self).mode = C23_LEXER_MODE_NORMAL;
            (*self).offset = end;
            SyntaxRole role = (*self).quote == '"' ? SYNTAX_ROLE_STRING : SYNTAX_ROLE_CHARACTER;
            return C23LexStep_2(true, makeSpan(self, start, end, role, nullptr));
        }
        if (byte >= 0x80u) {
            Utf8ScalarResult scalar = validUtf8Scalar((const unsigned char *) source,
                                                       (*self).length, end);
            if (!scalar.valid) {
                if (end != start) {
                    (*self).offset = end;
                    return C23LexStep_2(true, makeSpan(self, start, end,
                        (*self).quote == '"' ? SYNTAX_ROLE_STRING : SYNTAX_ROLE_CHARACTER, nullptr));
                }
                return invalidSpan(self, end, end + scalar.length);
            }
            end += scalar.length;
        } else {
            ++end;
        }
    }
    (*self).mode = C23_LEXER_MODE_NORMAL;
    return invalidSpan(self, start, (*self).length);
}

C23Lexer *C23Lexer_0(void) {
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

C23Lexer *C23Lexer_zero(void) {
    return C23Lexer_0();
}

void C23Lexer_free(C23Lexer *self) {
    free(self);
}

bool C23Lexer_setSource(C23Lexer *self, u32 documentId,
                        const char *source, bytes length) {
    if (self == nullptr || documentId == 0u || (source == nullptr && length != 0u)) {
        THROW("C23Lexer rejected source admission: document id/source invalid for %zu bytes", length);
        return false;
    }
    (*self).documentId = documentId;
    (*self).source = source == nullptr ? "" : source;
    (*self).length = length;
    (*self).offset = 0u;
    (*self).stopped = false;
    (*self).mode = C23_LEXER_MODE_NORMAL;
    (*self).quote = 0;
    (*self).docComment = false;
    return true;
}

bool C23Lexer_setProfile(C23Lexer *self, C23LexerProfile profile) {
    if (self == nullptr || (profile != C23_LEXER_PROFILE_STRICT_C23
                            && profile != C23_LEXER_PROFILE_VEX_ANNOTATION)) {
        THROW("C23Lexer rejected unsupported lexical profile %u", (unsigned) profile);
        return false;
    }
    (*self).profile = profile;
    return true;
}

u32 C23Lexer_getDocumentId(const C23Lexer *self) {
    return self == nullptr ? 0u : (*self).documentId;
}
const char *C23Lexer_getSource(const C23Lexer *self) {
    return self == nullptr ? nullptr : (*self).source;
}
bytes C23Lexer_getLength(const C23Lexer *self) {
    return self == nullptr ? 0u : (*self).length;
}
bytes C23Lexer_getOffset(const C23Lexer *self) {
    return self == nullptr ? 0u : (*self).offset;
}
C23LexerProfile C23Lexer_getProfile(const C23Lexer *self) {
    return self == nullptr ? C23_LEXER_PROFILE_STRICT_C23 : (*self).profile;
}
bool C23Lexer_isDone(const C23Lexer *self) {
    return self == nullptr || (*self).stopped
        || ((*self).mode == C23_LEXER_MODE_NORMAL
            && skipWhitespace(self, (*self).offset) >= (*self).length);
}

C23LexStep C23Lexer_next(C23Lexer *self) {
    if (self == nullptr || (*self).stopped)
        return C23LexStep_0();
    if ((*self).mode != C23_LEXER_MODE_NORMAL && (*self).offset >= (*self).length) {
        (*self).mode = C23_LEXER_MODE_NORMAL;
        return C23LexStep_0();
    }
    bytes start = (*self).mode == C23_LEXER_MODE_NORMAL
        ? skipWhitespace(self, (*self).offset) : (*self).offset;
    if (start >= (*self).length) {
        (*self).offset = (*self).length;
        return C23LexStep_0();
    }
    const char *source = (*self).source;
    bytes length = (*self).length;
    if ((*self).mode == C23_LEXER_MODE_LINE_COMMENT
        || (*self).mode == C23_LEXER_MODE_BLOCK_COMMENT)
        return scanComment(self, start);
    if ((*self).mode == C23_LEXER_MODE_STRING || (*self).mode == C23_LEXER_MODE_CHARACTER)
        return scanQuotedPart(self, start, false);
    bytes end = start;
    SyntaxRole role = SYNTAX_ROLE_PUNCTUATION;
    const char *styleKey = nullptr;
    unsigned char byte = (unsigned char) source[start];

    if (byte == 0u) {
        end = start + 1u;
        role = SYNTAX_ROLE_INVALID;
    } else if (byte >= 0x80u) {
        Utf8ScalarResult scalar = validUtf8Scalar((const unsigned char *) source,
                                                   length, start);
        end = start + scalar.length;
        role = SYNTAX_ROLE_INVALID;
    } else if (startsWith(source, length, start, "//") || startsWith(source, length, start, "/*")) {
        bool lineComment = startsWith(source, length, start, "//");
        (*self).mode = lineComment ? C23_LEXER_MODE_LINE_COMMENT : C23_LEXER_MODE_BLOCK_COMMENT;
        (*self).docComment = lineComment
            ? startsWith(source, length, start, "///") || startsWith(source, length, start, "//!")
            : startsWith(source, length, start, "/**") || startsWith(source, length, start, "/*!");
        (*self).offset = start + 2u;
        return scanComment(self, start);
    } else {
        QuotePrefixResult prefix = quotePrefix(source, length, start);
        if (prefix.matched) {
            return scanQuotedPart(self, start, true);
        } else if (identifierStart(byte)) {
            end = start + 1u;
            while (end < length && identifierContinue((unsigned char) source[end]))
                ++end;
            role = keywordRole(source + start, end - start);
        } else if ((byte >= '0' && byte <= '9')
                   || (byte == '.' && start + 1u < length
                       && source[start + 1u] >= '0' && source[start + 1u] <= '9')) {
            end = numberEnd(source, length, start);
            role = SYNTAX_ROLE_NUMBER;
        } else if ((*self).profile == C23_LEXER_PROFILE_VEX_ANNOTATION
                   && startsWith(source, length, start, ";;")
                   && start + 2u < length
                   && identifierStart((unsigned char) source[start + 2u])) {
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
        (*self).stopped = true;
        return C23LexStep_2(true, makeSpan(self, start, start + 1u, SYNTAX_ROLE_INVALID, nullptr));
    }
    (*self).offset = end;
    return C23LexStep_2(true, makeSpan(self, start, end, role, styleKey));
}

BoundedText C23Lexer_toString(const C23Lexer *self) {
    if (self == nullptr) return BoundedText_1("nullptr");
    return BoundedText_format("C23Lexer(document=%u, length=%zu, offset=%zu)",
                              (*self).documentId, (*self).length, (*self).offset);
}

BoundedText C23Lexer_toStringStruct(const C23Lexer *self) {
    if (self == nullptr) return BoundedText_1("nullptr");
    return BoundedText_format("C23Lexer { documentId=%u, source=%p, length=%zu, offset=%zu, profile=%u, stopped=%s, mode=%u, quote=%u, docComment=%s }",
                              (*self).documentId, (void*) (*self).source, (*self).length,
                              (*self).offset, (unsigned) (*self).profile,
                              (*self).stopped ? "true" : "false", (unsigned) (*self).mode,
                              (unsigned char) (*self).quote,
                              (*self).docComment ? "true" : "false");
}
