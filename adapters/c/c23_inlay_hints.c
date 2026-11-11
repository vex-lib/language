#include "adapters/c/c23_inlay_hints.h"
#include "adapters/c/c23_lexer.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/**
 * C23InlayHints performs a deliberately conservative, cold lexical pass over a
 * borrowed C23 source snapshot. It collects simple function prototypes and
 * definitions, then recognizes matching direct calls and places each declared
 * parameter name immediately before its argument. It does not infer types,
 * resolve external declarations, evaluate macros, or claim complete C grammar
 * support. Every hint is a virtual byte anchor; source and syntax spans remain
 * untouched. The caller retains the immutable source through owner destruction.
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: C23InlayHints (adapters/c/c23_inlay_hints.c)
 * STRUCT FIELDS (Mirroring adapters/c/c23_inlay_hints.h)
 *   u32 documentId   // identity shared by all produced hint anchors
 *   bytes count          // number of ordered virtual labels
 *   C23HintEntry *entries // owned offset/label records
 *   char *labelBytes      // owned NUL-terminated labels
 *   bytes labelByteCount // used bytes in the owned label storage
 *   const char *source    // borrowed immutable source snapshot
 *   bytes sourceLength   // byte length of that snapshot
 *
 * PRIVATE HELPERS
 *   C23HintEntry { bytes offset; bytes labelOffset; bytes labelLength; }
 *     // one call-argument byte anchor and its slice in labelBytes
 *   C23Token { bytes start; bytes end; SyntaxRole role; }
 *     // one lexer token retained during cold matching
 *   C23Parameter { bytes start; bytes end; }
 *     // borrowed byte range naming one declared parameter
 *   C23Signature { bytes nameStart; bytes nameEnd; bytes parameterStart;
 *                  bytes parameterCount; bytes declarationToken; }
 *     // token-indexed direct-call signature with a flat parameter range
 *
 * FUNCTION REGISTRY
 *   constructors: C23InlayHints_0, C23InlayHints_3, C23InlayHints(...),
 *                 C23InlayHints_zero
 *   lifecycle:    C23InlayHints_free
 *   getters:      C23InlayHints_getCount, C23InlayHints_getDocumentId,
 *                 C23InlayHints_getOffset, C23InlayHints_getLabel,
 *                 C23InlayHints_getLabelLength
 *   projections:  C23InlayHints_toString, C23InlayHints_toStringStruct
 *   private:      tokenize, collectSignatures, matchCalls, appendHint
 *
 * CONTRACT
 *   Input is validated before scanning; failure returns nullptr and reports a
 *   cold diagnostic. Entries and labels are owned by this object; source is
 *   borrowed and must remain immutable/alive until free. Only simple directly
 *   declared function names and ordinary parameter names are recognized.
 *   Function pointers, macros, variadics, complex declarators, cross-file
 *   declarations and ambiguous overload-like names are omitted. No source or
 *   span is rewritten. Construction is cold and may allocate; accessors do not.
 * ============================================================================
 */

typedef struct C23HintEntry {
    bytes offset;
    bytes labelOffset;
    bytes labelLength;
} C23HintEntry;

typedef struct C23Token {
    bytes start;
    bytes end;
    SyntaxRole role;
} C23Token;

typedef struct C23Parameter {
    bytes start;
    bytes end;
} C23Parameter;

typedef struct C23Signature {
    bytes nameStart;
    bytes nameEnd;
    bytes parameterStart;
    bytes parameterCount;
    bytes declarationToken;
} C23Signature;

struct C23InlayHints {
    u32 documentId;
    bytes count;
    C23HintEntry *entries;
    char *labelBytes;
    bytes labelByteCount;
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

static bool tokenize(u32 documentId, const char *source, bytes length,
                     C23Token **tokens, bytes *count) {
    C23Lexer *lexer = C23Lexer(documentId, source, length);
    if (lexer == nullptr)
        return false;
    bytes capacity = 0u;
    C23LexStep step = C23Lexer_next(lexer);
    while (step.hasSpan) {
        SyntaxSpan span = step.span;
        if (span.role != SYNTAX_ROLE_COMMENT && span.role != SYNTAX_ROLE_DOC_COMMENT) {
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
        }
        step = C23Lexer_next(lexer);
    }
    C23Lexer_free(lexer);
    return true;
}

static bytes matchingClose(const C23Token *tokens, bytes count, bytes open,
                            const char *source) {
    bytes depth = 0u;
    for (bytes index = open; index < count; ++index) {
        if (tokenIs(tokens, count, index, "(", source))
            ++depth;
        else if (tokenIs(tokens, count, index, ")", source) && --depth == 0u)
            return index;
    }
    return count;
}

static bool isIdentifier(C23Token token) {
    return token.role == SYNTAX_ROLE_IDENTIFIER;
}

static bool isTypePrefix(C23Token token, const char *source) {
    if (isIdentifier(token))
        return true;
    static const char *const typeWords[] = {
        "auto", "bool", "char", "char8_t", "d64", "f32", "int", "long",
        "short", "signed", "unsigned", "void", "wchar_t", "_Bool", "_BitInt",
        "_Complex", "_Decimal32", "_Decimal64", "_Decimal128", "struct", "union",
        "enum"
    };
    for (bytes index = 0u; index < sizeof typeWords / sizeof typeWords[0]; ++index)
        if (token.end - token.start == strlen(typeWords[index])
            && memcmp(source + token.start, typeWords[index], token.end - token.start) == 0)
            return true;
    return false;
}

static bool appendParameter(C23Parameter **parameters, bytes *count,
                            bytes *capacity, C23Parameter parameter) {
    if (*count == *capacity) {
        bytes next = *capacity == 0u ? 16u : *capacity * 2u;
        if (next < *capacity || next > SIZE_MAX / sizeof **parameters)
            return false;
        C23Parameter *grown = realloc(*parameters, next * sizeof **parameters);
        if (grown == nullptr)
            return false;
        *parameters = grown;
        *capacity = next;
    }
    (*parameters)[(*count)++] = parameter;
    return true;
}

static bool appendSignature(C23Signature **signatures, bytes *count,
                            bytes *capacity, C23Signature signature) {
    if (*count == *capacity) {
        bytes next = *capacity == 0u ? 16u : *capacity * 2u;
        if (next < *capacity || next > SIZE_MAX / sizeof **signatures)
            return false;
        C23Signature *grown = realloc(*signatures, next * sizeof **signatures);
        if (grown == nullptr)
            return false;
        *signatures = grown;
        *capacity = next;
    }
    (*signatures)[(*count)++] = signature;
    return true;
}

static bool appendHint(C23InlayHints *self, bytes offset, bytes nameStart,
                       bytes nameEnd) {
    if (nameEnd < nameStart || nameEnd - nameStart > SIZE_MAX - 2u
        || (*self).count == SIZE_MAX / sizeof *(*self).entries)
        return false;
    bytes labelLength = nameEnd - nameStart + 1u;
    if ((*self).count + 1u > SIZE_MAX / sizeof *(*self).entries)
        return false;
    C23HintEntry *entries = realloc((*self).entries,
                                    ((*self).count + 1u) * sizeof *(*self).entries);
    if (entries == nullptr)
        return false;
    (*self).entries = entries;
    bytes used = (*self).labelByteCount;
    if (used > SIZE_MAX - labelLength - 1u)
        return false;
    char *labels = realloc((*self).labelBytes, used + labelLength + 1u);
    if (labels == nullptr)
        return false;
    (*self).labelBytes = labels;
    memcpy(labels + used, (*self).source + nameStart, nameEnd - nameStart);
    labels[used + labelLength - 1u] = ':';
    labels[used + labelLength] = '\0';
    (*self).entries[(*self).count] = (C23HintEntry) {
        .offset = offset, .labelOffset = used, .labelLength = labelLength
    };
    (*self).labelByteCount = used + labelLength + 1u;
    ++(*self).count;
    return true;
}

static bool collectSignatures(C23InlayHints *self, const C23Token *tokens,
                              bytes tokenCount, C23Parameter **parameters,
                              bytes *parameterCount, C23Signature **signatures,
                              bytes *signatureCount) {
    bytes parameterCapacity = 0u;
    bytes signatureCapacity = 0u;
    const char *source = (*self).source;
    for (bytes open = 1u; open + 1u < tokenCount; ++open) {
        if (!tokenIs(tokens, tokenCount, open, "(", source)
            || !isIdentifier(tokens[open - 1u]) || open < 2u
            || !isTypePrefix(tokens[open - 2u], source))
            continue;
        bytes close = matchingClose(tokens, tokenCount, open, source);
        if (close >= tokenCount || close + 1u >= tokenCount
            || (!tokenIs(tokens, tokenCount, close + 1u, ";", source)
                && !tokenIs(tokens, tokenCount, close + 1u, "{", source)))
            continue;
        bytes paramStart = *parameterCount;
        bytes segment = open + 1u;
        unsigned parenDepth = 0u;
        bool valid = true;
        for (bytes index = segment; index <= close; ++index) {
            bool boundary = index == close
                || (parenDepth == 0u && tokenIs(tokens, tokenCount, index, ",", source));
            if (!boundary) {
                if (tokenIs(tokens, tokenCount, index, "(", source))
                    ++parenDepth;
                else if (tokenIs(tokens, tokenCount, index, ")", source) && parenDepth != 0u)
                    --parenDepth;
                continue;
            }
            if (segment == index) {
                if (segment != close)
                    valid = false;
                segment = index + 1u;
                continue;
            }
            if (index == open + 2u && tokenIs(tokens, tokenCount, open + 1u, "void", source)) {
                segment = index + 1u;
                continue;
            }
            bool simpleDeclarator = true;
            for (bytes part = segment; part < index; ++part)
                if (tokens[part].role != SYNTAX_ROLE_KEYWORD
                    && tokens[part].role != SYNTAX_ROLE_IDENTIFIER)
                    simpleDeclarator = false;
            bool unnamedTagType = index == segment + 2u
                && (tokenIs(tokens, tokenCount, segment, "struct", source)
                    || tokenIs(tokens, tokenCount, segment, "union", source)
                    || tokenIs(tokens, tokenCount, segment, "enum", source));
            if (!simpleDeclarator || unnamedTagType) {
                valid = false;
                break;
            }
            bytes name = index;
            while (name > segment && !isIdentifier(tokens[name - 1u]))
                --name;
            if (name == segment || name <= segment + 1u) {
                valid = false;
                break;
            }
            --name;
            if (!appendParameter(parameters, parameterCount, &parameterCapacity,
                                 (C23Parameter) {tokens[name].start, tokens[name].end}))
                return false;
            segment = index + 1u;
        }
        bytes count = *parameterCount - paramStart;
        if (!valid) {
            *parameterCount = paramStart;
            continue;
        }
        C23Signature signature = {
            .nameStart = tokens[open - 1u].start,
            .nameEnd = tokens[open - 1u].end,
            .parameterStart = paramStart,
            .parameterCount = count,
            .declarationToken = open - 1u
        };
        if (!appendSignature(signatures, signatureCount, &signatureCapacity, signature))
            return false;
    }
    return true;
}

static bool matchCalls(C23InlayHints *self, const C23Token *tokens,
                       bytes tokenCount, const C23Parameter *parameters,
                       const C23Signature *signatures, bytes signatureCount) {
    const char *source = (*self).source;
    for (bytes nameIndex = 0u; nameIndex + 1u < tokenCount; ++nameIndex) {
        if (!isIdentifier(tokens[nameIndex])
            || !tokenIs(tokens, tokenCount, nameIndex + 1u, "(", source))
            continue;
        if (nameIndex != 0u
            && (tokenIs(tokens, tokenCount, nameIndex - 1u, ".", source)
                || tokenIs(tokens, tokenCount, nameIndex - 1u, "->", source)))
            continue;
        if (nameIndex != 0u && isTypePrefix(tokens[nameIndex - 1u], source))
            continue;
        bool isDeclaration = false;
        for (bytes index = 0u; index < signatureCount; ++index) {
            C23Signature candidate = signatures[index];
            bytes candidateLength = candidate.nameEnd - candidate.nameStart;
            bytes nameLength = tokens[nameIndex].end - tokens[nameIndex].start;
            if (candidate.declarationToken == nameIndex && candidateLength == nameLength
                && memcmp(source + candidate.nameStart, source + tokens[nameIndex].start,
                          nameLength) == 0) {
                isDeclaration = true;
                break;
            }
        }
        if (isDeclaration)
            continue;
        const C23Signature *signature = nullptr;
        bool ambiguous = false;
        for (bytes index = 0u; index < signatureCount; ++index) {
            C23Signature candidate = signatures[index];
            if (candidate.nameEnd - candidate.nameStart != tokens[nameIndex].end - tokens[nameIndex].start
                || memcmp(source + candidate.nameStart, source + tokens[nameIndex].start,
                          tokens[nameIndex].end - tokens[nameIndex].start) != 0)
                continue;
            if (signature != nullptr) {
                bool sameParameters = (*signature).parameterCount == candidate.parameterCount;
                for (bytes parameter = 0u; sameParameters
                     && parameter < candidate.parameterCount; ++parameter) {
                    C23Parameter first = parameters[(*signature).parameterStart + parameter];
                    C23Parameter next = parameters[candidate.parameterStart + parameter];
                    bytes firstLength = first.end - first.start;
                    bytes nextLength = next.end - next.start;
                    sameParameters = firstLength == nextLength
                        && memcmp(source + first.start, source + next.start, firstLength) == 0;
                }
                if (!sameParameters) {
                    ambiguous = true;
                    break;
                }
            }
            signature = &signatures[index];
        }
        if (signature == nullptr || ambiguous)
            continue;
        bytes close = matchingClose(tokens, tokenCount, nameIndex + 1u, source);
        if (close >= tokenCount)
            continue;
        bytes actualArgumentCount = 0u;
        if (nameIndex + 2u < close) {
            actualArgumentCount = 1u;
            bytes nestedDepth = 0u;
            for (bytes index = nameIndex + 2u; index < close; ++index) {
                if (tokenIs(tokens, tokenCount, index, "(", source)
                    || tokenIs(tokens, tokenCount, index, "[", source)
                    || tokenIs(tokens, tokenCount, index, "{", source))
                    ++nestedDepth;
                else if ((tokenIs(tokens, tokenCount, index, ")", source)
                          || tokenIs(tokens, tokenCount, index, "]", source)
                          || tokenIs(tokens, tokenCount, index, "}", source))
                    && nestedDepth != 0u)
                    --nestedDepth;
                else if (nestedDepth == 0u
                         && tokenIs(tokens, tokenCount, index, ",", source))
                    ++actualArgumentCount;
            }
        }
        if (actualArgumentCount != (*signature).parameterCount)
            continue;
        bytes argStart = nameIndex + 2u;
        bytes argument = 0u;
        bytes depth = 0u;
        for (bytes index = argStart; index <= close; ++index) {
            bool boundary = index == close
                || (depth == 0u && tokenIs(tokens, tokenCount, index, ",", source));
            if (!boundary) {
                if (tokenIs(tokens, tokenCount, index, "(", source)
                    || tokenIs(tokens, tokenCount, index, "[", source)
                    || tokenIs(tokens, tokenCount, index, "{", source))
                    ++depth;
                else if ((tokenIs(tokens, tokenCount, index, ")", source)
                          || tokenIs(tokens, tokenCount, index, "]", source)
                          || tokenIs(tokens, tokenCount, index, "}", source)) && depth != 0u)
                    --depth;
                continue;
            }
            if (argStart < index && argument < (*signature).parameterCount) {
                C23Parameter parameter = parameters[(*signature).parameterStart + argument];
                bytes argOffset = tokens[argStart].start;
                if (!appendHint(self, argOffset, parameter.start, parameter.end))
                    return false;
            }
            ++argument;
            argStart = index + 1u;
        }
    }
    return true;
}

C23InlayHints *C23InlayHints_0(void) {
    C23InlayHints *self = calloc(1u, sizeof *self);
    if (self == nullptr)
        THROW("C23InlayHints allocation refused: request %zu bytes", sizeof *self);
    return self;
}

C23InlayHints *C23InlayHints_3(u32 documentId, const char *source, bytes length) {
    if (documentId == 0u || (source == nullptr && length != 0u)) {
        THROW("C23InlayHints rejected source admission for %zu bytes", length);
        return nullptr;
    }
    C23InlayHints *self = C23InlayHints_0();
    if (self == nullptr)
        return nullptr;
    (*self).documentId = documentId;
    (*self).source = source == nullptr ? "" : source;
    (*self).sourceLength = length;
    C23Token *tokens = nullptr;
    bytes tokenCount = 0u;
    C23Parameter *parameters = nullptr;
    bytes parameterCount = 0u;
    C23Signature *signatures = nullptr;
    bytes signatureCount = 0u;
    bool success = tokenize(documentId, (*self).source, length, &tokens, &tokenCount)
        && collectSignatures(self, tokens, tokenCount, &parameters, &parameterCount,
                             &signatures, &signatureCount)
        && matchCalls(self, tokens, tokenCount, parameters, signatures, signatureCount);
    free(tokens);
    free(parameters);
    free(signatures);
    if (!success) {
        THROW("C23InlayHints analysis allocation failed for %zu source bytes", length);
        C23InlayHints_free(self);
        return nullptr;
    }
    return self;
}

C23InlayHints *C23InlayHints_zero(void) { return C23InlayHints_0(); }

void C23InlayHints_free(C23InlayHints *self) {
    if (self == nullptr)
        return;
    free((*self).entries);
    free((*self).labelBytes);
    free(self);
}

bytes C23InlayHints_getCount(const C23InlayHints *self) {
    return self == nullptr ? 0u : (*self).count;
}
u32 C23InlayHints_getDocumentId(const C23InlayHints *self) {
    return self == nullptr ? 0u : (*self).documentId;
}
bytes C23InlayHints_getOffset(const C23InlayHints *self, bytes index) {
    if (self == nullptr || index >= (*self).count)
        return 0u;
    C23HintEntry entry = (*self).entries[index];
    return entry.offset;
}
const char *C23InlayHints_getLabel(const C23InlayHints *self, bytes index) {
    if (self == nullptr || index >= (*self).count)
        return nullptr;
    C23HintEntry entry = (*self).entries[index];
    return (*self).labelBytes + entry.labelOffset;
}
bytes C23InlayHints_getLabelLength(const C23InlayHints *self, bytes index) {
    if (self == nullptr || index >= (*self).count)
        return 0u;
    C23HintEntry entry = (*self).entries[index];
    return entry.labelLength;
}
BoundedText C23InlayHints_toString(const C23InlayHints *self) {
    if (self == nullptr)
        return BoundedText_1("nullptr");
    return BoundedText_format("C23InlayHints(document=%u, count=%zu)",
                              (*self).documentId, (*self).count);
}
BoundedText C23InlayHints_toStringStruct(const C23InlayHints *self) {
    if (self == nullptr)
        return BoundedText_1("nullptr");
    return BoundedText_format("C23InlayHints { documentId=%u, count=%zu, entries=%p, labelBytes=%p, labelByteCount=%zu, source=%p, sourceLength=%zu }",
        (*self).documentId, (*self).count, (void*) (*self).entries,
        (void*) (*self).labelBytes, (*self).labelByteCount,
        (const void*) (*self).source, (*self).sourceLength);
}
