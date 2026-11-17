#include "vex.h"
#include "index/file_analysis.h"
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/** FileAnalysis is one immutable-document and compilation-context contribution
 * before publication. It owns copies of symbol names and value copies of
 * references, so staged analysis can be safely retained or rejected. */

;;OVERVIEW
/** CLASS: FileAnalysis (index/file_analysis.c)
 * STRUCT FIELDS: file/document/revision/context identity, Symbol array with
 * owned names, and Reference array.
 * FUNCTION REGISTRY: constructor/free, append symbols/references and accessors.
 * CONTRACT: all ranges belong to documentId; symbol ids are unique within this
 * file contribution; unresolved refs use target id zero. */

struct FileAnalysis {
    u64 fileId;
    u32 documentId;
    u64 documentRevision;
    u64 compilationContextId;
    Symbol *symbols;
    bytes symbolCount;
    bytes symbolCapacity;
    Reference *references;
    bytes referenceCount;
    bytes referenceCapacity;
};

static char *copyName(const char *name) {
    bytes length = strlen(name);
    char *copy = malloc(length + 1u);
    if (copy != nullptr)
        memcpy(copy, name, length + 1u);
    return copy;
}

static any reserve(any items, bytes *capacity, bytes count,
                     bytes itemSize, const char *kind) {
    if (count < *capacity)
        return items;
    bytes next = *capacity == 0u ? 8u : *capacity * 2u;
    if (next < *capacity || next > SIZE_MAX / itemSize) {
        THROW("FileAnalysis: %s capacity overflow", kind);
        return nullptr;
    }
    any grown = realloc(items, next * itemSize);
    if (grown == nullptr) {
        THROW("FileAnalysis: allocation failed for %s capacity %zu", kind, next);
        return nullptr;
    }
    *capacity = next;
    return grown;
}

;;GETTER
static inline u64 FileAnalysis_fileId(const FileAnalysis *self) { return (*self).fileId; }
;;SETTER
static inline fn FileAnalysis_setFileId(FileAnalysis *self, u64 v) { (*self).fileId = v; }
;;GETTER
static inline u32 FileAnalysis_documentId(const FileAnalysis *self) { return (*self).documentId; }
;;SETTER
static inline fn FileAnalysis_setDocumentId(FileAnalysis *self, u32 v) { (*self).documentId = v; }
;;GETTER
static inline u64 FileAnalysis_documentRevision(const FileAnalysis *self) { return (*self).documentRevision; }
;;SETTER
static inline fn FileAnalysis_setDocumentRevision(FileAnalysis *self, u64 v) { (*self).documentRevision = v; }
;;GETTER
static inline u64 FileAnalysis_compilationContextId(const FileAnalysis *self) { return (*self).compilationContextId; }
;;SETTER
static inline fn FileAnalysis_setCompilationContextId(FileAnalysis *self, u64 v) { (*self).compilationContextId = v; }
;;GETTER
static inline Symbol *FileAnalysis_symbols(const FileAnalysis *self) { return (*self).symbols; }
;;SETTER
static inline fn FileAnalysis_setSymbols(FileAnalysis *self, Symbol *v) { (*self).symbols = v; }
;;GETTER
static inline bytes FileAnalysis_symbolCount(const FileAnalysis *self) { return (*self).symbolCount; }
;;SETTER
static inline fn FileAnalysis_setSymbolCount(FileAnalysis *self, bytes v) { (*self).symbolCount = v; }
;;GETTER
static inline bytes FileAnalysis_symbolCapacity(const FileAnalysis *self) { return (*self).symbolCapacity; }
;;SETTER
static inline fn FileAnalysis_setSymbolCapacity(FileAnalysis *self, bytes v) { (*self).symbolCapacity = v; }
;;GETTER
static inline Reference *FileAnalysis_references(const FileAnalysis *self) { return (*self).references; }
;;SETTER
static inline fn FileAnalysis_setReferences(FileAnalysis *self, Reference *v) { (*self).references = v; }
;;GETTER
static inline bytes FileAnalysis_referenceCount(const FileAnalysis *self) { return (*self).referenceCount; }
;;SETTER
static inline fn FileAnalysis_setReferenceCount(FileAnalysis *self, bytes v) { (*self).referenceCount = v; }
;;GETTER
static inline bytes FileAnalysis_referenceCapacity(const FileAnalysis *self) { return (*self).referenceCapacity; }
;;SETTER
static inline fn FileAnalysis_setReferenceCapacity(FileAnalysis *self, bytes v) { (*self).referenceCapacity = v; }

FileAnalysis *FileAnalysis_4(u64 fileId, u32 documentId,
                             u64 documentRevision,
                             u64 compilationContextId) {
    if (fileId == 0u || documentId == 0u || documentRevision == 0u
        || compilationContextId == 0u) {
        THROW("FileAnalysis: rejected zero file, document, revision or context identity");
        return nullptr;
    }
    FileAnalysis *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("FileAnalysis: allocation failed");
        return nullptr;
    }
    FileAnalysis_setFileId(self, fileId);
    FileAnalysis_setDocumentId(self, documentId);
    FileAnalysis_setDocumentRevision(self, documentRevision);
    FileAnalysis_setCompilationContextId(self, compilationContextId);
    return self;
}

fn FileAnalysis_free(FileAnalysis *self) {
    if (self == nullptr)
        return;
    for (bytes index = 0u; index < FileAnalysis_symbolCount(self); ++index)
        free((any) FileAnalysis_symbols(self)[index].name);
    free(FileAnalysis_symbols(self));
    FileAnalysis_setSymbols(self, nullptr);
    free(FileAnalysis_references(self));
    FileAnalysis_setReferences(self, nullptr);
    free(self);
}

bool FileAnalysis_addSymbol(FileAnalysis *self, Symbol symbol) {
    if (self == nullptr || symbol.id == 0u || symbol.name == nullptr || symbol.name[0] == '\0'
        || symbol.declarationRange.documentId != FileAnalysis_documentId(self)
        || !SourceRange_isValid(&symbol.declarationRange)
        || symbol.kind <= SYMBOL_KIND_UNKNOWN || symbol.kind > SYMBOL_KIND_TYPE_ALIAS) {
        THROW("FileAnalysis: rejected invalid declaration identity or range");
        return false;
    }
    for (bytes index = 0u; index < FileAnalysis_symbolCount(self); ++index)
        if (FileAnalysis_symbols(self)[index].id == symbol.id) {
            THROW("FileAnalysis: duplicate symbol id in one file contribution");
            return false;
        }
    char *name = copyName(symbol.name);
    if (name == nullptr) {
        THROW("FileAnalysis: allocation failed copying symbol name");
        return false;
    }
    bytes symbolCapacity = FileAnalysis_symbolCapacity(self);
    Symbol *symbols = reserve(FileAnalysis_symbols(self), &symbolCapacity,
                              FileAnalysis_symbolCount(self), sizeof(Symbol), "symbol");
    FileAnalysis_setSymbolCapacity(self, symbolCapacity);
    if (symbols == nullptr) {
        free(name);
        return false;
    }
    FileAnalysis_setSymbols(self, symbols);
    symbol.name = name;
    FileAnalysis_symbols(self)[FileAnalysis_symbolCount(self)] = symbol;
    FileAnalysis_setSymbolCount(self, FileAnalysis_symbolCount(self) + 1u);
    return true;
}

bool FileAnalysis_addReference(FileAnalysis *self, Reference reference) {
    if (self == nullptr || reference.range.documentId != FileAnalysis_documentId(self)
        || !SourceRange_isValid(&reference.range)
        || (reference.resolved && reference.targetSymbolId == 0u)
        || (!reference.resolved && reference.targetSymbolId != 0u)
        || reference.role < REFERENCE_ROLE_UNKNOWN
        || reference.role > REFERENCE_ROLE_CONTROL_FLOW) {
        THROW("FileAnalysis: rejected invalid reference target or range");
        return false;
    }
    bytes referenceCapacity = FileAnalysis_referenceCapacity(self);
    Reference *references = reserve(FileAnalysis_references(self), &referenceCapacity,
                                    FileAnalysis_referenceCount(self), sizeof(Reference), "reference");
    FileAnalysis_setReferenceCapacity(self, referenceCapacity);
    if (references == nullptr)
        return false;
    FileAnalysis_setReferences(self, references);
    FileAnalysis_references(self)[FileAnalysis_referenceCount(self)] = reference;
    FileAnalysis_setReferenceCount(self, FileAnalysis_referenceCount(self) + 1u);
    return true;
}
;;GETTER
u64 FileAnalysis_getFileId(const FileAnalysis *self) { return self == nullptr ? 0u : FileAnalysis_fileId(self); }
;;GETTER
u32 FileAnalysis_getDocumentId(const FileAnalysis *self) { return self == nullptr ? 0u : FileAnalysis_documentId(self); }
;;GETTER
u64 FileAnalysis_getDocumentRevision(const FileAnalysis *self) {
    return self == nullptr ? 0u : FileAnalysis_documentRevision(self);
}
;;GETTER
u64 FileAnalysis_getCompilationContextId(const FileAnalysis *self) {
    return self == nullptr ? 0u : FileAnalysis_compilationContextId(self);
}
;;GETTER
bytes FileAnalysis_getSymbolCount(const FileAnalysis *self) {
    return self == nullptr ? 0u : FileAnalysis_symbolCount(self);
}
Symbol FileAnalysis_getSymbolAt(const FileAnalysis *self, bytes index) {
    return self == nullptr || index >= FileAnalysis_symbolCount(self) ? Symbol_zero() : FileAnalysis_symbols(self)[index];
}
;;GETTER
bytes FileAnalysis_getReferenceCount(const FileAnalysis *self) {
    return self == nullptr ? 0u : FileAnalysis_referenceCount(self);
}
Reference FileAnalysis_getReferenceAt(const FileAnalysis *self, bytes index) {
    return self == nullptr || index >= FileAnalysis_referenceCount(self)
        ? Reference_zero() : FileAnalysis_references(self)[index];
}
