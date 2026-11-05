#include "adapters/c/c23_lex_step.h"

#include "annotation/annotation.h"

;;DEFINITION
/** C23LexStep is the returned value from one C23Lexer pull. It explicitly
 * distinguishes a real source span from end-of-input and owns no allocation;
 * when absent, its SyntaxSpan is the zero value. */

;;OVERVIEW
/** ============================================================================
 * CLASS: C23LexStep (adapters/c/c23_lex_step.c)
 * STRUCT FIELDS (Mirroring adapters/c/c23_lex_step.h)
 *   bool hasSpan       true when this result carries one source span
 *   SyntaxSpan span    copied range/role/style-key value; zero when absent
 * FUNCTION REGISTRY
 *   C23LexStep C23LexStep_0(void)
 *   C23LexStep C23LexStep_2(bool,SyntaxSpan)
 *   C23LexStep C23LexStep_zero(void)
 *   bool C23LexStep_hasSpan(const C23LexStep*)
 *   SyntaxSpan C23LexStep_getSpan(const C23LexStep*)
 *   BoundedText C23LexStep_toString/toStringStruct(const C23LexStep*)
 * CONTRACT: value-only, no borrowed destination; null getters return false/zero.
 * ============================================================================ */

C23LexStep C23LexStep_0(void) {
    return (C23LexStep) {.hasSpan = false, .span = {0}};
}

C23LexStep C23LexStep_2(bool hasSpan, SyntaxSpan span) {
    return (C23LexStep) {.hasSpan = hasSpan, .span = hasSpan ? span : (SyntaxSpan) {0}};
}

C23LexStep C23LexStep_zero(void) {
    return C23LexStep_0();
}

bool C23LexStep_hasSpan(const C23LexStep *self) {
    return self != nullptr && (*self).hasSpan;
}

SyntaxSpan C23LexStep_getSpan(const C23LexStep *self) {
    return self == nullptr ? (SyntaxSpan) {0} : (*self).span;
}

BoundedText C23LexStep_toString(const C23LexStep *self) {
    if (self == nullptr)
        return BoundedText_1("nullptr");
    SyntaxSpan span = (*self).span;
    return BoundedText_format("C23LexStep(%s, %s)", (*self).hasSpan ? "span" : "end",
                              SyntaxRole_name(span.role));
}

BoundedText C23LexStep_toStringStruct(const C23LexStep *self) {
    if (self == nullptr)
        return BoundedText_1("nullptr");
    BoundedText spanText = SyntaxSpan_toString(&(*self).span);
    return BoundedText_format("C23LexStep { hasSpan=%s, span=%s }",
                              (*self).hasSpan ? "true" : "false",
                              BoundedText_getText(&spanText));
}
