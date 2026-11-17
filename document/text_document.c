#include "vex.h"
#include "document/text_document.h"
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
    u32 documentId;
    u64 revision;
    TextSnapshot *current;
};

;;GETTER
static inline u32 TextDocument_documentId(const TextDocument *self) { return (*self).documentId; }
;;SETTER
static inline fn TextDocument_setDocumentId(TextDocument *self, u32 v) { (*self).documentId = v; }
;;GETTER
static inline u64 TextDocument_revision(const TextDocument *self) { return (*self).revision; }
;;SETTER
static inline fn TextDocument_setRevision(TextDocument *self, u64 v) { (*self).revision = v; }
;;GETTER
static inline TextSnapshot *TextDocument_current(const TextDocument *self) { return (*self).current; }
;;SETTER
static inline fn TextDocument_setCurrent(TextDocument *self, TextSnapshot *v) { (*self).current = v; }

TextDocument *TextDocument_3(u32 documentId, const char *data, bytes length) {
    if (documentId == 0u) {
        THROW("TextDocument: rejected zero document identity");
        return nullptr;
    }
    TextSnapshot *snapshot = TextSnapshot_4(documentId, 1u, data, length);
    if (snapshot == nullptr)
        return nullptr;
    TextDocument *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("TextDocument: allocation failed");
        TextSnapshot_free(snapshot);
        return nullptr;
    }
    TextDocument_setDocumentId(self, documentId);
    TextDocument_setRevision(self, 1u);
    TextDocument_setCurrent(self, snapshot);
    return self;
}

fn TextDocument_free(TextDocument *self) {
    if (self != nullptr) {
        TextSnapshot_free(TextDocument_current(self));
        TextDocument_setCurrent(self, nullptr);
        free(self);
    }
}

bool TextDocument_update(TextDocument *self, const char *data, bytes length) {
    if (self == nullptr || (data == nullptr && length != 0u)) {
        THROW("TextDocument: rejected invalid update bytes");
        return false;
    }
    if (TextDocument_revision(self) == UINT64_MAX) {
        THROW("TextDocument: revision bound UINT64_MAX reached");
        return false;
    }
    TextSnapshot *replacement = TextSnapshot_4(TextDocument_documentId(self),
                                               TextDocument_revision(self) + 1u,
                                               data, length);
    if (replacement == nullptr)
        return false;
    TextSnapshot *previous = TextDocument_current(self);
    TextDocument_setCurrent(self, replacement);
    TextDocument_setRevision(self, TextDocument_revision(self) + 1u);
    TextSnapshot_free(previous);
    return true;
}

TextSnapshot *TextDocument_snapshot(const TextDocument *self) {
    if (self == nullptr || !TextSnapshot_retain(TextDocument_current(self)))
        return nullptr;
    return TextDocument_current(self);
}
;;GETTER
u32 TextDocument_getDocumentId(const TextDocument *self) {
    return self == nullptr ? 0u : TextDocument_documentId(self);
}
;;GETTER
u64 TextDocument_getRevision(const TextDocument *self) {
    return self == nullptr ? 0u : TextDocument_revision(self);
}
