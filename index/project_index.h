#ifndef LANGUAGE_INDEX_PROJECT_INDEX_H
#define LANGUAGE_INDEX_PROJECT_INDEX_H

#include "index/file_analysis.h"
#include "project/language_project.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct ProjectIndex ProjectIndex;

ProjectIndex *ProjectIndex_1(const LanguageProject *project);
void ProjectIndex_free(ProjectIndex *self);
bool ProjectIndex_publish(ProjectIndex *self, const FileAnalysis *analysis);
const FileAnalysis *ProjectIndex_findAnalysis(const ProjectIndex *self, u64 fileId,
                                             u64 compilationContextId);
bytes ProjectIndex_getAnalysisCount(const ProjectIndex *self);
bytes ProjectIndex_getDeclarationCount(const ProjectIndex *self, u64 symbolId,
                                        u64 compilationContextId);
Symbol ProjectIndex_getDeclarationAt(const ProjectIndex *self, u64 symbolId,
                                     u64 compilationContextId, bytes index);
bytes ProjectIndex_getReferenceCount(const ProjectIndex *self, u64 symbolId,
                                      u64 compilationContextId);
Reference ProjectIndex_getReferenceAt(const ProjectIndex *self, u64 symbolId,
                                      u64 compilationContextId, bytes index);
u64 ProjectIndex_getProjectId(const ProjectIndex *self);

#endif
