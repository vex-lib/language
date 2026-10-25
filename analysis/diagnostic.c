#include "analysis/diagnostic.h"
#include "annotation/definition.h"
#include "annotation/overview.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: Diagnostic
 * ============================================================================
 * Diagnostic is one language-neutral report: a half-open source range, a
 * severity, a stable machine-readable code and a bounded message. An adapter
 * produces it after its own parse; a host renders it without learning the
 * adapter's private vocabulary. The record owns its message bytes inside
 * BoundedText and borrows nothing, so a diagnostic may outlive the adapter
 * that produced it. Severity and code spellings are cold projections; no
 * terminal or UI type appears here.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Diagnostic (analysis/diagnostic.c)
 * STRUCT FIELDS (Mirroring analysis/diagnostic.h)
 *   SourceRange range              // half-open byte range in one document
 *   DiagnosticSeverity severity    // hint, info, warning or error
 *   DiagnosticCode code            // stable category, never a display string
 *   BoundedText message            // owned bounded text, truncation-flagged
 *
 * PRIVATE DATA
 *   SEVERITIES[]  // severity spellings, indexed by DiagnosticSeverity
 *   CODES[]       // code spellings, indexed by DiagnosticCode
 *
 * FUNCTION REGISTRY
 *   constructors: Diagnostic_0, Diagnostic_4, Diagnostic(...), Diagnostic_zero
 *   enum names:   DiagnosticSeverity_name, DiagnosticCode_name
 *   projections:  Diagnostic_toString, Diagnostic_toStringStruct
 *
 * CONTRACT
 *   Unknown severity/code values project as "unknown". A null self projects
 *   "nullptr". Construction validates nothing; callers pass values from the
 *   declared enums. The message is copied into the value at construction.
 * ============================================================================
 */

static const char *SEVERITIES[] = {"hint", "info", "warning", "error"};
static const char *CODES[] = {"none", "unnecessary-semicolon", "empty-body", "invalid-claim", "ambiguous-syntax", "unknown-name", "style-key-invalid"};

Diagnostic Diagnostic_0(void) { return (Diagnostic) {0}; }
Diagnostic Diagnostic_zero(void) { return Diagnostic_0(); }
Diagnostic Diagnostic_4(SourceRange range, DiagnosticSeverity severity, DiagnosticCode code, BoundedText message) {
    return (Diagnostic) {.range = range, .severity = severity, .code = code, .message = message};
}
const char *DiagnosticSeverity_name(DiagnosticSeverity value) {
    return (unsigned) value < sizeof SEVERITIES / sizeof SEVERITIES[0] ? SEVERITIES[value] : "unknown";
}
const char *DiagnosticCode_name(DiagnosticCode value) {
    return (unsigned) value < sizeof CODES / sizeof CODES[0] ? CODES[value] : "unknown";
}
BoundedText Diagnostic_toString(const Diagnostic *self) {
    if (self == nullptr) return BoundedText_1("nullptr");
    BoundedText message = (*self).message;
    return BoundedText_format("%s: %s", DiagnosticSeverity_name((*self).severity), message.text);
}
BoundedText Diagnostic_toStringStruct(const Diagnostic *self) {
    if (self == nullptr) return BoundedText_1("nullptr");
    SourceRange range = (*self).range;
    return BoundedText_format("Diagnostic { severity=%s, code=%s, range=%u:%llu-%llu }",
        DiagnosticSeverity_name((*self).severity), DiagnosticCode_name((*self).code),
        range.documentId, (unsigned long long) range.startByte,
        (unsigned long long) range.endByte);
}
