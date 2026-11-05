#include "lang/language_lex_result.h"
#include <assert.h>

int main(void) {
    LanguageLexResult empty = LanguageLexResult_0();

    assert(!empty.hasSpan);

    SourceRange emptyRange = empty.span.range;

    assert(emptyRange.documentId == 0u);

    SyntaxSpan span = SyntaxSpan_3(SourceRange_3(7u, 2u, 5u), SYNTAX_ROLE_KEYWORD, nullptr);

    LanguageLexResult full = LanguageLexResult_2(true, span);

    assert(full.hasSpan);

    SourceRange fullRange = full.span.range;

    assert(fullRange.documentId == 7u);

    assert(LanguageLexResult_2(false, span).span.range.documentId == 0u);

    full = LanguageLexResult_4(true, span, 5u, 12u);

    assert(full.documentRevision == 5u && full.compilationContextId == 12u);

    full = LanguageLexResult_withContext(full, 8u, 14u);

    assert(full.documentRevision == 8u && full.compilationContextId == 14u);

    LanguageLexResult end = LanguageLexResult_4(false, span, 9u, 15u);

    assert(!end.hasSpan);

    SourceRange endRange = end.span.range;

    assert(endRange.documentId == 0u);

    assert(end.documentRevision == 9u && end.compilationContextId == 15u);

    end = LanguageLexResult_withContext(LanguageLexResult_0(), 10u, 16u);

    assert(!end.hasSpan);

    SourceRange emptyEndRange = end.span.range;

    assert(emptyEndRange.documentId == 0u);

    assert(end.documentRevision == 10u && end.compilationContextId == 16u);

    return 0;
}
