#include "style/text_style.h"
#include <assert.h>

int main(void) {
    TextStyle empty = TextStyle();
    assert(TextStyle_getForeground(&empty) == TEXT_STYLE_COLOR_DEFAULT);
    assert(TextStyle_getBackground(&empty) == TEXT_STYLE_COLOR_DEFAULT);
    assert(TextStyle_getDecorationColor(&empty) == TEXT_STYLE_COLOR_DEFAULT);
    assert(TextStyle_getFlags(&empty) == 0u);
    assert(TextStyle_getDecoration(&empty) == TEXT_DECORATION_NONE);
    TextStyle base = TextStyle(0x112233FFu, TEXT_STYLE_COLOR_DEFAULT);
    TextStyle warning = TextStyle(TEXT_STYLE_COLOR_DEFAULT, TEXT_STYLE_COLOR_DEFAULT,
                                  0xFFFF00FFu, TEXT_STYLE_ITALIC,
                                  TEXT_DECORATION_CURLY_UNDERLINE);
    TextStyle result = TextStyle_overlay(base, warning);
    assert(TextStyle_getForeground(&result) == 0x112233FFu);
    assert(TextStyle_getDecorationColor(&result) == 0xFFFF00FFu);
    assert(TextStyle_hasFlag(&result, TEXT_STYLE_ITALIC));
    assert(TextStyle_getDecoration(&result) == TEXT_DECORATION_CURLY_UNDERLINE);
    TextStyle error = TextStyle(0xFF0000FFu, TEXT_STYLE_COLOR_DEFAULT);
    result = TextStyle_overlay(result, error);
    assert(TextStyle_getForeground(&result) == 0xFF0000FFu);
    assert(TextStyle_getForeground(nullptr) == TEXT_STYLE_COLOR_DEFAULT);
    return 0;
}
