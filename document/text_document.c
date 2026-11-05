#include "document/text_document.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
#include <stdint.h>
#include <stdlib.h>

;;DEFINITION
/** TextDocument is an open document's current unsaved text state. Every
 * successful update publishes a new immutable snapshot revision; callers that
 * retained an older snapshot continue reading that exact version. */

;;OVERVIEW
/** CLASS: TextDocument (document/text_document.c)
 * STRUCT FIELDS: documentId, revision and owned current TextSnapshot pointer.
 * FUNCTION REGISTRY: constructor, free, update, snapshot, identity/revision getters.
 * CONTRACT: failed updates preserve both current bytes and revision; each
 * snapshot returned to a caller carries one retained reference. */

struct TextDocument {
    uint32_t documentId;
    uint64_t revision;
    TextSnapshot *current;
};

TextDocument *TextDocument_3(uint32_t documentId, const char *bytes, size_t length) {
    if (documentId == 0u) {
        THROW("TextDocument: rejected zero document identity");
        return nullptr;
    }
    TextSnapshot *snapshot = TextSnapshot_4(documentId, 1u, bytes, length);
    if (snapshot == nullptr)
        return nullptr;
    TextDocument *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("TextDocument: allocation failed");
        TextSnapshot_free(snapshot);
        return nullptr;
    }
    (*self).documentId = documentId;
    (*self).revision = 1u;
    (*self).current = snapshot;
    return self;
}

void TextDocument_free(TextDocument *self) {
    if (self != nullptr) {
        TextSnapshot_free((*self).current);
        (*self).current = nullptr;
        free(self);
    }
}

bool TextDocument_update(TextDocument *self, const char *bytes, size_t length) {
    if (self == nullptr || (bytes == nullptr && length != 0u)) {
        THROW("TextDocument: rejected invalid update bytes");
        return false;
    }
    if ((*self).revision == UINT64_MAX) {
        THROW("TextDocument: revision bound UINT64_MAX reached");
        return false;
    }
    TextSnapshot *replacement = TextSnapshot_4((*self).documentId,
                                               (*self).revision + 1u,
                                               bytes, length);
    if (replacement == nullptr)
        return false;
    TextSnapshot *previous = (*self).current;
    (*self).current = replacement;
    ++(*self).revision;
    TextSnapshot_free(previous);
    return true;
}

TextSnapshot *TextDocument_snapshot(const TextDocument *self) {
    if (self == nullptr || !TextSnapshot_retain((*self).current))
        return nullptr;
    return (*self).current;
}
uint32_t TextDocument_getDocumentId(const TextDocument *self) {
    return self == nullptr ? 0u : (*self).documentId;
}
uint64_t TextDocument_getRevision(const TextDocument *self) {
    return self == nullptr ? 0u : (*self).revision;
}
