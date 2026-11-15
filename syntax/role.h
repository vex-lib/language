#ifndef LANGUAGE_SYNTAX_ROLE_H
#define LANGUAGE_SYNTAX_ROLE_H

#include "vex.h"

// syntax/role.h — the shared SYNTACTIC vocabulary.
//
// A role is what the grammar decided about a span, independent of the glyph and
// independent of colour. `*` can be OPERATOR, POINTER, DEREFERENCE, ADDRESS or
// CAST_POINTER; `;;INTENTION` is ANNOTATION_SIGIL plus ANNOTATION plus an
// ANNOTATION_ARGUMENT. Adapters keep their own richer private vocabulary and
// map onto these names, exactly as they map onto SymbolKind.
//
// SyntaxRole_styleKey returns the CORE style key a role paints with by default.
// An adapter may override with a prefixed key in its own analysis output.

typedef enum SyntaxRole {
    SYNTAX_ROLE_NONE = 0,
    SYNTAX_ROLE_KEYWORD,
    SYNTAX_ROLE_TYPE,
    SYNTAX_ROLE_TYPE_PARAMETER,
    SYNTAX_ROLE_TYPE_ARGUMENT,
    SYNTAX_ROLE_FUNCTION,
    SYNTAX_ROLE_METHOD,
    SYNTAX_ROLE_CONSTRUCTOR,
    SYNTAX_ROLE_PARAMETER,
    SYNTAX_ROLE_FIELD,
    SYNTAX_ROLE_VARIABLE,
    SYNTAX_ROLE_CONSTANT,
    SYNTAX_ROLE_NAMESPACE,
    SYNTAX_ROLE_LABEL,
    SYNTAX_ROLE_MACRO,
    SYNTAX_ROLE_MACRO_PARAMETER,
    SYNTAX_ROLE_MACRO_INVOCATION,
    SYNTAX_ROLE_ANNOTATION,
    SYNTAX_ROLE_ANNOTATION_SIGIL,
    SYNTAX_ROLE_ANNOTATION_ARGUMENT,
    SYNTAX_ROLE_ATTRIBUTE,
    SYNTAX_ROLE_DIRECTIVE,
    SYNTAX_ROLE_STRING,
    SYNTAX_ROLE_STRING_ESCAPE,
    SYNTAX_ROLE_STRING_INTERPOLATION,
    SYNTAX_ROLE_REGEX,
    SYNTAX_ROLE_CHARACTER,
    SYNTAX_ROLE_NUMBER,
    SYNTAX_ROLE_BOOLEAN,
    SYNTAX_ROLE_NULL,
    SYNTAX_ROLE_OPERATOR,
    SYNTAX_ROLE_POINTER,
    SYNTAX_ROLE_CAST_POINTER,
    SYNTAX_ROLE_DEREFERENCE,
    SYNTAX_ROLE_ADDRESS,
    SYNTAX_ROLE_MEMBER,
    SYNTAX_ROLE_PUNCTUATION,
    SYNTAX_ROLE_BRACKET,
    SYNTAX_ROLE_COMMENT,
    SYNTAX_ROLE_DOC_COMMENT,
    SYNTAX_ROLE_INACTIVE,
    SYNTAX_ROLE_INVALID,
    SYNTAX_ROLE_IDENTIFIER
} SyntaxRole;

// Stable spelling of the enum value (for projections and diagnostics).
const char *SyntaxRole_name(SyntaxRole role);
// Core style key this role paints with by default; never null.
const char *SyntaxRole_styleKey(SyntaxRole role);
bool SyntaxRole_isValid(SyntaxRole role);

#endif
