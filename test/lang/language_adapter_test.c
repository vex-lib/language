#include "adapters/c23/c23_adapter.h"
#include "lang/language_adapter.h"
#include <assert.h>
#include <string.h>

int main(void) {
    const LanguageAdapter *adapter = C23Adapter_adapter();
    assert(LanguageAdapter_isValid(adapter));
    assert(strcmp(LanguageAdapter_getLanguageId(adapter), "c23") == 0);
    LanguageAdapter invalid = {0};
    assert(!LanguageAdapter_isValid(&invalid));
    assert(LanguageAdapter_getLanguageId(&invalid) == nullptr);
    return 0;
}
