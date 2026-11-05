# C language adapter

This directory contains an initial C23 lexical span producer and a conservative
same-snapshot inlay-hint producer. The lexer emits byte-accurate syntax roles
without preprocessing, parsing, semantic resolution, or color policy. The hint
producer recognizes only simple local function declarations/calls and places
presentation-only parameter labels at source byte offsets.

Owners are `../../../../tests/language/adapters/c/c23_lexer_test.c`,
`c23_lex_step_test.c`, and `c23_inlay_hints_test.c`, run by
`tests/language/adapters/c/c23_lexer_run.py`; syntax role coverage is owned by
`tests/language/syntax/role_test.c`. They exercise strict C23, ASan/UBSan and
constructor-arity rejection. This initial subset is not a C parser, preprocessor,
project scanner, or semantic resolver. It omits complex declarators, macros,
function pointers and cross-file declarations; consumers own colors and hint
presentation.
