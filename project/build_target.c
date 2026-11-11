#include "project/build_target.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/** BuildTarget groups file identities and inert compiler configuration under
 * an explicit language/dialect id. It describes a build context but never runs
 * a compiler, plugin, project script or command. */

;;OVERVIEW
/** CLASS: BuildTarget (project/build_target.c)
 * STRUCT FIELDS: target id, copied name/language id, source file ids, dependency
 * target ids, and copied compiler option strings.
 * FUNCTION REGISTRY: constructor/free, append methods, identity and indexed getters.
 * CONTRACT: ids are nonzero and unique within each vector; appends preserve
 * previous state on rejection; options are opaque data, never executed here. */

struct BuildTarget {
    u64 id;
    char *name;
    char *languageId;
    u64 *sourceFileIds;
    bytes sourceCount;
    bytes sourceCapacity;
    u64 *dependencyIds;
    bytes dependencyCount;
    bytes dependencyCapacity;
    char **compilerOptions;
    bytes compilerOptionCount;
    bytes compilerOptionCapacity;
};

static char *copyText(const char *text) {
    bytes length = strlen(text);
    char *copy = malloc(length + 1u);
    if (copy != nullptr)
        memcpy(copy, text, length + 1u);
    return copy;
}

static bool reserveIds(u64 **items, bytes *capacity, bytes count,
                       const char *owner) {
    if (count < *capacity)
        return true;
    bytes next = *capacity == 0u ? 8u : *capacity * 2u;
    if (next < *capacity || next > SIZE_MAX / sizeof(u64)) {
        THROW("BuildTarget: %s vector capacity overflow", owner);
        return false;
    }
    u64 *grown = realloc(*items, next * sizeof(u64));
    if (grown == nullptr) {
        THROW("BuildTarget: allocation failed for %s capacity %zu", owner, next);
        return false;
    }
    *items = grown;
    *capacity = next;
    return true;
}

static bool appendId(u64 **items, bytes *count, bytes *capacity,
                     u64 id, const char *owner) {
    if (id == 0u) {
        THROW("BuildTarget: rejected zero %s identity", owner);
        return false;
    }
    for (bytes index = 0u; index < *count; ++index)
        if ((*items)[index] == id) {
            THROW("BuildTarget: duplicate %s identity", owner);
            return false;
        }
    if (!reserveIds(items, capacity, *count, owner))
        return false;
    (*items)[*count] = id;
    ++*count;
    return true;
}

BuildTarget *BuildTarget_3(u64 targetId, const char *name,
                           const char *languageId) {
    if (targetId == 0u || name == nullptr || name[0] == '\0'
        || languageId == nullptr || languageId[0] == '\0') {
        THROW("BuildTarget: rejected invalid identity, name or language id");
        return nullptr;
    }
    BuildTarget *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("BuildTarget: allocation failed");
        return nullptr;
    }
    (*self).name = copyText(name);
    (*self).languageId = copyText(languageId);
    if ((*self).name == nullptr || (*self).languageId == nullptr) {
        THROW("BuildTarget: allocation failed while copying metadata");
        free((*self).name);
        free((*self).languageId);
        free(self);
        return nullptr;
    }
    (*self).id = targetId;
    return self;
}

void BuildTarget_free(BuildTarget *self) {
    if (self == nullptr)
        return;
    for (bytes index = 0u; index < (*self).compilerOptionCount; ++index)
        free((*self).compilerOptions[index]);
    free((*self).compilerOptions);
    (*self).compilerOptions = nullptr;
    free((*self).sourceFileIds);
    (*self).sourceFileIds = nullptr;
    free((*self).dependencyIds);
    (*self).dependencyIds = nullptr;
    free((*self).name);
    (*self).name = nullptr;
    free((*self).languageId);
    (*self).languageId = nullptr;
    free(self);
}

bool BuildTarget_addSourceFile(BuildTarget *self, u64 fileId) {
    if (self == nullptr) {
        THROW("BuildTarget: rejected null source owner");
        return false;
    }
    return appendId(&(*self).sourceFileIds, &(*self).sourceCount,
                    &(*self).sourceCapacity, fileId, "source file");
}
bool BuildTarget_addDependency(BuildTarget *self, u64 targetId) {
    if (self == nullptr) {
        THROW("BuildTarget: rejected null dependency owner");
        return false;
    }
    return appendId(&(*self).dependencyIds, &(*self).dependencyCount,
                    &(*self).dependencyCapacity, targetId, "dependency target");
}
bool BuildTarget_addCompilerOption(BuildTarget *self, const char *option) {
    if (self == nullptr || option == nullptr || option[0] == '\0') {
        THROW("BuildTarget: rejected null or empty compiler option");
        return false;
    }
    bytes capacity = (*self).compilerOptionCapacity;
    if ((*self).compilerOptionCount == capacity) {
        bytes next = capacity == 0u ? 8u : capacity * 2u;
        if (next < capacity || next > SIZE_MAX / sizeof(char*)) {
            THROW("BuildTarget: compiler-option capacity overflow");
            return false;
        }
        char **grown = realloc((*self).compilerOptions, next * sizeof(char*));
        if (grown == nullptr) {
            THROW("BuildTarget: allocation failed for %zu compiler options", next);
            return false;
        }
        (*self).compilerOptions = grown;
        (*self).compilerOptionCapacity = next;
    }
    char *copy = copyText(option);
    if (copy == nullptr) {
        THROW("BuildTarget: allocation failed copying compiler option");
        return false;
    }
    (*self).compilerOptions[(*self).compilerOptionCount] = copy;
    ++(*self).compilerOptionCount;
    return true;
}
u64 BuildTarget_getId(const BuildTarget *self) { return self == nullptr ? 0u : (*self).id; }
const char *BuildTarget_getName(const BuildTarget *self) { return self == nullptr ? nullptr : (*self).name; }
const char *BuildTarget_getLanguageId(const BuildTarget *self) {
    return self == nullptr ? nullptr : (*self).languageId;
}
bytes BuildTarget_getSourceCount(const BuildTarget *self) {
    return self == nullptr ? 0u : (*self).sourceCount;
}
u64 BuildTarget_getSourceAt(const BuildTarget *self, bytes index) {
    return self == nullptr || index >= (*self).sourceCount ? 0u : (*self).sourceFileIds[index];
}
bytes BuildTarget_getDependencyCount(const BuildTarget *self) {
    return self == nullptr ? 0u : (*self).dependencyCount;
}
u64 BuildTarget_getDependencyAt(const BuildTarget *self, bytes index) {
    return self == nullptr || index >= (*self).dependencyCount ? 0u : (*self).dependencyIds[index];
}
bytes BuildTarget_getCompilerOptionCount(const BuildTarget *self) {
    return self == nullptr ? 0u : (*self).compilerOptionCount;
}
const char *BuildTarget_getCompilerOptionAt(const BuildTarget *self, bytes index) {
    return self == nullptr || index >= (*self).compilerOptionCount
        ? nullptr : (*self).compilerOptions[index];
}
