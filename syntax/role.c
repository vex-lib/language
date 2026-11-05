#include "syntax/role.h"
#include "annotation/annotation.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: SyntaxRole
 * ============================================================================
 * SyntaxRole is the shared syntactic vocabulary an adapter maps its own tokens
 * onto. A role names what the grammar decided, independent of the glyph and of
 * colour: `*` may be OPERATOR, POINTER, DEREFERENCE, ADDRESS or CAST_POINTER.
 * The default style key is the role's core paint key; an adapter may override
 * it on the SyntaxSpan. This module owns two immutable lookup tables and no
 * allocation; the enum and the tables must stay in the same declaration order.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: SyntaxRole (syntax/role.c)
 * VOCABULARY (Mirroring syntax/role.h)
 *   enum SyntaxRole  // NONE, shared roles, INVALID, then valid IDENTIFIER
 *
 * PRIVATE DATA
 *   ROLE_NAMES[]  // stable spelling per role, indexed by SyntaxRole
 *   ROLE_KEYS[]   // default core style key per role, indexed by SyntaxRole
 *
 * FUNCTION REGISTRY
 *   SyntaxRole_name     // stable spelling; "invalid" out of range
 *   SyntaxRole_styleKey // default core style key; "invalid" out of range
 *   SyntaxRole_isValid  // false for NONE and out-of-range values
 *
 * CONTRACT
 *   The two arrays must have exactly one row per enum value. NONE is not a
 *   valid classification but has a spelling and a "text" key. INVALID is a real
 *   role (a span the adapter could not classify) and is valid; IDENTIFIER is
 *   also valid and maps to the core "text" style key. An out-of-range number is
 *   not valid. No function allocates or retains a pointer.
 * ============================================================================
 */

static const char *ROLE_NAMES[] = {
    "none", "keyword", "type", "type.parameter", "type.argument",
    "function", "method", "constructor", "parameter", "field", "variable",
    "constant", "namespace", "label", "macro", "macro.parameter",
    "macro.invocation", "annotation", "annotation.sigil", "annotation.argument",
    "attribute", "directive", "string", "string.escape", "string.interpolation",
    "regex", "character", "number", "boolean", "null", "operator", "operator.pointer",
    "operator.cast-pointer", "operator.deref", "operator.address", "member",
    "punctuation", "bracket", "comment", "comment.doc", "inactive", "invalid",
    "identifier"
};

static const char *ROLE_KEYS[] = {
    "text", "keyword", "type", "type.parameter", "type.argument", "function",
    "method", "constructor", "parameter", "field", "variable", "constant", "namespace",
    "label", "macro", "macro.parameter", "macro.invocation", "annotation",
    "annotation.sigil", "annotation.argument", "attribute", "directive", "string",
    "string.escape", "string.interpolation", "string.regex", "character", "number",
    "boolean", "null", "operator", "operator.pointer", "operator.cast-pointer",
    "operator.deref", "operator.address", "member", "punctuation", "bracket", "comment",
    "comment.doc", "inactive", "invalid", "text"
};

const char *SyntaxRole_name(SyntaxRole role) {
    if ((unsigned) role >= sizeof ROLE_NAMES / sizeof ROLE_NAMES[0])
        return "invalid";
    return ROLE_NAMES[role];
}

const char *SyntaxRole_styleKey(SyntaxRole role) {
    if ((unsigned) role >= sizeof ROLE_KEYS / sizeof ROLE_KEYS[0])
        return "invalid";
    return ROLE_KEYS[role];
}

bool SyntaxRole_isValid(SyntaxRole role) {
    return role != SYNTAX_ROLE_NONE && (unsigned) role < sizeof ROLE_NAMES / sizeof ROLE_NAMES[0];
}
