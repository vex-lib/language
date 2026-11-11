#include "index/dependency_graph.h"
#include "annotation/annotation.h"
#include "exception/throw.h"
#include <stdlib.h>

;;DEFINITION
/** DependencyGraph stores caller-provided dependency edges between inventoried
 * project files. It does not inspect imports or read the filesystem. */

;;OVERVIEW
/** CLASS: DependencyGraph (index/dependency_graph.c)
 * STRUCT FIELDS: borrowed LanguageProject, file-edge rows and capacity.
 * PRIVATE RECORD: DependencyRow owns one source id and dependency id array.
 * FUNCTION REGISTRY: constructor/free, atomic edge replacement, direct
 * dependency/dependent queries, transitive invalidation query, project id.
 * CONTRACT: all ids must identify project files; duplicate edges are rejected;
 * failed replacement preserves existing edges; cycles are permitted and
 * traversal is bounded by project file count. */

typedef struct DependencyRow {
    u64 fileId;
    u64 *dependencyFileIds;
    bytes dependencyCount;
} DependencyRow;

struct DependencyGraph {
    const LanguageProject *project;
    DependencyRow *rows;
    bytes rowCount;
    bytes rowCapacity;
};

static bytes findRow(const DependencyGraph *self, u64 fileId) {
    for (bytes index = 0u; index < (*self).rowCount; ++index)
        if ((*self).rows[index].fileId == fileId)
            return index;
    return SIZE_MAX;
}

DependencyGraph *DependencyGraph_1(const LanguageProject *project) {
    if (project == nullptr) {
        THROW("DependencyGraph: rejected null project");
        return nullptr;
    }
    DependencyGraph *self = calloc(1u, sizeof *self);
    if (self == nullptr) {
        THROW("DependencyGraph: allocation failed");
        return nullptr;
    }
    (*self).project = project;
    return self;
}

void DependencyGraph_free(DependencyGraph *self) {
    if (self == nullptr)
        return;
    for (bytes index = 0u; index < (*self).rowCount; ++index)
        free((*self).rows[index].dependencyFileIds);
    free((*self).rows);
    (*self).rows = nullptr;
    free(self);
}

bool DependencyGraph_setDependencies(DependencyGraph *self, u64 fileId,
                                     const u64 *dependencyFileIds,
                                     bytes dependencyCount) {
    if (self == nullptr || LanguageProject_findFileById((*self).project, fileId) == nullptr
        || (dependencyCount != 0u && dependencyFileIds == nullptr)
        || dependencyCount > SIZE_MAX / sizeof(u64)) {
        THROW("DependencyGraph: rejected invalid file or dependency list");
        return false;
    }
    for (bytes index = 0u; index < dependencyCount; ++index) {
        if (dependencyFileIds[index] == 0u
            || LanguageProject_findFileById((*self).project, dependencyFileIds[index]) == nullptr) {
            THROW("DependencyGraph: dependency does not identify a project file");
            return false;
        }
        for (bytes prior = 0u; prior < index; ++prior)
            if (dependencyFileIds[prior] == dependencyFileIds[index]) {
                THROW("DependencyGraph: duplicate dependency edge");
                return false;
            }
    }
    u64 *copy = nullptr;
    if (dependencyCount != 0u) {
        copy = malloc(dependencyCount * sizeof(u64));
        if (copy == nullptr) {
            THROW("DependencyGraph: allocation failed for dependency list");
            return false;
        }
        for (bytes index = 0u; index < dependencyCount; ++index)
            copy[index] = dependencyFileIds[index];
    }

    bytes rowIndex = findRow(self, fileId);
    if (rowIndex != SIZE_MAX) {
        u64 *previous = (*self).rows[rowIndex].dependencyFileIds;
        (*self).rows[rowIndex].dependencyFileIds = copy;
        (*self).rows[rowIndex].dependencyCount = dependencyCount;
        free(previous);
        return true;
    }
    if ((*self).rowCount == (*self).rowCapacity) {
        bytes next = (*self).rowCapacity == 0u ? 8u : (*self).rowCapacity * 2u;
        if (next < (*self).rowCapacity || next > SIZE_MAX / sizeof(DependencyRow)) {
            THROW("DependencyGraph: row capacity overflow");
            free(copy);
            return false;
        }
        DependencyRow *grown = realloc((*self).rows, next * sizeof(DependencyRow));
        if (grown == nullptr) {
            THROW("DependencyGraph: allocation failed for row capacity %zu", next);
            free(copy);
            return false;
        }
        (*self).rows = grown;
        (*self).rowCapacity = next;
    }
    (*self).rows[(*self).rowCount] = (DependencyRow) {
        .fileId = fileId, .dependencyFileIds = copy, .dependencyCount = dependencyCount
    };
    ++(*self).rowCount;
    return true;
}

bytes DependencyGraph_getDependencyCount(const DependencyGraph *self, u64 fileId) {
    if (self == nullptr || fileId == 0u)
        return 0u;
    bytes index = findRow(self, fileId);
    return index == SIZE_MAX ? 0u : (*self).rows[index].dependencyCount;
}

u64 DependencyGraph_getDependencyAt(const DependencyGraph *self,
                                         u64 fileId, bytes index) {
    if (self == nullptr || fileId == 0u)
        return 0u;
    bytes rowIndex = findRow(self, fileId);
    return rowIndex == SIZE_MAX || index >= (*self).rows[rowIndex].dependencyCount
        ? 0u : (*self).rows[rowIndex].dependencyFileIds[index];
}

bytes DependencyGraph_getDependentCount(const DependencyGraph *self, u64 fileId) {
    if (self == nullptr || LanguageProject_findFileById((*self).project, fileId) == nullptr)
        return 0u;
    bytes count = 0u;
    for (bytes row = 0u; row < (*self).rowCount; ++row)
        for (bytes edge = 0u; edge < (*self).rows[row].dependencyCount; ++edge)
            if ((*self).rows[row].dependencyFileIds[edge] == fileId)
                ++count;
    return count;
}

u64 DependencyGraph_getDependentAt(const DependencyGraph *self,
                                        u64 fileId, bytes targetIndex) {
    if (self == nullptr || LanguageProject_findFileById((*self).project, fileId) == nullptr)
        return 0u;
    bytes found = 0u;
    for (bytes row = 0u; row < (*self).rowCount; ++row)
        for (bytes edge = 0u; edge < (*self).rows[row].dependencyCount; ++edge)
            if ((*self).rows[row].dependencyFileIds[edge] == fileId
                && found++ == targetIndex)
                return (*self).rows[row].fileId;
    return 0u;
}

bool DependencyGraph_isAffectedBy(const DependencyGraph *self,
                                  u64 candidateFileId,
                                  u64 changedFileId) {
    if (self == nullptr || candidateFileId == 0u || changedFileId == 0u
        || LanguageProject_findFileById((*self).project, candidateFileId) == nullptr
        || LanguageProject_findFileById((*self).project, changedFileId) == nullptr)
        return false;
    if (candidateFileId == changedFileId)
        return true;
    bytes fileCount = LanguageProject_getFileCount((*self).project);
    if (fileCount > SIZE_MAX / sizeof(u64))
        return false;
    u64 *queue = malloc(fileCount * sizeof(u64));
    u64 *visited = malloc(fileCount * sizeof(u64));
    if (queue == nullptr || visited == nullptr) {
        free(queue);
        free(visited);
        THROW("DependencyGraph: allocation failed for bounded traversal");
        return false;
    }
    bytes queued = 1u;
    bytes visitedCount = 1u;
    queue[0] = candidateFileId;
    visited[0] = candidateFileId;
    bool affected = false;
    for (bytes cursor = 0u; cursor < queued && !affected; ++cursor) {
        u64 current = queue[cursor];
        bytes rowIndex = findRow(self, current);
        if (rowIndex == SIZE_MAX)
            continue;
        DependencyRow row = (*self).rows[rowIndex];
        for (bytes edge = 0u; edge < row.dependencyCount; ++edge) {
            u64 dependency = row.dependencyFileIds[edge];
            if (dependency == changedFileId) {
                affected = true;
                break;
            }
            bool seen = false;
            for (bytes index = 0u; index < visitedCount; ++index)
                if (visited[index] == dependency)
                    seen = true;
            if (!seen && visitedCount < fileCount) {
                visited[visitedCount++] = dependency;
                queue[queued++] = dependency;
            }
        }
    }
    free(queue);
    free(visited);
    return affected;
}

u64 DependencyGraph_getProjectId(const DependencyGraph *self) {
    return self == nullptr ? 0u : LanguageProject_getId((*self).project);
}
