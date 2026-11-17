#include "vex.h"
#include "document/text_snapshot.h"
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/** TextSnapshot owns one immutable copy of document bytes. Its identity and
 * monotonically assigned revision bind downstream spans to an exact text state.
 * Engines retain snapshots while adapter sessions borrow their bytes. */

;;OVERVIEW
/** CLASS: TextSnapshot (document/text_snapshot.c)
 * STRUCT FIELDS: documentId, revision, bytes, length and referenceCount.
 * FUNCTION REGISTRY: constructor, retain/release/free, symmetric read accessors.
 * CONTRACT: documentId and revision are nonzero; bytes are copied; null bytes
 * are accepted only for an empty snapshot; refcount is single-thread confined. */

struct TextSnapshot {
    u32 documentId;
    u64 revision;
    char *bytes;
    bytes length;
    u64 referenceCount;
};

;;GETTER
static inline char *TextSnapshot_bytes(const TextSnapshot *self) { return (*self).bytes; }
;;SETTER
static inline fn TextSnapshot_setBytes(TextSnapshot *self, char *v) { (*self).bytes = v; }
;;SETTER
static inline fn TextSnapshot_setDocumentId(TextSnapshot *self, u32 v) { (*self).documentId = v; }
;;SETTER
static inline fn TextSnapshot_setRevision(TextSnapshot *self, u64 v) { (*self).revision = v; }
;;SETTER
static inline fn TextSnapshot_setLength(TextSnapshot *self, bytes v) { (*self).length = v; }
;;GETTER
static inline u64 TextSnapshot_referenceCount(const TextSnapshot *self) { return (*self).referenceCount; }
;;SETTER
static inline fn TextSnapshot_setReferenceCount(TextSnapshot *self, u64 v) { (*self).referenceCount = v; }

TextSnapshot *TextSnapshot_4(u32 documentId, u64 revision,
                             const char *data, bytes length) {
    if (documentId == 0u || revision == 0u || (data == nullptr && length != 0u)
        || length == SIZE_MAX) {
        THROW("TextSnapshot: rejected invalid identity or byte span");
        return nullptr;
    }
    TextSnapshot *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("TextSnapshot: allocation failed for snapshot metadata");
        return nullptr;
    }
    TextSnapshot_setBytes(self, malloc(length + 1u));
    if (TextSnapshot_bytes(self) == nullptr) {
        THROW("TextSnapshot: allocation failed for %zu source bytes", length);
        free(self);
        return nullptr;
    }
    if (length != 0u)
        memcpy(TextSnapshot_bytes(self), data, length);
    TextSnapshot_bytes(self)[length] = '\0';
    TextSnapshot_setDocumentId(self, documentId);
    TextSnapshot_setRevision(self, revision);
    TextSnapshot_setLength(self, length);
    TextSnapshot_setReferenceCount(self, 1u);
    return self;
}

fn TextSnapshot_release(TextSnapshot *self) {
    if (self == nullptr)
        return;
    if (TextSnapshot_referenceCount(self) == 0u) {
        THROW("TextSnapshot: rejected release with zero references");
        return;
    }
    TextSnapshot_setReferenceCount(self, TextSnapshot_referenceCount(self) - 1u);
    if (TextSnapshot_referenceCount(self) == 0u) {
        free(TextSnapshot_bytes(self));
        TextSnapshot_setBytes(self, nullptr);
        free(self);
    }
}
fn TextSnapshot_free(TextSnapshot *self) { TextSnapshot_release(self); }
bool TextSnapshot_retain(TextSnapshot *self) {
    if (self == nullptr || TextSnapshot_referenceCount(self) == 0u
        || TextSnapshot_referenceCount(self) == UINT64_MAX) {
        THROW("TextSnapshot: rejected retain at reference-count bound");
        return false;
    }
    TextSnapshot_setReferenceCount(self, TextSnapshot_referenceCount(self) + 1u);
    return true;
}
;;GETTER
u32 TextSnapshot_getDocumentId(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).documentId;
}
;;GETTER
u64 TextSnapshot_getRevision(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).revision;
}
;;GETTER
const char *TextSnapshot_getBytes(const TextSnapshot *self) {
    return self == nullptr ? nullptr : (*self).bytes;
}
;;GETTER
bytes TextSnapshot_getLength(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).length;
}
;;GETTER
u64 TextSnapshot_getReferenceCount(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).referenceCount;
}
