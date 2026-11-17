#include "vex.h"
#include "project/build_target.h"
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

;;GETTER
static inline u64 BuildTarget_id(const BuildTarget *self) { return (*self).id; }
;;SETTER
static inline fn BuildTarget_setId(BuildTarget *self, u64 v) { (*self).id = v; }
;;GETTER
static inline char *BuildTarget_name(const BuildTarget *self) { return (*self).name; }
;;SETTER
static inline fn BuildTarget_setName(BuildTarget *self, char *v) { (*self).name = v; }
;;GETTER
static inline char *BuildTarget_languageId(const BuildTarget *self) { return (*self).languageId; }
;;SETTER
static inline fn BuildTarget_setLanguageId(BuildTarget *self, char *v) { (*self).languageId = v; }
;;GETTER
static inline u64 *BuildTarget_sourceFileIds(const BuildTarget *self) { return (*self).sourceFileIds; }
;;SETTER
static inline fn BuildTarget_setSourceFileIds(BuildTarget *self, u64 *v) { (*self).sourceFileIds = v; }
;;GETTER
static inline bytes BuildTarget_sourceCount(const BuildTarget *self) { return (*self).sourceCount; }
;;SETTER
static inline fn BuildTarget_setSourceCount(BuildTarget *self, bytes v) { (*self).sourceCount = v; }
;;GETTER
static inline bytes BuildTarget_sourceCapacity(const BuildTarget *self) { return (*self).sourceCapacity; }
;;SETTER
static inline fn BuildTarget_setSourceCapacity(BuildTarget *self, bytes v) { (*self).sourceCapacity = v; }
;;GETTER
static inline u64 *BuildTarget_dependencyIds(const BuildTarget *self) { return (*self).dependencyIds; }
;;SETTER
static inline fn BuildTarget_setDependencyIds(BuildTarget *self, u64 *v) { (*self).dependencyIds = v; }
;;GETTER
static inline bytes BuildTarget_dependencyCount(const BuildTarget *self) { return (*self).dependencyCount; }
;;SETTER
static inline fn BuildTarget_setDependencyCount(BuildTarget *self, bytes v) { (*self).dependencyCount = v; }
;;GETTER
static inline bytes BuildTarget_dependencyCapacity(const BuildTarget *self) { return (*self).dependencyCapacity; }
;;SETTER
static inline fn BuildTarget_setDependencyCapacity(BuildTarget *self, bytes v) { (*self).dependencyCapacity = v; }
;;GETTER
static inline char **BuildTarget_compilerOptions(const BuildTarget *self) { return (*self).compilerOptions; }
;;SETTER
static inline fn BuildTarget_setCompilerOptions(BuildTarget *self, char **v) { (*self).compilerOptions = v; }
;;GETTER
static inline bytes BuildTarget_compilerOptionCount(const BuildTarget *self) { return (*self).compilerOptionCount; }
;;SETTER
static inline fn BuildTarget_setCompilerOptionCount(BuildTarget *self, bytes v) { (*self).compilerOptionCount = v; }
;;GETTER
static inline bytes BuildTarget_compilerOptionCapacity(const BuildTarget *self) { return (*self).compilerOptionCapacity; }
;;SETTER
static inline fn BuildTarget_setCompilerOptionCapacity(BuildTarget *self, bytes v) { (*self).compilerOptionCapacity = v; }

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
    BuildTarget_setName(self, copyText(name));
    BuildTarget_setLanguageId(self, copyText(languageId));
    if (BuildTarget_name(self) == nullptr || BuildTarget_languageId(self) == nullptr) {
        THROW("BuildTarget: allocation failed while copying metadata");
        free(BuildTarget_name(self));
        free(BuildTarget_languageId(self));
        free(self);
        return nullptr;
    }
    BuildTarget_setId(self, targetId);
    return self;
}

fn BuildTarget_free(BuildTarget *self) {
    if (self == nullptr)
        return;
    for (bytes index = 0u; index < BuildTarget_compilerOptionCount(self); ++index)
        free(BuildTarget_compilerOptions(self)[index]);
    free(BuildTarget_compilerOptions(self));
    BuildTarget_setCompilerOptions(self, nullptr);
    free(BuildTarget_sourceFileIds(self));
    BuildTarget_setSourceFileIds(self, nullptr);
    free(BuildTarget_dependencyIds(self));
    BuildTarget_setDependencyIds(self, nullptr);
    free(BuildTarget_name(self));
    BuildTarget_setName(self, nullptr);
    free(BuildTarget_languageId(self));
    BuildTarget_setLanguageId(self, nullptr);
    free(self);
}

bool BuildTarget_addSourceFile(BuildTarget *self, u64 fileId) {
    if (self == nullptr) {
        THROW("BuildTarget: rejected null source owner");
        return false;
    }
    u64 *ids = BuildTarget_sourceFileIds(self);
    bytes count = BuildTarget_sourceCount(self);
    bytes cap = BuildTarget_sourceCapacity(self);
    bool ok = appendId(&ids, &count, &cap, fileId, "source file");
    BuildTarget_setSourceFileIds(self, ids);
    BuildTarget_setSourceCount(self, count);
    BuildTarget_setSourceCapacity(self, cap);
    return ok;
}
bool BuildTarget_addDependency(BuildTarget *self, u64 targetId) {
    if (self == nullptr) {
        THROW("BuildTarget: rejected null dependency owner");
        return false;
    }
    u64 *ids = BuildTarget_dependencyIds(self);
    bytes count = BuildTarget_dependencyCount(self);
    bytes cap = BuildTarget_dependencyCapacity(self);
    bool ok = appendId(&ids, &count, &cap, targetId, "dependency target");
    BuildTarget_setDependencyIds(self, ids);
    BuildTarget_setDependencyCount(self, count);
    BuildTarget_setDependencyCapacity(self, cap);
    return ok;
}
bool BuildTarget_addCompilerOption(BuildTarget *self, const char *option) {
    if (self == nullptr || option == nullptr || option[0] == '\0') {
        THROW("BuildTarget: rejected null or empty compiler option");
        return false;
    }
    bytes capacity = BuildTarget_compilerOptionCapacity(self);
    if (BuildTarget_compilerOptionCount(self) == capacity) {
        bytes next = capacity == 0u ? 8u : capacity * 2u;
        if (next < capacity || next > SIZE_MAX / sizeof(char*)) {
            THROW("BuildTarget: compiler-option capacity overflow");
            return false;
        }
        char **grown = realloc(BuildTarget_compilerOptions(self), next * sizeof(char*));
        if (grown == nullptr) {
            THROW("BuildTarget: allocation failed for %zu compiler options", next);
            return false;
        }
        BuildTarget_setCompilerOptions(self, grown);
        BuildTarget_setCompilerOptionCapacity(self, next);
    }
    char *copy = copyText(option);
    if (copy == nullptr) {
        THROW("BuildTarget: allocation failed copying compiler option");
        return false;
    }
    BuildTarget_compilerOptions(self)[BuildTarget_compilerOptionCount(self)] = copy;
    BuildTarget_setCompilerOptionCount(self, BuildTarget_compilerOptionCount(self) + 1u);
    return true;
}
;;GETTER
u64 BuildTarget_getId(const BuildTarget *self) { return self == nullptr ? 0u : BuildTarget_id(self); }
;;GETTER
const char *BuildTarget_getName(const BuildTarget *self) { return self == nullptr ? nullptr : BuildTarget_name(self); }
;;GETTER
const char *BuildTarget_getLanguageId(const BuildTarget *self) {
    return self == nullptr ? nullptr : BuildTarget_languageId(self);
}
;;GETTER
bytes BuildTarget_getSourceCount(const BuildTarget *self) {
    return self == nullptr ? 0u : BuildTarget_sourceCount(self);
}
u64 BuildTarget_getSourceAt(const BuildTarget *self, bytes index) {
    return self == nullptr || index >= BuildTarget_sourceCount(self) ? 0u : BuildTarget_sourceFileIds(self)[index];
}
;;GETTER
bytes BuildTarget_getDependencyCount(const BuildTarget *self) {
    return self == nullptr ? 0u : BuildTarget_dependencyCount(self);
}
u64 BuildTarget_getDependencyAt(const BuildTarget *self, bytes index) {
    return self == nullptr || index >= BuildTarget_dependencyCount(self) ? 0u : BuildTarget_dependencyIds(self)[index];
}
;;GETTER
bytes BuildTarget_getCompilerOptionCount(const BuildTarget *self) {
    return self == nullptr ? 0u : BuildTarget_compilerOptionCount(self);
}
const char *BuildTarget_getCompilerOptionAt(const BuildTarget *self, bytes index) {
    return self == nullptr || index >= BuildTarget_compilerOptionCount(self)
        ? nullptr : BuildTarget_compilerOptions(self)[index];
}
