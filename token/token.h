#ifndef LANGUAGE_TOKEN_TOKEN_H
#define LANGUAGE_TOKEN_TOKEN_H

#include "document/source_range.h"
#include <stdbool.h>
#include <stdint.h>

// Shared coarse lexical categories. Adapters may retain finer private classes;
// these values do not encode semantic identity or presentation color.
typedef enum TokenKind {
    TOKEN_KIND_UNKNOWN = 0, TOKEN_KIND_IDENTIFIER, TOKEN_KIND_KEYWORD,
    TOKEN_KIND_NUMBER, TOKEN_KIND_STRING, TOKEN_KIND_CHARACTER,
    TOKEN_KIND_COMMENT, TOKEN_KIND_OPERATOR, TOKEN_KIND_PUNCTUATION,
    TOKEN_KIND_PREPROCESSOR, TOKEN_KIND_DIFF_FILE, TOKEN_KIND_DIFF_METADATA,
    TOKEN_KIND_DIFF_HUNK, TOKEN_KIND_DIFF_ADDITION, TOKEN_KIND_DIFF_REMOVAL,
    TOKEN_KIND_DIFF_CONTEXT
} TokenKind;

// One token is a category and source location; spelling remains in Document.
typedef struct Token {
    TokenKind kind;  // coarse lexical category; zero is unknown
    SourceRange range; // half-open source span; producer supplies valid range
} Token;

// Construct an empty token, a kind-only token, or kind plus source range.
Token Token_0();
Token Token_1(TokenKind kind);
Token Token_2(TokenKind kind, SourceRange range);
// Dispatch Token(), Token(kind), or Token(kind, range); other arities reject.
#define Token(...) TOKEN_SELECT(_ __VA_OPT__(,) __VA_ARGS__, Token_2, Token_1, Token_0)(__VA_ARGS__)
#define TOKEN_SELECT(_0, _1, _2, NAME, ...) NAME
// Return the empty/unknown token identity.
Token Token_zero();
// Symmetric field accessors. Null getters return UNKNOWN/zero range; null
// setters do nothing.
TokenKind Token_getKind(const Token *self);
fn Token_setKind(Token *self, TokenKind value);
SourceRange Token_getRange(const Token *self);
fn Token_setRange(Token *self, SourceRange value);

#endif
