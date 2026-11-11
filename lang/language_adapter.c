#include "lang/language_adapter.h"
#include "annotation/annotation.h"
#include "exception/throw.h"

;;DEFINITION
/** LanguageAdapter is a borrowed callback table for one grammar and language
 * mode. Its stable id is explicit so extension-based guesses cannot silently
 * confuse C23 with C99 or C++. The host owns the table and callback code; an
 * opened opaque session owns only adapter state. */

;;OVERVIEW
/** CLASS: LanguageAdapter (lang/language_adapter.c)
 * STRUCT FIELDS: languageId, open, next, close (borrowed callback table).
 * FUNCTION REGISTRY: isValid/getLanguageId/open/next/close.
 * CONTRACT: callbacks are called only after complete table validation; a
 * missing session or callback is rejected and produces a zero span. */

bool LanguageAdapter_isValid(const LanguageAdapter *self) {
    return self != nullptr && (*self).languageId != nullptr
        && (*self).languageId[0] != '\0' && (*self).open != nullptr
        && (*self).next != nullptr && (*self).close != nullptr;
}

const char *LanguageAdapter_getLanguageId(const LanguageAdapter *self) {
    return LanguageAdapter_isValid(self) ? (*self).languageId : nullptr;
}

void *LanguageAdapter_open(const LanguageAdapter *self, u32 documentId,
                           const char *source, bytes length) {
    if (!LanguageAdapter_isValid(self) || documentId == 0u
        || (source == nullptr && length != 0u)) {
        THROW("language adapter rejected invalid open request");
        return nullptr;
    }
    return (*self).open(documentId, source, length);
}

LanguageLexResult LanguageAdapter_next(const LanguageAdapter *self, void *session) {
    if (!LanguageAdapter_isValid(self) || session == nullptr) {
        THROW("language adapter rejected invalid next request");
        return LanguageLexResult_zero();
    }
    return (*self).next(session);
}

void LanguageAdapter_close(const LanguageAdapter *self, void *session) {
    if (session == nullptr)
        return;
    if (!LanguageAdapter_isValid(self)) {
        THROW("language adapter rejected invalid close request");
        return;
    }
    (*self).close(session);
}
