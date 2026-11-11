#include "index/project_index.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/** ProjectIndex owns published FileAnalysis contributions, keyed by file id
 * and compilation context. A contribution is copied and validated before it
 * replaces anything, and its revision must advance monotonically in that key. */

;;OVERVIEW
/** CLASS: ProjectIndex (index/project_index.c)
 * STRUCT FIELDS: borrowed LanguageProject, owned FileAnalysis pointer table,
 * contribution count and capacity.
 * FUNCTION REGISTRY: constructor/free/publish, analysis lookup, declaration and
 * reference queries, project identity getter.
 * CONTRACT: published analyses must target an inventoried file; same-id symbols
 * across the project preserve name/kind/scope identity; failed publish preserves
 * the prior contribution. */

struct ProjectIndex {
    const LanguageProject *project;
    FileAnalysis **analyses;
    bytes analysisCount;
    bytes analysisCapacity;
};

static bool symbolMatches(const Symbol *left, const Symbol *right) {
    return (*left).id == (*right).id && (*left).scopeId == (*right).scopeId
        && (*left).kind == (*right).kind && (*left).name != nullptr
        && (*right).name != nullptr && strcmp((*left).name, (*right).name) == 0;
}

static FileAnalysis *findMutable(ProjectIndex *self, u64 fileId, u64 contextId,
                                 bytes *indexOut) {
    for (bytes index = 0u; index < (*self).analysisCount; ++index) {
        FileAnalysis *analysis = (*self).analyses[index];
        if (FileAnalysis_getFileId(analysis) == fileId
            && FileAnalysis_getCompilationContextId(analysis) == contextId) {
            if (indexOut != nullptr)
                *indexOut = index;
            return analysis;
        }
    }
    return nullptr;
}

static FileAnalysis *copyAnalysis(const FileAnalysis *source) {
    FileAnalysis *copy = FileAnalysis_4(FileAnalysis_getFileId(source),
        FileAnalysis_getDocumentId(source), FileAnalysis_getDocumentRevision(source),
        FileAnalysis_getCompilationContextId(source));
    if (copy == nullptr)
        return nullptr;
    for (bytes index = 0u; index < FileAnalysis_getSymbolCount(source); ++index)
        if (!FileAnalysis_addSymbol(copy, FileAnalysis_getSymbolAt(source, index))) {
            FileAnalysis_free(copy);
            return nullptr;
        }
    for (bytes index = 0u; index < FileAnalysis_getReferenceCount(source); ++index)
        if (!FileAnalysis_addReference(copy, FileAnalysis_getReferenceAt(source, index))) {
            FileAnalysis_free(copy);
            return nullptr;
        }
    return copy;
}

ProjectIndex *ProjectIndex_1(const LanguageProject *project) {
    if (project == nullptr) {
        THROW("ProjectIndex: rejected null project");
        return nullptr;
    }
    ProjectIndex *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("ProjectIndex: allocation failed");
        return nullptr;
    }
    (*self).project = project;
    return self;
}

void ProjectIndex_free(ProjectIndex *self) {
    if (self == nullptr)
        return;
    for (bytes index = 0u; index < (*self).analysisCount; ++index)
        FileAnalysis_free((*self).analyses[index]);
    free((*self).analyses);
    (*self).analyses = nullptr;
    free(self);
}

bool ProjectIndex_publish(ProjectIndex *self, const FileAnalysis *analysis) {
    if (self == nullptr || analysis == nullptr
        || LanguageProject_findFileById((*self).project,
                                        FileAnalysis_getFileId(analysis)) == nullptr) {
        THROW("ProjectIndex: rejected contribution for unknown project file");
        return false;
    }
    u64 fileId = FileAnalysis_getFileId(analysis);
    u64 contextId = FileAnalysis_getCompilationContextId(analysis);
    bytes replaceIndex = 0u;
    FileAnalysis *previous = findMutable(self, fileId, contextId, &replaceIndex);
    if (previous != nullptr
        && FileAnalysis_getDocumentRevision(analysis)
            <= FileAnalysis_getDocumentRevision(previous)) {
        THROW("ProjectIndex: rejected stale or repeated document revision");
        return false;
    }
    for (bytes incomingIndex = 0u;
         incomingIndex < FileAnalysis_getSymbolCount(analysis); ++incomingIndex) {
        Symbol incoming = FileAnalysis_getSymbolAt(analysis, incomingIndex);
        for (bytes existingIndex = 0u; existingIndex < (*self).analysisCount; ++existingIndex) {
            FileAnalysis *existingAnalysis = (*self).analyses[existingIndex];
            if (existingAnalysis == previous)
                continue;
            for (bytes symbolIndex = 0u;
                 symbolIndex < FileAnalysis_getSymbolCount(existingAnalysis); ++symbolIndex) {
                Symbol existing = FileAnalysis_getSymbolAt(existingAnalysis, symbolIndex);
                if (existing.id == incoming.id && !symbolMatches(&existing, &incoming)) {
                    THROW("ProjectIndex: rejected conflicting project symbol identity");
                    return false;
                }
            }
        }
    }
    FileAnalysis *copy = copyAnalysis(analysis);
    if (copy == nullptr)
        return false;
    if (previous != nullptr) {
        (*self).analyses[replaceIndex] = copy;
        FileAnalysis_free(previous);
        return true;
    }
    if ((*self).analysisCount == (*self).analysisCapacity) {
        bytes capacity = (*self).analysisCapacity == 0u ? 8u : (*self).analysisCapacity * 2u;
        if (capacity < (*self).analysisCapacity || capacity > SIZE_MAX / sizeof(FileAnalysis*)) {
            THROW("ProjectIndex: contribution table capacity overflow");
            FileAnalysis_free(copy);
            return false;
        }
        FileAnalysis **grown = realloc((*self).analyses, capacity * sizeof(FileAnalysis*));
        if (grown == nullptr) {
            THROW("ProjectIndex: allocation failed for %zu contributions", capacity);
            FileAnalysis_free(copy);
            return false;
        }
        (*self).analyses = grown;
        (*self).analysisCapacity = capacity;
    }
    (*self).analyses[(*self).analysisCount] = copy;
    ++(*self).analysisCount;
    return true;
}

const FileAnalysis *ProjectIndex_findAnalysis(const ProjectIndex *self, u64 fileId,
                                             u64 compilationContextId) {
    if (self == nullptr || fileId == 0u || compilationContextId == 0u)
        return nullptr;
    for (bytes index = 0u; index < (*self).analysisCount; ++index) {
        FileAnalysis *analysis = (*self).analyses[index];
        if (FileAnalysis_getFileId(analysis) == fileId
            && FileAnalysis_getCompilationContextId(analysis) == compilationContextId)
            return analysis;
    }
    return nullptr;
}
bytes ProjectIndex_getAnalysisCount(const ProjectIndex *self) {
    return self == nullptr ? 0u : (*self).analysisCount;
}
bytes ProjectIndex_getDeclarationCount(const ProjectIndex *self, u64 symbolId,
                                        u64 compilationContextId) {
    if (self == nullptr || symbolId == 0u || compilationContextId == 0u)
        return 0u;
    bytes count = 0u;
    for (bytes fileIndex = 0u; fileIndex < (*self).analysisCount; ++fileIndex) {
        FileAnalysis *analysis = (*self).analyses[fileIndex];
        if (FileAnalysis_getCompilationContextId(analysis) != compilationContextId)
            continue;
        for (bytes index = 0u; index < FileAnalysis_getSymbolCount(analysis); ++index)
            if (FileAnalysis_getSymbolAt(analysis, index).id == symbolId)
                ++count;
    }
    return count;
}
Symbol ProjectIndex_getDeclarationAt(const ProjectIndex *self, u64 symbolId,
                                     u64 compilationContextId, bytes targetIndex) {
    if (self == nullptr || symbolId == 0u || compilationContextId == 0u)
        return Symbol_zero();
    bytes found = 0u;
    for (bytes fileIndex = 0u; fileIndex < (*self).analysisCount; ++fileIndex) {
        FileAnalysis *analysis = (*self).analyses[fileIndex];
        if (FileAnalysis_getCompilationContextId(analysis) != compilationContextId)
            continue;
        for (bytes index = 0u; index < FileAnalysis_getSymbolCount(analysis); ++index) {
            Symbol symbol = FileAnalysis_getSymbolAt(analysis, index);
            if (symbol.id == symbolId && found++ == targetIndex)
                return symbol;
        }
    }
    return Symbol_zero();
}
bytes ProjectIndex_getReferenceCount(const ProjectIndex *self, u64 symbolId,
                                      u64 compilationContextId) {
    if (self == nullptr || symbolId == 0u || compilationContextId == 0u)
        return 0u;
    bytes count = 0u;
    for (bytes fileIndex = 0u; fileIndex < (*self).analysisCount; ++fileIndex) {
        FileAnalysis *analysis = (*self).analyses[fileIndex];
        if (FileAnalysis_getCompilationContextId(analysis) != compilationContextId)
            continue;
        for (bytes index = 0u; index < FileAnalysis_getReferenceCount(analysis); ++index) {
            Reference reference = FileAnalysis_getReferenceAt(analysis, index);
            if (reference.resolved && reference.targetSymbolId == symbolId)
                ++count;
        }
    }
    return count;
}
Reference ProjectIndex_getReferenceAt(const ProjectIndex *self, u64 symbolId,
                                      u64 compilationContextId, bytes targetIndex) {
    if (self == nullptr || symbolId == 0u || compilationContextId == 0u)
        return Reference_zero();
    bytes found = 0u;
    for (bytes fileIndex = 0u; fileIndex < (*self).analysisCount; ++fileIndex) {
        FileAnalysis *analysis = (*self).analyses[fileIndex];
        if (FileAnalysis_getCompilationContextId(analysis) != compilationContextId)
            continue;
        for (bytes index = 0u; index < FileAnalysis_getReferenceCount(analysis); ++index) {
            Reference reference = FileAnalysis_getReferenceAt(analysis, index);
            if (reference.resolved && reference.targetSymbolId == symbolId
                && found++ == targetIndex)
                return reference;
        }
    }
    return Reference_zero();
}
u64 ProjectIndex_getProjectId(const ProjectIndex *self) {
    return self == nullptr ? 0u : LanguageProject_getId((*self).project);
}
