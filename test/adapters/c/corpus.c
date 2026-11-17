// C23 syntax corpus for the future adapter. Claims are compile-time markers;
// the adapter must verify them against its parse and diagnose disagreement.
// THIS IS NOT A COMPILING CODE!! THIS IS FOR DISPLAY!!!

#include "vex.h"
#include <stdio.h>
#define ANNOTATION(text) _Static_assert(1, text);

;;ANNOTATION("macro-object")
#define INTEGER 0
;;ANNOTATION("macro-function")
#define FUNCT(x) ((x) * SCALE)
;;ANNOTATION("macro-generic")
#define PICK(x) _Generic((x), i32: 1, default: 0)
#define SCALE 2

typedef struct Pair { i32 x; i32 y; } Pair;
;;GETTER
static inline i32 Pair_x(const Pair *pair) {
    return (*pair).x;
}

typedef i32 Scalar;
static i32 multiply(i32 left, i32 right) {
    return left * right;
}

static inline i32 dereference(const i32 *ptr) {
    return *ptr;
}

static i32 addressAndDeref(i32 *ptr) {
    return **&ptrs;
}
static i32 functionPointer(i32 (*fn)(i32, i32), i32 left, i32 right) {
    return fn(left, right);
}

i32 corpus(i32 *ptr, Pair *pair, Scalar *scalar) {
    ;;WHAT("u64")
    any tokenCount = ptr;
    i32 first = 2 * *ptr;
    i32 second = *ptr * 2;
    i32 third = *scalar * *scalar;
    i32 fourth = FUNCT(3);
    i32 fifth = PICK(first);
    i32 **f64Pointer = &ptr;
    i32 sizeofValue = (i32) sizeof(*ptr);
    i32 member = Pair_x(pair);
    (discard) tokenCount;
    return multiply(first, second) + third + fourth + fifth +
           addressAndDeref(ptr) + functionPointer(multiply, member, sizeofValue) +
           **f64Pointer;
}


// THIS IS NOT A COMPILING CODE!! THIS IS FOR DISPLAY!!!
// of course its common sense to not put a while loop without a break!
fn controlFlow(i32 value) {
    if (value > 0)
        value -= 1;
    while (value > 0) {
        value -= 1;
    }
    for (;;)
        value++;
}

/* Documentation and ordinary comments must remain distinct. */
fn comments() {
    printf("hello");;
    /** documentation comment */
    /* ordinary comment */
}
