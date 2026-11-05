#include "project/project_file.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/** ProjectFile describes one inventoried path without assuming every file is
 * source code. It stores explicit host-supplied classification and an optional
 * language id; it does not open, decode or execute the file. */

;;OVERVIEW
/** CLASS: ProjectFile (project/project_file.c)
 * STRUCT FIELDS: file id, copied relative path, kind, optional copied language
 * id, and analysis status.
 * FUNCTION REGISTRY: constructor/free, field getters and status setter.
 * CONTRACT: paths are normalized relative slash paths with no dot traversal;
 * non-text or language-less files start unsupported; state changes are validated. */

struct ProjectFile {
    uint64_t id;
    char *relativePath;
    ProjectFileKind kind;
    char *languageId;
    ProjectFileAnalysisStatus status;
};

static bool pathIsSafeRelative(const char *path) {
    if (path == nullptr || path[0] == '\0' || path[0] == '/' || strchr(path, '\\') != nullptr
        || (path[0] != '\0' && path[1] == ':'))
        return false;
    const char *part = path;
    for (const char *cursor = path;; ++cursor) {
        if (*cursor != '/' && *cursor != '\0')
            continue;
        size_t length = (size_t) (cursor - part);
        if (length == 0u || (length == 1u && part[0] == '.')
            || (length == 2u && part[0] == '.' && part[1] == '.'))
            return false;
        if (*cursor == '\0')
            return true;
        part = cursor + 1u;
    }
}

static char *copyString(const char *text) {
    size_t length = strlen(text);
    char *copy = malloc(length + 1u);
    if (copy != nullptr)
        memcpy(copy, text, length + 1u);
    return copy;
}

ProjectFile *ProjectFile_4(uint64_t fileId, const char *relativePath,
                           ProjectFileKind kind, const char *languageId) {
    if (fileId == 0u || !pathIsSafeRelative(relativePath)
        || kind < PROJECT_FILE_KIND_TEXT || kind > PROJECT_FILE_KIND_ASSET
        || (languageId != nullptr && languageId[0] == '\0')) {
        THROW("ProjectFile: rejected invalid identity, path, kind or language id");
        return nullptr;
    }
    ProjectFile *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("ProjectFile: allocation failed");
        return nullptr;
    }
    (*self).relativePath = copyString(relativePath);
    (*self).languageId = languageId == nullptr ? nullptr : copyString(languageId);
    if ((*self).relativePath == nullptr || (languageId != nullptr && (*self).languageId == nullptr)) {
        THROW("ProjectFile: allocation failed while copying metadata");
        free((*self).relativePath);
        free((*self).languageId);
        free(self);
        return nullptr;
    }
    (*self).id = fileId;
    (*self).kind = kind;
    (*self).status = kind == PROJECT_FILE_KIND_TEXT && languageId != nullptr
        ? PROJECT_FILE_ANALYSIS_PENDING : PROJECT_FILE_ANALYSIS_UNSUPPORTED;
    return self;
}

void ProjectFile_free(ProjectFile *self) {
    if (self != nullptr) {
        free((*self).relativePath);
        (*self).relativePath = nullptr;
        free((*self).languageId);
        (*self).languageId = nullptr;
        free(self);
    }
}
uint64_t ProjectFile_getId(const ProjectFile *self) { return self == nullptr ? 0u : (*self).id; }
const char *ProjectFile_getRelativePath(const ProjectFile *self) {
    return self == nullptr ? nullptr : (*self).relativePath;
}
ProjectFileKind ProjectFile_getKind(const ProjectFile *self) {
    return self == nullptr ? PROJECT_FILE_KIND_TEXT : (*self).kind;
}
const char *ProjectFile_getLanguageId(const ProjectFile *self) {
    return self == nullptr ? nullptr : (*self).languageId;
}
ProjectFileAnalysisStatus ProjectFile_getAnalysisStatus(const ProjectFile *self) {
    return self == nullptr ? PROJECT_FILE_ANALYSIS_UNSUPPORTED : (*self).status;
}
bool ProjectFile_setAnalysisStatus(ProjectFile *self, ProjectFileAnalysisStatus status) {
    if (self == nullptr || status < PROJECT_FILE_ANALYSIS_PENDING
        || status > PROJECT_FILE_ANALYSIS_UNSUPPORTED
        || (status != PROJECT_FILE_ANALYSIS_UNSUPPORTED
            && ((*self).kind != PROJECT_FILE_KIND_TEXT || (*self).languageId == nullptr))) {
        THROW("ProjectFile: rejected incompatible analysis status");
        return false;
    }
    (*self).status = status;
    return true;
}
