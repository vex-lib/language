#include "adapters/c/c23_lex_step.h"
#include <assert.h>

int main(void) {
    C23LexStep empty = C23LexStep();
    assert(!empty.hasSpan);
    SyntaxSpan emptySpan = empty.span;
    SourceRange emptyRange = emptySpan.range;
    assert(emptyRange.documentId == 0u);
    assert(emptyRange.startByte == 0u);
    assert(emptyRange.endByte == 0u);
    C23LexStep zero = C23LexStep_zero();
    assert(!zero.hasSpan);
    C23LexStep directEmpty = C23LexStep_0();
    assert(!directEmpty.hasSpan);
    C23LexStep present = C23LexStep(true,
        SyntaxSpan_3(SourceRange_3(7u, 2u, 5u), SYNTAX_ROLE_IDENTIFIER, nullptr));
    C23LexStep directPresent = C23LexStep_2(true,
        SyntaxSpan_3(SourceRange_3(8u, 1u, 2u), SYNTAX_ROLE_KEYWORD, nullptr));
    assert(directPresent.hasSpan);
    assert(C23LexStep_hasSpan(&present));
    SyntaxSpan presentSpan = C23LexStep_getSpan(&present);
    assert(presentSpan.range.documentId == 7u);
    assert(!C23LexStep_hasSpan(nullptr));
    SyntaxSpan nullSpan = C23LexStep_getSpan(nullptr);
    assert(nullSpan.range.documentId == 0u);
    C23LexStep suppressed = C23LexStep(false,
        SyntaxSpan_3(SourceRange_3(9u, 3u, 6u), SYNTAX_ROLE_KEYWORD, nullptr));
    assert(!suppressed.hasSpan);
    SyntaxSpan suppressedSpan = suppressed.span;
    assert(suppressedSpan.range.documentId == 0u);
    assert(!C23LexStep_toString(&present).truncated);
    BoundedText nullProjection = C23LexStep_toString(nullptr);
    assert(BoundedText_equals(&nullProjection, "nullptr"));
    assert(!C23LexStep_toStringStruct(&present).truncated);
    return 0;
}
