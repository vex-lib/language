#include "adapters/diff/diff_adapter.h"
#include <assert.h>
#include <stdint.h>

int main(void) {
    bool inHunk = false;
    Token token = Token(TOKEN_KIND_COMMENT);
    assert(DiffAdapter_classifyLine(&inHunk, 3, 0, "diff --git a/a.c b.sh/a.c", 22, &token));
    assert(Token_getKind(&token) == TOKEN_KIND_DIFF_FILE);
    assert(!inHunk);
    assert(DiffAdapter_classifyLine(&inHunk, 3, 22, "--- a/a.c", 9, &token));
    assert(Token_getKind(&token) == TOKEN_KIND_DIFF_FILE);
    assert(DiffAdapter_classifyLine(&inHunk, 3, 31, "+++ b.sh/a.c", 9, &token));
    assert(Token_getKind(&token) == TOKEN_KIND_DIFF_FILE);
    assert(DiffAdapter_classifyLine(&inHunk, 3, 40, "@@ -1,2 +1,2 @@", 15, &token));
    assert(Token_getKind(&token) == TOKEN_KIND_DIFF_HUNK);
    assert(inHunk);

    assert(DiffAdapter_classifyLine(&inHunk, 3, 55, "-old", 4, &token));
    assert(Token_getKind(&token) == TOKEN_KIND_DIFF_REMOVAL);
    assert(DiffAdapter_classifyLine(&inHunk, 3, 59, "+new\r", 5, &token));
    assert(Token_getKind(&token) == TOKEN_KIND_DIFF_ADDITION);
    assert(DiffAdapter_classifyLine(&inHunk, 3, 64, " context", 8, &token));
    assert(Token_getKind(&token) == TOKEN_KIND_DIFF_CONTEXT);
    assert(DiffAdapter_classifyLine(&inHunk, 3, 72, "\\ No newline at end of file", 27, &token));
    assert(Token_getKind(&token) == TOKEN_KIND_DIFF_METADATA);
    assert(DiffAdapter_classifyLine(&inHunk, 3, 99, "diff --git a/b.sh.c b.sh/b.sh.c", 22, &token));
    assert(Token_getKind(&token) == TOKEN_KIND_DIFF_FILE);
    assert(!inHunk);

    Token before = token;
    assert(!DiffAdapter_classifyLine(&inHunk, 3, 121, "not a diff line", 15, &token));
    assert(Token_getKind(&token) == Token_getKind(&before));
    assert(!inHunk);
    assert(!DiffAdapter_classifyLine(&inHunk, 3, UINT64_MAX, "+x", 2, &token));
    assert(Token_getKind(&token) == Token_getKind(&before));
    assert(!inHunk);
    assert(!DiffAdapter_classifyLine(nullptr, 3, 0, "+x", 2, &token));
    assert(!DiffAdapter_classifyLine(&inHunk, 3, 0, nullptr, 2, &token));
    assert(!DiffAdapter_classifyLine(&inHunk, 3, 0, "+x", 2, nullptr));
    assert(!DiffAdapter_classifyLine(&inHunk, 0, 0, "+x", 2, &token));
    assert(!DiffAdapter_classifyLine(&inHunk, 3, 0, "+x", 0, &token));
    assert(!DiffAdapter_classifyLine(&inHunk, 3, 0, "\r", 1, &token));
    const char embeddedNul[] = {'+', '\0', 'x'};
    assert(!DiffAdapter_classifyLine(&inHunk, 3, 0, embeddedNul, sizeof(embeddedNul), &token));
    assert(Token_getKind(&token) == Token_getKind(&before));
    assert(!inHunk);

    assert(DiffAdapter_classifyLine(&inHunk, 3, UINT64_MAX - 2, "@@", 2, &token));
    assert(Token_getKind(&token) == TOKEN_KIND_DIFF_HUNK);
    SourceRange edgeRange = Token_getRange(&token);
    assert(SourceRange_getEndByte(&edgeRange) == UINT64_MAX);
    assert(DiffAdapter_classifyLine(&inHunk, 3, 0, "diff --git a/c b.sh/c", 18, &token));
    assert(!inHunk);
    return 0;
}
