#include "vex.h"
#include "document/line_index.h"
#include <stdlib.h>

;;DEFINITION
/** LineIndex stores line starts for one retained immutable TextSnapshot and
 * converts byte offsets to zero-based UTF-8/UTF-16/UTF-32 positions. A byte
 * offset inside a UTF-8 scalar or malformed text is rejected. CRLF is one line
 * ending and both ending bytes map to the content end of the preceding line. */

;;OVERVIEW
/** CLASS: LineIndex (document/line_index.c)
 * STRUCT FIELDS: retained snapshot, line-start byte array and line count.
 * PRIVATE HELPERS: decodeScalar validates one Unicode scalar and byte extent.
 * FUNCTION REGISTRY: constructor/free/positionAtByte and identity getters.
 * CONTRACT: line numbering is zero-based; UTF16 supplementary scalars count 2;
 * invalid input returns SourcePosition_zero and reports a cold rejection. */

struct LineIndex {
    TextSnapshot *snapshot;
    bytes *lineStarts;
    bytes lineCount;
};

typedef struct ScalarStep {
    bool valid;
    bytes byteCount;
    bytes utf16Units;
} ScalarStep;

static ScalarStep decodeScalar(const u8 *data, bytes length, bytes at) {
    u8 lead = data[at];
    if (lead < 0x80u)
        return (ScalarStep) {.valid = true, .byteCount = 1u, .utf16Units = 1u};
    if (lead >= 0xC2u && lead <= 0xDFu) {
        if (at + 1u < length && data[at + 1u] >= 0x80u && data[at + 1u] <= 0xBFu)
            return (ScalarStep) {.valid = true, .byteCount = 2u, .utf16Units = 1u};
        return (ScalarStep) {0};
    }
    if (lead >= 0xE0u && lead <= 0xEFu && at + 2u < length) {
        u8 second = data[at + 1u];
        u8 third = data[at + 2u];
        bool valid = second >= 0x80u && second <= 0xBFu
            && third >= 0x80u && third <= 0xBFu
            && !(lead == 0xE0u && second < 0xA0u)
            && !(lead == 0xEDu && second > 0x9Fu);
        if (valid)
            return (ScalarStep) {.valid = true, .byteCount = 3u, .utf16Units = 1u};
        return (ScalarStep) {0};
    }
    if (lead >= 0xF0u && lead <= 0xF4u && at + 3u < length) {
        u8 second = data[at + 1u];
        bool valid = second >= 0x80u && second <= 0xBFu
            && data[at + 2u] >= 0x80u && data[at + 2u] <= 0xBFu
            && data[at + 3u] >= 0x80u && data[at + 3u] <= 0xBFu
            && !(lead == 0xF0u && second < 0x90u)
            && !(lead == 0xF4u && second > 0x8Fu);
        if (valid)
            return (ScalarStep) {.valid = true, .byteCount = 4u, .utf16Units = 2u};
    }
    return (ScalarStep) {0};
}

;;GETTER
static inline TextSnapshot *LineIndex_snapshot(const LineIndex *self) { return (*self).snapshot; }
;;SETTER
static inline fn LineIndex_setSnapshot(LineIndex *self, TextSnapshot *v) { (*self).snapshot = v; }
;;GETTER
static inline bytes *LineIndex_lineStarts(const LineIndex *self) { return (*self).lineStarts; }
;;SETTER
static inline fn LineIndex_setLineStarts(LineIndex *self, bytes *v) { (*self).lineStarts = v; }
;;GETTER
static inline bytes LineIndex_lineCount(const LineIndex *self) { return (*self).lineCount; }
;;SETTER
static inline fn LineIndex_setLineCount(LineIndex *self, bytes v) { (*self).lineCount = v; }

LineIndex *LineIndex_1(TextSnapshot *snapshot) {
    if (snapshot == nullptr || !TextSnapshot_retain(snapshot))
        return nullptr;
    bytes length = TextSnapshot_getLength(snapshot);
    const char *data = TextSnapshot_getBytes(snapshot);
    bytes lineCount = 1u;
    for (bytes index = 0u; index < length; ++index)
        if (data[index] == '\n')
            ++lineCount;
    if (lineCount > SIZE_MAX / sizeof(bytes)) {
        THROW("LineIndex: line-start table size overflow");
        TextSnapshot_free(snapshot);
        return nullptr;
    }
    LineIndex *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("LineIndex: allocation failed for metadata");
        TextSnapshot_free(snapshot);
        return nullptr;
    }
    LineIndex_setLineStarts(self, malloc(lineCount * sizeof(bytes)));
    if (LineIndex_lineStarts(self) == nullptr) {
        THROW("LineIndex: allocation failed for %zu line starts", lineCount);
        TextSnapshot_free(snapshot);
        free(self);
        return nullptr;
    }
    LineIndex_setSnapshot(self, snapshot);
    LineIndex_setLineCount(self, lineCount);
    LineIndex_lineStarts(self)[0] = 0u;
    bytes next = 1u;
    for (bytes index = 0u; index < length; ++index)
        if (data[index] == '\n')
            LineIndex_lineStarts(self)[next++] = index + 1u;
    return self;
}

LineIndex *LineIndex_zero() { return nullptr; }
fn LineIndex_free(LineIndex *self) {
    if (self != nullptr) {
        TextSnapshot_free(LineIndex_snapshot(self));
        LineIndex_setSnapshot(self, nullptr);
        free(LineIndex_lineStarts(self));
        LineIndex_setLineStarts(self, nullptr);
        free(self);
    }
}

SourcePosition LineIndex_positionAtByte(const LineIndex *self, bytes byteOffset,
                                         PositionEncoding encoding) {
    if (self == nullptr || encoding < POSITION_ENCODING_UTF8
        || encoding > POSITION_ENCODING_UTF32
        || byteOffset > TextSnapshot_getLength(LineIndex_snapshot(self))) {
        THROW("LineIndex: rejected byte offset or position encoding");
        return SourcePosition_zero();
    }
    const char *source = TextSnapshot_getBytes(LineIndex_snapshot(self));
    bytes length = TextSnapshot_getLength(LineIndex_snapshot(self));
    bytes low = 0u;
    bytes high = LineIndex_lineCount(self);
    while (low + 1u < high) {
        bytes middle = low + (high - low) / 2u;
        if (LineIndex_lineStarts(self)[middle] <= byteOffset)
            low = middle;
        else
            high = middle;
    }
    bytes lineStart = LineIndex_lineStarts(self)[low];
    bytes lineEnd = low + 1u < LineIndex_lineCount(self)
        ? LineIndex_lineStarts(self)[low + 1u] - 1u : length;
    if (lineEnd > lineStart && source[lineEnd - 1u] == '\r')
        --lineEnd;
    bytes target = byteOffset > lineEnd ? lineEnd : byteOffset;
    bytes offset = lineStart;
    bytes utf16 = 0u;
    bytes utf32 = 0u;
    while (offset < target) {
        ScalarStep scalar = decodeScalar((const u8*) source, lineEnd, offset);
        if (!scalar.valid || scalar.byteCount > target - offset) {
            THROW("LineIndex: byte offset is not a valid UTF-8 scalar boundary");
            return SourcePosition_zero();
        }
        offset += scalar.byteCount;
        utf16 += scalar.utf16Units;
        ++utf32;
    }
    if (offset != target) {
        THROW("LineIndex: byte offset splits a UTF-8 scalar");
        return SourcePosition_zero();
    }
    bytes character = encoding == POSITION_ENCODING_UTF8 ? target - lineStart
        : encoding == POSITION_ENCODING_UTF16 ? utf16 : utf32;
    return SourcePosition_3(true, low, character);
}

;;GETTER
bytes LineIndex_getLineCount(const LineIndex *self) {
    return self == nullptr ? 0u : LineIndex_lineCount(self);
}
u32 LineIndex_getDocumentId(const LineIndex *self) {
    return self == nullptr ? 0u : TextSnapshot_getDocumentId(LineIndex_snapshot(self));
}
u64 LineIndex_getRevision(const LineIndex *self) {
    return self == nullptr ? 0u : TextSnapshot_getRevision(LineIndex_snapshot(self));
}
