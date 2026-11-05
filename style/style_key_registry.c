#include "style/style_key_registry.h"
#include "annotation/annotation.h"
#include <string.h>

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: StyleKeyRegistry
 * ============================================================================
 * StyleKeyRegistry gives settings, themes and persistence ONE name-addressed
 * namespace. The shared vocabulary owns the core keys; an adapter registers
 * prefixed keys ("c.pointer") that name a core fallback, so a theme can address
 * the union without learning each adapter. The registry borrows adapter rows
 * and allocates nothing. Resolution is one declared hop from an adapter key to
 * a core key, then the core chain to "text"; the walk is bounded by the current
 * key count so malformed input cannot loop.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: StyleKeyRegistry (style/style_key_registry.c)
 * STRUCT FIELDS (Mirroring style/style_key_registry.h)
 *   const StyleKeyEntry *adapterEntries // borrowed rows, nullable
 *   size_t adapterCount                 // rows in adapterEntries
 *
 * SLOT RECORD (owned by StyleKeyRegistry)
 *   StyleKeyEntry
 *     const char *name      // stable prefixed spelling, e.g. "c.pointer"
 *     const char *fallback  // core key this adapter key resolves through
 *
 * PRIVATE DATA
 *   CORE_KEYS[]  // the shared key vocabulary, in display order
 *
 * PRIVATE HELPERS
 *   equal        // null-safe string equality
 *   validPrefix  // "<lowercase/digits/->." prefix before the first dot
 *
 * FUNCTION REGISTRY
 *   constructors: StyleKeyRegistry_0, StyleKeyRegistry_2,
 *                 StyleKeyRegistry(...), StyleKeyRegistry_zero
 *   counts:       StyleKeyRegistry_coreCount, StyleKeyRegistry_count
 *   lookup:       StyleKeyRegistry_name, StyleKeyRegistry_fallback,
 *                 StyleKeyRegistry_resolve
 *   predicates:   StyleKeyRegistry_isCore, StyleKeyRegistry_isAdapter,
 *                 StyleKeyRegistry_check, StyleKeyRegistry_isValid
 *   projections:  StyleKeyRegistry_toString, StyleKeyRegistry_toStringStruct
 *
 * CONTRACT
 *   Adapter rows and their names/fallbacks are borrowed and immutable for the
 *   registry's lifetime; construction is the only admission. A null self is a
 *   core-only view for count/name/isCore, and resolves nothing. Core keys fall
 *   back to "text" except "text" itself. check() reports the first structural
 *   defect, including duplicate rows, core collisions and non-core fallbacks.
 * ============================================================================
 */

static const char *CORE_KEYS[] = {
    "text", "keyword", "type", "type.parameter", "type.argument", "function", "method",
    "constructor", "parameter", "field", "variable", "constant", "namespace", "label",
    "macro", "macro.parameter", "macro.invocation", "annotation", "annotation.sigil",
    "annotation.argument", "attribute", "directive", "string", "string.escape",
    "string.interpolation", "string.regex", "character", "number", "boolean", "null",
    "operator", "operator.pointer", "operator.cast-pointer", "operator.deref",
    "operator.address", "member", "punctuation", "bracket", "comment", "comment.doc",
    "inactive", "invalid", "hint", "hint.deref", "hint.multiply", "hint.address",
    "hint.macro-expansion", "hint.claim", "hint.ambiguous", "diag", "diag.info",
    "diag.hint", "diag.warning", "diag.error"
};

static bool equal(const char *a, const char *b) {
    return a != nullptr && b != nullptr && strcmp(a, b) == 0;
}

static bool validPrefix(const char *name) {
    if (name == nullptr || name[0] == '\0')
        return false;
    bool dot = false;
    for (size_t i = 0u; name[i] != '\0'; ++i) {
        char c = name[i];
        if (c == '.') { dot = true; break; }
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-'))
            return false;
    }
    return dot;
}

StyleKeyRegistry StyleKeyRegistry_0(void) { return (StyleKeyRegistry) {0}; }
StyleKeyRegistry StyleKeyRegistry_zero(void) { return StyleKeyRegistry_0(); }
StyleKeyRegistry StyleKeyRegistry_2(const StyleKeyEntry *entries, size_t count) {
    return (StyleKeyRegistry) {.adapterEntries = entries, .adapterCount = count};
}

size_t StyleKeyRegistry_coreCount(void) { return sizeof CORE_KEYS / sizeof CORE_KEYS[0]; }
size_t StyleKeyRegistry_count(const StyleKeyRegistry *self) {
    return self == nullptr ? 0u : StyleKeyRegistry_coreCount() + (*self).adapterCount;
}
const char *StyleKeyRegistry_name(const StyleKeyRegistry *self, size_t index) {
    if (self == nullptr) return nullptr;
    if (index < StyleKeyRegistry_coreCount()) return CORE_KEYS[index];
    index -= StyleKeyRegistry_coreCount();
    return index < (*self).adapterCount && (*self).adapterEntries != nullptr
        ? (*self).adapterEntries[index].name : nullptr;
}

bool StyleKeyRegistry_isCore(const StyleKeyRegistry *self, const char *name) {
    (void) self;
    for (size_t i = 0u; i < StyleKeyRegistry_coreCount(); ++i)
        if (equal(CORE_KEYS[i], name)) return true;
    return false;
}
bool StyleKeyRegistry_isAdapter(const StyleKeyRegistry *self, const char *name) {
    if (self == nullptr || (*self).adapterEntries == nullptr) return false;
    for (size_t i = 0u; i < (*self).adapterCount; ++i)
        if (equal((*self).adapterEntries[i].name, name)) return true;
    return false;
}
const char *StyleKeyRegistry_fallback(const StyleKeyRegistry *self, const char *name) {
    if (self == nullptr || name == nullptr) return nullptr;
    if (StyleKeyRegistry_isCore(self, name)) return equal(name, "text") ? nullptr : "text";
    if ((*self).adapterEntries != nullptr)
        for (size_t i = 0u; i < (*self).adapterCount; ++i)
            if (equal((*self).adapterEntries[i].name, name)) return (*self).adapterEntries[i].fallback;
    return nullptr;
}
const char *StyleKeyRegistry_resolve(const StyleKeyRegistry *self, const char *name) {
    if (self == nullptr || name == nullptr) return nullptr;
    const char *current = name;
    for (size_t step = 0u; step <= StyleKeyRegistry_count(self); ++step) {
        if (StyleKeyRegistry_isCore(self, current)) return current;
        current = StyleKeyRegistry_fallback(self, current);
        if (current == nullptr) return nullptr;
    }
    return nullptr;
}

StyleKeyIssue StyleKeyRegistry_check(const StyleKeyRegistry *self) {
    if (self == nullptr) return STYLE_KEY_ISSUE_ENTRY_NULL;
    if ((*self).adapterCount > 0u && (*self).adapterEntries == nullptr) return STYLE_KEY_ISSUE_ENTRY_NULL;
    for (size_t i = 0u; i < (*self).adapterCount; ++i) {
        const StyleKeyEntry *entry = &(*self).adapterEntries[i];
        if ((*entry).name == nullptr || (*entry).name[0] == '\0') return STYLE_KEY_ISSUE_NAME_EMPTY;
        if (!validPrefix((*entry).name)) return STYLE_KEY_ISSUE_PREFIX_INVALID;
        if (StyleKeyRegistry_isCore(self, (*entry).name)) return STYLE_KEY_ISSUE_COLLIDES_CORE;
        if ((*entry).fallback == nullptr || (*entry).fallback[0] == '\0') return STYLE_KEY_ISSUE_FALLBACK_EMPTY;
        if (!StyleKeyRegistry_isCore(self, (*entry).fallback)) return STYLE_KEY_ISSUE_FALLBACK_NOT_CORE;
        for (size_t j = 0u; j < i; ++j)
            if (equal((*entry).name, (*self).adapterEntries[j].name)) return STYLE_KEY_ISSUE_DUPLICATE;
    }
    return STYLE_KEY_ISSUE_NONE;
}
bool StyleKeyRegistry_isValid(const StyleKeyRegistry *self) { return StyleKeyRegistry_check(self) == STYLE_KEY_ISSUE_NONE; }
BoundedText StyleKeyRegistry_toString(const StyleKeyRegistry *self) {
    return self == nullptr ? BoundedText_1("nullptr") : BoundedText_format("StyleKeyRegistry(count=%zu)", StyleKeyRegistry_count(self));
}
BoundedText StyleKeyRegistry_toStringStruct(const StyleKeyRegistry *self) {
    return self == nullptr ? BoundedText_1("nullptr") : BoundedText_format("StyleKeyRegistry { adapterCount=%zu }", (*self).adapterCount);
}
