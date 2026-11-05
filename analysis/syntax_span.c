#include "analysis/syntax_span.h"
#include "annotation/annotation.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: SyntaxSpan
 * ============================================================================
 * SyntaxSpan is the language-neutral result of classification: one byte range,
 * the role the adapter assigned to it, and an optional adapter style key. It is
 * a value, not a node in a tree; an adapter emits a flat, ordered sequence of
 * spans and a host paints them. A null styleKey means "use the role's core
 * key", which is what lets a host render a language it has no adapter for.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: SyntaxSpan (analysis/syntax_span.c)
 * STRUCT FIELDS (Mirroring analysis/syntax_span.h)
 *   SourceRange range     // half-open byte range classified by the adapter
 *   SyntaxRole role       // shared syntactic vocabulary
 *   const char *styleKey  // borrowed adapter override, nullable
 *
 * FUNCTION REGISTRY
 *   constructors: SyntaxSpan_0, SyntaxSpan_3, SyntaxSpan(...), SyntaxSpan_zero
 *   getters:      SyntaxSpan_getStyleKey, SyntaxSpan_resolvedStyleKey
 *   projections:  SyntaxSpan_toString, SyntaxSpan_toStringStruct
 *
 * CONTRACT
 *   styleKey is borrowed and must outlive the span's consumers. resolvedStyleKey
 *   returns the override when present, otherwise SyntaxRole_styleKey(role), and
 *   "text" for a null self. The span owns no allocation and mutates nothing.
 * ============================================================================
 */

SyntaxSpan SyntaxSpan_0(void) { return (SyntaxSpan) {0}; }
SyntaxSpan SyntaxSpan_zero(void) { return SyntaxSpan_0(); }
SyntaxSpan SyntaxSpan_3(SourceRange range, SyntaxRole role, const char *styleKey) {
    return (SyntaxSpan) {.range = range, .role = role, .styleKey = styleKey};
}
const char *SyntaxSpan_getStyleKey(const SyntaxSpan *self) {
    return self == nullptr ? nullptr : (*self).styleKey;
}
const char *SyntaxSpan_resolvedStyleKey(const SyntaxSpan *self) {
    if (self == nullptr) return "text";
    return (*self).styleKey == nullptr ? SyntaxRole_styleKey((*self).role) : (*self).styleKey;
}
BoundedText SyntaxSpan_toString(const SyntaxSpan *self) {
    if (self == nullptr) return BoundedText_1("nullptr");
    SourceRange range = (*self).range;
    return BoundedText_format("%s [%llu,%llu)", SyntaxRole_name((*self).role),
        (unsigned long long) range.startByte, (unsigned long long) range.endByte);
}
BoundedText SyntaxSpan_toStringStruct(const SyntaxSpan *self) {
    if (self == nullptr) return BoundedText_1("nullptr");
    return BoundedText_format("SyntaxSpan { role=%s, styleKey=%s }", SyntaxRole_name((*self).role),
        (*self).styleKey == nullptr ? "nullptr" : (*self).styleKey);
}
