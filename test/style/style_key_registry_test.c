#include "style/style_key_registry.h"
#include <assert.h>

int main(void) {
    StyleKeyRegistry core = StyleKeyRegistry();
    assert(StyleKeyRegistry_count(&core) == StyleKeyRegistry_coreCount());
    assert(StyleKeyRegistry_isCore(&core, "text"));
    assert(StyleKeyRegistry_name(&core, 0u) != nullptr);
    assert(StyleKeyRegistry_name(&core, StyleKeyRegistry_coreCount()) == nullptr);
    assert(!StyleKeyRegistry_isAdapter(&core, "c.pointer"));
    const StyleKeyEntry entries[] = {
        {"c.pointer", "operator.pointer"},
        {"c.annotation", "annotation"}
    };
    StyleKeyRegistry registry = StyleKeyRegistry(entries, 2u);
    assert(StyleKeyRegistry_isValid(&registry));
    assert(StyleKeyRegistry_isCore(&registry, "type"));
    assert(StyleKeyRegistry_isAdapter(&registry, "c.pointer"));
    assert(StyleKeyRegistry_resolve(&registry, "c.pointer") != nullptr);
    assert(StyleKeyRegistry_fallback(&registry, "c.pointer")[0] == 'o');
    assert(StyleKeyRegistry_count(&registry) == StyleKeyRegistry_coreCount() + 2u);
    const StyleKeyEntry bad[] = {{"c.bad", "not-a-core-key"}};
    StyleKeyRegistry invalid = StyleKeyRegistry(bad, 1u);
    assert(!StyleKeyRegistry_isValid(&invalid));
    return 0;
}
