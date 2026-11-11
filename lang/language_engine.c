#include "lang/language_engine.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
#include <stdlib.h>

;;DEFINITION
/** LanguageEngine is the small host-facing lexer coordinator. A host selects
 * an explicit language id and provides an immutable source snapshot. The engine
 * keeps adapter sessions and byte spans; it does not open projects, infer a
 * dialect from a suffix, own document bytes, or present colors. */

;;OVERVIEW
/** CLASS: LanguageEngine (lang/language_engine.c)
 * STRUCT FIELDS: borrowed registry, selected adapter, opaque session.
 * FUNCTION REGISTRY: constructor, free, open, next, close, selected language
 * and current document/revision/context identity accessors.
 * CONTRACT: open replaces a session only on success; close releases adapter
 * state. Registry and source bytes outlive use. */

struct LanguageEngine {
    const LanguageRegistry *registry;
    const LanguageAdapter *adapter;
    void *session;
    TextSnapshot *snapshot;
    u64 compilationContextId;
};

LanguageEngine *LanguageEngine_1(const LanguageRegistry *registry) {
    if (registry == nullptr) {
        THROW("LanguageEngine: rejected null registry");
        return nullptr;
    }
    LanguageEngine *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("LanguageEngine: allocation failed");
        return nullptr;
    }
    (*self).registry = registry;
    return self;
}
LanguageEngine *LanguageEngine_zero(void) { return nullptr; }
void LanguageEngine_close(LanguageEngine *self) {
    if (self != nullptr && (*self).session != nullptr) {
        LanguageAdapter_close((*self).adapter, (*self).session);
        (*self).session = nullptr;
        (*self).adapter = nullptr;
        TextSnapshot_release((*self).snapshot);
        (*self).snapshot = nullptr;
        (*self).compilationContextId = 0u;
    }
}
void LanguageEngine_free(LanguageEngine *self) {
    if (self != nullptr) {
        LanguageEngine_close(self);
        free(self);
    }
}
bool LanguageEngine_open(LanguageEngine *self, const char *languageId,
                         u64 compilationContextId, TextSnapshot *snapshot) {
    if (self == nullptr || languageId == nullptr || snapshot == nullptr
        || compilationContextId == 0u
        || TextSnapshot_getDocumentId(snapshot) == 0u
        || TextSnapshot_getRevision(snapshot) == 0u) {
        THROW("LanguageEngine: rejected invalid open request");
        return false;
    }
    const LanguageAdapter *adapter = LanguageRegistry_find((*self).registry, languageId);
    if (adapter == nullptr) {
        THROW("LanguageEngine: language id is not registered");
        return false;
    }
    if (!TextSnapshot_retain(snapshot))
        return false;
    void *session = LanguageAdapter_open(adapter, TextSnapshot_getDocumentId(snapshot),
                                         TextSnapshot_getBytes(snapshot),
                                         TextSnapshot_getLength(snapshot));
    if (session == nullptr) {
        TextSnapshot_release(snapshot);
        return false;
    }
    LanguageEngine_close(self);
    (*self).adapter = adapter;
    (*self).session = session;
    (*self).snapshot = snapshot;
    (*self).compilationContextId = compilationContextId;
    return true;
}
LanguageLexResult LanguageEngine_next(LanguageEngine *self) {
    if (self == nullptr || (*self).session == nullptr) {
        THROW("LanguageEngine: rejected next without an open snapshot");
        return LanguageLexResult_zero();
    }
    LanguageLexResult result = LanguageAdapter_next((*self).adapter, (*self).session);
    return LanguageLexResult_withContext(result, TextSnapshot_getRevision((*self).snapshot),
                                         (*self).compilationContextId);
}
const char *LanguageEngine_getLanguageId(const LanguageEngine *self) {
    return self == nullptr || (*self).adapter == nullptr ? nullptr
        : LanguageAdapter_getLanguageId((*self).adapter);
}
u32 LanguageEngine_getDocumentId(const LanguageEngine *self) {
    return self == nullptr || (*self).snapshot == nullptr
        ? 0u : TextSnapshot_getDocumentId((*self).snapshot);
}
u64 LanguageEngine_getDocumentRevision(const LanguageEngine *self) {
    return self == nullptr || (*self).snapshot == nullptr
        ? 0u : TextSnapshot_getRevision((*self).snapshot);
}
u64 LanguageEngine_getCompilationContextId(const LanguageEngine *self) {
    return self == nullptr ? 0u : (*self).compilationContextId;
}
