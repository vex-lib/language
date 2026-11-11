#include "adapters/c/c23_highlights.h"
#include "adapters/c/c23_lexer.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/**
 * C23Highlights is the cold presentation classifier behind the editor's C23
 * colors. It walks one borrowed source snapshot through the strict lexer,
 * preserving structural roles (keywords, literals, comments, brackets) and
 * refining identifier roles with deliberately local evidence: spelling and the
 * immediately neighboring significant token. The pass is useful precisely
 * because it needs no parser, includes, macro expansion or symbol table; it is
 * also honest about that limit, so a capitalized variable can read as a type and
 * an all-caps local can read as a macro. Preprocessor directives at the start of
 * a line (`#include`, `#define`, `#pragma`, ...) keep their marker and name in
 * the macro role, matching the way macros are painted. After `#include`, a
 * well-formed angle-bracket header name (`<locale.h>`) is one string-role span
 * so the consumer paints the path with its string pen; a quoted header already
 * arrives as a string token. The path is deliberately recognized only there, not
 * every `<`/`>` comparison in the file. It never rewrites bytes, allocates on
 * queries, or decides diagnostics. This is presentation classification, not
 * semantic resolution, and the consumer owns colors.
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: C23Highlights (adapters/c/c23_highlights.c)
 * STRUCT FIELDS (Mirroring adapters/c/c23_highlights.h)
 *   u32 documentId          // identity shared by every produced span
 *   bytes count                 // number of ordered highlight spans
 *   bytes capacity              // allocated entry slots
 *   C23HighlightEntry *entries   // owned start/end/role records
 *   const char *source           // borrowed immutable source snapshot
 *   bytes sourceLength          // byte length of that snapshot
 *
 * PRIVATE HELPERS
 *   C23HighlightEntry { bytes start; bytes end; SyntaxRole role; }
 *     // one ordered, non-overlapping presentation span
 *   C23Token { bytes start; bytes end; SyntaxRole role; }
 *     // one lexer token retained for neighbor-aware classification
 *
 * FUNCTION REGISTRY
 *   constructors: C23Highlights_0, C23Highlights_3, C23Highlights(...),
 *                 C23Highlights_zero
 *   lifecycle:    C23Highlights_free
 *   getters:      C23Highlights_getCount, C23Highlights_getDocumentId,
 *                 C23Highlights_getStart, C23Highlights_getEnd,
 *                 C23Highlights_getRole, C23Highlights_roleAt
 *   projections:  C23Highlights_toString, C23Highlights_toStringStruct
 *   private:      tokenize, appendEntry, previousSignificant,
 *                 nextSignificant, tokenIs, tokenStartsLine,
 *                 previousIsDirectiveHash, containsNewline, includeHeaderEnd,
 *                 identifierAllCaps,
 *                 identifierEndsTypeSuffix, identifierStartsUpper,
 *                 identifierHasLower, classifyIdentifier
 *
 * CONTRACT
 *   Input is validated before lexing; failure returns nullptr and reports one
 *   cold diagnostic. Entries are owned here; source is borrowed and must remain
 *   immutable/alive until free. Spans are sorted by start byte and non-
 *   overlapping. roleAt is read-only and allocation-free. Construction is cold
 *   and may allocate. No parser, preprocessor, symbol resolution or diagnostics
 *   are claimed; comments and doc comments keep their distinct lexer roles.
 *   A line-start preprocessor marker and its directive name are painted as
 *   macros; a `#` or `##` inside code stays punctuation. A well-formed
 *   `<...>` header name after `#include` is one string span, while a quoted
 *   header keeps the lexer's string role.
 * ============================================================================
 */

typedef struct C23HighlightEntry {
    bytes start;
    bytes end;
    SyntaxRole role;
} C23HighlightEntry;

typedef struct C23Token {
    bytes start;
    bytes end;
    SyntaxRole role;
} C23Token;

struct C23Highlights {
    u32 documentId;
    bytes count;
    bytes capacity;
    C23HighlightEntry *entries;
    const char *source;
    bytes sourceLength;
};

static bool tokenIs(const C23Token *tokens, bytes count, bytes index, const char *text,
                    const char *source) {
    if (index >= count)
        return false;
    bytes length = strlen(text);
    C23Token token = tokens[index];
    return token.end - token.start == length
        && memcmp(source + token.start, text, length) == 0;
}

static bool tokenStartsLine(const C23Token *tokens, bytes count, bytes index,
                            const char *source) {
    if (!tokenIs(tokens, count, index, "#", source))
        return false;
    bytes at = tokens[index].start;
    while (at > 0u) {
        unsigned char byte = (unsigned char) source[at - 1u];
        if (byte == '\n' || byte == '\r')
            break;
        if (byte != ' ' && byte != '\t' && byte != '\f' && byte != '\v')
            return false;
        --at;
    }
    return true;
}

static bool appendEntry(C23Highlights *self, C23HighlightEntry entry) {
    if ((*self).count == (*self).capacity) {
        bytes next = (*self).capacity == 0u ? 64u : (*self).capacity * 2u;
        if (next < (*self).capacity || next > SIZE_MAX / sizeof *(*self).entries)
            return false;
        C23HighlightEntry *grown = realloc((*self).entries,
                                           next * sizeof *(*self).entries);
        if (grown == nullptr)
            return false;
        (*self).entries = grown;
        (*self).capacity = next;
    }
    (*self).entries[(*self).count] = entry;
    ++(*self).count;
    return true;
}

static bool appendToken(C23Token **tokens, bytes *count, bytes *capacity,
                        C23Token token) {
    if (*count == *capacity) {
        bytes next = *capacity == 0u ? 64u : *capacity * 2u;
        if (next < *capacity || next > SIZE_MAX / sizeof **tokens)
            return false;
        C23Token *grown = realloc(*tokens, next * sizeof **tokens);
        if (grown == nullptr)
            return false;
        *tokens = grown;
        *capacity = next;
    }
    (*tokens)[(*count)++] = token;
    return true;
}

static bool tokenizeC23(u32 documentId, const char *source, bytes length,
                        C23Token **tokens, bytes *count) {
    C23Lexer *lexer = C23Lexer(documentId, source, length);
    if (lexer == nullptr)
        return false;
    bytes capacity = 0u;
    C23LexStep step = C23Lexer_next(lexer);
    while (step.hasSpan) {
        SyntaxSpan span = step.span;
        SourceRange range = span.range;
        C23Token token = {
            .start = (bytes) range.startByte,
            .end = (bytes) range.endByte,
            .role = span.role
        };
        if (!appendToken(tokens, count, &capacity, token)) {
            C23Lexer_free(lexer);
            return false;
        }
        step = C23Lexer_next(lexer);
    }
    C23Lexer_free(lexer);
    return true;
}

static bytes previousSignificant(const C23Token *tokens, bytes index) {
    while (index > 0u) {
        --index;
        if (tokens[index].role != SYNTAX_ROLE_COMMENT
            && tokens[index].role != SYNTAX_ROLE_DOC_COMMENT)
            return index;
    }
    return SIZE_MAX;
}

static bool previousIsDirectiveHash(const C23Token *tokens, bytes count, bytes index,
                                    const char *source) {
    bytes previous = previousSignificant(tokens, index);
    return previous != SIZE_MAX
        && tokenStartsLine(tokens, count, previous, source);
}

static bytes nextSignificant(const C23Token *tokens, bytes count, bytes index) {
    bytes next = index + 1u;
    while (next < count) {
        if (tokens[next].role != SYNTAX_ROLE_COMMENT
            && tokens[next].role != SYNTAX_ROLE_DOC_COMMENT)
            return next;
        ++next;
    }
    return SIZE_MAX;
}

static bool containsNewline(const char *source, bytes start, bytes end) {
    for (bytes at = start; at < end; ++at) {
        if (source[at] == '\n' || source[at] == '\r')
            return true;
    }
    return false;
}

// A header-name after #include is one lexical unit even though the token stream
// splits <locale.h> into punctuation and identifiers. Return the closing '>'
// index and the opening '<' index only when the whole name stays on one line.
static bytes includeHeaderEnd(const C23Token *tokens, bytes count, bytes includeIndex,
                               const char *source, bytes *openIndex) {
    bytes open = includeIndex + 1u;
    if (open >= count || !tokenIs(tokens, count, open, "<", source))
        return SIZE_MAX;
    if (containsNewline(source, tokens[includeIndex].end, tokens[open].start))
        return SIZE_MAX;
    bytes previousEnd = tokens[open].end;
    for (bytes index = open + 1u; index < count; ++index) {
        if (containsNewline(source, previousEnd, tokens[index].start))
            return SIZE_MAX;
        if (tokenIs(tokens, count, index, ">", source)) {
            *openIndex = open;
            return index;
        }
        previousEnd = tokens[index].end;
    }
    return SIZE_MAX;
}

static bool identifierAllCaps(const char *source, C23Token token) {
    bool letter = false;
    for (bytes at = token.start; at < token.end; ++at) {
        unsigned char byte = (unsigned char) source[at];
        if (byte >= 'A' && byte <= 'Z')
            letter = true;
        else if ((byte < '0' || byte > '9') && byte != '_')
            return false;
    }
    return letter;
}

static bool identifierEndsTypeSuffix(const char *source, C23Token token) {
    bytes length = token.end - token.start;
    return length >= 2u && source[token.end - 2u] == '_' && source[token.end - 1u] == 't';
}

static bool identifierStartsUpper(const char *source, C23Token token) {
    unsigned char first = (unsigned char) source[token.start];
    return first >= 'A' && first <= 'Z';
}

static bool identifierHasLower(const char *source, C23Token token) {
    for (bytes at = token.start; at < token.end; ++at) {
        unsigned char byte = (unsigned char) source[at];
        if (byte >= 'a' && byte <= 'z')
            return true;
    }
    return false;
}

static bool previousIsTagWord(const C23Token *tokens, bytes tokenCount, bytes index,
                              const char *source) {
    bytes previous = previousSignificant(tokens, index);
    return previous != SIZE_MAX
        && (tokenIs(tokens, tokenCount, previous, "struct", source)
            || tokenIs(tokens, tokenCount, previous, "union", source)
            || tokenIs(tokens, tokenCount, previous, "enum", source));
}

static SyntaxRole classifyIdentifier(const C23Token *tokens, bytes count, bytes index,
                                     const char *source) {
    C23Token token = tokens[index];
    if (identifierAllCaps(source, token))
        return SYNTAX_ROLE_MACRO;
    if (previousIsDirectiveHash(tokens, count, index, source))
        return SYNTAX_ROLE_MACRO;
    bytes previous = previousSignificant(tokens, index);
    bool member = previous != SIZE_MAX
        && (tokenIs(tokens, count, previous, ".", source)
            || tokenIs(tokens, count, previous, "->", source));
    if (member)
        return SYNTAX_ROLE_MEMBER;
    if (previousIsTagWord(tokens, count, index, source))
        return SYNTAX_ROLE_TYPE;
    if (identifierEndsTypeSuffix(source, token))
        return SYNTAX_ROLE_TYPE;
    bytes next = nextSignificant(tokens, count, index);
    if (next != SIZE_MAX && tokenIs(tokens, count, next, "(", source))
        return SYNTAX_ROLE_FUNCTION;
    if (identifierStartsUpper(source, token) && identifierHasLower(source, token))
        return SYNTAX_ROLE_TYPE;
    return SYNTAX_ROLE_VARIABLE;
}

C23Highlights *C23Highlights_0(void) {
    C23Highlights *self = calloc(1u, sizeof *self);
    if (self == nullptr)
        THROW("C23Highlights allocation refused: request %zu bytes", sizeof *self);
    return self;
}

C23Highlights *C23Highlights_3(u32 documentId, const char *source, bytes length) {
    if (documentId == 0u || (source == nullptr && length != 0u)) {
        THROW("C23Highlights rejected source admission for %zu bytes", length);
        return nullptr;
    }
    C23Highlights *self = C23Highlights_0();
    if (self == nullptr)
        return nullptr;
    (*self).documentId = documentId;
    (*self).source = source == nullptr ? "" : source;
    (*self).sourceLength = length;
    C23Token *tokens = nullptr;
    bytes tokenCount = 0u;
    bool success = tokenizeC23(documentId, (*self).source, length, &tokens, &tokenCount);
    for (bytes index = 0u; success && index < tokenCount; ++index) {
        C23Token token = tokens[index];
        SyntaxRole role = token.role == SYNTAX_ROLE_IDENTIFIER
            ? classifyIdentifier(tokens, tokenCount, index, (*self).source)
            : token.role;
        if (tokenStartsLine(tokens, tokenCount, index, (*self).source))
            role = SYNTAX_ROLE_MACRO;
        C23HighlightEntry entry = {
            .start = token.start, .end = token.end, .role = role
        };
        if (!appendEntry(self, entry)) {
            success = false;
            break;
        }
        if (token.role == SYNTAX_ROLE_IDENTIFIER
            && tokenIs(tokens, tokenCount, index, "include", (*self).source)
            && previousIsDirectiveHash(tokens, tokenCount, index, (*self).source)) {
            bytes open = SIZE_MAX;
            bytes close = includeHeaderEnd(tokens, tokenCount, index, (*self).source, &open);
            if (close != SIZE_MAX) {
                C23HighlightEntry header = {
                    .start = tokens[open].start,
                    .end = tokens[close].end,
                    .role = SYNTAX_ROLE_STRING
                };
                if (!appendEntry(self, header)) {
                    success = false;
                    break;
                }
                index = close;
            }
        }
    }
    free(tokens);
    if (!success) {
        THROW("C23Highlights analysis allocation failed for %zu source bytes", length);
        free((*self).entries);
        free(self);
        return nullptr;
    }
    return self;
}

C23Highlights *C23Highlights_zero(void) { return C23Highlights_0(); }

void C23Highlights_free(C23Highlights *self) {
    if (self == nullptr)
        return;
    free((*self).entries);
    free(self);
}

bytes C23Highlights_getCount(const C23Highlights *self) {
    return self == nullptr ? 0u : (*self).count;
}

u32 C23Highlights_getDocumentId(const C23Highlights *self) {
    return self == nullptr ? 0u : (*self).documentId;
}

bytes C23Highlights_getStart(const C23Highlights *self, bytes index) {
    if (self == nullptr || index >= (*self).count)
        return 0u;
    return (*self).entries[index].start;
}

bytes C23Highlights_getEnd(const C23Highlights *self, bytes index) {
    if (self == nullptr || index >= (*self).count)
        return 0u;
    return (*self).entries[index].end;
}

SyntaxRole C23Highlights_getRole(const C23Highlights *self, bytes index) {
    if (self == nullptr || index >= (*self).count)
        return SYNTAX_ROLE_NONE;
    return (*self).entries[index].role;
}

SyntaxRole C23Highlights_roleAt(const C23Highlights *self, bytes byte) {
    if (self == nullptr)
        return SYNTAX_ROLE_NONE;
    bytes low = 0u;
    bytes high = (*self).count;
    while (low < high) {
        bytes middle = low + (high - low) / 2u;
        C23HighlightEntry entry = (*self).entries[middle];
        if (byte < entry.start)
            high = middle;
        else if (byte >= entry.end)
            low = middle + 1u;
        else
            return entry.role;
    }
    return SYNTAX_ROLE_NONE;
}

BoundedText C23Highlights_toString(const C23Highlights *self) {
    if (self == nullptr)
        return BoundedText_1("nullptr");
    return BoundedText_format("C23Highlights(document=%u, count=%zu)",
                              (*self).documentId, (*self).count);
}

BoundedText C23Highlights_toStringStruct(const C23Highlights *self) {
    if (self == nullptr)
        return BoundedText_1("nullptr");
    return BoundedText_format("C23Highlights { documentId=%u, count=%zu, capacity=%zu, entries=%p, source=%p, sourceLength=%zu }",
        (*self).documentId, (*self).count, (*self).capacity, (void*) (*self).entries,
        (const void*) (*self).source, (*self).sourceLength);
}
