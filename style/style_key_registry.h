#ifndef LANGUAGE_STYLE_STYLE_KEY_REGISTRY_H
#define LANGUAGE_STYLE_STYLE_KEY_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>

#include "text/bounded_text.h"

// style/style_key_registry.h — ONE name-addressed namespace for settings.
//
// Classification is layered: the shared vocabulary owns core keys ("type",
// "operator.deref", "comment.doc"), and each language adapter registers its own
// prefixed keys ("c.pointer", "rust.lifetime") with a core fallback. Settings,
// themes and persistence see the union as one flat list addressed by name.
//
// The registry owns no colour and no storage. Core keys are static; adapter
// rows are borrowed from the caller. Resolution is one declared hop from an
// adapter key to a core key, then the core chain to "text"; the walk is bounded
// so a malformed registry cannot loop.

// One borrowed registry row. Behaviourless slot record owned by
// StyleKeyRegistry: `fallback` must name a core key.
typedef struct StyleKeyEntry {
    const char *name;      // stable key spelling, e.g. "c.pointer"
    const char *fallback;  // core key, e.g. "operator.pointer"
} StyleKeyEntry;

typedef enum StyleKeyIssue {
    STYLE_KEY_ISSUE_NONE = 0,
    STYLE_KEY_ISSUE_ENTRY_NULL,
    STYLE_KEY_ISSUE_NAME_EMPTY,
    STYLE_KEY_ISSUE_PREFIX_MISSING,     // adapter keys begin "<adapter>."
    STYLE_KEY_ISSUE_PREFIX_INVALID,     // prefix is not lowercase letters/digits/'-'
    STYLE_KEY_ISSUE_DUPLICATE,
    STYLE_KEY_ISSUE_COLLIDES_CORE,
    STYLE_KEY_ISSUE_FALLBACK_EMPTY,
    STYLE_KEY_ISSUE_FALLBACK_NOT_CORE
} StyleKeyIssue;

typedef struct StyleKeyRegistry {
    const StyleKeyEntry *adapterEntries; // borrowed adapter rows, nullable
    bytes adapterCount;                 // rows in adapterEntries
} StyleKeyRegistry;

// Construct the core-only registry or one with borrowed adapter rows.
StyleKeyRegistry StyleKeyRegistry_0(void);
StyleKeyRegistry StyleKeyRegistry_2(const StyleKeyEntry *entries, bytes count);
#define StyleKeyRegistry(...) STYLE_KEY_REGISTRY_SELECT(_ __VA_OPT__(,) __VA_ARGS__, StyleKeyRegistry_2, StyleKeyRegistry_invalid_arity, StyleKeyRegistry_0)(__VA_ARGS__)
#define STYLE_KEY_REGISTRY_SELECT(_0, _1, _2, NAME, ...) NAME
// Return the core-only registry.
StyleKeyRegistry StyleKeyRegistry_zero(void);

// Number of core keys the shared vocabulary defines.
bytes StyleKeyRegistry_coreCount(void);
// Core keys first, then adapter rows in registration order.
bytes StyleKeyRegistry_count(const StyleKeyRegistry *self);
const char *StyleKeyRegistry_name(const StyleKeyRegistry *self, bytes index);

// The declared fallback of one key (one hop), or nullptr when unknown.
const char *StyleKeyRegistry_fallback(const StyleKeyRegistry *self, const char *name);
// Walk the declared chain to its terminal key, bounded. Unknown or cyclic
// input returns nullptr rather than guessing.
const char *StyleKeyRegistry_resolve(const StyleKeyRegistry *self, const char *name);

bool StyleKeyRegistry_isCore(const StyleKeyRegistry *self, const char *name);
bool StyleKeyRegistry_isAdapter(const StyleKeyRegistry *self, const char *name);

// First structural defect in the adapter rows, if any.
StyleKeyIssue StyleKeyRegistry_check(const StyleKeyRegistry *self);
bool StyleKeyRegistry_isValid(const StyleKeyRegistry *self);

BoundedText StyleKeyRegistry_toString(const StyleKeyRegistry *self);
BoundedText StyleKeyRegistry_toStringStruct(const StyleKeyRegistry *self);

#endif
