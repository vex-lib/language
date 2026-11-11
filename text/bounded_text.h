#ifndef LANGUAGE_TEXT_BOUNDED_TEXT_H
#define LANGUAGE_TEXT_BOUNDED_TEXT_H

#include <stdbool.h>
#include <stddef.h>

// text/bounded_text.h — a returned, bounded string VALUE.
//
// Language projections (toString, diagnostic messages, hint text) never take a
// caller destination and never allocate. They return a BoundedText: a fixed
// inline buffer, the byte length actually stored, and an explicit truncation
// flag. The flag is the contract — a cut is reported, never silently shortened
// (the Cold-Strict, Hot-Minimal Validation Law's truncation clause).
//
// The record is deliberately small: it is a projection for humans and tools,
// not a document. It owns its bytes and frees nothing.

#define BOUNDED_TEXT_CAPACITY 128u

typedef struct BoundedText {
    char text[BOUNDED_TEXT_CAPACITY]; // NUL-terminated stored bytes
    bytes length;                    // stored byte count, excluding the NUL
    bool truncated;                   // the source did not fit
} BoundedText;

// Construct empty, a copied C string, or a copied byte span. Truncation is
// flagged, never silent. A null source or zero length yields the empty value.
BoundedText BoundedText_0(void);
BoundedText BoundedText_1(const char *text);
BoundedText BoundedText_2(const char *text, bytes length);
// Dispatch BoundedText(), BoundedText(text), or BoundedText(text, length).
#define BoundedText(...) BOUNDED_TEXT_SELECT(_ __VA_OPT__(,) __VA_ARGS__, BoundedText_2, BoundedText_1, BoundedText_0)(__VA_ARGS__)
#define BOUNDED_TEXT_SELECT(_0, _1, _2, NAME, ...) NAME
// Return the empty value.
BoundedText BoundedText_zero(void);
// Format into the bounded buffer; a cut reports truncation.
BoundedText BoundedText_format(const char *format, ...);

// Symmetric accessors. Null getters return the empty value/false.
const char *BoundedText_getText(const BoundedText *self);
bytes BoundedText_getLength(const BoundedText *self);
bool BoundedText_isTruncated(const BoundedText *self);
bool BoundedText_equals(const BoundedText *self, const char *text);

// Bounded projections. toString quotes and escapes; toStringStruct lists the
// record's own fields.
BoundedText BoundedText_toString(const BoundedText *self);
BoundedText BoundedText_toStringStruct(const BoundedText *self);

#endif
