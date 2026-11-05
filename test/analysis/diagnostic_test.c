#include "analysis/diagnostic.h"
#include <assert.h>
#include <string.h>

int main(void) {
    Diagnostic empty = Diagnostic();
    assert(empty.severity == DIAGNOSTIC_SEVERITY_HINT);
    assert(empty.code == DIAGNOSTIC_CODE_NONE);
    assert(empty.message.length == 0u);
    Diagnostic diagnostic = Diagnostic(SourceRange(1u, 4u, 5u),
                                        DIAGNOSTIC_SEVERITY_WARNING,
                                        DIAGNOSTIC_CODE_UNNECESSARY_SEMICOLON,
                                        BoundedText("unnecessary semicolon"));
    assert(strcmp(DiagnosticSeverity_name(diagnostic.severity), "warning") == 0);
    assert(strcmp(DiagnosticCode_name(diagnostic.code), "unnecessary-semicolon") == 0);
    assert(Diagnostic_toString(&diagnostic).length > 0u);
    assert(Diagnostic_toString(nullptr).length == 7u);
    return 0;
}
