#include "document/source_range.h"
#include <assert.h>

int main(void) {
    SourceRange empty = SourceRange();
    SourceRange zero = SourceRange_zero();
    assert(!SourceRange_isValid(&empty));
    assert(!SourceRange_isValid(&zero));
    SourceRange range = SourceRange(7, 2, 9);
    assert(SourceRange_isValid(&range));
    assert(SourceRange_getDocumentId(&range) == 7);
    SourceRange_setStartByte(&range, 10);
    assert(!SourceRange_isValid(&range));
    assert(SourceRange_getEndByte(nullptr) == 0);
    SourceRange_setEndByte(nullptr, 4);
    return 0;
}
