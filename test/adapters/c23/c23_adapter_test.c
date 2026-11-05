#include "adapters/c23/c23_adapter.h"
#include "lang/language_engine.h"
#include <assert.h>
#include <string.h>

int main(void) {
    const char source[] = "nullptr typeof_unqual int value;";

    TextSnapshot *snapshot = TextSnapshot(42u, 6u, source, sizeof source - 1u);

    assert(snapshot != nullptr);

    LanguageRegistry *registry = LanguageRegistry_0();

    assert(registry != nullptr);

    const LanguageAdapter *adapter = C23Adapter_adapter();

    assert(LanguageAdapter_isValid(adapter));

    assert(strcmp(LanguageAdapter_getLanguageId(adapter), "c23") == 0);

    assert(LanguageRegistry_register(registry, adapter));

    assert(LanguageRegistry_find(registry, "c23") == adapter);

    assert(LanguageRegistry_find(registry, "c99") == nullptr);

    LanguageEngine *engine = LanguageEngine_1(registry);

    assert(engine != nullptr);

    assert(LanguageEngine_open(engine, "c23", 7u, snapshot));

    TextSnapshot_free(snapshot);

    assert(strcmp(LanguageEngine_getLanguageId(engine), "c23") == 0);

    LanguageLexResult token = LanguageEngine_next(engine);

    assert(token.hasSpan && token.span.role == SYNTAX_ROLE_KEYWORD);

    SourceRange tokenRange = token.span.range;

    assert(tokenRange.documentId == 42u);

    assert(token.documentRevision == 6u && token.compilationContextId == 7u);

    assert(tokenRange.startByte == 0u && tokenRange.endByte == 7u);

    token = LanguageEngine_next(engine);

    assert(token.hasSpan && token.span.role == SYNTAX_ROLE_KEYWORD);

    SourceRange secondRange = token.span.range;

    assert(secondRange.startByte == 8u && secondRange.endByte == 21u);

    TextSnapshot *other = TextSnapshot(43u, 7u, source, sizeof source - 1u);

    assert(other != nullptr);

    assert(!LanguageEngine_open(engine, "c99", 8u, other));

    TextSnapshot_free(other);

    assert(strcmp(LanguageEngine_getLanguageId(engine), "c23") == 0);

    token = LanguageEngine_next(engine);

    assert(token.hasSpan && token.span.role == SYNTAX_ROLE_KEYWORD);

    SourceRange thirdRange = token.span.range;

    assert(thirdRange.startByte == 22u && thirdRange.endByte == 25u);

    token = LanguageEngine_next(engine);

    assert(token.hasSpan && token.span.role == SYNTAX_ROLE_IDENTIFIER);

    token = LanguageEngine_next(engine);

    assert(token.hasSpan && token.span.role == SYNTAX_ROLE_PUNCTUATION);

    assert(!LanguageEngine_next(engine).hasSpan);

    LanguageEngine_close(engine);

    LanguageEngine_free(engine);

    LanguageRegistry_free(registry);

    return 0;
}
