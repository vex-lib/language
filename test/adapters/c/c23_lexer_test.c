#include "adapters/c/c23_lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static C23LexStep take(C23Lexer *lexer, SyntaxRole role, size_t start, size_t end) {
    C23LexStep step = C23Lexer_next(lexer);
    SyntaxSpan span = step.span;
    SourceRange range = span.range;
    if (step.hasSpan && (span.role != role || range.startByte != start
                         || range.endByte != end))
        fprintf(stderr, "expected role %s [%zu,%zu), got %s [%llu,%llu)\n",
                SyntaxRole_name(role), start, end, SyntaxRole_name(span.role),
                (unsigned long long) range.startByte,
                (unsigned long long) range.endByte);
    assert(step.hasSpan);
    assert(span.role == role);
    assert(range.documentId == C23Lexer_getDocumentId(lexer));
    assert(range.startByte == start);
    assert(range.endByte == end);
    assert(end <= C23Lexer_getLength(lexer));
    return step;
}

static bool sourceWhitespace(unsigned char byte) {
    return byte == ' ' || byte == '\t' || byte == '\r' || byte == '\n'
        || byte == '\f' || byte == '\v';
}

static void assertCoverage(uint32_t documentId, const char *source, size_t length) {
    C23Lexer *lexer = C23Lexer(documentId, source, length);
    assert(lexer != nullptr);
    size_t coveredThrough = 0u;
    C23LexStep step = C23Lexer_next(lexer);
    while (step.hasSpan) {
        SyntaxSpan span = step.span;
        SourceRange range = span.range;
        assert(range.documentId == documentId);
        assert(range.startByte >= coveredThrough);
        for (size_t byte = coveredThrough; byte < range.startByte; ++byte)
            assert(sourceWhitespace((unsigned char) source[byte]));
        assert(range.startByte < range.endByte);
        assert(range.endByte <= length);
        coveredThrough = range.endByte;
        step = C23Lexer_next(lexer);
    }
    for (size_t byte = coveredThrough; byte < length; ++byte)
        assert(sourceWhitespace((unsigned char) source[byte]));
    C23Lexer_free(lexer);
}

int main(void) {
    _Static_assert(SYNTAX_ROLE_INVALID == 41, "existing syntax role ordinals stay stable");
    _Static_assert(SYNTAX_ROLE_IDENTIFIER == 42, "identifier is appended after existing roles");
    assert(strcmp(SyntaxRole_name(SYNTAX_ROLE_IDENTIFIER), "identifier") == 0);
    assert(strcmp(SyntaxRole_styleKey(SYNTAX_ROLE_IDENTIFIER), "text") == 0);
    assert(C23Lexer_getDocumentId(nullptr) == 0u);
    assert(C23Lexer_getSource(nullptr) == nullptr);
    assert(C23Lexer_getLength(nullptr) == 0u);
    assert(C23Lexer_getOffset(nullptr) == 0u);
    assert(C23Lexer_getProfile(nullptr) == C23_LEXER_PROFILE_STRICT_C23);
    assert(C23Lexer_isDone(nullptr));
    const char *keywords[] = {
        "alignas", "alignof", "auto", "bool", "break", "case", "char", "char8_t",
        "const", "constexpr", "continue", "default", "do", "double", "else", "enum",
        "extern", "false", "float", "for", "goto", "if", "inline", "int", "long",
        "nullptr", "register", "restrict", "return", "short", "signed", "sizeof", "static",
        "static_assert", "struct", "switch", "thread_local", "true", "typedef", "typeof",
        "typeof_unqual", "union", "unsigned", "void", "volatile", "while", "_Alignas",
        "_Alignof", "_Atomic", "_BitInt", "_Bool", "_Complex", "_Decimal32", "_Decimal64",
        "_Decimal128", "_Generic", "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
    };
    char keywordSource[1024] = {0};
    size_t keywordLength = 0u;
    for (size_t index = 0u; index < sizeof keywords / sizeof keywords[0]; ++index) {
        if (index != 0u)
            keywordSource[keywordLength++] = ' ';
        size_t wordLength = strlen(keywords[index]);
        memcpy(keywordSource + keywordLength, keywords[index], wordLength);
        keywordLength += wordLength;
    }
    C23Lexer *lexer = C23Lexer(16u, keywordSource, keywordLength);
    size_t keywordOffset = 0u;
    for (size_t index = 0u; index < sizeof keywords / sizeof keywords[0]; ++index) {
        size_t wordLength = strlen(keywords[index]);
        take(lexer, SYNTAX_ROLE_KEYWORD, keywordOffset, keywordOffset + wordLength);
        keywordOffset += wordLength + 1u;
    }
    assert(!C23Lexer_next(lexer).hasSpan);
    C23Lexer_free(lexer);
    lexer = C23Lexer();
    assert(lexer != nullptr);
    assert(C23Lexer_isDone(lexer));
    C23Lexer_free(lexer);
    C23Lexer_free(nullptr);
    lexer = C23Lexer_zero();
    assert(lexer != nullptr && C23Lexer_isDone(lexer));
    C23Lexer_free(lexer);
    lexer = C23Lexer_0();
    assert(lexer != nullptr && C23Lexer_isDone(lexer));
    C23Lexer_free(lexer);
    lexer = C23Lexer_3(32u, nullptr, 0u);
    assert(lexer != nullptr && C23Lexer_getLength(lexer) == 0u);
    C23Lexer_free(lexer);
    assert(C23Lexer_3(0u, "x", 1u) == nullptr);

    const char *source = "alignas bool true false nullptr char8_t ordinary TYPE call() [[ ]] 1e+2\n";
    lexer = C23Lexer(17u, source, strlen(source));
    assert(lexer != nullptr);
    take(lexer, SYNTAX_ROLE_KEYWORD, 0u, 7u);
    take(lexer, SYNTAX_ROLE_KEYWORD, 8u, 12u);
    take(lexer, SYNTAX_ROLE_KEYWORD, 13u, 17u);
    take(lexer, SYNTAX_ROLE_KEYWORD, 18u, 23u);
    take(lexer, SYNTAX_ROLE_KEYWORD, 24u, 31u);
    take(lexer, SYNTAX_ROLE_KEYWORD, 32u, 39u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 40u, 48u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 49u, 53u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 54u, 58u);
    take(lexer, SYNTAX_ROLE_BRACKET, 58u, 59u);
    take(lexer, SYNTAX_ROLE_BRACKET, 59u, 60u);
    take(lexer, SYNTAX_ROLE_BRACKET, 61u, 62u);
    take(lexer, SYNTAX_ROLE_BRACKET, 62u, 63u);
    take(lexer, SYNTAX_ROLE_BRACKET, 64u, 65u);
    take(lexer, SYNTAX_ROLE_BRACKET, 65u, 66u);
    take(lexer, SYNTAX_ROLE_NUMBER, 67u, 71u);
    assert(!C23Lexer_next(lexer).hasSpan);
    assert(C23Lexer_isDone(lexer));
    C23Lexer_free(lexer);

    source = "u8\"hi\" 0x1.fp+2 1'000 %:%: [[ ]]";
    lexer = C23Lexer(23u, source, strlen(source));
    take(lexer, SYNTAX_ROLE_STRING, 0u, 6u);
    take(lexer, SYNTAX_ROLE_NUMBER, 7u, 15u);
    take(lexer, SYNTAX_ROLE_NUMBER, 16u, 21u);
    take(lexer, SYNTAX_ROLE_PUNCTUATION, 22u, 26u);
    take(lexer, SYNTAX_ROLE_BRACKET, 27u, 28u);
    take(lexer, SYNTAX_ROLE_BRACKET, 28u, 29u);
    take(lexer, SYNTAX_ROLE_BRACKET, 30u, 31u);
    take(lexer, SYNTAX_ROLE_BRACKET, 31u, 32u);
    C23Lexer_free(lexer);

    source = "u\"s\" U\"s\" L\"s\" u8'c' L'c'";
    lexer = C23Lexer(28u, source, strlen(source));
    take(lexer, SYNTAX_ROLE_STRING, 0u, 4u);
    take(lexer, SYNTAX_ROLE_STRING, 5u, 9u);
    take(lexer, SYNTAX_ROLE_STRING, 10u, 14u);
    take(lexer, SYNTAX_ROLE_CHARACTER, 15u, 20u);
    take(lexer, SYNTAX_ROLE_CHARACTER, 21u, 25u);
    C23Lexer_free(lexer);

    static const char *const punctuators[] = {
        "%:%:", "<<=", ">>=", "...", "->", "++", "--", "&&", "||",
        "<=", ">=", "==", "!=", "*=", "/=", "%=", "+=", "-=", "<<", ">>",
        "&=", "^=", "|=", "##", "<:", ":>", "<%", "%>", "%:",
        "[", "]", "(", ")", "{", "}", ".", "&", "*", "+", "-", "~", "!", "/",
        "%", "<", ">", "^", "|", "?", ":", ";", "=", ",", "#"
    };
    char punctuatorSource[512] = {0};
    size_t punctuatorLength = 0u;
    size_t punctuatorOffsets[sizeof punctuators / sizeof punctuators[0]] = {0};
    for (size_t index = 0u; index < sizeof punctuators / sizeof punctuators[0]; ++index) {
        if (index != 0u)
            punctuatorSource[punctuatorLength++] = ' ';
        punctuatorOffsets[index] = punctuatorLength;
        size_t tokenLength = strlen(punctuators[index]);
        memcpy(punctuatorSource + punctuatorLength, punctuators[index], tokenLength);
        punctuatorLength += tokenLength;
    }
    lexer = C23Lexer(27u, punctuatorSource, punctuatorLength);
    for (size_t index = 0u; index < sizeof punctuators / sizeof punctuators[0]; ++index) {
        size_t tokenLength = strlen(punctuators[index]);
        C23LexStep step = C23Lexer_next(lexer);
        SyntaxSpan span = step.span;
        SourceRange range = span.range;
        assert(step.hasSpan);
        assert(range.startByte == punctuatorOffsets[index]);
        assert(range.endByte == punctuatorOffsets[index] + tokenLength);
        assert(memcmp(punctuatorSource + range.startByte, punctuators[index], tokenLength) == 0);
        if (strcmp(punctuators[index], "<:") == 0 || strcmp(punctuators[index], ":>") == 0
            || strcmp(punctuators[index], "<%") == 0 || strcmp(punctuators[index], "%>") == 0
            || (tokenLength == 1u && strchr("()[]{}", punctuators[index][0]) != nullptr))
            assert(span.role == SYNTAX_ROLE_BRACKET);
        else if ((tokenLength == 1u && strchr(".,:;#", punctuators[index][0]) != nullptr)
                 || strcmp(punctuators[index], "...") == 0
                 || strcmp(punctuators[index], "##") == 0
                 || strcmp(punctuators[index], "%:") == 0
                 || strcmp(punctuators[index], "%:%:") == 0)
            assert(span.role == SYNTAX_ROLE_PUNCTUATION);
        else
            assert(span.role == SYNTAX_ROLE_OPERATOR);
    }
    assert(!C23Lexer_next(lexer).hasSpan);
    C23Lexer_free(lexer);

    const unsigned char malformed[] = {0xE1u, 0x80u, 'x', 0u, 'y'};
    lexer = C23Lexer(18u, (const char*) malformed, sizeof malformed);
    take(lexer, SYNTAX_ROLE_INVALID, 0u, 2u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 2u, 3u);
    take(lexer, SYNTAX_ROLE_INVALID, 3u, 4u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 4u, 5u);
    assert(!C23Lexer_next(lexer).hasSpan);

    assert(!C23Lexer_setSource(lexer, 0u, "bad", 3u));
    assert(C23Lexer_getDocumentId(lexer) == 18u);
    assert(C23Lexer_getOffset(lexer) == 5u);
    assert(!C23Lexer_setSource(lexer, 19u, nullptr, 1u));
    assert(C23Lexer_getDocumentId(lexer) == 18u);
    assert(!C23Lexer_setProfile(lexer, (C23LexerProfile) 99));
    assert(C23Lexer_getProfile(lexer) == C23_LEXER_PROFILE_STRICT_C23);
    assert(C23Lexer_setProfile(lexer, C23_LEXER_PROFILE_VEX_ANNOTATION));
    const char *replacement = "name";
    assert(C23Lexer_setSource(lexer, 26u, replacement, 4u));
    assert(C23Lexer_getProfile(lexer) == C23_LEXER_PROFILE_VEX_ANNOTATION);
    assert(C23Lexer_getSource(lexer) == replacement);
    assert(C23Lexer_getLength(lexer) == 4u);
    assert(C23Lexer_getOffset(lexer) == 0u);
    assert(!C23Lexer_isDone(lexer));
    BoundedText structProjection = C23Lexer_toStringStruct(lexer);
    assert(strstr(structProjection.text, "documentId=26") != nullptr);
    BoundedText valueProjection = C23Lexer_toString(lexer);
    assert(strstr(valueProjection.text, "document=26") != nullptr);
    C23Lexer_free(lexer);

    const unsigned char nulInString[] = {'"', 'a', 0u, 'b', '"'};
    lexer = C23Lexer(35u, (const char*) nulInString, sizeof nulInString);
    take(lexer, SYNTAX_ROLE_INVALID, 0u, 3u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 3u, 4u);
    take(lexer, SYNTAX_ROLE_INVALID, 4u, 5u);
    C23Lexer_free(lexer);

    const unsigned char nulEscapedInString[] = {'"', 'a', '\\', 0u, 'b', '"'};
    lexer = C23Lexer(36u, (const char*) nulEscapedInString, sizeof nulEscapedInString);
    take(lexer, SYNTAX_ROLE_INVALID, 0u, 4u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 4u, 5u);
    take(lexer, SYNTAX_ROLE_INVALID, 5u, 6u);
    C23Lexer_free(lexer);

    const unsigned char nulInCharacter[] = {'\'', 'a', 0u, '\''};
    lexer = C23Lexer(37u, (const char*) nulInCharacter, sizeof nulInCharacter);
    take(lexer, SYNTAX_ROLE_INVALID, 0u, 3u);
    take(lexer, SYNTAX_ROLE_INVALID, 3u, 4u);
    C23Lexer_free(lexer);

    const unsigned char maximalSubparts[] = {
        0xE0u, 0x80u, 0x80u, 0xF1u, 0x80u, 'x'
    };
    lexer = C23Lexer(29u, (const char*) maximalSubparts, sizeof maximalSubparts);
    take(lexer, SYNTAX_ROLE_INVALID, 0u, 1u);
    take(lexer, SYNTAX_ROLE_INVALID, 1u, 2u);
    take(lexer, SYNTAX_ROLE_INVALID, 2u, 3u);
    take(lexer, SYNTAX_ROLE_INVALID, 3u, 5u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 5u, 6u);
    C23Lexer_free(lexer);

    const unsigned char commentBytes[] = {'/', '*', 'a', 0xE1u, 0x80u, 'x', '*', '/', 'z'};
    lexer = C23Lexer(24u, (const char*) commentBytes, sizeof commentBytes);
    take(lexer, SYNTAX_ROLE_COMMENT, 0u, 3u);
    take(lexer, SYNTAX_ROLE_INVALID, 3u, 5u);
    take(lexer, SYNTAX_ROLE_COMMENT, 5u, 8u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 8u, 9u);
    C23Lexer_free(lexer);

    const unsigned char stringBytes[] = {'"', 'a', 0xE1u, 0x80u, 'b', '"'};
    lexer = C23Lexer(25u, (const char*) stringBytes, sizeof stringBytes);
    take(lexer, SYNTAX_ROLE_STRING, 0u, 2u);
    take(lexer, SYNTAX_ROLE_INVALID, 2u, 4u);
    take(lexer, SYNTAX_ROLE_STRING, 4u, 6u);
    C23Lexer_free(lexer);

    const char validUtf8String[] = {'"', (char) 0xC3u, (char) 0xA9u, '"'};
    lexer = C23Lexer(33u, validUtf8String, sizeof validUtf8String);
    take(lexer, SYNTAX_ROLE_STRING, 0u, sizeof validUtf8String);
    C23Lexer_free(lexer);

    source = ";;INTENTION(\"x\")\n";
    lexer = C23Lexer(20u, source, strlen(source));
    take(lexer, SYNTAX_ROLE_PUNCTUATION, 0u, 1u);
    take(lexer, SYNTAX_ROLE_PUNCTUATION, 1u, 2u);
    C23Lexer_free(lexer);

    lexer = C23Lexer(20u, source, strlen(source));
    assert(C23Lexer_setProfile(lexer, C23_LEXER_PROFILE_VEX_ANNOTATION));
    C23LexStep sigil = take(lexer, SYNTAX_ROLE_ANNOTATION_SIGIL, 0u, 2u);
    assert(strcmp(sigil.span.styleKey, "annotation.sigil") == 0);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 2u, 11u);
    take(lexer, SYNTAX_ROLE_BRACKET, 11u, 12u);
    take(lexer, SYNTAX_ROLE_STRING, 12u, 15u);
    take(lexer, SYNTAX_ROLE_BRACKET, 15u, 16u);
    C23Lexer_free(lexer);

    source = "int value; ;;INTENTION(\"x\")";
    lexer = C23Lexer(39u, source, strlen(source));
    assert(C23Lexer_setProfile(lexer, C23_LEXER_PROFILE_VEX_ANNOTATION));
    take(lexer, SYNTAX_ROLE_KEYWORD, 0u, 3u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 4u, 9u);
    take(lexer, SYNTAX_ROLE_PUNCTUATION, 9u, 10u);
    take(lexer, SYNTAX_ROLE_PUNCTUATION, 11u, 12u);
    take(lexer, SYNTAX_ROLE_PUNCTUATION, 12u, 13u);
    C23Lexer_free(lexer);

    source = "/* open";
    lexer = C23Lexer(21u, source, strlen(source));
    take(lexer, SYNTAX_ROLE_INVALID, 0u, strlen(source));
    C23Lexer_free(lexer);

    source = "\"bad\nnext";
    lexer = C23Lexer(22u, source, strlen(source));
    take(lexer, SYNTAX_ROLE_INVALID, 0u, 4u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 5u, 9u);
    C23Lexer_free(lexer);

    source = "// doc\nname /* done */";
    lexer = C23Lexer(30u, source, strlen(source));
    take(lexer, SYNTAX_ROLE_COMMENT, 0u, 6u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 7u, 11u);
    take(lexer, SYNTAX_ROLE_COMMENT, 12u, 22u);
    C23Lexer_free(lexer);

    source = "/** doc */ /*! doc */ /// doc\n//! doc";
    lexer = C23Lexer(34u, source, strlen(source));
    take(lexer, SYNTAX_ROLE_DOC_COMMENT, 0u, 10u);
    take(lexer, SYNTAX_ROLE_DOC_COMMENT, 11u, 21u);
    take(lexer, SYNTAX_ROLE_DOC_COMMENT, 22u, 29u);
    take(lexer, SYNTAX_ROLE_DOC_COMMENT, 30u, 37u);
    C23Lexer_free(lexer);

    source = "a\\\nb.sh";
    lexer = C23Lexer(31u, source, strlen(source));
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 0u, 1u);
    take(lexer, SYNTAX_ROLE_INVALID, 1u, 2u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 3u, 4u);
    C23Lexer_free(lexer);

    source = "\"a\\\nb.sh";
    lexer = C23Lexer(38u, source, strlen(source));
    take(lexer, SYNTAX_ROLE_INVALID, 0u, 3u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 4u, 5u);
    C23Lexer_free(lexer);

    source = "'c\\\nd";
    lexer = C23Lexer(39u, source, strlen(source));
    take(lexer, SYNTAX_ROLE_INVALID, 0u, 3u);
    take(lexer, SYNTAX_ROLE_IDENTIFIER, 4u, 5u);
    C23Lexer_free(lexer);

    const char *coverageSamples[] = {
        "int a; // comment\nchar *b.sh = \"text\";",
        "a\\\nb.sh",
        "\"a\\\nb.sh\"",
        "'a\\\nb.sh'",
        (const char*) nulInString,
        (const char*) nulEscapedInString,
        (const char*) nulInCharacter
    };
    const size_t coverageLengths[] = {
        strlen(coverageSamples[0]), strlen(coverageSamples[1]), strlen(coverageSamples[2]),
        strlen(coverageSamples[3]), sizeof nulInString, sizeof nulEscapedInString,
        sizeof nulInCharacter
    };
    for (size_t index = 0u; index < sizeof coverageSamples / sizeof coverageSamples[0]; ++index)
        assertCoverage((uint32_t) (40u + index), coverageSamples[index], coverageLengths[index]);

    assert(!C23Lexer_setSource(nullptr, 1u, "x", 1u));
    assert(!C23Lexer_setProfile(nullptr, C23_LEXER_PROFILE_STRICT_C23));
    assert(C23Lexer_getOffset(nullptr) == 0u);
    assert(!C23Lexer_next(nullptr).hasSpan);
    BoundedText nullProjection = C23Lexer_toString(nullptr);
    assert(BoundedText_equals(&nullProjection, "nullptr"));
    BoundedText nullStructProjection = C23Lexer_toStringStruct(nullptr);
    assert(BoundedText_equals(&nullStructProjection, "nullptr"));
    return 0;
}
