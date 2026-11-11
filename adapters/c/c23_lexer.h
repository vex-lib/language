#ifndef LANGUAGE_ADAPTERS_C_C23_LEXER_H
#define LANGUAGE_ADAPTERS_C_C23_LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "adapters/c/c23_lex_step.h"
#include "text/bounded_text.h"

typedef struct C23Lexer C23Lexer;

// Strict C23 treats ;; as two punctuation tokens. The opt-in VEX profile only
// changes a line-leading ;; followed by an ASCII identifier into an annotated
// sigil span; it does not validate the annotation name or arguments.
typedef enum C23LexerProfile {
    C23_LEXER_PROFILE_STRICT_C23 = 0,
    C23_LEXER_PROFILE_VEX_ANNOTATION = 1
} C23LexerProfile;

C23Lexer *C23Lexer_0(void);
C23Lexer *C23Lexer_3(u32 documentId, const char *source, bytes length);
#define C23Lexer_1 C23Lexer_invalid_arity
#define C23Lexer_2 C23Lexer_invalid_arity
#define C23Lexer_4 C23Lexer_invalid_arity
#define C23Lexer_5 C23Lexer_invalid_arity
#define C23Lexer_6 C23Lexer_invalid_arity
#define C23LEXER_CAT_INNER(left, right) left##right
#define C23LEXER_CAT(left, right) C23LEXER_CAT_INNER(left, right)
#define C23LEXER_NARG_INNER(_0, _1, _2, _3, _4, _5, _6, count, ...) count
#define C23LEXER_NARG(...) \
    C23LEXER_NARG_INNER(_ __VA_OPT__(,) __VA_ARGS__, 6, 5, 4, 3, 2, 1, 0)
#define C23Lexer(...) \
    C23LEXER_CAT(C23Lexer_, C23LEXER_NARG(__VA_ARGS__))(__VA_ARGS__)
C23Lexer *C23Lexer_zero(void);
void C23Lexer_free(C23Lexer *self);
// Borrows immutable bytes until the next successful setSource or free. A nonzero
// document identity is required; nullptr is allowed only with length zero.
// Rejection reports a cold THROW and preserves the prior snapshot/cursor/profile.
bool C23Lexer_setSource(C23Lexer *self, u32 documentId,
                        const char *source, bytes length);
// Profile changes affect future tokenization and reject unsupported values,
// preserving the current profile. New snapshots retain the selected profile.
bool C23Lexer_setProfile(C23Lexer *self, C23LexerProfile profile);
u32 C23Lexer_getDocumentId(const C23Lexer *self);
const char *C23Lexer_getSource(const C23Lexer *self);
bytes C23Lexer_getLength(const C23Lexer *self);
bytes C23Lexer_getOffset(const C23Lexer *self);
C23LexerProfile C23Lexer_getProfile(const C23Lexer *self);
bool C23Lexer_isDone(const C23Lexer *self);
// Returns one fully initialized half-open byte span, or a no-span zero value at
// end. ASCII identifiers only; every reserved C23 keyword including true/false/
// nullptr has KEYWORD role. C23 has no [[/]] tokens; each bracket is separate.
// Non-ASCII scalars and malformed UTF-8 are INVALID;
// malformed subsequences follow Unicode maximal-subpart boundaries. NUL is one
// invalid byte. String and character literals support u8/u/U/L prefixes;
// physical newline ends/rejects a literal even after backslash. Comments are
// non-nesting.
// Pp-number-like text is not numerically validated. No line splicing,
// preprocessing, parser, symbol resolution or hint generation is performed.
C23LexStep C23Lexer_next(C23Lexer *self);
BoundedText C23Lexer_toString(const C23Lexer *self);
BoundedText C23Lexer_toStringStruct(const C23Lexer *self);

#endif
