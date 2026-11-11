#ifndef LANGUAGE_SEMANTIC_REFERENCE_H
#define LANGUAGE_SEMANTIC_REFERENCE_H

#include "document/source_range.h"
#include <stdbool.h>
#include <stdint.h>

// Broad use-site roles shared across adapters. Exact language rules for each
// role, overloads, macros, and control-flow constructs remain adapter-owned.
typedef enum ReferenceRole {
    REFERENCE_ROLE_UNKNOWN = 0, REFERENCE_ROLE_READ, REFERENCE_ROLE_WRITE,
    REFERENCE_ROLE_READ_WRITE, REFERENCE_ROLE_CALL, REFERENCE_ROLE_TYPE,
    REFERENCE_ROLE_ADDRESS, REFERENCE_ROLE_CONTROL_FLOW
} ReferenceRole;

// One occurrence that may refer to a Symbol. A nonzero target id and resolved
// flag are supplied by a resolver; this record does not prove target liveness.
typedef struct Reference {
    u64 targetSymbolId; // zero in the unresolved/empty form
    SourceRange range;       // bytes occupied by this use
    ReferenceRole role;      // read/write/call/type/address/control flow
    bool resolved;           // result asserted by the producer/resolver
} Reference;

// Construct empty, target-id-only, or complete reference values. Construction
// is value-only and does not resolve names or check the target id.
Reference Reference_0(void);
Reference Reference_1(u64 targetSymbolId);
Reference Reference_4(u64 targetSymbolId, SourceRange range, ReferenceRole role, bool resolved);
// Dispatch Reference(), Reference(targetSymbolId), or the four-field form;
// other arities reject at compile time.
#define Reference(...) REFERENCE_SELECT(_ __VA_OPT__(,) __VA_ARGS__, Reference_4, Reference_invalid_arity, Reference_invalid_arity, Reference_1, Reference_0)(__VA_ARGS__)
#define REFERENCE_SELECT(_0, _1, _2, _3, _4, NAME, ...) NAME
// Return the empty identity and provide symmetric field accessors. Null
// getters return safe defaults; null setters do nothing.
Reference Reference_zero(void);
u64 Reference_getTargetSymbolId(const Reference *self);
void Reference_setTargetSymbolId(Reference *self, u64 value);
SourceRange Reference_getRange(const Reference *self);
void Reference_setRange(Reference *self, SourceRange value);
ReferenceRole Reference_getRole(const Reference *self);
void Reference_setRole(Reference *self, ReferenceRole value);
bool Reference_isResolved(const Reference *self);
void Reference_setResolved(Reference *self, bool value);

#endif
