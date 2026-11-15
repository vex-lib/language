#ifndef LANGUAGE_ANALYSIS_DIAGNOSTIC_H
#define LANGUAGE_ANALYSIS_DIAGNOSTIC_H

#include "document/source_range.h"
#include "text/bounded_text.h"

typedef enum DiagnosticSeverity {
    DIAGNOSTIC_SEVERITY_HINT = 0,
    DIAGNOSTIC_SEVERITY_INFO,
    DIAGNOSTIC_SEVERITY_WARNING,
    DIAGNOSTIC_SEVERITY_ERROR
} DiagnosticSeverity;

typedef enum DiagnosticCode {
    DIAGNOSTIC_CODE_NONE = 0,
    DIAGNOSTIC_CODE_UNNECESSARY_SEMICOLON,
    DIAGNOSTIC_CODE_EMPTY_BODY,
    DIAGNOSTIC_CODE_INVALID_CLAIM,
    DIAGNOSTIC_CODE_AMBIGUOUS_SYNTAX,
    DIAGNOSTIC_CODE_UNKNOWN_NAME,
    DIAGNOSTIC_CODE_STYLE_KEY_INVALID
} DiagnosticCode;

typedef struct Diagnostic {
    SourceRange range;
    DiagnosticSeverity severity;
    DiagnosticCode code;
    BoundedText message;
} Diagnostic;

Diagnostic Diagnostic_0();
Diagnostic Diagnostic_4(SourceRange range, DiagnosticSeverity severity,
                        DiagnosticCode code, BoundedText message);
#define Diagnostic(...) DIAGNOSTIC_SELECT(_ __VA_OPT__(,) __VA_ARGS__, Diagnostic_4, Diagnostic_invalid_arity, Diagnostic_invalid_arity, Diagnostic_invalid_arity, Diagnostic_0)(__VA_ARGS__)
#define DIAGNOSTIC_SELECT(_0, _1, _2, _3, _4, NAME, ...) NAME
Diagnostic Diagnostic_zero();
const char *DiagnosticSeverity_name(DiagnosticSeverity value);
const char *DiagnosticCode_name(DiagnosticCode value);
BoundedText Diagnostic_toString(const Diagnostic *self);
BoundedText Diagnostic_toStringStruct(const Diagnostic *self);

#endif
