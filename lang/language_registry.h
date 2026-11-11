#ifndef LANGUAGE_LANG_LANGUAGE_REGISTRY_H
#define LANGUAGE_LANG_LANGUAGE_REGISTRY_H

#include "lang/language_adapter.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct LanguageRegistry LanguageRegistry;

LanguageRegistry *LanguageRegistry_0(void);
LanguageRegistry *LanguageRegistry_zero(void);
void LanguageRegistry_free(LanguageRegistry *self);
bool LanguageRegistry_register(LanguageRegistry *self, const LanguageAdapter *adapter);
const LanguageAdapter *LanguageRegistry_find(const LanguageRegistry *self,
                                              const char *languageId);
bytes LanguageRegistry_getCount(const LanguageRegistry *self);
u64 LanguageRegistry_getExhaustionCount(const LanguageRegistry *self);

#endif
