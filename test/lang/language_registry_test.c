#include "adapters/c23/c23_adapter.h"
#include "lang/language_registry.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    LanguageRegistry *registry = LanguageRegistry_0();
    assert(registry != nullptr && LanguageRegistry_getCount(registry) == 0u);
    const LanguageAdapter *adapter = C23Adapter_adapter();
    assert(LanguageRegistry_register(registry, adapter));
    assert(LanguageRegistry_getCount(registry) == 1u);
    assert(LanguageRegistry_find(registry, "c23") == adapter);
    assert(LanguageRegistry_find(registry, "c99") == nullptr);
    assert(!LanguageRegistry_register(registry, adapter));
    assert(LanguageRegistry_getCount(registry) == 1u);
    LanguageRegistry_free(registry);

    registry = LanguageRegistry_0();
    assert(registry != nullptr);
    LanguageAdapter copies[32];
    char ids[32][8];
    for (size_t index = 0u; index < 32u; ++index) {
        copies[index] = *adapter;
        (void) snprintf(ids[index], sizeof ids[index], "lang%zu", index);
        copies[index].languageId = ids[index];
        assert(LanguageRegistry_register(registry, &copies[index]));
    }
    assert(LanguageRegistry_getCount(registry) == 32u);
    assert(LanguageRegistry_getExhaustionCount(registry) == 0u);
    assert(LanguageRegistry_find(registry, "lang31") == &copies[31]);
    assert(!LanguageRegistry_register(registry, &copies[0]));
    assert(LanguageRegistry_getCount(registry) == 32u);
    LanguageRegistry_free(registry);
    return 0;
}
