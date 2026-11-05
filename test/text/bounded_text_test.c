#include "text/bounded_text.h"
#include <assert.h>
#include <string.h>

int main(void) {
    BoundedText empty = BoundedText();
    assert(BoundedText_getLength(&empty) == 0u);
    BoundedText text = BoundedText("hello");
    assert(BoundedText_equals(&text, "hello"));
    assert(!BoundedText_isTruncated(&text));
    char longText[BOUNDED_TEXT_CAPACITY + 4u];
    memset(longText, 'x', sizeof longText);
    longText[sizeof longText - 1u] = '\0';
    BoundedText cut = BoundedText(longText);
    assert(BoundedText_isTruncated(&cut));
    assert(BoundedText_getLength(&cut) == BOUNDED_TEXT_CAPACITY - 1u);
    assert(BoundedText_equals(nullptr, "x") == false);
    return 0;
}
