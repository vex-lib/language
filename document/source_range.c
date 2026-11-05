#include "document/source_range.h"
#include "annotation/annotation.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: SourceRange
 * ============================================================================
 * SourceRange identifies one half-open byte interval in one document snapshot.
 * It is the shared location value used by tokens and semantic records: the
 * first byte belongs to the range and the end byte does not. The class stores
 * no source pointer and owns no document memory. Its documentId is supplied by
 * a future document owner; zero is reserved for the empty/unbound value.
 *
 * This record deliberately speaks byte offsets only. UTF-8 scalar boundaries,
 * line/column lookup, LSP UTF-8/UTF-16 position conversion, and document-version
 * validation belong to the document service and are not inferred here. Values
 * are copied by value and may be retained only as long as their document id and
 * revision are meaningful to the consumer.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: SourceRange (document/source_range.c)
 * ============================================================================
 * A small document-relative, half-open byte interval.
 *
 * STRUCT FIELDS (Mirroring document/source_range.h):
 * ----------------------------------------------------------------------------
 *   SourceRange {
 *     uint32_t documentId; // owner-assigned identity; zero means unbound
 *     uint64_t startByte;  // inclusive byte offset into the document snapshot
 *     uint64_t endByte;    // exclusive byte offset; must not precede startByte
 *   }
 *
 * PRIVATE HELPERS: none.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   SourceRange SourceRange_0(void)
 *     Construct the all-zero, unbound range.
 *   SourceRange SourceRange_1(uint32_t documentId)
 *     Construct a document-associated empty range at byte offset zero.
 *   SourceRange SourceRange_3(uint32_t documentId, uint64_t startByte,
 *                             uint64_t endByte)
 *     Construct a range value; it does not validate it.
 *   SourceRange SourceRange_zero(void)
 *     Return the additive/empty identity, equivalent to SourceRange_0.
 *   SourceRange(...) selects the advertised zero-, one-, or three-argument form.
 *
 * Core operations:
 *   bool SourceRange_isValid(const SourceRange *self)
 *     True only for a non-null range with nonzero document id and ordered ends.
 *
 * Setters:
 *   void SourceRange_setDocumentId(SourceRange *self, uint32_t value)
 *   void SourceRange_setStartByte(SourceRange *self, uint64_t value)
 *   void SourceRange_setEndByte(SourceRange *self, uint64_t value)
 *     Replace the selected field. Setters are null-safe; cross-field validity
 *     is observed by SourceRange_isValid and is not maintained automatically.
 *
 * Getters:
 *   uint32_t SourceRange_getDocumentId(const SourceRange *self)
 *   uint64_t SourceRange_getStartByte(const SourceRange *self)
 *   uint64_t SourceRange_getEndByte(const SourceRange *self)
 *     Return the field, or zero when self is nullptr.
 *
 * CONTRACT: this is a value record, not a source-map or line-index service.
 * Invalid endpoint order remains representable so callers can validate and
 * reject it at their cold admission seam. Null getters return zero/false;
 * null setters do nothing. No allocation, IO, parsing, or UTF conversion occurs.
 * ============================================================================
 */
SourceRange SourceRange_0(void) { return (SourceRange) {0}; }
SourceRange SourceRange_1(uint32_t documentId) { return (SourceRange) {.documentId = documentId}; }
SourceRange SourceRange_3(uint32_t documentId, uint64_t startByte, uint64_t endByte) {
    return (SourceRange) {.documentId = documentId, .startByte = startByte, .endByte = endByte};
}
SourceRange SourceRange_zero(void) { return SourceRange_0(); }
bool SourceRange_isValid(const SourceRange *self) {
    return self != nullptr && (*self).documentId != 0 && (*self).startByte <= (*self).endByte;
}
uint32_t SourceRange_getDocumentId(const SourceRange *self) { return self == nullptr ? 0 : (*self).documentId; }
void SourceRange_setDocumentId(SourceRange *self, uint32_t value) { if (self != nullptr) (*self).documentId = value; }
uint64_t SourceRange_getStartByte(const SourceRange *self) { return self == nullptr ? 0 : (*self).startByte; }
void SourceRange_setStartByte(SourceRange *self, uint64_t value) { if (self != nullptr) (*self).startByte = value; }
uint64_t SourceRange_getEndByte(const SourceRange *self) { return self == nullptr ? 0 : (*self).endByte; }
void SourceRange_setEndByte(SourceRange *self, uint64_t value) { if (self != nullptr) (*self).endByte = value; }
