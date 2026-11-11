#include "project/language_project.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
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
    (*self).rootPath = copyRoot(rootPath);
    if ((*self).rootPath == nullptr) {
        THROW("LanguageProject: allocation failed while copying root path");
        free(self);
        return nullptr;
    }
    (*self).configuration = ProjectConfiguration_0();
    if ((*self).configuration == nullptr) {
        THROW("LanguageProject: unable to create project configuration");
        free((*self).rootPath);
        free(self);
        return nullptr;
    }
    (*self).id = projectId;
    return self;
}

void LanguageProject_free(LanguageProject *self) {
    if (self == nullptr)
        return;
    for (bytes index = 0u; index < (*self).fileCount; ++index)
        ProjectFile_free((*self).files[index]);
    for (bytes index = 0u; index < (*self).targetCount; ++index)
        BuildTarget_free((*self).targets[index]);
    ProjectConfiguration_free((*self).configuration);
    (*self).configuration = nullptr;
    free((*self).targets);
    (*self).targets = nullptr;
    free((*self).files);
    (*self).files = nullptr;
    free((*self).rootPath);
    (*self).rootPath = nullptr;
    free(self);
}

const ProjectFile *LanguageProject_findFile(const LanguageProject *self,
                                             const char *relativePath) {
    if (self == nullptr || relativePath == nullptr)
        return nullptr;
    for (bytes index = 0u; index < (*self).fileCount; ++index)
        if (strcmp(ProjectFile_getRelativePath((*self).files[index]), relativePath) == 0)
            return (*self).files[index];
    return nullptr;
}
const ProjectFile *LanguageProject_findFileById(const LanguageProject *self, u64 fileId) {
    if (self == nullptr || fileId == 0u)
        return nullptr;
    for (bytes index = 0u; index < (*self).fileCount; ++index)
        if (ProjectFile_getId((*self).files[index]) == fileId)
            return (*self).files[index];
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
    if ((*self).fileCount == SIZE_MAX) {
        THROW("LanguageProject: file count bound reached, request 1 file");
        return nullptr;
    }
    if ((*self).fileCount == (*self).fileCapacity) {
        bytes capacity = (*self).fileCapacity == 0u ? 8u : (*self).fileCapacity * 2u;
        if (capacity < (*self).fileCapacity || capacity > SIZE_MAX / sizeof(ProjectFile*)) {
            THROW("LanguageProject: file table capacity overflow");
            return nullptr;
        }
        ProjectFile **grown = realloc((*self).files, capacity * sizeof(ProjectFile*));
        if (grown == nullptr) {
            THROW("LanguageProject: allocation failed for %zu file records", capacity);
            return nullptr;
        }
        (*self).files = grown;
        (*self).fileCapacity = capacity;
    }
    u64 fileId = (u64) (*self).fileCount + 1u;
    ProjectFile *file = ProjectFile_4(fileId, relativePath, kind, languageId);
    if (file == nullptr)
        return nullptr;
    (*self).files[(*self).fileCount] = file;
    ++(*self).fileCount;
    return file;
}

const ProjectFile *LanguageProject_getFileAt(const LanguageProject *self, bytes index) {
    if (self == nullptr || index >= (*self).fileCount)
        return nullptr;
    return (*self).files[index];
}

BuildTarget *LanguageProject_findTarget(const LanguageProject *self, const char *name) {
    if (self == nullptr || name == nullptr)
        return nullptr;
    for (bytes index = 0u; index < (*self).targetCount; ++index)
        if (strcmp(BuildTarget_getName((*self).targets[index]), name) == 0)
            return (*self).targets[index];
    return nullptr;
}
BuildTarget *LanguageProject_addTarget(LanguageProject *self, const char *name,
                                       const char *languageId) {
    if (self == nullptr || name == nullptr || LanguageProject_findTarget(self, name) != nullptr) {
        THROW("LanguageProject: rejected invalid or duplicate build target");
        return nullptr;
    }
    if ((*self).targetCount == SIZE_MAX) {
        THROW("LanguageProject: target count bound reached, request 1 target");
        return nullptr;
    }
    if ((*self).targetCount == (*self).targetCapacity) {
        bytes capacity = (*self).targetCapacity == 0u ? 4u : (*self).targetCapacity * 2u;
        if (capacity < (*self).targetCapacity || capacity > SIZE_MAX / sizeof(BuildTarget*)) {
            THROW("LanguageProject: target table capacity overflow");
            return nullptr;
        }
        BuildTarget **grown = realloc((*self).targets, capacity * sizeof(BuildTarget*));
        if (grown == nullptr) {
            THROW("LanguageProject: allocation failed for %zu build targets", capacity);
            return nullptr;
        }
        (*self).targets = grown;
        (*self).targetCapacity = capacity;
    }
    BuildTarget *target = BuildTarget_3((u64) (*self).targetCount + 1u,
                                       name, languageId);
    if (target == nullptr)
        return nullptr;
    (*self).targets[(*self).targetCount] = target;
    ++(*self).targetCount;
    return target;
}
bool LanguageProject_addSourceToTarget(LanguageProject *self, u64 targetId,
                                       u64 fileId) {
    if (self == nullptr) {
        THROW("LanguageProject: rejected source membership without project");
        return false;
    }
    BuildTarget *target = nullptr;
    for (bytes index = 0u; index < (*self).targetCount; ++index)
        if (BuildTarget_getId((*self).targets[index]) == targetId)
            target = (*self).targets[index];
    const ProjectFile *file = nullptr;
    for (bytes index = 0u; index < (*self).fileCount; ++index)
        if (ProjectFile_getId((*self).files[index]) == fileId)
            file = (*self).files[index];
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
    for (bytes index = 0u; index < (*self).targetCount; ++index) {
        BuildTarget *candidate = (*self).targets[index];
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
    return self == nullptr || index >= (*self).targetCount ? nullptr : (*self).targets[index];
}
ProjectConfiguration *LanguageProject_getConfiguration(LanguageProject *self) {
    return self == nullptr ? nullptr : (*self).configuration;
}
u64 LanguageProject_getId(const LanguageProject *self) {
    return self == nullptr ? 0u : (*self).id;
}
const char *LanguageProject_getRootPath(const LanguageProject *self) {
    return self == nullptr ? nullptr : (*self).rootPath;
}
bytes LanguageProject_getFileCount(const LanguageProject *self) {
    return self == nullptr ? 0u : (*self).fileCount;
}
bytes LanguageProject_getTargetCount(const LanguageProject *self) {
    return self == nullptr ? 0u : (*self).targetCount;
}
