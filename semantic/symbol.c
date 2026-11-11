#include "semantic/symbol.h"
#include "annotation/annotation.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: Symbol
 * ============================================================================
 * Symbol is the shared record for a named declaration in a project: a function,
 * class, method, variable, field, macro, label, type alias, or another listed
 * SymbolKind. These are kinds of the same identity-bearing concept, not separate
 * object classes with duplicated naming and location behavior. A global variable
 * is represented as a variable whose scope is the project/module's global scope;
 * this record does not reserve a magic global scope id.
 *
 * The id and scope id are assigned by a future semantic/index owner. Name bytes
 * are borrowed, not copied or freed. declarationRange identifies the declaration
 * spelling, while references to that symbol are separate Reference records.
 * Keeping those roles distinct allows one declaration to have many reads, writes,
 * calls, or type uses without pretending every use is a new variable/function.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Symbol (semantic/symbol.c)
 * ============================================================================
 * A borrowed name and declaration location attached to caller-assigned identity.
 *
 * STRUCT FIELDS (Mirroring semantic/symbol.h):
 * ----------------------------------------------------------------------------
 *   Symbol {
 *     u64 id;                    // stable identity within an index epoch
 *     u64 scopeId;               // declaring scope; policy owned elsewhere
 *     SymbolKind kind;                // function/class/variable/macro/etc.
 *     const char *name;               // borrowed spelling; never freed here
 *     SourceRange declarationRange;   // where the declaration is written
 *   }
 *
 * ENUM VOCABULARY:
 *   UNKNOWN, MODULE, NAMESPACE, CLASS, STRUCT, INTERFACE, ENUM, FUNCTION,
 *   METHOD, CONSTRUCTOR, VARIABLE, PARAMETER, FIELD, CONSTANT, MACRO, LABEL,
 *   TYPE_ALIAS. Language adapters may map native declaration forms to these
 *   broad categories; exact language-specific modifiers are not stored here.
 *
 * PRIVATE HELPERS: none.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   Symbol Symbol_0(void)
 *     Construct an all-zero symbol.
 *   Symbol Symbol_1(u64 id)
 *     Construct an identity-only symbol; all other fields remain zero/null.
 *   Symbol Symbol_5(u64 id, u64 scopeId, SymbolKind kind,
 *                   const char *name, SourceRange declarationRange)
 *     Construct the complete record without validating or copying borrowed data.
 *   Symbol Symbol_zero(void)
 *     Return the empty symbol identity.
 *   Symbol(...) dispatches the supported zero-, one-, and five-argument forms.
 *
 * Setters:
 *   void Symbol_setId(Symbol *self, u64 value)
 *   void Symbol_setScopeId(Symbol *self, u64 value)
 *   void Symbol_setKind(Symbol *self, SymbolKind value)
 *   void Symbol_setName(Symbol *self, const char *value)
 *   void Symbol_setDeclarationRange(Symbol *self, SourceRange value)
 *     Replace only the selected value. Name storage remains borrowed; callers
 *     must keep it alive while the Symbol is consumed.
 *
 * Getters:
 *   u64 Symbol_getId(const Symbol *self)
 *   u64 Symbol_getScopeId(const Symbol *self)
 *   SymbolKind Symbol_getKind(const Symbol *self)
 *   const char *Symbol_getName(const Symbol *self)
 *   SourceRange Symbol_getDeclarationRange(const Symbol *self)
 *     Return the selected field; null self returns its safe default.
 *
 * CONTRACT: identity uniqueness, scope existence, name lifetime, language
 * binding, overload resolution, and declaration validity belong to the future
 * project index/adapter. This class does not parse or resolve anything and owns
 * no allocations. Null setters do nothing.
 * ============================================================================
 */
Symbol Symbol_0(void) { return (Symbol) {0}; }
Symbol Symbol_1(u64 id) { return (Symbol) {.id = id}; }
Symbol Symbol_5(u64 id, u64 scopeId, SymbolKind kind, const char *name, SourceRange declarationRange) {
    return (Symbol) {.id = id, .scopeId = scopeId, .kind = kind, .name = name, .declarationRange = declarationRange};
}
Symbol Symbol_zero(void) { return Symbol_0(); }
u64 Symbol_getId(const Symbol *self) { return self == nullptr ? 0 : (*self).id; }
void Symbol_setId(Symbol *self, u64 value) { if (self != nullptr) (*self).id = value; }
u64 Symbol_getScopeId(const Symbol *self) { return self == nullptr ? 0 : (*self).scopeId; }
void Symbol_setScopeId(Symbol *self, u64 value) { if (self != nullptr) (*self).scopeId = value; }
SymbolKind Symbol_getKind(const Symbol *self) { return self == nullptr ? SYMBOL_KIND_UNKNOWN : (*self).kind; }
void Symbol_setKind(Symbol *self, SymbolKind value) { if (self != nullptr) (*self).kind = value; }
const char *Symbol_getName(const Symbol *self) { return self == nullptr ? nullptr : (*self).name; }
void Symbol_setName(Symbol *self, const char *value) { if (self != nullptr) (*self).name = value; }
SourceRange Symbol_getDeclarationRange(const Symbol *self) { return self == nullptr ? SourceRange_zero() : (*self).declarationRange; }
void Symbol_setDeclarationRange(Symbol *self, SourceRange value) { if (self != nullptr) (*self).declarationRange = value; }
