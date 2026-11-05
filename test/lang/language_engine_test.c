#include "adapters/c23/c23_adapter.h"
#include "lang/language_engine.h"
#include <assert.h>
#include <string.h>

int main(void) {
    char source[] = "nullptr x";

    TextSnapshot *snapshot = TextSnapshot(3u, 17u, source, sizeof source - 1u);

    assert(snapshot != nullptr && TextSnapshot_getReferenceCount(snapshot) == 1u);

    LanguageRegistry *registry = LanguageRegistry_0();

    assert(registry != nullptr);

    assert(LanguageRegistry_register(registry, C23Adapter_adapter()));

    LanguageEngine *engine = LanguageEngine_1(registry);

    assert(engine != nullptr);

    assert(LanguageEngine_open(engine, "c23", 91u, snapshot));

    assert(LanguageEngine_getDocumentId(engine) == 3u);

    assert(LanguageEngine_getDocumentRevision(engine) == 17u);

    assert(LanguageEngine_getCompilationContextId(engine) == 91u);

    assert(TextSnapshot_getReferenceCount(snapshot) == 2u);

    TextSnapshot_free(snapshot);

    source[0] = 'X';

    LanguageLexResult result = LanguageEngine_next(engine);

    assert(result.hasSpan && result.span.role == SYNTAX_ROLE_KEYWORD);

    SourceRange resultRange = result.span.range;

    assert(resultRange.documentId == 3u);

    assert(result.documentRevision == 17u && result.compilationContextId == 91u);

    TextSnapshot *other = TextSnapshot(4u, 1u, "x", 1u);

    assert(other != nullptr);

    assert(!LanguageEngine_open(engine, "c99", 92u, other));

    assert(!LanguageEngine_open(engine, "c23", 0u, other));

    TextSnapshot_free(other);

    assert(strcmp(LanguageEngine_getLanguageId(engine), "c23") == 0);

    assert(LanguageEngine_getDocumentId(engine) == 3u);

    assert(LanguageEngine_getCompilationContextId(engine) == 91u);

    LanguageEngine_close(engine);

    assert(LanguageEngine_getDocumentId(engine) == 0u);

    assert(!LanguageEngine_next(engine).hasSpan);

    LanguageEngine_free(engine);

    LanguageRegistry_free(registry);

    return 0;
}
