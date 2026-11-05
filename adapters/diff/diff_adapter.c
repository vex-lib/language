#include "adapters/diff/diff_adapter.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
#include <string.h>

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: DiffAdapter module
 * ============================================================================
 * The DiffAdapter module maps one unified-diff line at a time to the shared
 * Token categories. This is enough for consumers to theme file headers, hunk
 * markers, additions, removals, context, and recognized metadata consistently
 * in Primeagen or Semicolon. It deliberately does not own a document buffer,
 * parser thread, UI palette, or the programming-language lexer for changed code.
 *
 * Unified diff has context-sensitive prefixes: before a hunk, `---` and `+++`
 * identify old/new file paths; inside a hunk those same leading bytes can be
 * ordinary removed/added source content. Therefore the caller supplies one
 * `inHunk` state bit for each stream. A `diff --git` line resets the state and an
 * `@@` hunk header enters it. This first slice handles unified Git patches, not
 * combined diffs, binary payload decoding, word-level changes, or arbitrary
 * vendor-specific patch formats.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: DiffAdapter (adapters/diff/diff_adapter.c)
 * ============================================================================
 * Stateless code with caller-owned per-stream state; classifies diff line roles.
 *
 * STATE RECORD (caller-owned, not defined or owned by this module):
 * ----------------------------------------------------------------------------
 *   bool inHunk; // false outside a hunk; true after a recognized @@ header
 * The caller initializes it to false for each patch stream. A new `diff --git`
 * line resets it. Multiple open documents must not share this state bit.
 *
 * TOKEN CATEGORIES PRODUCED:
 * ----------------------------------------------------------------------------
 *   TOKEN_KIND_DIFF_FILE      diff --git and pre-hunk --- / +++ path headers
 *   TOKEN_KIND_DIFF_METADATA index/mode/rename/binary metadata and no-newline
 *   TOKEN_KIND_DIFF_HUNK      @@ hunk headers; also enters hunk state
 *   TOKEN_KIND_DIFF_ADDITION  + content while in a hunk
 *   TOKEN_KIND_DIFF_REMOVAL   - content while in a hunk
 *   TOKEN_KIND_DIFF_CONTEXT   space-prefixed content while in a hunk
 * Each output Token range covers the original supplied line bytes, excluding
 * the LF delimiter. The classifier accepts a CR immediately before LF.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Public module operation:
 *   bool DiffAdapter_classifyLine(bool *inHunk, uint32_t documentId,
 *       uint64_t lineStartByte, const char *line, size_t lineLength,
 *       Token *outToken)
 *     Classify a recognized unified-diff line and publish its token/state.
 *     Returns false for invalid arguments, offset overflow, embedded NUL, empty
 *     lines, and unrecognized line forms. Rejection preserves both outputs and
 *     reports through THROW. Success does not allocate or retain the line.
 *
 * Private static helper:
 *   bool startsWith(const char *line, size_t length, const char *prefix,
 *       size_t prefixLength)
 *     Bounded byte-prefix test; never reads beyond the caller's span.
 *
 * CONTRACT: synchronous cold parsing, one caller serializes each stream. This
 * recognizes unified Git patch structure only. It does not parse source code,
 * resolve symbols, calculate line-number mappings, or render colors. Consumers
 * may compose this adapter with the source language adapter for added/deleted
 * line contents. Combined diff and binary patch payload semantics are unsupported.
 * ============================================================================
 */

static bool startsWith(const char *line, size_t length, const char *prefix,
                       size_t prefixLength) {
    return length >= prefixLength && memcmp(line, prefix, prefixLength) == 0;
}

bool DiffAdapter_classifyLine(bool *inHunk, uint32_t documentId,
                              uint64_t lineStartByte, const char *line,
                              size_t lineLength, Token *outToken) {
    if (inHunk == nullptr || line == nullptr || outToken == nullptr ||
        documentId == 0 || lineLength == 0 ||
        (uint64_t) lineLength > UINT64_MAX - lineStartByte ||
        memchr(line, '\0', lineLength) != nullptr) {
        THROW("diff adapter: invalid line span, identity, state, or output");
        return false;
    }

    size_t visibleLength = lineLength;
    if (line[visibleLength - 1] == '\r')
        --visibleLength;
    if (visibleLength == 0) {
        THROW("diff adapter: empty diff line");
        return false;
    }

    static const char diffGit[] = "diff --git ";
    static const char index[] = "index ";
    static const char oldFile[] = "--- ";
    static const char newFile[] = "+++ ";
    static const char hunk[] = "@@";
    static const char noNewline[] = "\\ No newline at end of file";
    static const char newFileMode[] = "new file mode ";
    static const char deletedFileMode[] = "deleted file mode ";
    static const char oldMode[] = "old mode ";
    static const char newMode[] = "new mode ";
    static const char similarity[] = "similarity index ";
    static const char renameFrom[] = "rename from ";
    static const char renameTo[] = "rename to ";
    static const char binaryFiles[] = "Binary files ";
    static const char gitBinary[] = "GIT binary patch";
    static const char literal[] = "literal ";
    static const char delta[] = "delta ";
    TokenKind kind = TOKEN_KIND_UNKNOWN;
    bool nextInHunk = *inHunk;

    if (startsWith(line, visibleLength, diffGit, sizeof(diffGit) - 1)) {
        kind = TOKEN_KIND_DIFF_FILE;
        nextInHunk = false;
    } else if (startsWith(line, visibleLength, hunk, sizeof(hunk) - 1)) {
        kind = TOKEN_KIND_DIFF_HUNK;
        nextInHunk = true;
    } else if (startsWith(line, visibleLength, noNewline, sizeof(noNewline) - 1) ||
               startsWith(line, visibleLength, index, sizeof(index) - 1) ||
               startsWith(line, visibleLength, newFileMode, sizeof(newFileMode) - 1) ||
               startsWith(line, visibleLength, deletedFileMode, sizeof(deletedFileMode) - 1) ||
               startsWith(line, visibleLength, oldMode, sizeof(oldMode) - 1) ||
               startsWith(line, visibleLength, newMode, sizeof(newMode) - 1) ||
               startsWith(line, visibleLength, similarity, sizeof(similarity) - 1) ||
               startsWith(line, visibleLength, renameFrom, sizeof(renameFrom) - 1) ||
               startsWith(line, visibleLength, renameTo, sizeof(renameTo) - 1) ||
               startsWith(line, visibleLength, binaryFiles, sizeof(binaryFiles) - 1) ||
               startsWith(line, visibleLength, gitBinary, sizeof(gitBinary) - 1) ||
               startsWith(line, visibleLength, literal, sizeof(literal) - 1) ||
               startsWith(line, visibleLength, delta, sizeof(delta) - 1)) {
        kind = TOKEN_KIND_DIFF_METADATA;
    } else if (!(*inHunk) &&
               (startsWith(line, visibleLength, oldFile, sizeof(oldFile) - 1) ||
                startsWith(line, visibleLength, newFile, sizeof(newFile) - 1))) {
        kind = TOKEN_KIND_DIFF_FILE;
    } else if (*inHunk) {
        switch (line[0]) {
            case '+': kind = TOKEN_KIND_DIFF_ADDITION; break;
            case '-': kind = TOKEN_KIND_DIFF_REMOVAL; break;
            case ' ': kind = TOKEN_KIND_DIFF_CONTEXT; break;
            default: break;
        }
    }

    if (kind == TOKEN_KIND_UNKNOWN) {
        THROW("diff adapter: unrecognized unified diff line");
        return false;
    }

    Token result = Token(kind, SourceRange(documentId, lineStartByte,
                                           lineStartByte + lineLength));
    *outToken = result;
    *inHunk = nextInHunk;
    return true;
}
