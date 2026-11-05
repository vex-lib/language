#include "document/text_document.h"
#include <assert.h>
#include <string.h>

int main(void) {
    TextDocument *document = TextDocument(10u, "old", 3u);
    assert(document != nullptr);
    assert(TextDocument_getDocumentId(document) == 10u);
    assert(TextDocument_getRevision(document) == 1u);
    TextSnapshot *old = TextDocument_snapshot(document);
    assert(old != nullptr && TextSnapshot_getReferenceCount(old) == 2u);
    assert(TextDocument_update(document, "newer", 5u));
    assert(TextDocument_getRevision(document) == 2u);
    assert(strcmp(TextSnapshot_getBytes(old), "old") == 0);
    TextSnapshot *current = TextDocument_snapshot(document);
    assert(TextSnapshot_getRevision(current) == 2u);
    assert(strcmp(TextSnapshot_getBytes(current), "newer") == 0);
    assert(!TextDocument_update(document, nullptr, 1u));
    assert(TextDocument_getRevision(document) == 2u);
    assert(strcmp(TextSnapshot_getBytes(current), "newer") == 0);
    assert(TextDocument_update(document, nullptr, 0u));
    assert(TextDocument_getRevision(document) == 3u);
    TextSnapshot *empty = TextDocument_snapshot(document);
    assert(TextSnapshot_getLength(empty) == 0u);
    TextSnapshot_free(old);
    TextSnapshot_free(current);
    TextSnapshot_free(empty);
    TextDocument_free(document);
    return 0;
}
