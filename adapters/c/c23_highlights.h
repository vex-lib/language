#ifndef LANGUAGE_ADAPTERS_C_C23_HIGHLIGHTS_H
#define LANGUAGE_ADAPTERS_C_C23_HIGHLIGHTS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "analysis/syntax_span.h"
#include "text/bounded_text.h"

typedef struct C23Highlights C23Highlights;

// The source is borrowed and must remain immutable/alive until free. The class
// is a conservative lexical presentation pass over a C23 snapshot: it keeps the
// lexer's keyword/literal/comment roles and refines identifier roles from local
// spelling and neighboring tokens. Uppercase names become macros/constants;
// names ending in _t, capitalized names, and struct/union/enum tags become
// types; a name before "(" becomes a function; a name after "." or "->" becomes
// a member; every other identifier becomes a variable. A line-start
// preprocessor marker and its directive name (`#include`, `#define`, ...) join
// the macro role, while `#`/`##` inside code stay punctuation. After `#include`,
// a well-formed `<...>` header name is emitted as one string-role span (a quoted
// header already comes from the lexer as a string). It is not a parser,
// preprocessor or symbol resolver, and it never changes source bytes.
C23Highlights *C23Highlights_0(void);
C23Highlights *C23Highlights_3(u32 documentId, const char *source, bytes length);
#define C23Highlights_1 C23Highlights_invalid_arity
#define C23Highlights_2 C23Highlights_invalid_arity
#define C23Highlights_4 C23Highlights_invalid_arity
#define C23_HIGHLIGHTS_CAT_INNER(left, right) left##right
#define C23_HIGHLIGHTS_CAT(left, right) C23_HIGHLIGHTS_CAT_INNER(left, right)
#define C23_HIGHLIGHTS_NARG_INNER(_0, _1, _2, _3, _4, count, ...) count
#define C23_HIGHLIGHTS_NARG(...) \
    C23_HIGHLIGHTS_NARG_INNER(_ __VA_OPT__(,) __VA_ARGS__, 4, 3, 2, 1, 0)
#define C23Highlights(...) \
    C23_HIGHLIGHTS_CAT(C23Highlights_, C23_HIGHLIGHTS_NARG(__VA_ARGS__))(__VA_ARGS__)
C23Highlights *C23Highlights_zero(void);
void C23Highlights_free(C23Highlights *self);
bytes C23Highlights_getCount(const C23Highlights *self);
u32 C23Highlights_getDocumentId(const C23Highlights *self);
// Spans are ordered and non-overlapping. Out-of-range queries return safe zero
// values; roleAt returns SYNTAX_ROLE_NONE for whitespace or an unknown byte.
bytes C23Highlights_getStart(const C23Highlights *self, bytes index);
bytes C23Highlights_getEnd(const C23Highlights *self, bytes index);
SyntaxRole C23Highlights_getRole(const C23Highlights *self, bytes index);
SyntaxRole C23Highlights_roleAt(const C23Highlights *self, bytes byte);
BoundedText C23Highlights_toString(const C23Highlights *self);
BoundedText C23Highlights_toStringStruct(const C23Highlights *self);

#endif
