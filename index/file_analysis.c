#include "index/file_analysis.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
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

static void *reserve(void *items, bytes *capacity, bytes count,
                     bytes itemSize, const char *kind) {
    if (count < *capacity)
        return items;
    bytes next = *capacity == 0u ? 8u : *capacity * 2u;
    if (next < *capacity || next > SIZE_MAX / itemSize) {
        THROW("FileAnalysis: %s capacity overflow", kind);
        return nullptr;
    }
    void *grown = realloc(items, next * itemSize);
    if (grown == nullptr) {
        THROW("FileAnalysis: allocation failed for %s capacity %zu", kind, next);
        return nullptr;
    }
    *capacity = next;
    return grown;
}

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
    (*self).fileId = fileId;
    (*self).documentId = documentId;
    (*self).documentRevision = documentRevision;
    (*self).compilationContextId = compilationContextId;
    return self;
}

void FileAnalysis_free(FileAnalysis *self) {
    if (self == nullptr)
        return;
    for (bytes index = 0u; index < (*self).symbolCount; ++index)
        free((void*) (*self).symbols[index].name);
    free((*self).symbols);
    (*self).symbols = nullptr;
    free((*self).references);
    (*self).references = nullptr;
    free(self);
}

bool FileAnalysis_addSymbol(FileAnalysis *self, Symbol symbol) {
    if (self == nullptr || symbol.id == 0u || symbol.name == nullptr || symbol.name[0] == '\0'
        || symbol.declarationRange.documentId != (*self).documentId
        || !SourceRange_isValid(&symbol.declarationRange)
        || symbol.kind <= SYMBOL_KIND_UNKNOWN || symbol.kind > SYMBOL_KIND_TYPE_ALIAS) {
        THROW("FileAnalysis: rejected invalid declaration identity or range");
        return false;
    }
    for (bytes index = 0u; index < (*self).symbolCount; ++index)
        if ((*self).symbols[index].id == symbol.id) {
            THROW("FileAnalysis: duplicate symbol id in one file contribution");
            return false;
        }
    char *name = copyName(symbol.name);
    if (name == nullptr) {
        THROW("FileAnalysis: allocation failed copying symbol name");
        return false;
    }
    Symbol *symbols = reserve((*self).symbols, &(*self).symbolCapacity,
                              (*self).symbolCount, sizeof(Symbol), "symbol");
    if (symbols == nullptr) {
        free(name);
        return false;
    }
    (*self).symbols = symbols;
    symbol.name = name;
    (*self).symbols[(*self).symbolCount] = symbol;
    ++(*self).symbolCount;
    return true;
}

bool FileAnalysis_addReference(FileAnalysis *self, Reference reference) {
    if (self == nullptr || reference.range.documentId != (*self).documentId
        || !SourceRange_isValid(&reference.range)
        || (reference.resolved && reference.targetSymbolId == 0u)
        || (!reference.resolved && reference.targetSymbolId != 0u)
        || reference.role < REFERENCE_ROLE_UNKNOWN
        || reference.role > REFERENCE_ROLE_CONTROL_FLOW) {
        THROW("FileAnalysis: rejected invalid reference target or range");
        return false;
    }
    Reference *references = reserve((*self).references, &(*self).referenceCapacity,
                                    (*self).referenceCount, sizeof(Reference), "reference");
    if (references == nullptr)
        return false;
    (*self).references = references;
    (*self).references[(*self).referenceCount] = reference;
    ++(*self).referenceCount;
    return true;
}
u64 FileAnalysis_getFileId(const FileAnalysis *self) { return self == nullptr ? 0u : (*self).fileId; }
u32 FileAnalysis_getDocumentId(const FileAnalysis *self) { return self == nullptr ? 0u : (*self).documentId; }
u64 FileAnalysis_getDocumentRevision(const FileAnalysis *self) {
    return self == nullptr ? 0u : (*self).documentRevision;
}
u64 FileAnalysis_getCompilationContextId(const FileAnalysis *self) {
    return self == nullptr ? 0u : (*self).compilationContextId;
}
bytes FileAnalysis_getSymbolCount(const FileAnalysis *self) {
    return self == nullptr ? 0u : (*self).symbolCount;
}
Symbol FileAnalysis_getSymbolAt(const FileAnalysis *self, bytes index) {
    return self == nullptr || index >= (*self).symbolCount ? Symbol_zero() : (*self).symbols[index];
}
bytes FileAnalysis_getReferenceCount(const FileAnalysis *self) {
    return self == nullptr ? 0u : (*self).referenceCount;
}
Reference FileAnalysis_getReferenceAt(const FileAnalysis *self, bytes index) {
    return self == nullptr || index >= (*self).referenceCount
        ? Reference_zero() : (*self).references[index];
}
