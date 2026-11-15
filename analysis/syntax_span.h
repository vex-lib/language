#ifndef LANGUAGE_ANALYSIS_SYNTAX_SPAN_H
#define LANGUAGE_ANALYSIS_SYNTAX_SPAN_H

#include "vex.h"
#include "document/source_range.h"
#include "syntax/role.h"
#include "text/bounded_text.h"

typedef struct SyntaxSpan {
    SourceRange range;
    SyntaxRole role;
    const char *styleKey; // borrowed adapter override; null means role default
} SyntaxSpan;

SyntaxSpan SyntaxSpan_0();
SyntaxSpan SyntaxSpan_3(SourceRange range, SyntaxRole role, const char *styleKey);
#define SyntaxSpan(...) SYNTAX_SPAN_SELECT(_ __VA_OPT__(,) __VA_ARGS__, SyntaxSpan_3, SyntaxSpan_invalid_arity, SyntaxSpan_invalid_arity, SyntaxSpan_0)(__VA_ARGS__)
#define SYNTAX_SPAN_SELECT(_0, _1, _2, _3, NAME, ...) NAME
SyntaxSpan SyntaxSpan_zero();
const char *SyntaxSpan_getStyleKey(const SyntaxSpan *self);
const char *SyntaxSpan_resolvedStyleKey(const SyntaxSpan *self);
BoundedText SyntaxSpan_toString(const SyntaxSpan *self);
BoundedText SyntaxSpan_toStringStruct(const SyntaxSpan *self);

#endif
