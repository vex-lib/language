#include "token/token.h"
#include <assert.h>

int main(void) {
    Token empty = Token();
    Token zero = Token_zero();
    assert(Token_getKind(&empty) == TOKEN_KIND_UNKNOWN);
    assert(Token_getKind(&zero) == TOKEN_KIND_UNKNOWN);
    SourceRange range = SourceRange(1, 4, 8);
    Token token = Token(TOKEN_KIND_IDENTIFIER, range);
    assert(Token_getKind(&token) == TOKEN_KIND_IDENTIFIER);
    SourceRange tokenRange = Token_getRange(&token);
    assert(SourceRange_getStartByte(&tokenRange) == 4);
    Token_setKind(&token, TOKEN_KIND_KEYWORD);
    assert(Token_getKind(&token) == TOKEN_KIND_KEYWORD);
    assert(Token_getKind(nullptr) == TOKEN_KIND_UNKNOWN);
    return 0;
}
