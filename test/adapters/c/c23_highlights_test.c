// Owner: conservative C23 lexical highlight roles, not parser/symbol semantics.
// The borrowed source stays immutable; spans stay ordered and non-overlapping.
#include "adapters/c/c23_highlights.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static SyntaxRole roleOf(C23Highlights *highlights, const char *source, const char *needle) {
    const char *found = strstr(source, needle);
    assert(found != nullptr);
    return C23Highlights_roleAt(highlights, (size_t) (found - source));
}

static void testClassification(void) {
    const char source[] =
        "#include <stdio.h>\n"
        "#include \"annotation/debug.h\"\n"
        "#define STR(x) #x\n"
        "/** doc */\n"
        "int add(int left, int right); // regular\n"
        "typedef uint8_t Byte;\n"
        "THROW(\"boom\");\n"
        "Console_log(message);\n"
        "value.field = 1;\n"
        "int lower = left < right;\n";
    char original[sizeof source];
    memcpy(original, source, sizeof source);
    size_t length = sizeof source - 1u;
    C23Highlights *highlights = C23Highlights(1u, source, length);
    assert(highlights != nullptr);
    assert(C23Highlights_getDocumentId(highlights) == 1u);
    assert(C23Highlights_getCount(highlights) > 20u);
    assert(roleOf(highlights, source, "#include") == SYNTAX_ROLE_MACRO);
    assert(roleOf(highlights, source, "include") == SYNTAX_ROLE_MACRO);
    assert(roleOf(highlights, source, "#define") == SYNTAX_ROLE_MACRO);
    assert(roleOf(highlights, source, "define") == SYNTAX_ROLE_MACRO);
    assert(roleOf(highlights, source, "<stdio.h>") == SYNTAX_ROLE_STRING);
    assert(roleOf(highlights, source, "stdio") == SYNTAX_ROLE_STRING);
    assert(roleOf(highlights, source, "annotation/debug.h") == SYNTAX_ROLE_STRING);
    const char *headerClose = strstr(source, "h>");
    assert(headerClose != nullptr);
    assert(C23Highlights_roleAt(highlights, (size_t) (headerClose - source))
           == SYNTAX_ROLE_STRING);
    assert(roleOf(highlights, source, "< right") == SYNTAX_ROLE_OPERATOR);
    const char *stringize = strstr(source, "#x");
    assert(stringize != nullptr);
    assert(C23Highlights_roleAt(highlights, (size_t) (stringize - source))
           == SYNTAX_ROLE_PUNCTUATION);
    assert(roleOf(highlights, source, "/** doc */") == SYNTAX_ROLE_DOC_COMMENT);
    assert(roleOf(highlights, source, "// regular") == SYNTAX_ROLE_COMMENT);
    assert(roleOf(highlights, source, "int add") == SYNTAX_ROLE_KEYWORD);
    assert(roleOf(highlights, source, "add(") == SYNTAX_ROLE_FUNCTION);
    assert(roleOf(highlights, source, "left") == SYNTAX_ROLE_VARIABLE);
    assert(roleOf(highlights, source, "uint8_t") == SYNTAX_ROLE_TYPE);
    assert(roleOf(highlights, source, "Byte") == SYNTAX_ROLE_TYPE);
    assert(roleOf(highlights, source, "THROW") == SYNTAX_ROLE_MACRO);
    assert(roleOf(highlights, source, "Console_log") == SYNTAX_ROLE_FUNCTION);
    assert(roleOf(highlights, source, "message") == SYNTAX_ROLE_VARIABLE);
    assert(roleOf(highlights, source, "value") == SYNTAX_ROLE_VARIABLE);
    assert(roleOf(highlights, source, "field") == SYNTAX_ROLE_MEMBER);
    size_t previousEnd = 0u;
    for (size_t index = 0u; index < C23Highlights_getCount(highlights); ++index) {
        size_t start = C23Highlights_getStart(highlights, index);
        size_t end = C23Highlights_getEnd(highlights, index);
        assert(start >= previousEnd);
        assert(end > start);
        assert(end <= length);
        assert(C23Highlights_getRole(highlights, index) != SYNTAX_ROLE_NONE);
        previousEnd = end;
    }
    // Every non-whitespace byte belongs to exactly one ordered span.
    for (size_t at = 0u; at < length; ++at) {
        unsigned char byte = (unsigned char) source[at];
        bool whitespace = byte == ' ' || byte == '\t' || byte == '\r' || byte == '\n';
        assert(whitespace || C23Highlights_roleAt(highlights, at) != SYNTAX_ROLE_NONE);
    }
    assert(roleOf(highlights, source, "typedef") == SYNTAX_ROLE_KEYWORD);
    assert(memcmp(original, source, sizeof source) == 0);
    assert(C23Highlights_getStart(highlights, SIZE_MAX) == 0u);
    assert(C23Highlights_getEnd(highlights, SIZE_MAX) == 0u);
    assert(C23Highlights_getRole(highlights, SIZE_MAX) == SYNTAX_ROLE_NONE);
    assert(C23Highlights_roleAt(highlights, SIZE_MAX) == SYNTAX_ROLE_NONE);
    BoundedText projection = C23Highlights_toString(highlights);
    assert(strstr(projection.text, "document=1") != nullptr);
    projection = C23Highlights_toStringStruct(highlights);
    assert(strstr(projection.text, "capacity=") != nullptr);
    C23Highlights_free(highlights);
}

static void testAdmission(void) {
    assert(C23Highlights_zero() != nullptr);
    C23Highlights *empty = C23Highlights(1u, nullptr, 0u);
    assert(empty != nullptr);
    assert(C23Highlights_getCount(empty) == 0u);
    assert(C23Highlights_roleAt(empty, 0u) == SYNTAX_ROLE_NONE);
    C23Highlights_free(empty);
    assert(C23Highlights(0u, "int x;", 6u) == nullptr);
    assert(C23Highlights(1u, nullptr, 1u) == nullptr);
    C23Highlights_free(nullptr);
    BoundedText projection = C23Highlights_toString(nullptr);
    assert(strcmp(projection.text, "nullptr") == 0);
    projection = C23Highlights_toStringStruct(nullptr);
    assert(strcmp(projection.text, "nullptr") == 0);
    assert(C23Highlights_getCount(nullptr) == 0u);
    assert(C23Highlights_getDocumentId(nullptr) == 0u);
    assert(C23Highlights_roleAt(nullptr, 0u) == SYNTAX_ROLE_NONE);
}

int main(void) {
    testClassification();
    testAdmission();
    return 0;
}
