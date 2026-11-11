#include "text/bounded_text.h"
#include "annotation/annotation.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: BoundedText
 * ============================================================================
 * BoundedText is the language repository's returned-string value. Every
 * projection (toString, diagnostic message, hint text) writes into one fixed
 * inline buffer and reports whether the source fit. It owns its bytes, frees
 * nothing, allocates nothing, and keeps no pointer into another object.
 *
 * The record exists so no API needs a caller-provided destination parameter:
 * the caller receives the value and stores it. Truncation is a first-class
 * field because a silently shortened message is a defect.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: BoundedText (text/bounded_text.c)
 * ============================================================================
 * STRUCT FIELDS (Mirroring text/bounded_text.h)
 * ----------------------------------------------------------------------------
 *   BoundedText {
 *     char text[BOUNDED_TEXT_CAPACITY]; // NUL-terminated stored bytes
 *     bytes length;                    // stored byte count, excluding NUL
 *     bool truncated;                   // source did not fit
 *   }
 *
 * PRIVATE HELPERS
 *   static bytes copyBytes(char *dst, bytes capacity, const char *src,
 *                           bytes length)
 *     Copy at most capacity-1 bytes, always NUL-terminate, return stored count.
 *   static bytes appendEscaped(char *dst, bytes capacity, bytes at,
 *                               const char *src, bytes length)
 *     Append a quoted/escaped rendering, bounded.
 *
 * FUNCTION REGISTRY
 *   Constructors: BoundedText_0, _1, _2, BoundedText(...), BoundedText_zero
 *   Formatting:   BoundedText_format
 *   Accessors:    getText, getLength, isTruncated, equals
 *   Projections:  toString, toStringStruct
 *
 * CONTRACT: null input yields the empty value. No allocation, no logging. A
 * source longer than the buffer stores capacity-1 bytes and sets truncated.
 * ============================================================================
 */

static bytes copyBytes(char *dst, bytes capacity, const char *src, bytes length) {
    if (capacity == 0u)
        return 0u;
    bytes stored = length < capacity - 1u ? length : capacity - 1u;
    if (stored > 0u && src != nullptr)
        memcpy(dst, src, stored);
    dst[stored] = '\0';
    return stored;
}

static bytes appendEscaped(char *dst, bytes capacity, bytes at,
                            const char *src, bytes length) {
    for (bytes i = 0u; i < length && at + 1u < capacity; ++i) {
        char c = src[i];
        const char *escape = nullptr;
        switch (c) {
            case '\n': escape = "\\n"; break;
            case '\t': escape = "\\t"; break;
            case '\r': escape = "\\r"; break;
            case '"':  escape = "\\\""; break;
            case '\\': escape = "\\\\"; break;
            default: break;
        }
        if (escape != nullptr) {
            for (bytes j = 0u; escape[j] != '\0' && at + 1u < capacity; ++j)
                dst[at++] = escape[j];
        } else {
            dst[at++] = c;
        }
    }
    dst[at] = '\0';
    return at;
}

BoundedText BoundedText_0(void) { return (BoundedText) {0}; }
BoundedText BoundedText_zero(void) { return BoundedText_0(); }

BoundedText BoundedText_1(const char *text) {
    if (text == nullptr)
        return BoundedText_0();
    return BoundedText_2(text, strlen(text));
}

BoundedText BoundedText_2(const char *text, bytes length) {
    BoundedText self = {0};
    if (text == nullptr || length == 0u)
        return self;
    (*(&self)).length = copyBytes((*(&self)).text, BOUNDED_TEXT_CAPACITY, text, length);
    (*(&self)).truncated = length > (*(&self)).length;
    return self;
}

BoundedText BoundedText_format(const char *format, ...) {
    BoundedText self = {0};
    if (format == nullptr)
        return self;
    va_list arguments;
    va_start(arguments, format);
    int written = vsnprintf((*((&self))).text, BOUNDED_TEXT_CAPACITY, format, arguments);
    va_end(arguments);
    if (written < 0) {
        (*((&self))).text[0] = '\0';
        (*((&self))).truncated = true;
        return self;
    }
    if ((bytes) written >= BOUNDED_TEXT_CAPACITY) {
        (*((&self))).length = BOUNDED_TEXT_CAPACITY - 1u;
        (*((&self))).truncated = true;
    } else {
        (*((&self))).length = (bytes) written;
    }
    return self;
}

const char *BoundedText_getText(const BoundedText *self) {
    return self == nullptr ? "" : (*self).text;
}

bytes BoundedText_getLength(const BoundedText *self) {
    return self == nullptr ? 0u : (*self).length;
}

bool BoundedText_isTruncated(const BoundedText *self) {
    return self == nullptr ? false : (*self).truncated;
}

bool BoundedText_equals(const BoundedText *self, const char *text) {
    if (self == nullptr || text == nullptr)
        return false;
    bytes length = strlen(text);
    return length == (*self).length && memcmp((*self).text, text, length) == 0;
}

BoundedText BoundedText_toString(const BoundedText *self) {
    if (self == nullptr)
        return BoundedText_1("nullptr");
    char buffer[BOUNDED_TEXT_CAPACITY];
    bytes at = 0u;
    if (at + 1u < sizeof buffer)
        buffer[at++] = '"';
    at = appendEscaped(buffer, sizeof buffer, at, (*self).text, (*self).length);
    if (at + 1u < sizeof buffer)
        buffer[at++] = '"';
    buffer[at] = '\0';
    BoundedText result = BoundedText_2(buffer, at);
    if ((*self).truncated)
        (*((&result))).truncated = true;
    return result;
}

BoundedText BoundedText_toStringStruct(const BoundedText *self) {
    if (self == nullptr)
        return BoundedText_1("nullptr");
    return BoundedText_format("BoundedText { length=%zu, truncated=%s }",
                              (*self).length, (*self).truncated ? "true" : "false");
}
