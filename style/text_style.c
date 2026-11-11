#include "style/text_style.h"
#include "annotation/annotation.h"
#include <string.h>

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: TextStyle
 * ============================================================================
 * TextStyle is one themed appearance: ink, background, decoration and its
 * colour, plus attribute flags. It is a value record with no device knowledge;
 * a terminal, a GUI cell renderer or a web surface maps it, and the language
 * repository never names a colour for a code class.
 *
 * The inherit sentinel is what makes composition work. A diagnostic style can
 * leave its foreground DEFAULT so the syntax colour survives underneath and
 * only the warning squiggle is added, or set a red foreground so an error
 * replaces the syntax colour. Decoration is a kind rather than a bit because
 * CURLY_UNDERLINE and UNDERLINE are different drawings, and decorationColor is
 * separate so a yellow squiggle can sit under syntax-coloured text.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: TextStyle (style/text_style.c)
 * ============================================================================
 * STRUCT FIELDS (Mirroring style/text_style.h)
 * ----------------------------------------------------------------------------
 *   TextStyle {
 *     u32 fg;                     // packed 0xRRGGBBAA; DEFAULT inherits
 *     u32 bg;                     // packed 0xRRGGBBAA; DEFAULT inherits
 *     u32 decorationColor;        // packed; DEFAULT inherits
 *     u16 flags;                  // BOLD/ITALIC/DIM/REVERSE/BLINK/HIDDEN
 *     TextStyleDecoration decoration;  // NONE/underline variants/strike
 *   }
 *
 * PRIVATE HELPERS
 *   static const char *decorationName(TextStyleDecoration decoration)
 *     Stable spelling used by the projections; unknown values report "unknown".
 *
 * FUNCTION REGISTRY
 *   Constructors: TextStyle_0, _2, _5, TextStyle(...), TextStyle_zero
 *   Accessors:    get/setForeground, get/setBackground,
 *                 get/setDecorationColor, get/setFlags, get/setDecoration,
 *                 hasFlag
 *   Composition:  TextStyle_overlay(base, over) -> TextStyle
 *   Projections:  toString, toStringStruct
 *
 * CONTRACT: overlay treats TEXT_STYLE_COLOR_DEFAULT as "keep the base", unions
 * flags, and replaces the decoration only when `over` names one. No allocation,
 * no logging, null-safe getters.
 * ============================================================================
 */

static const char *decorationName(TextStyleDecoration decoration) {
    switch (decoration) {
        case TEXT_DECORATION_NONE: return "none";
        case TEXT_DECORATION_UNDERLINE: return "underline";
        case TEXT_DECORATION_f64_UNDERLINE: return "d64-underline";
        case TEXT_DECORATION_CURLY_UNDERLINE: return "curly-underline";
        case TEXT_DECORATION_DOTTED_UNDERLINE: return "dotted-underline";
        case TEXT_DECORATION_DASHED_UNDERLINE: return "dashed-underline";
        case TEXT_DECORATION_STRIKE: return "strike";
    }
    return "unknown";
}

TextStyle TextStyle_0(void) { return (TextStyle) {0}; }
TextStyle TextStyle_zero(void) { return TextStyle_0(); }

TextStyle TextStyle_2(u32 fg, u32 bg) {
    return (TextStyle) {.fg = fg, .bg = bg};
}

TextStyle TextStyle_5(u32 fg, u32 bg, u32 decorationColor,
                      u16 flags, TextStyleDecoration decoration) {
    return (TextStyle) {
        .fg = fg,
        .bg = bg,
        .decorationColor = decorationColor,
        .flags = flags,
        .decoration = decoration
    };
}

u32 TextStyle_getForeground(const TextStyle *self) {
    return self == nullptr ? TEXT_STYLE_COLOR_DEFAULT : (*self).fg;
}
void TextStyle_setForeground(TextStyle *self, u32 value) {
    if (self != nullptr)
        (*self).fg = value;
}
u32 TextStyle_getBackground(const TextStyle *self) {
    return self == nullptr ? TEXT_STYLE_COLOR_DEFAULT : (*self).bg;
}
void TextStyle_setBackground(TextStyle *self, u32 value) {
    if (self != nullptr)
        (*self).bg = value;
}
u32 TextStyle_getDecorationColor(const TextStyle *self) {
    return self == nullptr ? TEXT_STYLE_COLOR_DEFAULT : (*self).decorationColor;
}
void TextStyle_setDecorationColor(TextStyle *self, u32 value) {
    if (self != nullptr)
        (*self).decorationColor = value;
}
u16 TextStyle_getFlags(const TextStyle *self) {
    return self == nullptr ? 0u : (*self).flags;
}
void TextStyle_setFlags(TextStyle *self, u16 value) {
    if (self != nullptr)
        (*self).flags = value;
}
TextStyleDecoration TextStyle_getDecoration(const TextStyle *self) {
    return self == nullptr ? TEXT_DECORATION_NONE : (*self).decoration;
}
void TextStyle_setDecoration(TextStyle *self, TextStyleDecoration value) {
    if (self != nullptr)
        (*self).decoration = value;
}

bool TextStyle_hasFlag(const TextStyle *self, u16 flag) {
    if (self == nullptr || flag == 0u)
        return false;
    return ((*self).flags & flag) == flag;
}

TextStyle TextStyle_overlay(TextStyle base, TextStyle over) {
    TextStyle result = base;
    if ((*((&over))).fg != TEXT_STYLE_COLOR_DEFAULT)
        (*((&result))).fg = (*((&over))).fg;
    if ((*((&over))).bg != TEXT_STYLE_COLOR_DEFAULT)
        (*((&result))).bg = (*((&over))).bg;
    if ((*((&over))).decorationColor != TEXT_STYLE_COLOR_DEFAULT)
        (*((&result))).decorationColor = (*((&over))).decorationColor;
    (*((&result))).flags = (u16) ((*((&base))).flags | (*((&over))).flags);
    if ((*((&over))).decoration != TEXT_DECORATION_NONE)
        (*((&result))).decoration = (*((&over))).decoration;
    return result;
}

BoundedText TextStyle_toString(const TextStyle *self) {
    if (self == nullptr)
        return BoundedText_1("nullptr");
    return BoundedText_format("TextStyle(fg=0x%08X, bg=0x%08X, deco=%s, decoColor=0x%08X, flags=0x%04X)",
                              (*self).fg, (*self).bg, decorationName((*self).decoration),
                              (*self).decorationColor, (unsigned) (*self).flags);
}

BoundedText TextStyle_toStringStruct(const TextStyle *self) {
    if (self == nullptr)
        return BoundedText_1("nullptr");
    return BoundedText_format("TextStyle { fg=0x%08X, bg=0x%08X, decorationColor=0x%08X, flags=0x%04X, decoration=%s }",
                              (*self).fg, (*self).bg, (*self).decorationColor,
                              (unsigned) (*self).flags, decorationName((*self).decoration));
}
