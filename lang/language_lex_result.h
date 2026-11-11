#ifndef LANGUAGE_LANG_LANGUAGE_LEX_RESULT_H
#define LANGUAGE_LANG_LANGUAGE_LEX_RESULT_H

#include "analysis/syntax_span.h"
#include <stdbool.h>

typedef struct LanguageLexResult {
    bool hasSpan;
    SyntaxSpan span;
    u64 documentRevision;
    u64 compilationContextId;
} LanguageLexResult;

LanguageLexResult LanguageLexResult_0(void);
LanguageLexResult LanguageLexResult_2(bool hasSpan, SyntaxSpan span);
LanguageLexResult LanguageLexResult_4(bool hasSpan, SyntaxSpan span,
                                     u64 documentRevision,
                                     u64 compilationContextId);
// Context stamping applies to end-of-stream values too; absent spans remain
// canonical zero spans while retaining their snapshot/context identity.
LanguageLexResult LanguageLexResult_zero(void);
LanguageLexResult LanguageLexResult_withContext(LanguageLexResult self,
                                                u64 documentRevision,
                                                u64 compilationContextId);

#endif
