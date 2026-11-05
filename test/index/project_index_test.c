#include "index/project_index.h"
#include <assert.h>
#include <string.h>

static Symbol makeFunction(uint64_t id, uint32_t documentId, size_t start, size_t end,
                           const char *name) {
    return Symbol_5(id, 1u, SYMBOL_KIND_FUNCTION, name,
                    SourceRange_3(documentId, start, end));
}

int main(void) {
    LanguageProject *project = LanguageProject_2(6u, "/work/project");
    assert(project != nullptr);
    assert(LanguageProject_addFile(project, "include/api.h", PROJECT_FILE_KIND_TEXT, "c23") != nullptr);
    assert(LanguageProject_addFile(project, "src/api.c", PROJECT_FILE_KIND_TEXT, "c23") != nullptr);
    ProjectIndex *index = ProjectIndex_1(project);
    assert(index != nullptr && ProjectIndex_getProjectId(index) == 6u);

    FileAnalysis *header = FileAnalysis_4(1u, 101u, 4u, 70u);
    assert(header != nullptr);
    assert(FileAnalysis_addSymbol(header, makeFunction(400u, 101u, 2u, 6u, "load")));
    assert(ProjectIndex_publish(index, header));
    FileAnalysis_free(header);

    FileAnalysis *implementation = FileAnalysis_4(2u, 102u, 1u, 70u);
    assert(implementation != nullptr);
    assert(FileAnalysis_addSymbol(implementation,
                                  makeFunction(400u, 102u, 8u, 12u, "load")));
    assert(FileAnalysis_addReference(implementation,
        Reference_4(400u, SourceRange_3(102u, 20u, 24u), REFERENCE_ROLE_CALL, true)));
    assert(FileAnalysis_addReference(implementation,
        Reference_4(0u, SourceRange_3(102u, 30u, 34u), REFERENCE_ROLE_CALL, false)));
    assert(ProjectIndex_publish(index, implementation));
    FileAnalysis_free(implementation);

    assert(ProjectIndex_getAnalysisCount(index) == 2u);
    assert(ProjectIndex_getDeclarationCount(index, 400u, 70u) == 2u);
    Symbol declaration = ProjectIndex_getDeclarationAt(index, 400u, 70u, 1u);
    assert(strcmp(declaration.name, "load") == 0);
    assert(declaration.declarationRange.documentId == 102u);
    assert(ProjectIndex_getReferenceCount(index, 400u, 70u) == 1u);
    Reference usage = ProjectIndex_getReferenceAt(index, 400u, 70u, 0u);
    assert(usage.range.documentId == 102u && usage.resolved);
    assert(ProjectIndex_getReferenceAt(index, 400u, 70u, 1u).targetSymbolId == 0u);
    assert(ProjectIndex_findAnalysis(index, 1u, 70u) != nullptr);

    FileAnalysis *stale = FileAnalysis_4(1u, 101u, 3u, 70u);
    assert(stale != nullptr);
    assert(FileAnalysis_addSymbol(stale, makeFunction(400u, 101u, 2u, 6u, "load")));
    assert(!ProjectIndex_publish(index, stale));
    FileAnalysis_free(stale);
    assert(FileAnalysis_getDocumentRevision(ProjectIndex_findAnalysis(index, 1u, 70u)) == 4u);

    FileAnalysis *conflict = FileAnalysis_4(2u, 102u, 2u, 70u);
    assert(conflict != nullptr);
    assert(FileAnalysis_addSymbol(conflict, makeFunction(400u, 102u, 8u, 12u, "unrelated")));
    assert(!ProjectIndex_publish(index, conflict));
    FileAnalysis_free(conflict);
    assert(ProjectIndex_getDeclarationCount(index, 400u, 70u) == 2u);

    FileAnalysis *unknown = FileAnalysis_4(90u, 190u, 1u, 70u);
    assert(unknown != nullptr);
    assert(!ProjectIndex_publish(index, unknown));
    FileAnalysis_free(unknown);

    ProjectIndex_free(index);
    LanguageProject_free(project);
    return 0;
}
