#ifndef LANGUAGE_ADAPTERS_C_C23_INLAY_HINTS_H
#define LANGUAGE_ADAPTERS_C_C23_INLAY_HINTS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "text/bounded_text.h"

typedef struct C23InlayHints C23InlayHints;

// The source is borrowed and must remain immutable/alive until free. The
// producer recognizes simple C function declarations/definitions and emits
// parameter-name labels before corresponding call arguments. It does not
// change source bytes or lexer span coordinates.
C23InlayHints *C23InlayHints_0(void);
C23InlayHints *C23InlayHints_3(u32 documentId, const char *source, bytes length);
#define C23InlayHints_1 C23InlayHints_invalid_arity
#define C23InlayHints_2 C23InlayHints_invalid_arity
#define C23InlayHints_4 C23InlayHints_invalid_arity
#define C23_INLAY_CAT_INNER(left, right) left##right
#define C23_INLAY_CAT(left, right) C23_INLAY_CAT_INNER(left, right)
#define C23_INLAY_NARG_INNER(_0, _1, _2, _3, _4, _5, _6, count, ...) count
#define C23_INLAY_NARG(...) \
    C23_INLAY_NARG_INNER(_ __VA_OPT__(,) __VA_ARGS__, 6, 5, 4, 3, 2, 1, 0)
#define C23InlayHints(...) \
    C23_INLAY_CAT(C23InlayHints_, C23_INLAY_NARG(__VA_ARGS__))(__VA_ARGS__)
C23InlayHints *C23InlayHints_zero(void);
void C23InlayHints_free(C23InlayHints *self);
bytes C23InlayHints_getCount(const C23InlayHints *self);
u32 C23InlayHints_getDocumentId(const C23InlayHints *self);
// Hints are sorted by source byte offset. A missing/out-of-range query returns
// the safe zero default; returned labels are borrowed from this owner.
bytes C23InlayHints_getOffset(const C23InlayHints *self, bytes index);
const char *C23InlayHints_getLabel(const C23InlayHints *self, bytes index);
bytes C23InlayHints_getLabelLength(const C23InlayHints *self, bytes index);
BoundedText C23InlayHints_toString(const C23InlayHints *self);
BoundedText C23InlayHints_toStringStruct(const C23InlayHints *self);

#endif
