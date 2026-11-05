#include "semantic/symbol.h"
#include <assert.h>
#include <string.h>

int main(void) {
    SourceRange range = SourceRange(2, 0, 3);
    Symbol zero = Symbol_zero();
    assert(Symbol_getId(&zero) == 0);
    Symbol symbol = Symbol(42, 9, SYMBOL_KIND_FUNCTION, "paint", range);
    assert(Symbol_getId(&symbol) == 42);
    assert(Symbol_getScopeId(&symbol) == 9);
    assert(Symbol_getKind(&symbol) == SYMBOL_KIND_FUNCTION);
    assert(strcmp(Symbol_getName(&symbol), "paint") == 0);
    Symbol_setName(&symbol, "draw");
    assert(strcmp(Symbol_getName(&symbol), "draw") == 0);
    assert(Symbol_getName(nullptr) == nullptr);
    Symbol_setId(nullptr, 3);
    return 0;
}
