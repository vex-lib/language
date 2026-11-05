#include "analysis/syntax_span.h"
#include <assert.h>
#include <string.h>

int main(void) {
    SyntaxSpan span = SyntaxSpan(SourceRange(1u, 2u, 3u), SYNTAX_ROLE_DEREFERENCE, nullptr);
    assert(strcmp(SyntaxSpan_resolvedStyleKey(&span), "operator.deref") == 0);
    SyntaxSpan cspan = SyntaxSpan(SourceRange(1u, 2u, 3u), SYNTAX_ROLE_DEREFERENCE, "c.pointer");
    assert(strcmp(SyntaxSpan_resolvedStyleKey(&cspan), "c.pointer") == 0);
    assert(strcmp(SyntaxSpan_resolvedStyleKey(nullptr), "text") == 0);
    return 0;
}
