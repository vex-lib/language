#ifndef LANGUAGE_DOCUMENT_LINE_INDEX_H
#define LANGUAGE_DOCUMENT_LINE_INDEX_H

#include "document/source_position.h"
#include "document/text_snapshot.h"
#include <stddef.h>

typedef struct LineIndex LineIndex;

typedef enum PositionEncoding {
    POSITION_ENCODING_UTF8 = 0,
    POSITION_ENCODING_UTF16,
    POSITION_ENCODING_UTF32
} PositionEncoding;

LineIndex *LineIndex_1(TextSnapshot *snapshot);
LineIndex *LineIndex_zero(void);
void LineIndex_free(LineIndex *self);
SourcePosition LineIndex_positionAtByte(const LineIndex *self, bytes byteOffset,
                                        PositionEncoding encoding);
bytes LineIndex_getLineCount(const LineIndex *self);
u32 LineIndex_getDocumentId(const LineIndex *self);
u64 LineIndex_getRevision(const LineIndex *self);

#endif
