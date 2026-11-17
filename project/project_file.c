#include "vex.h"
#include "project/project_file.h"
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
    u64 id;
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
        bytes length = (bytes) (cursor - part);
        if (length == 0u || (length == 1u && part[0] == '.')
            || (length == 2u && part[0] == '.' && part[1] == '.'))
            return false;
        if (*cursor == '\0')
            return true;
        part = cursor + 1u;
    }
}

static char *copyString(const char *text) {
    bytes length = strlen(text);
    char *copy = malloc(length + 1u);
    if (copy != nullptr)
        memcpy(copy, text, length + 1u);
    return copy;
}

;;GETTER
static inline u64 ProjectFile_id(const ProjectFile *self) { return (*self).id; }
;;SETTER
static inline fn ProjectFile_setId(ProjectFile *self, u64 v) { (*self).id = v; }
;;GETTER
static inline char *ProjectFile_relativePath(const ProjectFile *self) { return (*self).relativePath; }
;;SETTER
static inline fn ProjectFile_setRelativePath(ProjectFile *self, char *v) { (*self).relativePath = v; }
;;GETTER
static inline ProjectFileKind ProjectFile_kind(const ProjectFile *self) { return (*self).kind; }
;;SETTER
static inline fn ProjectFile_setKind(ProjectFile *self, ProjectFileKind v) { (*self).kind = v; }
;;GETTER
static inline char *ProjectFile_languageId(const ProjectFile *self) { return (*self).languageId; }
;;SETTER
static inline fn ProjectFile_setLanguageId(ProjectFile *self, char *v) { (*self).languageId = v; }
;;GETTER
static inline ProjectFileAnalysisStatus ProjectFile_status(const ProjectFile *self) { return (*self).status; }
;;SETTER
static inline fn ProjectFile_setStatus(ProjectFile *self, ProjectFileAnalysisStatus v) { (*self).status = v; }

ProjectFile *ProjectFile_4(u64 fileId, const char *relativePath,
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
    ProjectFile_setRelativePath(self, copyString(relativePath));
    ProjectFile_setLanguageId(self, languageId == nullptr ? nullptr : copyString(languageId));
    if (ProjectFile_relativePath(self) == nullptr || (languageId != nullptr && ProjectFile_languageId(self) == nullptr)) {
        THROW("ProjectFile: allocation failed while copying metadata");
        free(ProjectFile_relativePath(self));
        free(ProjectFile_languageId(self));
        free(self);
        return nullptr;
    }
    ProjectFile_setId(self, fileId);
    ProjectFile_setKind(self, kind);
    ProjectFile_setStatus(self, kind == PROJECT_FILE_KIND_TEXT && languageId != nullptr
        ? PROJECT_FILE_ANALYSIS_PENDING : PROJECT_FILE_ANALYSIS_UNSUPPORTED);
    return self;
}

fn ProjectFile_free(ProjectFile *self) {
    if (self != nullptr) {
        free(ProjectFile_relativePath(self));
        ProjectFile_setRelativePath(self, nullptr);
        free(ProjectFile_languageId(self));
        ProjectFile_setLanguageId(self, nullptr);
        free(self);
    }
}
;;GETTER
u64 ProjectFile_getId(const ProjectFile *self) { return self == nullptr ? 0u : ProjectFile_id(self); }
;;GETTER
const char *ProjectFile_getRelativePath(const ProjectFile *self) {
    return self == nullptr ? nullptr : ProjectFile_relativePath(self);
}
;;GETTER
ProjectFileKind ProjectFile_getKind(const ProjectFile *self) {
    return self == nullptr ? PROJECT_FILE_KIND_TEXT : ProjectFile_kind(self);
}
;;GETTER
const char *ProjectFile_getLanguageId(const ProjectFile *self) {
    return self == nullptr ? nullptr : ProjectFile_languageId(self);
}
;;GETTER
ProjectFileAnalysisStatus ProjectFile_getAnalysisStatus(const ProjectFile *self) {
    return self == nullptr ? PROJECT_FILE_ANALYSIS_UNSUPPORTED : ProjectFile_status(self);
}
bool ProjectFile_setAnalysisStatus(ProjectFile *self, ProjectFileAnalysisStatus status) {
    if (self == nullptr || status < PROJECT_FILE_ANALYSIS_PENDING
        || status > PROJECT_FILE_ANALYSIS_UNSUPPORTED
        || (status != PROJECT_FILE_ANALYSIS_UNSUPPORTED
            && (ProjectFile_kind(self) != PROJECT_FILE_KIND_TEXT || ProjectFile_languageId(self) == nullptr))) {
        THROW("ProjectFile: rejected incompatible analysis status");
        return false;
    }
    ProjectFile_setStatus(self, status);
    return true;
}
