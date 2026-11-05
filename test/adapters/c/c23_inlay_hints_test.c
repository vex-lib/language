#include "adapters/c/c23_inlay_hints.h"
#include <assert.h>
#include <string.h>

int main(void) {
    const char source[] =
        "int add(int left, int right);\n"
        "int main(void) { return add(1, add(2, 3)); }\n";
    char original[sizeof source];
    memcpy(original, source, sizeof source);

    C23InlayHints *hints = C23InlayHints(77u, source, sizeof source - 1u);
    assert(hints != nullptr);
    assert(C23InlayHints_getDocumentId(hints) == 77u);
    assert(C23InlayHints_getCount(hints) == 4u);
    BoundedText valueProjection = C23InlayHints_toString(hints);
    assert(strstr(valueProjection.text, "document=77") != nullptr);
    BoundedText structProjection = C23InlayHints_toStringStruct(hints);
    assert(strstr(structProjection.text, "labelByteCount=") != nullptr);
    assert(structProjection.truncated);
    const char *first = strstr(source, "add(1") + strlen("add(");
    const char *outerSecond = strstr(source, "add(2");
    const char *nestedFirst = outerSecond + strlen("add(");
    const char *nestedSecond = strstr(source, "2, 3") + strlen("2, ");
    assert(C23InlayHints_getOffset(hints, 0u) == (size_t) (first - source));
    assert(C23InlayHints_getOffset(hints, 1u) == (size_t) (outerSecond - source));
    assert(C23InlayHints_getOffset(hints, 2u) == (size_t) (nestedFirst - source));
    assert(C23InlayHints_getOffset(hints, 3u) == (size_t) (nestedSecond - source));
    assert(strcmp(C23InlayHints_getLabel(hints, 0u), "left:") == 0);
    assert(strcmp(C23InlayHints_getLabel(hints, 1u), "right:") == 0);
    assert(strcmp(C23InlayHints_getLabel(hints, 2u), "left:") == 0);
    assert(C23InlayHints_getLabelLength(hints, 3u) == 6u);
    assert(strcmp(C23InlayHints_getLabel(hints, 3u), "right:") == 0);
    assert(C23InlayHints_getOffset(hints, 4u) == 0u);
    assert(C23InlayHints_getLabel(hints, 4u) == nullptr);
    assert(C23InlayHints_getLabelLength(hints, 4u) == 0u);
    assert(memcmp(source, original, sizeof source) == 0);
    C23InlayHints_free(hints);

    const char duplicateDeclaration[] =
        "int f(int a);\nint f(int a) { return a; }\nint g(void) { return f(1); }";
    hints = C23InlayHints(81u, duplicateDeclaration, strlen(duplicateDeclaration));
    assert(hints != nullptr);
    assert(C23InlayHints_getCount(hints) == 1u);
    const char *definitionParameter = strstr(duplicateDeclaration, "int f(int a) {")
        + strlen("int f(int ");
    const char *callArgument = strstr(duplicateDeclaration, "f(1)") + strlen("f(");
    assert(C23InlayHints_getOffset(hints, 0u) == (size_t) (callArgument - duplicateDeclaration));
    assert(C23InlayHints_getOffset(hints, 0u) != (size_t) (definitionParameter - duplicateDeclaration));
    assert(strcmp(C23InlayHints_getLabel(hints, 0u), "a:") == 0);
    C23InlayHints_free(hints);

    const char unsupportedDeclarators[][96] = {
        "int f(int a[N]); int g(void) { return f(1); }",
        "int f(void (*cb)(size_t)); int g(void) { return f(0); }",
        "int f(struct Foo); int g(void) { return f(0); }",
        "int f(int a[3]); int g(void) { return f(1); }"
    };
    for (size_t index = 0u; index < sizeof unsupportedDeclarators / sizeof unsupportedDeclarators[0]; ++index) {
        hints = C23InlayHints((uint32_t) (82u + index), unsupportedDeclarators[index],
                              strlen(unsupportedDeclarators[index]));
        assert(hints != nullptr);
        assert(C23InlayHints_getCount(hints) == 0u);
        C23InlayHints_free(hints);
    }

    const char memberCall[] = "int f(int a); int g(struct S *s) { return s->f(1); }";
    hints = C23InlayHints(86u, memberCall, strlen(memberCall));
    assert(hints != nullptr);
    assert(C23InlayHints_getCount(hints) == 0u);
    C23InlayHints_free(hints);

    const char expressionCall[] = "int g(void) { f(a + b.sh); f(1); return 0; }";
    hints = C23InlayHints(90u, expressionCall, strlen(expressionCall));
    assert(hints != nullptr);
    assert(C23InlayHints_getCount(hints) == 0u);
    C23InlayHints_free(hints);

    const char unsupportedDefinition[] =
        "int f(int a); int f(int a[3]) { return a[0]; } int g(void) { return f(1); }";
    hints = C23InlayHints(91u, unsupportedDefinition, strlen(unsupportedDefinition));
    assert(hints != nullptr);
    assert(C23InlayHints_getCount(hints) == 1u);
    const char *validCall = strstr(unsupportedDefinition, "f(1)") + strlen("f(");
    assert(C23InlayHints_getOffset(hints, 0u) == (size_t) (validCall - unsupportedDefinition));
    C23InlayHints_free(hints);

    const char emptySource[] = "";
    hints = C23InlayHints(87u, emptySource, 0u);
    assert(hints != nullptr);
    assert(C23InlayHints_getCount(hints) == 0u);
    C23InlayHints_free(hints);
    hints = C23InlayHints(88u, nullptr, 0u);
    assert(hints != nullptr);
    assert(C23InlayHints_getCount(hints) == 0u);
    C23InlayHints_free(hints);

    const char zeroParameterCall[] = "int done(void); int run(void) { return done(); }";
    hints = C23InlayHints(89u, zeroParameterCall, strlen(zeroParameterCall));
    assert(hints != nullptr);
    assert(C23InlayHints_getCount(hints) == 0u);
    C23InlayHints_free(hints);

    hints = C23InlayHints_0();
    assert(hints != nullptr);
    assert(C23InlayHints_getDocumentId(hints) == 0u);
    assert(C23InlayHints_getCount(hints) == 0u);
    C23InlayHints_free(hints);
    hints = C23InlayHints_zero();
    assert(hints != nullptr);
    assert(C23InlayHints_getDocumentId(hints) == 0u);
    assert(C23InlayHints_getCount(hints) == 0u);
    C23InlayHints_free(hints);

    const char noSignature[] = "int call(void) { return unknown(1); }";
    hints = C23InlayHints(78u, noSignature, strlen(noSignature));
    assert(hints != nullptr);
    assert(C23InlayHints_getCount(hints) == 0u);
    C23InlayHints_free(hints);

    const char tooManyArguments[] = "int f(int value); int g(void) { return f(1, 2); }";
    hints = C23InlayHints(80u, tooManyArguments, strlen(tooManyArguments));
    assert(hints != nullptr);
    assert(C23InlayHints_getCount(hints) == 0u);
    C23InlayHints_free(hints);

    const char wrongCount[] = "int f(int value); int g(void) { return f(); }";
    hints = C23InlayHints(79u, wrongCount, strlen(wrongCount));
    assert(hints != nullptr);
    assert(C23InlayHints_getCount(hints) == 0u);
    C23InlayHints_free(hints);

    hints = C23InlayHints(0u, source, sizeof source - 1u);
    assert(hints == nullptr);
    assert(C23InlayHints_getCount(nullptr) == 0u);
    assert(C23InlayHints_getDocumentId(nullptr) == 0u);
    assert(C23InlayHints_getOffset(nullptr, 0u) == 0u);
    assert(C23InlayHints_getLabel(nullptr, 0u) == nullptr);
    assert(C23InlayHints_getLabelLength(nullptr, 0u) == 0u);
    valueProjection = C23InlayHints_toString(nullptr);
    assert(strcmp(valueProjection.text, "nullptr") == 0);
    structProjection = C23InlayHints_toStringStruct(nullptr);
    assert(strcmp(structProjection.text, "nullptr") == 0);
    C23InlayHints_free(nullptr);
    return 0;
}
