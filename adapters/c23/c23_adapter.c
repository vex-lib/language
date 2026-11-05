#include "adapters/c23/c23_adapter.h"
#include "adapters/c/c23_lexer.h"
#include "annotation/annotation.h"

;;DEFINITION
/** C23Adapter binds the explicit language id "c23" to the existing strict C23
 * lexical subset. C99 and C++ require separate registered adapters and cannot
 * be inferred from a .c or .h suffix. The lexer borrows the host snapshot. */

;;OVERVIEW
/** CLASS: C23Adapter (adapters/c23/c23_adapter.c)
 * STRUCT FIELDS: none; the static LanguageAdapter table is immutable.
 * PRIVATE HELPERS: open, next, close bridge the generic session to C23Lexer.
 * FUNCTION REGISTRY: C23Adapter_adapter returns the borrowed static table.
 * CONTRACT: one adapter instance serves strict C23 tokens only. */

static void *openC23(uint32_t documentId, const char *source, size_t length) {
    return C23Lexer_3(documentId, source, length);
}

static LanguageLexResult nextC23(void *session) {
    C23LexStep step = C23Lexer_next(session);
    return LanguageLexResult_2(step.hasSpan, step.span);
}

static void closeC23(void *session) { C23Lexer_free(session); }

const LanguageAdapter *C23Adapter_adapter(void) {
    static const LanguageAdapter adapter = {
        .languageId = "c23",
        .open = openC23,
        .next = nextC23,
        .close = closeC23
    };
    return &adapter;
}
