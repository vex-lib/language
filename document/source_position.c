#include "document/source_position.h"
#include "annotation/annotation.h"

;;DEFINITION
/** SourcePosition is a zero-based line and negotiated character position.
 * Invalid byte boundaries use the all-zero invalid form. */

;;OVERVIEW
/** CLASS: SourcePosition (document/source_position.c)
 * STRUCT FIELDS: valid, line, character.
 * FUNCTION REGISTRY: zero and complete value constructors.
 * CONTRACT: invalid construction canonicalizes coordinates to zero. */

SourcePosition SourcePosition_0(void) { return (SourcePosition) {0}; }
SourcePosition SourcePosition_3(bool valid, size_t line, size_t character) {
    return valid ? (SourcePosition) {.valid = true, .line = line, .character = character}
                 : SourcePosition_0();
}
SourcePosition SourcePosition_zero(void) { return SourcePosition_0(); }
