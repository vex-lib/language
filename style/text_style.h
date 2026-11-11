#ifndef LANGUAGE_STYLE_TEXT_STYLE_H
#define LANGUAGE_STYLE_TEXT_STYLE_H

#include <stdbool.h>
#include <stdint.h>

#include "text/bounded_text.h"

// style/text_style.h — the themed appearance of one style key.
//
// A TextStyle is a VALUE: foreground, background, a decoration with its own
// colour, and attribute flags. It names no terminal, no ANSI escape and no
// library — a device backend maps it, exactly like determinant's TerminalPen.
//
// Colour is packed 0xRRGGBBAA (the Strict 0xRRGGBBAA Color Law). The sentinel
// TEXT_STYLE_COLOR_DEFAULT is fully transparent (alpha 0) and means INHERIT:
// the layer below supplies the value. Opaque black is 0x000000FF and is a real
// colour, not the sentinel.
//
// Decoration is a kind, not an attribute bit, because a warning squiggle is a
// CURVE under text whose colour may differ from the text itself.

#define TEXT_STYLE_COLOR_DEFAULT 0x00000000u

#define TEXT_STYLE_BOLD   0x0001u
#define TEXT_STYLE_ITALIC 0x0002u
#define TEXT_STYLE_DIM    0x0004u
#define TEXT_STYLE_REVERSE 0x0008u
#define TEXT_STYLE_BLINK  0x0010u
#define TEXT_STYLE_HIDDEN 0x0020u

typedef enum TextStyleDecoration {
    TEXT_DECORATION_NONE = 0,
    TEXT_DECORATION_UNDERLINE,
    TEXT_DECORATION_f64_UNDERLINE,
    TEXT_DECORATION_CURLY_UNDERLINE,   // the warning squiggle
    TEXT_DECORATION_DOTTED_UNDERLINE,
    TEXT_DECORATION_DASHED_UNDERLINE,
    TEXT_DECORATION_STRIKE
} TextStyleDecoration;

typedef struct TextStyle {
    u32 fg;               // 0xRRGGBBAA, DEFAULT means inherit
    u32 bg;               // 0xRRGGBBAA, DEFAULT means inherit
    u32 decorationColor;  // DEFAULT means inherit (usually the resolved fg)
    u16 flags;            // TEXT_STYLE_* bits
    TextStyleDecoration decoration; // what is drawn under/through the text
} TextStyle;

// Construct empty, colour-only, or the complete style. Other arities reject.
TextStyle TextStyle_0(void);
TextStyle TextStyle_2(u32 fg, u32 bg);
TextStyle TextStyle_5(u32 fg, u32 bg, u32 decorationColor,
                      u16 flags, TextStyleDecoration decoration);
#define TextStyle(...) TEXT_STYLE_SELECT(_ __VA_OPT__(,) __VA_ARGS__, TextStyle_5, TextStyle_invalid_arity, TextStyle_invalid_arity, TextStyle_2, TextStyle_invalid_arity, TextStyle_0)(__VA_ARGS__)
#define TEXT_STYLE_SELECT(_0, _1, _2, _3, _4, _5, NAME, ...) NAME
// Return the all-default (inherit everything) value.
TextStyle TextStyle_zero(void);

// Symmetric field accessors. A null self is safe: getters return the default,
// setters do nothing.
u32 TextStyle_getForeground(const TextStyle *self);
void TextStyle_setForeground(TextStyle *self, u32 value);
u32 TextStyle_getBackground(const TextStyle *self);
void TextStyle_setBackground(TextStyle *self, u32 value);
u32 TextStyle_getDecorationColor(const TextStyle *self);
void TextStyle_setDecorationColor(TextStyle *self, u32 value);
u16 TextStyle_getFlags(const TextStyle *self);
void TextStyle_setFlags(TextStyle *self, u16 value);
TextStyleDecoration TextStyle_getDecoration(const TextStyle *self);
void TextStyle_setDecoration(TextStyle *self, TextStyleDecoration value);

// True when every bit of `flag` is set.
bool TextStyle_hasFlag(const TextStyle *self, u16 flag);

// Compose `over` on top of `base`: a set colour replaces, an inheriting colour
// keeps the base, flags union, and a decoration replaces only when it is not
// NONE. Both arguments are values; the result is a new value.
TextStyle TextStyle_overlay(TextStyle base, TextStyle over);

BoundedText TextStyle_toString(const TextStyle *self);
BoundedText TextStyle_toStringStruct(const TextStyle *self);

#endif
