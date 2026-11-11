#ifndef LANGUAGE_LANG_LANGUAGE_ENGINE_H
#define LANGUAGE_LANG_LANGUAGE_ENGINE_H

#include "lang/language_registry.h"
#include "document/text_snapshot.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct LanguageEngine LanguageEngine;

LanguageEngine *LanguageEngine_1(const LanguageRegistry *registry);
LanguageEngine *LanguageEngine_zero(void);
void LanguageEngine_free(LanguageEngine *self);
bool LanguageEngine_open(LanguageEngine *self, const char *languageId,
                         u64 compilationContextId, TextSnapshot *snapshot);
LanguageLexResult LanguageEngine_next(LanguageEngine *self);
void LanguageEngine_close(LanguageEngine *self);
const char *LanguageEngine_getLanguageId(const LanguageEngine *self);
u32 LanguageEngine_getDocumentId(const LanguageEngine *self);
u64 LanguageEngine_getDocumentRevision(const LanguageEngine *self);
u64 LanguageEngine_getCompilationContextId(const LanguageEngine *self);

#endif
