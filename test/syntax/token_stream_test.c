#include "adapters/c23/c23_adapter.h"
#include "syntax/token_stream.h"
#include <assert.h>
#include <string.h>

static int fakeSession;

static void *openBad(uint32_t documentId, const char *bytes, size_t length) {
    (void) documentId;
    (void) bytes;
    (void) length;
    return &fakeSession;
}

static LanguageLexResult nextOutOfBounds(void *session) {
    assert(session == &fakeSession);
    SyntaxSpan span = SyntaxSpan_3(SourceRange_3(40u, 0u, UINT64_MAX),
                                   SYNTAX_ROLE_IDENTIFIER, nullptr);
    return LanguageLexResult_2(true, span);
}

static void closeBad(void *session) { assert(session == &fakeSession); }

int main(void) {
    const char source[] = "int value = 23; // declaration\n";
    TextSnapshot *snapshot = TextSnapshot(40u, 7u, source, sizeof source - 1u);
    assert(snapshot != nullptr);
    LanguageRegistry *registry = LanguageRegistry_0();
    assert(registry != nullptr);
    assert(LanguageRegistry_register(registry, C23Adapter_adapter()));
    LanguageEngine *engine = LanguageEngine_1(registry);
    assert(engine != nullptr);
    assert(LanguageEngine_open(engine, "c23", 55u, snapshot));
    TokenStream *stream = TokenStream_collect(engine, snapshot);
    assert(stream != nullptr);
    assert(TextSnapshot_getReferenceCount(snapshot) == 3u);
    assert(TokenStream_isComplete(stream));
    assert(TokenStream_getDocumentId(stream) == 40u);
    assert(TokenStream_getDocumentRevision(stream) == 7u);
    assert(TokenStream_getCompilationContextId(stream) == 55u);
    assert(TokenStream_getCount(stream) == 6u);
    Token first = TokenStream_getAt(stream, 0u);
    assert(first.kind == TOKEN_KIND_KEYWORD);
    assert(first.range.documentId == 40u && first.range.startByte == 0u
           && first.range.endByte == 3u);
    Token second = TokenStream_getAt(stream, 1u);
    assert(second.kind == TOKEN_KIND_IDENTIFIER);
    assert(second.range.startByte == 4u && second.range.endByte == 9u);
    Token third = TokenStream_getAt(stream, 2u);
    assert(third.kind == TOKEN_KIND_OPERATOR);
    Token fourth = TokenStream_getAt(stream, 3u);
    assert(fourth.kind == TOKEN_KIND_NUMBER);
    Token fifth = TokenStream_getAt(stream, 4u);
    assert(fifth.kind == TOKEN_KIND_PUNCTUATION);
    Token sixth = TokenStream_getAt(stream, 5u);
    assert(sixth.kind == TOKEN_KIND_COMMENT);
    assert(TokenStream_getAt(stream, 6u).kind == TOKEN_KIND_UNKNOWN);
    BoundedText projection = TokenStream_toString(stream);
    assert(strstr(projection.text, "document=40") != nullptr);
    projection = TokenStream_toStringStruct(stream);
    assert(strstr(projection.text, "context=55") != nullptr);

    TextSnapshot_free(snapshot);
    assert(TextSnapshot_getReferenceCount(snapshot) == 2u);
    LanguageEngine_close(engine);
    assert(TokenStream_isComplete(stream));
    assert(TokenStream_collect(engine, snapshot) == nullptr);

    const LanguageAdapter invalidAdapter = {
        .languageId = "invalid-test", .open = openBad,
        .next = nextOutOfBounds, .close = closeBad
    };
    assert(LanguageRegistry_register(registry, &invalidAdapter));
    assert(LanguageEngine_open(engine, "invalid-test", 56u, snapshot));
    assert(TokenStream_collect(engine, snapshot) == nullptr);
    TokenStream_free(stream);
    LanguageEngine_free(engine);
    LanguageRegistry_free(registry);
    return 0;
}
