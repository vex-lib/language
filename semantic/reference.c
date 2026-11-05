#include "semantic/reference.h"
#include "annotation/annotation.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: Reference
 * ============================================================================
 * Reference represents one use-site occurrence in source and, when semantic
 * analysis has succeeded, the identity of its target Symbol. role distinguishes
 * common use behavior such as reading, writing, calling, using a type, or taking
 * an address. A control-flow reference can represent an adapter's relationship
 * to a label/jump target, but this record is not a control-flow graph.
 *
 * Unresolved and ambiguous occurrences must not be guessed into a target. This
 * initial record uses resolved=false and targetSymbolId=0 for the unresolved
 * shape; a future resolver owns candidate sets, overload rules, macro expansion,
 * and stale-document invalidation. A resolved bit is data supplied by that owner,
 * not proof that the target id still exists.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Reference (semantic/reference.c)
 * ============================================================================
 * A source use-site, its role, and an optional resolved symbol identity.
 *
 * STRUCT FIELDS (Mirroring semantic/reference.h):
 * ----------------------------------------------------------------------------
 *   Reference {
 *     uint64_t targetSymbolId; // target identity; zero for the unresolved form
 *     SourceRange range;       // bytes containing this occurrence in source
 *     ReferenceRole role;      // read/write/call/type/address/control flow
 *     bool resolved;           // resolver assertion; not a liveness guarantee
 *   }
 *
 * ENUM VOCABULARY:
 *   UNKNOWN, READ, WRITE, READ_WRITE, CALL, TYPE, ADDRESS, CONTROL_FLOW.
 *   These roles are model categories; a language adapter defines their exact
 *   meaning for operators, overloads, macros, and language-specific constructs.
 *
 * PRIVATE HELPERS: none.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   Reference Reference_0(void)
 *     Construct an unresolved, empty reference.
 *   Reference Reference_1(uint64_t targetSymbolId)
 *     Construct an identity-only record; it does not set resolved=true.
 *   Reference Reference_4(uint64_t targetSymbolId, SourceRange range,
 *                         ReferenceRole role, bool resolved)
 *     Construct a complete record from caller-provided analysis results.
 *   Reference Reference_zero(void)
 *     Return the empty reference identity.
 *   Reference(...) dispatches the supported zero-, one-, and four-argument forms.
 *
 * Setters:
 *   void Reference_setTargetSymbolId(Reference *self, uint64_t value)
 *   void Reference_setRange(Reference *self, SourceRange value)
 *   void Reference_setRole(Reference *self, ReferenceRole value)
 *   void Reference_setResolved(Reference *self, bool value)
 *     Replace one field. Related fields are not normalized automatically.
 *
 * Getters:
 *   uint64_t Reference_getTargetSymbolId(const Reference *self)
 *   SourceRange Reference_getRange(const Reference *self)
 *   ReferenceRole Reference_getRole(const Reference *self)
 *   bool Reference_isResolved(const Reference *self)
 *     Return the selected field; null self returns a safe default.
 *
 * CONTRACT: target lookup, ambiguity, identity lifetime, document revision, and
 * resolution policy are external. This value does not own its target Symbol and
 * does not perform parsing, lookup, control-flow analysis, or allocation. Null
 * setters do nothing.
 * ============================================================================
 */
Reference Reference_0(void) { return (Reference) {0}; }
Reference Reference_1(uint64_t targetSymbolId) { return (Reference) {.targetSymbolId = targetSymbolId}; }
Reference Reference_4(uint64_t targetSymbolId, SourceRange range, ReferenceRole role, bool resolved) {
    return (Reference) {.targetSymbolId = targetSymbolId, .range = range, .role = role, .resolved = resolved};
}
Reference Reference_zero(void) { return Reference_0(); }
uint64_t Reference_getTargetSymbolId(const Reference *self) { return self == nullptr ? 0 : (*self).targetSymbolId; }
void Reference_setTargetSymbolId(Reference *self, uint64_t value) { if (self != nullptr) (*self).targetSymbolId = value; }
SourceRange Reference_getRange(const Reference *self) { return self == nullptr ? SourceRange_zero() : (*self).range; }
void Reference_setRange(Reference *self, SourceRange value) { if (self != nullptr) (*self).range = value; }
ReferenceRole Reference_getRole(const Reference *self) { return self == nullptr ? REFERENCE_ROLE_UNKNOWN : (*self).role; }
void Reference_setRole(Reference *self, ReferenceRole value) { if (self != nullptr) (*self).role = value; }
bool Reference_isResolved(const Reference *self) { return self != nullptr && (*self).resolved; }
void Reference_setResolved(Reference *self, bool value) { if (self != nullptr) (*self).resolved = value; }
