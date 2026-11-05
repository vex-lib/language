#include "document/text_snapshot.h"
#include <assert.h>
#include <string.h>

int main(void) {
    char source[] = {'a', '\0', 'b'};
    TextSnapshot *snapshot = TextSnapshot(8u, 2u, source, sizeof source);
    assert(snapshot != nullptr);
    assert(TextSnapshot_getDocumentId(snapshot) == 8u);
    assert(TextSnapshot_getRevision(snapshot) == 2u);
    assert(TextSnapshot_getLength(snapshot) == sizeof source);
    source[0] = 'z';
    assert(memcmp(TextSnapshot_getBytes(snapshot), "a\0b.sh", 3u) == 0);
    assert(TextSnapshot_getReferenceCount(snapshot) == 1u);
    assert(TextSnapshot_retain(snapshot));
    assert(TextSnapshot_getReferenceCount(snapshot) == 2u);
    TextSnapshot_release(snapshot);
    assert(TextSnapshot_getReferenceCount(snapshot) == 1u);
    TextSnapshot_free(snapshot);
    assert(TextSnapshot_getBytes(nullptr) == nullptr);
    return 0;
}
