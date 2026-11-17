#include "vex.h"
#include "lang/language_registry.h"
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
    bytes count;
    bytes capacity;
    const LanguageAdapter **adapters;
    u64 exhaustionCount;
    bool exhaustionReported;
};

;;GETTER
static inline bytes LanguageRegistry_count(const LanguageRegistry *self) { return (*self).count; }
;;SETTER
static inline fn LanguageRegistry_setCount(LanguageRegistry *self, bytes v) { (*self).count = v; }
;;GETTER
static inline bytes LanguageRegistry_capacity(const LanguageRegistry *self) { return (*self).capacity; }
;;SETTER
static inline fn LanguageRegistry_setCapacity(LanguageRegistry *self, bytes v) { (*self).capacity = v; }
;;GETTER
static inline const LanguageAdapter **LanguageRegistry_adapters(const LanguageRegistry *self) { return (*self).adapters; }
;;SETTER
static inline fn LanguageRegistry_setAdapters(LanguageRegistry *self, const LanguageAdapter **v) { (*self).adapters = v; }
;;GETTER
static inline u64 LanguageRegistry_exhaustionCount(const LanguageRegistry *self) { return (*self).exhaustionCount; }
;;SETTER
static inline fn LanguageRegistry_setExhaustionCount(LanguageRegistry *self, u64 v) { (*self).exhaustionCount = v; }
;;GETTER
static inline bool LanguageRegistry_exhaustionReported(const LanguageRegistry *self) { return (*self).exhaustionReported; }
;;SETTER
static inline fn LanguageRegistry_setExhaustionReported(LanguageRegistry *self, bool v) { (*self).exhaustionReported = v; }

LanguageRegistry *LanguageRegistry_0() {
    LanguageRegistry *self = calloc(1u, sizeof *self);
    if (self == nullptr)
        THROW("LanguageRegistry: allocation failed for registry object");
    return self;
}
LanguageRegistry *LanguageRegistry_zero() { return LanguageRegistry_0(); }
fn LanguageRegistry_free(LanguageRegistry *self) {
    if (self == nullptr)
        return;
    free(LanguageRegistry_adapters(self));
    LanguageRegistry_setAdapters(self, nullptr);
    free(self);
}

bool LanguageRegistry_register(LanguageRegistry *self, const LanguageAdapter *adapter) {
    if (self == nullptr || !LanguageAdapter_isValid(adapter)) {
        THROW("LanguageRegistry: rejected invalid adapter");
        return false;
    }
    const char *id = LanguageAdapter_getLanguageId(adapter);
    for (bytes index = 0u; index < LanguageRegistry_count(self); ++index) {
        if (strcmp(LanguageAdapter_getLanguageId(LanguageRegistry_adapters(self)[index]), id) == 0) {
            THROW("LanguageRegistry: duplicate language id");
            return false;
        }
    }
    if (LanguageRegistry_count(self) == LanguageRegistry_capacity(self)) {
        bytes capacity = LanguageRegistry_capacity(self) == 0u ? 4u : LanguageRegistry_capacity(self) * 2u;
        if (capacity < LanguageRegistry_capacity(self) || capacity > SIZE_MAX / sizeof(LanguageAdapter*)) {
            if (LanguageRegistry_exhaustionCount(self) != UINT64_MAX)
                LanguageRegistry_setExhaustionCount(self, LanguageRegistry_exhaustionCount(self) + 1u);
            if (!LanguageRegistry_exhaustionReported(self)) {
                THROW("LanguageRegistry: capacity overflow, request %zu adapters",
                      LanguageRegistry_count(self) + 1u);
                LanguageRegistry_setExhaustionReported(self, true);
            }
            return false;
        }
        const LanguageAdapter **grown = realloc(LanguageRegistry_adapters(self),
                                                 capacity * sizeof(LanguageAdapter*));
        if (grown == nullptr) {
            if (LanguageRegistry_exhaustionCount(self) != UINT64_MAX)
                LanguageRegistry_setExhaustionCount(self, LanguageRegistry_exhaustionCount(self) + 1u);
            if (!LanguageRegistry_exhaustionReported(self)) {
                THROW("LanguageRegistry: allocation refused, request %zu adapters (capacity %zu)",
                      LanguageRegistry_count(self) + 1u, LanguageRegistry_capacity(self));
                LanguageRegistry_setExhaustionReported(self, true);
            }
            return false;
        }
        LanguageRegistry_setAdapters(self, grown);
        LanguageRegistry_setCapacity(self, capacity);
    }
    LanguageRegistry_adapters(self)[LanguageRegistry_count(self)] = adapter;
    LanguageRegistry_setCount(self, LanguageRegistry_count(self) + 1u);
    return true;
}

const LanguageAdapter *LanguageRegistry_find(const LanguageRegistry *self,
                                             const char *languageId) {
    if (self == nullptr || languageId == nullptr)
        return nullptr;
    for (bytes index = 0u; index < LanguageRegistry_count(self); ++index)
        if (strcmp(LanguageAdapter_getLanguageId(LanguageRegistry_adapters(self)[index]), languageId) == 0)
            return LanguageRegistry_adapters(self)[index];
    return nullptr;
}
;;GETTER
bytes LanguageRegistry_getCount(const LanguageRegistry *self) {
    return self == nullptr ? 0u : LanguageRegistry_count(self);
}
;;GETTER
u64 LanguageRegistry_getExhaustionCount(const LanguageRegistry *self) {
    return self == nullptr ? 0u : LanguageRegistry_exhaustionCount(self);
}
