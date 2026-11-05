#include "project/project_file.h"
#include <assert.h>
#include <string.h>

int main(void) {
    ProjectFile *file = ProjectFile_4(5u, "src/main.c", PROJECT_FILE_KIND_TEXT, "c23");
    assert(file != nullptr);
    assert(ProjectFile_getId(file) == 5u);
    assert(strcmp(ProjectFile_getRelativePath(file), "src/main.c") == 0);
    assert(ProjectFile_getKind(file) == PROJECT_FILE_KIND_TEXT);
    assert(strcmp(ProjectFile_getLanguageId(file), "c23") == 0);
    assert(ProjectFile_getAnalysisStatus(file) == PROJECT_FILE_ANALYSIS_PENDING);
    assert(ProjectFile_setAnalysisStatus(file, PROJECT_FILE_ANALYSIS_CURRENT));
    assert(ProjectFile_getAnalysisStatus(file) == PROJECT_FILE_ANALYSIS_CURRENT);
    ProjectFile_free(file);

    file = ProjectFile_4(6u, "assets/icon.bin", PROJECT_FILE_KIND_BINARY, nullptr);
    assert(file != nullptr);
    assert(ProjectFile_getAnalysisStatus(file) == PROJECT_FILE_ANALYSIS_UNSUPPORTED);
    assert(!ProjectFile_setAnalysisStatus(file, PROJECT_FILE_ANALYSIS_CURRENT));
    assert(ProjectFile_getAnalysisStatus(file) == PROJECT_FILE_ANALYSIS_UNSUPPORTED);
    ProjectFile_free(file);
    assert(ProjectFile_4(7u, "../outside.c", PROJECT_FILE_KIND_TEXT, "c23") == nullptr);
    return 0;
}
