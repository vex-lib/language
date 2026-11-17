#include "vex.h"
#include "lang/language_adapter.h"

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

;;GETTER
static inline const char *LanguageAdapter_languageId(const LanguageAdapter *self) { return (*self).languageId; }
;;GETTER
static inline LanguageAdapterOpenFn LanguageAdapter_openFn(const LanguageAdapter *self) { return (*self).open; }
;;GETTER
static inline LanguageAdapterNextFn LanguageAdapter_nextFn(const LanguageAdapter *self) { return (*self).next; }
;;GETTER
static inline LanguageAdapterCloseFn LanguageAdapter_closeFn(const LanguageAdapter *self) { return (*self).close; }

bool LanguageAdapter_isValid(const LanguageAdapter *self) {
    return self != nullptr && LanguageAdapter_languageId(self) != nullptr
        && LanguageAdapter_languageId(self)[0] != '\0' && LanguageAdapter_openFn(self) != nullptr
        && LanguageAdapter_nextFn(self) != nullptr && LanguageAdapter_closeFn(self) != nullptr;
}

;;GETTER
const char *LanguageAdapter_getLanguageId(const LanguageAdapter *self) {
    return LanguageAdapter_isValid(self) ? LanguageAdapter_languageId(self) : nullptr;
}

any LanguageAdapter_open(const LanguageAdapter *self, u32 documentId,
                           const char *source, bytes length) {
    if (!LanguageAdapter_isValid(self) || documentId == 0u
        || (source == nullptr && length != 0u)) {
        THROW("language adapter rejected invalid open request");
        return nullptr;
    }
    return LanguageAdapter_openFn(self)(documentId, source, length);
}

LanguageLexResult LanguageAdapter_next(const LanguageAdapter *self, any session) {
    if (!LanguageAdapter_isValid(self) || session == nullptr) {
        THROW("language adapter rejected invalid next request");
        return LanguageLexResult_zero();
    }
    return LanguageAdapter_nextFn(self)(session);
}

fn LanguageAdapter_close(const LanguageAdapter *self, any session) {
    if (session == nullptr)
        return;
    if (!LanguageAdapter_isValid(self)) {
        THROW("language adapter rejected invalid close request");
        return;
    }
    LanguageAdapter_closeFn(self)(session);
}
