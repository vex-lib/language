#include "vex.h"
#include "project/language_project.h"
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/** LanguageProject is the Language-side inventory and identity for one open
 * project. The host owns project opening and discovery, then supplies relative
 * file records. This class performs no filesystem reads or build execution. */

;;OVERVIEW
/** CLASS: LanguageProject (project/language_project.c)
 * STRUCT FIELDS: project id, copied root path, owned ProjectFile pointer array,
 * file count and capacity.
 * FUNCTION REGISTRY: constructor/free, add/find/get file, identity/count getters.
 * CONTRACT: file ids are assigned monotonically from one; duplicate paths
 * reject without changing the inventory; returned file pointers are borrowed. */

struct LanguageProject {
    u64 id;
    char *rootPath;
    ProjectFile **files;
    bytes fileCount;
    bytes fileCapacity;
    BuildTarget **targets;
    bytes targetCount;
    bytes targetCapacity;
    ProjectConfiguration *configuration;
};

static char *copyRoot(const char *path) {
    bytes length = strlen(path);
    char *copy = malloc(length + 1u);
    if (copy != nullptr)
        memcpy(copy, path, length + 1u);
    return copy;
}

;;GETTER
static inline u64 LanguageProject_id(const LanguageProject *self) { return (*self).id; }
;;SETTER
static inline fn LanguageProject_setId(LanguageProject *self, u64 v) { (*self).id = v; }
;;GETTER
static inline char *LanguageProject_rootPath(const LanguageProject *self) { return (*self).rootPath; }
;;SETTER
static inline fn LanguageProject_setRootPath(LanguageProject *self, char *v) { (*self).rootPath = v; }
;;GETTER
static inline ProjectFile **LanguageProject_files(const LanguageProject *self) { return (*self).files; }
;;SETTER
static inline fn LanguageProject_setFiles(LanguageProject *self, ProjectFile **v) { (*self).files = v; }
;;GETTER
static inline bytes LanguageProject_fileCount(const LanguageProject *self) { return (*self).fileCount; }
;;SETTER
static inline fn LanguageProject_setFileCount(LanguageProject *self, bytes v) { (*self).fileCount = v; }
;;GETTER
static inline bytes LanguageProject_fileCapacity(const LanguageProject *self) { return (*self).fileCapacity; }
;;SETTER
static inline fn LanguageProject_setFileCapacity(LanguageProject *self, bytes v) { (*self).fileCapacity = v; }
;;GETTER
static inline BuildTarget **LanguageProject_targets(const LanguageProject *self) { return (*self).targets; }
;;SETTER
static inline fn LanguageProject_setTargets(LanguageProject *self, BuildTarget **v) { (*self).targets = v; }
;;GETTER
static inline bytes LanguageProject_targetCount(const LanguageProject *self) { return (*self).targetCount; }
;;SETTER
static inline fn LanguageProject_setTargetCount(LanguageProject *self, bytes v) { (*self).targetCount = v; }
;;GETTER
static inline bytes LanguageProject_targetCapacity(const LanguageProject *self) { return (*self).targetCapacity; }
;;SETTER
static inline fn LanguageProject_setTargetCapacity(LanguageProject *self, bytes v) { (*self).targetCapacity = v; }
;;GETTER
static inline ProjectConfiguration *LanguageProject_configuration(const LanguageProject *self) { return (*self).configuration; }
;;SETTER
static inline fn LanguageProject_setConfiguration(LanguageProject *self, ProjectConfiguration *v) { (*self).configuration = v; }

LanguageProject *LanguageProject_2(u64 projectId, const char *rootPath) {
    if (projectId == 0u || rootPath == nullptr || rootPath[0] == '\0') {
        THROW("LanguageProject: rejected invalid id or root path");
        return nullptr;
    }
    LanguageProject *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("LanguageProject: allocation failed");
        return nullptr;
    }
    LanguageProject_setRootPath(self, copyRoot(rootPath));
    if (LanguageProject_rootPath(self) == nullptr) {
        THROW("LanguageProject: allocation failed while copying root path");
        free(self);
        return nullptr;
    }
    LanguageProject_setConfiguration(self, ProjectConfiguration_0());
    if (LanguageProject_configuration(self) == nullptr) {
        THROW("LanguageProject: unable to create project configuration");
        free(LanguageProject_rootPath(self));
        free(self);
        return nullptr;
    }
    LanguageProject_setId(self, projectId);
    return self;
}

fn LanguageProject_free(LanguageProject *self) {
    if (self == nullptr)
        return;
    for (bytes index = 0u; index < LanguageProject_fileCount(self); ++index)
        ProjectFile_free(LanguageProject_files(self)[index]);
    for (bytes index = 0u; index < LanguageProject_targetCount(self); ++index)
        BuildTarget_free(LanguageProject_targets(self)[index]);
    ProjectConfiguration_free(LanguageProject_configuration(self));
    LanguageProject_setConfiguration(self, nullptr);
    free(LanguageProject_targets(self));
    LanguageProject_setTargets(self, nullptr);
    free(LanguageProject_files(self));
    LanguageProject_setFiles(self, nullptr);
    free(LanguageProject_rootPath(self));
    LanguageProject_setRootPath(self, nullptr);
    free(self);
}

const ProjectFile *LanguageProject_findFile(const LanguageProject *self,
                                             const char *relativePath) {
    if (self == nullptr || relativePath == nullptr)
        return nullptr;
    for (bytes index = 0u; index < LanguageProject_fileCount(self); ++index)
        if (strcmp(ProjectFile_getRelativePath(LanguageProject_files(self)[index]), relativePath) == 0)
            return LanguageProject_files(self)[index];
    return nullptr;
}
const ProjectFile *LanguageProject_findFileById(const LanguageProject *self, u64 fileId) {
    if (self == nullptr || fileId == 0u)
        return nullptr;
    for (bytes index = 0u; index < LanguageProject_fileCount(self); ++index)
        if (ProjectFile_getId(LanguageProject_files(self)[index]) == fileId)
            return LanguageProject_files(self)[index];
    return nullptr;
}

const ProjectFile *LanguageProject_addFile(LanguageProject *self,
                                           const char *relativePath,
                                           ProjectFileKind kind,
                                           const char *languageId) {
    if (self == nullptr || relativePath == nullptr
        || LanguageProject_findFile(self, relativePath) != nullptr) {
        THROW("LanguageProject: rejected invalid or duplicate file path");
        return nullptr;
    }
    if (LanguageProject_fileCount(self) == SIZE_MAX) {
        THROW("LanguageProject: file count bound reached, request 1 file");
        return nullptr;
    }
    if (LanguageProject_fileCount(self) == LanguageProject_fileCapacity(self)) {
        bytes capacity = LanguageProject_fileCapacity(self) == 0u ? 8u : LanguageProject_fileCapacity(self) * 2u;
        if (capacity < LanguageProject_fileCapacity(self) || capacity > SIZE_MAX / sizeof(ProjectFile*)) {
            THROW("LanguageProject: file table capacity overflow");
            return nullptr;
        }
        ProjectFile **grown = realloc(LanguageProject_files(self), capacity * sizeof(ProjectFile*));
        if (grown == nullptr) {
            THROW("LanguageProject: allocation failed for %zu file records", capacity);
            return nullptr;
        }
        LanguageProject_setFiles(self, grown);
        LanguageProject_setFileCapacity(self, capacity);
    }
    u64 fileId = (u64) LanguageProject_fileCount(self) + 1u;
    ProjectFile *file = ProjectFile_4(fileId, relativePath, kind, languageId);
    if (file == nullptr)
        return nullptr;
    LanguageProject_files(self)[LanguageProject_fileCount(self)] = file;
    LanguageProject_setFileCount(self, LanguageProject_fileCount(self) + 1u);
    return file;
}

const ProjectFile *LanguageProject_getFileAt(const LanguageProject *self, bytes index) {
    if (self == nullptr || index >= LanguageProject_fileCount(self))
        return nullptr;
    return LanguageProject_files(self)[index];
}

BuildTarget *LanguageProject_findTarget(const LanguageProject *self, const char *name) {
    if (self == nullptr || name == nullptr)
        return nullptr;
    for (bytes index = 0u; index < LanguageProject_targetCount(self); ++index)
        if (strcmp(BuildTarget_getName(LanguageProject_targets(self)[index]), name) == 0)
            return LanguageProject_targets(self)[index];
    return nullptr;
}
BuildTarget *LanguageProject_addTarget(LanguageProject *self, const char *name,
                                       const char *languageId) {
    if (self == nullptr || name == nullptr || LanguageProject_findTarget(self, name) != nullptr) {
        THROW("LanguageProject: rejected invalid or duplicate build target");
        return nullptr;
    }
    if (LanguageProject_targetCount(self) == SIZE_MAX) {
        THROW("LanguageProject: target count bound reached, request 1 target");
        return nullptr;
    }
    if (LanguageProject_targetCount(self) == LanguageProject_targetCapacity(self)) {
        bytes capacity = LanguageProject_targetCapacity(self) == 0u ? 4u : LanguageProject_targetCapacity(self) * 2u;
        if (capacity < LanguageProject_targetCapacity(self) || capacity > SIZE_MAX / sizeof(BuildTarget*)) {
            THROW("LanguageProject: target table capacity overflow");
            return nullptr;
        }
        BuildTarget **grown = realloc(LanguageProject_targets(self), capacity * sizeof(BuildTarget*));
        if (grown == nullptr) {
            THROW("LanguageProject: allocation failed for %zu build targets", capacity);
            return nullptr;
        }
        LanguageProject_setTargets(self, grown);
        LanguageProject_setTargetCapacity(self, capacity);
    }
    BuildTarget *target = BuildTarget_3((u64) LanguageProject_targetCount(self) + 1u,
                                       name, languageId);
    if (target == nullptr)
        return nullptr;
    LanguageProject_targets(self)[LanguageProject_targetCount(self)] = target;
    LanguageProject_setTargetCount(self, LanguageProject_targetCount(self) + 1u);
    return target;
}
bool LanguageProject_addSourceToTarget(LanguageProject *self, u64 targetId,
                                       u64 fileId) {
    if (self == nullptr) {
        THROW("LanguageProject: rejected source membership without project");
        return false;
    }
    BuildTarget *target = nullptr;
    for (bytes index = 0u; index < LanguageProject_targetCount(self); ++index)
        if (BuildTarget_getId(LanguageProject_targets(self)[index]) == targetId)
            target = LanguageProject_targets(self)[index];
    const ProjectFile *file = nullptr;
    for (bytes index = 0u; index < LanguageProject_fileCount(self); ++index)
        if (ProjectFile_getId(LanguageProject_files(self)[index]) == fileId)
            file = LanguageProject_files(self)[index];
    if (target == nullptr || file == nullptr
        || ProjectFile_getKind(file) != PROJECT_FILE_KIND_TEXT
        || (ProjectFile_getLanguageId(file) != nullptr
            && strcmp(ProjectFile_getLanguageId(file), BuildTarget_getLanguageId(target)) != 0)) {
        THROW("LanguageProject: rejected source membership for target/file identity or language");
        return false;
    }
    return BuildTarget_addSourceFile(target, fileId);
}
bool LanguageProject_addTargetDependency(LanguageProject *self, u64 targetId,
                                         u64 dependencyId) {
    if (self == nullptr) {
        THROW("LanguageProject: rejected target dependency without project");
        return false;
    }
    BuildTarget *target = nullptr;
    bool dependencyExists = false;
    for (bytes index = 0u; index < LanguageProject_targetCount(self); ++index) {
        BuildTarget *candidate = LanguageProject_targets(self)[index];
        if (BuildTarget_getId(candidate) == targetId)
            target = candidate;
        if (BuildTarget_getId(candidate) == dependencyId)
            dependencyExists = true;
    }
    if (target == nullptr || !dependencyExists) {
        THROW("LanguageProject: rejected unknown target dependency");
        return false;
    }
    return BuildTarget_addDependency(target, dependencyId);
}
BuildTarget *LanguageProject_getTargetAt(const LanguageProject *self, bytes index) {
    return self == nullptr || index >= LanguageProject_targetCount(self) ? nullptr : LanguageProject_targets(self)[index];
}
;;GETTER
ProjectConfiguration *LanguageProject_getConfiguration(LanguageProject *self) {
    return self == nullptr ? nullptr : LanguageProject_configuration(self);
}
;;GETTER
u64 LanguageProject_getId(const LanguageProject *self) {
    return self == nullptr ? 0u : LanguageProject_id(self);
}
;;GETTER
const char *LanguageProject_getRootPath(const LanguageProject *self) {
    return self == nullptr ? nullptr : LanguageProject_rootPath(self);
}
;;GETTER
bytes LanguageProject_getFileCount(const LanguageProject *self) {
    return self == nullptr ? 0u : LanguageProject_fileCount(self);
}
;;GETTER
bytes LanguageProject_getTargetCount(const LanguageProject *self) {
    return self == nullptr ? 0u : LanguageProject_targetCount(self);
}
