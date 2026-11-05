#include "lang/language_lex_result.h"
#include "annotation/annotation.h"

;;DEFINITION
/** LanguageLexResult is one pull-lexer result. End-of-input always carries a
 * zero span so callers never inspect uninitialized output storage. */

;;OVERVIEW
/** CLASS: LanguageLexResult (lang/language_lex_result.c)
 * STRUCT FIELDS: hasSpan and SyntaxSpan span.
 * FUNCTION REGISTRY: zero and complete value constructors.
 * CONTRACT: absent results are canonicalized to a zero span. */

LanguageLexResult LanguageLexResult_0(void) { return (LanguageLexResult) {0}; }
LanguageLexResult LanguageLexResult_2(bool hasSpan, SyntaxSpan span) {
    return hasSpan ? (LanguageLexResult) {.hasSpan = true, .span = span}
                   : LanguageLexResult_0();
}
LanguageLexResult LanguageLexResult_4(bool hasSpan, SyntaxSpan span,
                                      uint64_t documentRevision,
                                      uint64_t compilationContextId) {
    return (LanguageLexResult) {
        .hasSpan = hasSpan,
        .span = hasSpan ? span : SyntaxSpan_zero(),
        .documentRevision = documentRevision,
        .compilationContextId = compilationContextId
    };
}
LanguageLexResult LanguageLexResult_zero(void) { return LanguageLexResult_0(); }
LanguageLexResult LanguageLexResult_withContext(LanguageLexResult self,
                                                 uint64_t documentRevision,
                                                 uint64_t compilationContextId) {
    self.documentRevision = documentRevision;
    self.compilationContextId = compilationContextId;
    if (!self.hasSpan)
        self.span = SyntaxSpan_zero();
    return self;
}
