#include "vex.h"
#include "lang/language_engine.h"
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
    any session;
    TextSnapshot *snapshot;
    u64 compilationContextId;
};

;;GETTER
static inline const LanguageRegistry *LanguageEngine_registry(const LanguageEngine *self) { return (*self).registry; }
;;SETTER
static inline fn LanguageEngine_setRegistry(LanguageEngine *self, const LanguageRegistry *v) { (*self).registry = v; }
;;GETTER
static inline const LanguageAdapter *LanguageEngine_adapter(const LanguageEngine *self) { return (*self).adapter; }
;;SETTER
static inline fn LanguageEngine_setAdapter(LanguageEngine *self, const LanguageAdapter *v) { (*self).adapter = v; }
;;GETTER
static inline any LanguageEngine_session(const LanguageEngine *self) { return (*self).session; }
;;SETTER
static inline fn LanguageEngine_setSession(LanguageEngine *self, any v) { (*self).session = v; }
;;GETTER
static inline TextSnapshot *LanguageEngine_snapshot(const LanguageEngine *self) { return (*self).snapshot; }
;;SETTER
static inline fn LanguageEngine_setSnapshot(LanguageEngine *self, TextSnapshot *v) { (*self).snapshot = v; }
;;GETTER
static inline u64 LanguageEngine_compilationContextId(const LanguageEngine *self) { return (*self).compilationContextId; }
;;SETTER
static inline fn LanguageEngine_setCompilationContextId(LanguageEngine *self, u64 v) { (*self).compilationContextId = v; }

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
    LanguageEngine_setRegistry(self, registry);
    return self;
}
LanguageEngine *LanguageEngine_zero() { return nullptr; }
fn LanguageEngine_close(LanguageEngine *self) {
    if (self != nullptr && LanguageEngine_session(self) != nullptr) {
        LanguageAdapter_close(LanguageEngine_adapter(self), LanguageEngine_session(self));
        LanguageEngine_setSession(self, nullptr);
        LanguageEngine_setAdapter(self, nullptr);
        TextSnapshot_release(LanguageEngine_snapshot(self));
        LanguageEngine_setSnapshot(self, nullptr);
        LanguageEngine_setCompilationContextId(self, 0u);
    }
}
fn LanguageEngine_free(LanguageEngine *self) {
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
    const LanguageAdapter *adapter = LanguageRegistry_find(LanguageEngine_registry(self), languageId);
    if (adapter == nullptr) {
        THROW("LanguageEngine: language id is not registered");
        return false;
    }
    if (!TextSnapshot_retain(snapshot))
        return false;
    any session = LanguageAdapter_open(adapter, TextSnapshot_getDocumentId(snapshot),
                                         TextSnapshot_getBytes(snapshot),
                                         TextSnapshot_getLength(snapshot));
    if (session == nullptr) {
        TextSnapshot_release(snapshot);
        return false;
    }
    LanguageEngine_close(self);
    LanguageEngine_setAdapter(self, adapter);
    LanguageEngine_setSession(self, session);
    LanguageEngine_setSnapshot(self, snapshot);
    LanguageEngine_setCompilationContextId(self, compilationContextId);
    return true;
}
LanguageLexResult LanguageEngine_next(LanguageEngine *self) {
    if (self == nullptr || LanguageEngine_session(self) == nullptr) {
        THROW("LanguageEngine: rejected next without an open snapshot");
        return LanguageLexResult_zero();
    }
    LanguageLexResult result = LanguageAdapter_next(LanguageEngine_adapter(self), LanguageEngine_session(self));
    return LanguageLexResult_withContext(result, TextSnapshot_getRevision(LanguageEngine_snapshot(self)),
                                         LanguageEngine_compilationContextId(self));
}
const char *LanguageEngine_getLanguageId(const LanguageEngine *self) {
    return self == nullptr || LanguageEngine_adapter(self) == nullptr ? nullptr
        : LanguageAdapter_getLanguageId(LanguageEngine_adapter(self));
}
u32 LanguageEngine_getDocumentId(const LanguageEngine *self) {
    return self == nullptr || LanguageEngine_snapshot(self) == nullptr
        ? 0u : TextSnapshot_getDocumentId(LanguageEngine_snapshot(self));
}
u64 LanguageEngine_getDocumentRevision(const LanguageEngine *self) {
    return self == nullptr || LanguageEngine_snapshot(self) == nullptr
        ? 0u : TextSnapshot_getRevision(LanguageEngine_snapshot(self));
}
;;GETTER
u64 LanguageEngine_getCompilationContextId(const LanguageEngine *self) {
    return self == nullptr ? 0u : LanguageEngine_compilationContextId(self);
}
