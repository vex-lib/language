#ifndef LANGUAGE_INDEX_FILE_ANALYSIS_H
#define LANGUAGE_INDEX_FILE_ANALYSIS_H

#include "semantic/reference.h"
#include "semantic/symbol.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct FileAnalysis FileAnalysis;

FileAnalysis *FileAnalysis_4(u64 fileId, u32 documentId,
                             u64 documentRevision,
                             u64 compilationContextId);
void FileAnalysis_free(FileAnalysis *self);
bool FileAnalysis_addSymbol(FileAnalysis *self, Symbol symbol);
bool FileAnalysis_addReference(FileAnalysis *self, Reference reference);
u64 FileAnalysis_getFileId(const FileAnalysis *self);
u32 FileAnalysis_getDocumentId(const FileAnalysis *self);
u64 FileAnalysis_getDocumentRevision(const FileAnalysis *self);
u64 FileAnalysis_getCompilationContextId(const FileAnalysis *self);
bytes FileAnalysis_getSymbolCount(const FileAnalysis *self);
Symbol FileAnalysis_getSymbolAt(const FileAnalysis *self, bytes index);
bytes FileAnalysis_getReferenceCount(const FileAnalysis *self);
Reference FileAnalysis_getReferenceAt(const FileAnalysis *self, bytes index);

#endif
