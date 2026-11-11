#ifndef LANGUAGE_DOCUMENT_SOURCE_POSITION_H
#define LANGUAGE_DOCUMENT_SOURCE_POSITION_H

#include <stdbool.h>
#include <stddef.h>

typedef struct SourcePosition {
    bool valid;
    bytes line;
    bytes character;
} SourcePosition;

SourcePosition SourcePosition_0(void);
SourcePosition SourcePosition_3(bool valid, bytes line, bytes character);
SourcePosition SourcePosition_zero(void);

#endif
