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
    uint32_t documentId;
    uint64_t revision;
    char *bytes;
    size_t length;
    uint64_t referenceCount;
};

TextSnapshot *TextSnapshot_4(uint32_t documentId, uint64_t revision,
                             const char *bytes, size_t length) {
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
uint32_t TextSnapshot_getDocumentId(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).documentId;
}
uint64_t TextSnapshot_getRevision(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).revision;
}
const char *TextSnapshot_getBytes(const TextSnapshot *self) {
    return self == nullptr ? nullptr : (*self).bytes;
}
size_t TextSnapshot_getLength(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).length;
}
uint64_t TextSnapshot_getReferenceCount(const TextSnapshot *self) {
    return self == nullptr ? 0u : (*self).referenceCount;
}
