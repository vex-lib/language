#ifndef LANGUAGE_ADAPTERS_C_C23_LEX_STEP_H
#define LANGUAGE_ADAPTERS_C_C23_LEX_STEP_H

#include <stdbool.h>
#include "analysis/syntax_span.h"
#include "text/bounded_text.h"

// A pull result. hasSpan=false always pairs with a zero SyntaxSpan.
typedef struct C23LexStep {
    bool hasSpan;
    SyntaxSpan span;
} C23LexStep;

C23LexStep C23LexStep_0();
C23LexStep C23LexStep_2(bool hasSpan, SyntaxSpan span);
#define C23LexStep_1 C23LexStep_invalid_arity
#define C23LexStep_3 C23LexStep_invalid_arity
#define C23LexStep_4 C23LexStep_invalid_arity
#define C23LEXSTEP_CAT_INNER(left, right) left##right
#define C23LEXSTEP_CAT(left, right) C23LEXSTEP_CAT_INNER(left, right)
#define C23LEXSTEP_NARG_INNER(_0, _1, _2, _3, _4, count, ...) count
#define C23LEXSTEP_NARG(...) \
    C23LEXSTEP_NARG_INNER(_ __VA_OPT__(,) __VA_ARGS__, 4, 3, 2, 1, 0)
#define C23LexStep(...) \
    C23LEXSTEP_CAT(C23LexStep_, C23LEXSTEP_NARG(__VA_ARGS__))(__VA_ARGS__)
C23LexStep C23LexStep_zero();
bool C23LexStep_hasSpan(const C23LexStep *self);
SyntaxSpan C23LexStep_getSpan(const C23LexStep *self);
BoundedText C23LexStep_toString(const C23LexStep *self);
BoundedText C23LexStep_toStringStruct(const C23LexStep *self);

#endif
