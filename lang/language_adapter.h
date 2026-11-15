#ifndef LANGUAGE_LANG_LANGUAGE_ADAPTER_H
#define LANGUAGE_LANG_LANGUAGE_ADAPTER_H

#include "vex.h"
#include "analysis/syntax_span.h"
#include "lang/language_lex_result.h"

/* A language adapter is selected by an explicit language id, never by suffix
 * alone: the same .c/.h file may be compiled under different language modes. */
typedef struct LanguageAdapter {
    const char *languageId;
    any (*open)(u32 documentId, const char *source, bytes length);
    LanguageLexResult (*next)(any session);
    fn (*close)(any session);
} LanguageAdapter;

bool LanguageAdapter_isValid(const LanguageAdapter *self);
const char *LanguageAdapter_getLanguageId(const LanguageAdapter *self);
any LanguageAdapter_open(const LanguageAdapter *self, u32 documentId,
                           const char *source, bytes length);
LanguageLexResult LanguageAdapter_next(const LanguageAdapter *self, any session);
fn LanguageAdapter_close(const LanguageAdapter *self, any session);

#endif
