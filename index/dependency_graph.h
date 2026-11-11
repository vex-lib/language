#ifndef LANGUAGE_INDEX_DEPENDENCY_GRAPH_H
#define LANGUAGE_INDEX_DEPENDENCY_GRAPH_H

#include "project/language_project.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct DependencyGraph DependencyGraph;

// The graph borrows a stable LanguageProject for its lifetime. Edges point
// from a file to files it depends on; replacing one file's edges is atomic.
DependencyGraph *DependencyGraph_1(const LanguageProject *project);
void DependencyGraph_free(DependencyGraph *self);
bool DependencyGraph_setDependencies(DependencyGraph *self, u64 fileId,
                                     const u64 *dependencyFileIds,
                                     bytes dependencyCount);
bytes DependencyGraph_getDependencyCount(const DependencyGraph *self,
                                          u64 fileId);
u64 DependencyGraph_getDependencyAt(const DependencyGraph *self,
                                         u64 fileId, bytes index);
bytes DependencyGraph_getDependentCount(const DependencyGraph *self,
                                         u64 fileId);
u64 DependencyGraph_getDependentAt(const DependencyGraph *self,
                                        u64 fileId, bytes index);
// True when candidate directly or transitively depends on changedFileId.
// A file is considered affected by itself, including in an empty graph.
bool DependencyGraph_isAffectedBy(const DependencyGraph *self,
                                  u64 candidateFileId,
                                  u64 changedFileId);
u64 DependencyGraph_getProjectId(const DependencyGraph *self);

#endif
