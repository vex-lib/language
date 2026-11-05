#include "index/file_analysis.h"
#include <assert.h>
#include <string.h>

int main(void) {
    FileAnalysis *analysis = FileAnalysis_4(2u, 20u, 4u, 99u);
    assert(analysis != nullptr);
    assert(FileAnalysis_getFileId(analysis) == 2u);
    assert(FileAnalysis_getDocumentId(analysis) == 20u);
    assert(FileAnalysis_getDocumentRevision(analysis) == 4u);
    assert(FileAnalysis_getCompilationContextId(analysis) == 99u);

    char name[] = "load";
    Symbol symbol = Symbol_5(200u, 1u, SYMBOL_KIND_FUNCTION, name,
                             SourceRange_3(20u, 0u, 4u));
    assert(FileAnalysis_addSymbol(analysis, symbol));
    name[0] = 'X';
    Symbol saved = FileAnalysis_getSymbolAt(analysis, 0u);
    assert(strcmp(saved.name, "load") == 0);
    assert(!FileAnalysis_addSymbol(analysis, saved));
    assert(FileAnalysis_getSymbolCount(analysis) == 1u);

    Reference unresolved = Reference_4(0u, SourceRange_3(20u, 8u, 12u),
                                      REFERENCE_ROLE_CALL, false);
    assert(FileAnalysis_addReference(analysis, unresolved));
    Reference invalid = Reference_4(8u, SourceRange_3(20u, 8u, 12u),
                                   REFERENCE_ROLE_CALL, false);
    assert(!FileAnalysis_addReference(analysis, invalid));
    assert(FileAnalysis_getReferenceCount(analysis) == 1u);
    assert(!FileAnalysis_getReferenceAt(analysis, 0u).resolved);
    FileAnalysis_free(analysis);
    return 0;
}
