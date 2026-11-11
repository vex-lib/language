#ifndef LANGUAGE_SEMANTIC_SYMBOL_H
#define LANGUAGE_SEMANTIC_SYMBOL_H

#include "document/source_range.h"
#include <stdbool.h>
#include <stdint.h>

// Language-neutral named declaration kinds. A language adapter maps its native
// declarations into these categories; global-ness comes from the scope, not a
// separate variable kind.
typedef enum SymbolKind {
    SYMBOL_KIND_UNKNOWN = 0, SYMBOL_KIND_MODULE, SYMBOL_KIND_NAMESPACE,
    SYMBOL_KIND_CLASS, SYMBOL_KIND_STRUCT, SYMBOL_KIND_INTERFACE, SYMBOL_KIND_ENUM,
    SYMBOL_KIND_FUNCTION, SYMBOL_KIND_METHOD, SYMBOL_KIND_CONSTRUCTOR,
    SYMBOL_KIND_VARIABLE, SYMBOL_KIND_PARAMETER, SYMBOL_KIND_FIELD,
    SYMBOL_KIND_CONSTANT, SYMBOL_KIND_MACRO, SYMBOL_KIND_LABEL, SYMBOL_KIND_TYPE_ALIAS
} SymbolKind;

// One identity-bearing declaration. name and its bytes are borrowed; the
// project index must retain them while readers use this record.
typedef struct Symbol {
    u64 id;                    // index-assigned identity within an epoch
    u64 scopeId;               // declaring scope identity
    SymbolKind kind;                // function/class/variable/macro/etc.
    const char *name;               // borrowed source-independent spelling
    SourceRange declarationRange;   // declaration location, not a use site
} Symbol;

// Construct empty, identity-only, or complete symbol values. These store the
// name pointer without copying and do not validate id/scope/range relationships.
Symbol Symbol_0(void);
Symbol Symbol_1(u64 id);
Symbol Symbol_5(u64 id, u64 scopeId, SymbolKind kind, const char *name, SourceRange declarationRange);
// Dispatch Symbol(), Symbol(id), or the five-field constructor; other arities
// reject at compile time.
#define Symbol(...) SYMBOL_SELECT(_ __VA_OPT__(,) __VA_ARGS__, Symbol_5, Symbol_invalid_arity, Symbol_invalid_arity, Symbol_invalid_arity, Symbol_1, Symbol_0)(__VA_ARGS__)
#define SYMBOL_SELECT(_0, _1, _2, _3, _4, _5, NAME, ...) NAME
// Return the all-zero identity and provide symmetric field accessors. Null
// getters return safe defaults; null setters do nothing.
Symbol Symbol_zero(void);
u64 Symbol_getId(const Symbol *self);
void Symbol_setId(Symbol *self, u64 value);
u64 Symbol_getScopeId(const Symbol *self);
void Symbol_setScopeId(Symbol *self, u64 value);
SymbolKind Symbol_getKind(const Symbol *self);
void Symbol_setKind(Symbol *self, SymbolKind value);
const char *Symbol_getName(const Symbol *self);
void Symbol_setName(Symbol *self, const char *value);
SourceRange Symbol_getDeclarationRange(const Symbol *self);
void Symbol_setDeclarationRange(Symbol *self, SourceRange value);

#endif
