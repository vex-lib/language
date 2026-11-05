// C23 syntax corpus for the future adapter. Claims are compile-time markers;
// the adapter must verify them against its parse and diagnose disagreement.
#include <stdio.h>
#define ANNOTATION(text) _Static_assert(1, text);
#define WHAT(text) _Static_assert(1, text);

;;ANNOTATION("macro-object")
#define INTEGER 0
;;ANNOTATION("macro-function")
#define FUNCT(x) ((x) * SCALE)
;;ANNOTATION("macro-generic")
#define PICK(x) _Generic((x), int: 1, default: 0)
#define SCALE 2

typedef struct Pair { int x; int y; } Pair;
typedef int Scalar;
static int multiply(int left, int right) { return left * right; }
int dereference(const int *ptr) { return *ptr; }
static int addressAndDeref(int *ptr) { return **&ptr; }
static int functionPointer(int (*fn)(int, int), int left, int right) {
    return fn(left, right);
}

int corpus(int *ptr, Pair *pair, Scalar *scalar) {
    ;;WHAT("uint64_t")
    void *tokenCount = ptr;
    int first = 2 * *ptr;
    int second = *ptr * 2;
    int third = *scalar * *scalar;
    int fourth = FUNCT(3);
    int fifth = PICK(first);
    int **doublePointer = &ptr;
    int sizeofValue = (int) sizeof *ptr;
    int member = (*pair).x;
    (void) tokenCount;
    return multiply(first, second) + third + fourth + fifth +
           addressAndDeref(ptr) + functionPointer(multiply, member, sizeofValue) +
           **doublePointer;
}

void controlFlow(int value) {
    if (value > 0)
        value -= 1;
    for (;;)
        break;
    while (value > 0) {
        value -= 1;
    }
}

/* Documentation and ordinary comments must remain distinct. */
void comments(void) {
    printf("hello");;
    /** documentation comment */
    /* ordinary comment */
}
