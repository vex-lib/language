#include "lang/language_registry.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/** LanguageRegistry maps explicit language ids to borrowed adapter tables.
 * Its pointer table grows on demand; an allocation refusal preserves every
 * previously registered adapter and is counted for the registry lifetime. */

;;OVERVIEW
/** CLASS: LanguageRegistry (lang/language_registry.c)
 * STRUCT FIELDS: count, capacity, owned pointer table of borrowed adapters,
 * allocation-refusal count and once-per-lifetime report flag.
 * FUNCTION REGISTRY: constructor, free, register, find, getCount.
 * CONTRACT: adapters and their callback code outlive the registry; ids are
 * unique; the table grows geometrically; rejected registration preserves it. */

struct LanguageRegistry {
    size_t count;
    size_t capacity;
    const LanguageAdapter **adapters;
    uint64_t exhaustionCount;
    bool exhaustionReported;
};

LanguageRegistry *LanguageRegistry_0(void) {
    LanguageRegistry *self = calloc(1u, sizeof *self);
    if (self == nullptr)
        THROW("LanguageRegistry: allocation failed for registry object");
    return self;
}
LanguageRegistry *LanguageRegistry_zero(void) { return LanguageRegistry_0(); }
void LanguageRegistry_free(LanguageRegistry *self) {
    if (self == nullptr)
        return;
    free((*self).adapters);
    (*self).adapters = nullptr;
    free(self);
}

bool LanguageRegistry_register(LanguageRegistry *self, const LanguageAdapter *adapter) {
    if (self == nullptr || !LanguageAdapter_isValid(adapter)) {
        THROW("LanguageRegistry: rejected invalid adapter");
        return false;
    }
    const char *id = LanguageAdapter_getLanguageId(adapter);
    for (size_t index = 0u; index < (*self).count; ++index) {
        if (strcmp(LanguageAdapter_getLanguageId((*self).adapters[index]), id) == 0) {
            THROW("LanguageRegistry: duplicate language id");
            return false;
        }
    }
    if ((*self).count == (*self).capacity) {
        size_t capacity = (*self).capacity == 0u ? 4u : (*self).capacity * 2u;
        if (capacity < (*self).capacity || capacity > SIZE_MAX / sizeof(LanguageAdapter*)) {
            if ((*self).exhaustionCount != UINT64_MAX)
                ++(*self).exhaustionCount;
            if (!(*self).exhaustionReported) {
                THROW("LanguageRegistry: capacity overflow, request %zu adapters",
                      (*self).count + 1u);
                (*self).exhaustionReported = true;
            }
            return false;
        }
        const LanguageAdapter **grown = realloc((*self).adapters,
                                                 capacity * sizeof(LanguageAdapter*));
        if (grown == nullptr) {
            if ((*self).exhaustionCount != UINT64_MAX)
                ++(*self).exhaustionCount;
            if (!(*self).exhaustionReported) {
                THROW("LanguageRegistry: allocation refused, request %zu adapters (capacity %zu)",
                      (*self).count + 1u, (*self).capacity);
                (*self).exhaustionReported = true;
            }
            return false;
        }
        (*self).adapters = grown;
        (*self).capacity = capacity;
    }
    (*self).adapters[(*self).count] = adapter;
    ++(*self).count;
    return true;
}

const LanguageAdapter *LanguageRegistry_find(const LanguageRegistry *self,
                                             const char *languageId) {
    if (self == nullptr || languageId == nullptr)
        return nullptr;
    for (size_t index = 0u; index < (*self).count; ++index)
        if (strcmp(LanguageAdapter_getLanguageId((*self).adapters[index]), languageId) == 0)
            return (*self).adapters[index];
    return nullptr;
}
size_t LanguageRegistry_getCount(const LanguageRegistry *self) {
    return self == nullptr ? 0u : (*self).count;
}
uint64_t LanguageRegistry_getExhaustionCount(const LanguageRegistry *self) {
    return self == nullptr ? 0u : (*self).exhaustionCount;
}
