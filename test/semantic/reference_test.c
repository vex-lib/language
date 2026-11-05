#include "semantic/reference.h"
#include <assert.h>

int main(void) {
    Reference unresolved = Reference();
    Reference zero = Reference_zero();
    assert(!Reference_isResolved(&unresolved));
    assert(!Reference_isResolved(&zero));
    SourceRange range = SourceRange(4, 11, 15);
    Reference reference = Reference(8, range, REFERENCE_ROLE_WRITE, true);
    assert(Reference_getTargetSymbolId(&reference) == 8);
    assert(Reference_getRole(&reference) == REFERENCE_ROLE_WRITE);
    assert(Reference_isResolved(&reference));
    Reference_setResolved(&reference, false);
    assert(!Reference_isResolved(&reference));
    assert(!Reference_isResolved(nullptr));
    Reference_setRole(nullptr, REFERENCE_ROLE_READ);
    return 0;
}
