#include "document/text_snapshot.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
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

TextSnapshot *TextSnapshot_4(u32 documentId, u64 revision,
                             const char *bytes, bytes length) {
    if (documentId == 0u || revision == 0u || (bytes == nullptr && length != 0u)
        || length == SIZE_MAX) {
        THROW("TextSnapshot: rejected invalid identity or byte span");
        return nullptr;
    }
    TextSnapshot *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("TextSnapshot: allocation failed for snapshot metadata");
        return nullptr;
    }
    (*self).bytes = malloc(length + 1u);
    if ((*self).bytes == nullptr) {
        THROW("TextSnapshot: allocation failed for %zu source bytes", length);
        free(self);
        return nullptr;
    }
    if (length != 0u)
        memcpy((*self).bytes, bytes, length);
    (*self).bytes[length] = '\0';
    (*self).documentId = documentId;
    (*self).revision = revision;
    (*self).length = length;
    (*self).referenceCount = 1u;
    return self;
}

void TextSnapshot_release(TextSnapshot *self) {
    if (self == nullptr)
        return;
    if ((*self).referenceCount == 0u) {
        THROW("TextSnapshot: rejected release with zero references");
        return;
    }
    --(*self).referenceCount;
    if ((*self).referenceCount == 0u) {
        free((*self).bytes);
        (*self).bytes = nullptr;
        free(self);
    }
}
void TextSnapshot_free(TextSnapshot *self) { TextSnapshot_release(self); }
bool TextSnapshot_retain(TextSnapshot *self) {
    if (self == nullptr || (*self).referenceCount == 0u
        || (*self).referenceCount == UINT64_MAX) {
        THROW("TextSnapshot: rejected retain at reference-count bound");
        return false;
    }
    ++(*self).referenceCount;
    return true;
}
u32 TextSnapshot_getDocumentId(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).documentId;
}
u64 TextSnapshot_getRevision(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).revision;
}
const char *TextSnapshot_getBytes(const TextSnapshot *self) {
    return self == nullptr ? nullptr : (*self).bytes;
}
bytes TextSnapshot_getLength(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).length;
}
u64 TextSnapshot_getReferenceCount(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).referenceCount;
}
