#include "token/token.h"
#include "annotation/annotation.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: Token
 * ============================================================================
 * Token is the common lexical fact shared by language adapters and consumers:
 * one broad token category and one document-relative source range. It does not
 * carry token text; consumers obtain spelling from the associated document and
 * range, avoiding a second owned copy of every lexeme. The enum is a deliberately
 * small interchange vocabulary, not a complete taxonomy for any one grammar.
 *
 * A C, Rust, or other adapter may preserve richer distinctions in its private
 * parser data and map them to this common category for generic highlighting and
 * indexing. This value alone cannot establish whether an identifier names a
 * function, variable, type, macro, or unresolved name; semantic services answer
 * those questions later.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Token (token/token.c)
 * ============================================================================
 * Language-neutral lexical category plus a borrowed document span.
 *
 * STRUCT FIELDS (Mirroring token/token.h):
 * ----------------------------------------------------------------------------
 *   Token {
 *     TokenKind kind;       // common lexical category; UNKNOWN is zero
 *     SourceRange range;    // half-open source bytes, including no text copy
 *   }
 *
 * ENUM VOCABULARY:
 *   TOKEN_KIND_UNKNOWN, IDENTIFIER, KEYWORD, NUMBER, STRING, CHARACTER,
 *   COMMENT, OPERATOR, PUNCTUATION, PREPROCESSOR, DIFF_FILE, DIFF_METADATA,
 *   DIFF_HUNK, DIFF_ADDITION, DIFF_REMOVAL, DIFF_CONTEXT.
 *   These are lexical buckets, not semantic classifications or theme colors.
 *   Diff categories identify patch line roles only; they do not classify source
 *   code contained within an added/deleted line.
 *
 * PRIVATE HELPERS: none.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   Token Token_0(void)
 *     Construct the zero/unknown token.
 *   Token Token_1(TokenKind kind)
 *     Construct a kind with an empty source range.
 *   Token Token_2(TokenKind kind, SourceRange range)
 *     Construct a token value with its location.
 *   Token Token_zero(void)
 *     Return the empty token identity.
 *   Token(...) dispatches the supported zero-, one-, and two-argument forms.
 *
 * Setters:
 *   void Token_setKind(Token *self, TokenKind value)
 *   void Token_setRange(Token *self, SourceRange value)
 *     Replace a field; a null self is ignored.
 *
 * Getters:
 *   TokenKind Token_getKind(const Token *self)
 *     Return the kind, or TOKEN_KIND_UNKNOWN for nullptr.
 *   SourceRange Token_getRange(const Token *self)
 *     Return the range, or SourceRange_zero() for nullptr.
 *
 * CONTRACT: Token owns no document, spelling, parser node, or style. Producers
 * are responsible for valid ranges. The class performs no allocation, scanning,
 * syntax analysis, semantic lookup, UTF conversion, or rendering.
 * ============================================================================
 */
Token Token_0(void) { return (Token) {0}; }
Token Token_1(TokenKind kind) { return (Token) {.kind = kind}; }
Token Token_2(TokenKind kind, SourceRange range) { return (Token) {.kind = kind, .range = range}; }
Token Token_zero(void) { return Token_0(); }
TokenKind Token_getKind(const Token *self) { return self == nullptr ? TOKEN_KIND_UNKNOWN : (*self).kind; }
void Token_setKind(Token *self, TokenKind value) { if (self != nullptr) (*self).kind = value; }
SourceRange Token_getRange(const Token *self) { return self == nullptr ? SourceRange_zero() : (*self).range; }
void Token_setRange(Token *self, SourceRange value) { if (self != nullptr) (*self).range = value; }
