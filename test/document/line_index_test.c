#include "document/line_index.h"
#include <assert.h>

int main(void) {
    const char text[] = "a😀\r\nβ";
    TextSnapshot *snapshot = TextSnapshot(31u, 4u, text, sizeof text - 1u);
    assert(snapshot != nullptr);
    LineIndex *index = LineIndex_1(snapshot);
    assert(index != nullptr && TextSnapshot_getReferenceCount(snapshot) == 2u);
    assert(LineIndex_getLineCount(index) == 2u);
    assert(LineIndex_getDocumentId(index) == 31u && LineIndex_getRevision(index) == 4u);
    TextSnapshot_free(snapshot);

    SourcePosition position = LineIndex_positionAtByte(index, 5u, POSITION_ENCODING_UTF8);
    assert(position.valid && position.line == 0u && position.character == 5u);
    position = LineIndex_positionAtByte(index, 5u, POSITION_ENCODING_UTF16);
    assert(position.valid && position.character == 3u);
    position = LineIndex_positionAtByte(index, 5u, POSITION_ENCODING_UTF32);
    assert(position.valid && position.character == 2u);
    position = LineIndex_positionAtByte(index, 6u, POSITION_ENCODING_UTF16);
    assert(position.valid && position.line == 0u && position.character == 3u);
    position = LineIndex_positionAtByte(index, 7u, POSITION_ENCODING_UTF16);
    assert(position.valid && position.line == 1u && position.character == 0u);
    position = LineIndex_positionAtByte(index, 9u, POSITION_ENCODING_UTF8);
    assert(position.valid && position.line == 1u && position.character == 2u);
    assert(!LineIndex_positionAtByte(index, 2u, POSITION_ENCODING_UTF16).valid);
    assert(!LineIndex_positionAtByte(index, 10u, POSITION_ENCODING_UTF16).valid);
    LineIndex_free(index);

    snapshot = TextSnapshot(32u, 1u, nullptr, 0u);
    index = LineIndex_1(snapshot);
    assert(index != nullptr && LineIndex_getLineCount(index) == 1u);
    position = LineIndex_positionAtByte(index, 0u, POSITION_ENCODING_UTF16);
    assert(position.valid && position.line == 0u && position.character == 0u);
    LineIndex_free(index);
    TextSnapshot_free(snapshot);
    return 0;
}
