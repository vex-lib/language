#include "project/language_project.h"
#include <assert.h>
#include <string.h>

int main(void) {
    LanguageProject *project = LanguageProject_2(90u, "/work/sample");
    assert(project != nullptr);
    assert(LanguageProject_getId(project) == 90u);
    assert(strcmp(LanguageProject_getRootPath(project), "/work/sample") == 0);
    assert(LanguageProject_getFileCount(project) == 0u);

    const ProjectFile *source = LanguageProject_addFile(
        project, "src/main.c", PROJECT_FILE_KIND_TEXT, "c23");
    const ProjectFile *header = LanguageProject_addFile(
        project, "include/api.h", PROJECT_FILE_KIND_TEXT, "c23");
    const ProjectFile *asset = LanguageProject_addFile(
        project, "assets/logo.png", PROJECT_FILE_KIND_BINARY, nullptr);
    const ProjectFile *legacy = LanguageProject_addFile(
        project, "compat/legacy.c", PROJECT_FILE_KIND_TEXT, "c99");
    assert(source != nullptr && header != nullptr && asset != nullptr && legacy != nullptr);
    assert(ProjectFile_getId(source) == 1u && ProjectFile_getId(header) == 2u);
    assert(ProjectFile_getId(asset) == 3u && ProjectFile_getId(legacy) == 4u);
    assert(LanguageProject_getFileCount(project) == 4u);
    assert(LanguageProject_findFile(project, "include/api.h") == header);
    assert(LanguageProject_getFileAt(project, 0u) == source);
    assert(LanguageProject_getFileAt(project, 4u) == nullptr);
    assert(LanguageProject_findFile(project, "missing.c") == nullptr);
    assert(LanguageProject_addFile(project, "src/main.c", PROJECT_FILE_KIND_TEXT, "c23") == nullptr);
    assert(LanguageProject_getFileCount(project) == 4u);

    ProjectConfiguration *configuration = LanguageProject_getConfiguration(project);
    assert(configuration != nullptr);
    assert(ProjectConfiguration_setDetectedLanguage(configuration, "c23"));
    assert(strcmp(ProjectConfiguration_getLanguageId(configuration), "c23") == 0);
    BuildTarget *app = LanguageProject_addTarget(project, "app", "c23");
    BuildTarget *library = LanguageProject_addTarget(project, "library", "c23");
    assert(app != nullptr && library != nullptr);
    assert(LanguageProject_getTargetCount(project) == 2u);
    assert(LanguageProject_findTarget(project, "app") == app);
    assert(LanguageProject_getTargetAt(project, 1u) == library);
    assert(LanguageProject_getTargetAt(project, 2u) == nullptr);
    assert(LanguageProject_addSourceToTarget(project, BuildTarget_getId(app),
                                             ProjectFile_getId(source)));
    assert(LanguageProject_addSourceToTarget(project, BuildTarget_getId(app),
                                             ProjectFile_getId(header)));
    assert(!LanguageProject_addSourceToTarget(project, BuildTarget_getId(app),
                                              ProjectFile_getId(asset)));
    assert(!LanguageProject_addSourceToTarget(project, BuildTarget_getId(app),
                                              ProjectFile_getId(legacy)));
    assert(LanguageProject_addTargetDependency(project, BuildTarget_getId(app),
                                               BuildTarget_getId(library)));
    assert(!LanguageProject_addTargetDependency(project, BuildTarget_getId(app), 99u));
    assert(LanguageProject_addTarget(project, "app", "c23") == nullptr);
    LanguageProject_free(project);
    return 0;
}
