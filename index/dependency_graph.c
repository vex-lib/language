#include "vex.h"
#include "index/dependency_graph.h"
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

;;GETTER
static inline const LanguageProject *DependencyGraph_project(const DependencyGraph *self) { return (*self).project; }
;;SETTER
static inline fn DependencyGraph_setProject(DependencyGraph *self, const LanguageProject *v) { (*self).project = v; }
;;GETTER
static inline DependencyRow *DependencyGraph_rows(const DependencyGraph *self) { return (*self).rows; }
;;SETTER
static inline fn DependencyGraph_setRows(DependencyGraph *self, DependencyRow *v) { (*self).rows = v; }
;;GETTER
static inline bytes DependencyGraph_rowCount(const DependencyGraph *self) { return (*self).rowCount; }
;;SETTER
static inline fn DependencyGraph_setRowCount(DependencyGraph *self, bytes v) { (*self).rowCount = v; }
;;GETTER
static inline bytes DependencyGraph_rowCapacity(const DependencyGraph *self) { return (*self).rowCapacity; }
;;SETTER
static inline fn DependencyGraph_setRowCapacity(DependencyGraph *self, bytes v) { (*self).rowCapacity = v; }

static bytes findRow(const DependencyGraph *self, u64 fileId) {
    for (bytes index = 0u; index < DependencyGraph_rowCount(self); ++index)
        if (DependencyGraph_rows(self)[index].fileId == fileId)
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
    DependencyGraph_setProject(self, project);
    return self;
}

fn DependencyGraph_free(DependencyGraph *self) {
    if (self == nullptr)
        return;
    for (bytes index = 0u; index < DependencyGraph_rowCount(self); ++index)
        free(DependencyGraph_rows(self)[index].dependencyFileIds);
    free(DependencyGraph_rows(self));
    DependencyGraph_setRows(self, nullptr);
    free(self);
}

bool DependencyGraph_setDependencies(DependencyGraph *self, u64 fileId,
                                     const u64 *dependencyFileIds,
                                     bytes dependencyCount) {
    if (self == nullptr || LanguageProject_findFileById(DependencyGraph_project(self), fileId) == nullptr
        || (dependencyCount != 0u && dependencyFileIds == nullptr)
        || dependencyCount > SIZE_MAX / sizeof(u64)) {
        THROW("DependencyGraph: rejected invalid file or dependency list");
        return false;
    }
    for (bytes index = 0u; index < dependencyCount; ++index) {
        if (dependencyFileIds[index] == 0u
            || LanguageProject_findFileById(DependencyGraph_project(self), dependencyFileIds[index]) == nullptr) {
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
        u64 *previous = DependencyGraph_rows(self)[rowIndex].dependencyFileIds;
        DependencyGraph_rows(self)[rowIndex].dependencyFileIds = copy;
        DependencyGraph_rows(self)[rowIndex].dependencyCount = dependencyCount;
        free(previous);
        return true;
    }
    if (DependencyGraph_rowCount(self) == DependencyGraph_rowCapacity(self)) {
        bytes next = DependencyGraph_rowCapacity(self) == 0u ? 8u : DependencyGraph_rowCapacity(self) * 2u;
        if (next < DependencyGraph_rowCapacity(self) || next > SIZE_MAX / sizeof(DependencyRow)) {
            THROW("DependencyGraph: row capacity overflow");
            free(copy);
            return false;
        }
        DependencyRow *grown = realloc(DependencyGraph_rows(self), next * sizeof(DependencyRow));
        if (grown == nullptr) {
            THROW("DependencyGraph: allocation failed for row capacity %zu", next);
            free(copy);
            return false;
        }
        DependencyGraph_setRows(self, grown);
        DependencyGraph_setRowCapacity(self, next);
    }
    DependencyGraph_rows(self)[DependencyGraph_rowCount(self)] = (DependencyRow) {
        .fileId = fileId, .dependencyFileIds = copy, .dependencyCount = dependencyCount
    };
    DependencyGraph_setRowCount(self, DependencyGraph_rowCount(self) + 1u);
    return true;
}

bytes DependencyGraph_getDependencyCount(const DependencyGraph *self, u64 fileId) {
    if (self == nullptr || fileId == 0u)
        return 0u;
    bytes index = findRow(self, fileId);
    return index == SIZE_MAX ? 0u : DependencyGraph_rows(self)[index].dependencyCount;
}

u64 DependencyGraph_getDependencyAt(const DependencyGraph *self,
                                         u64 fileId, bytes index) {
    if (self == nullptr || fileId == 0u)
        return 0u;
    bytes rowIndex = findRow(self, fileId);
    return rowIndex == SIZE_MAX || index >= DependencyGraph_rows(self)[rowIndex].dependencyCount
        ? 0u : DependencyGraph_rows(self)[rowIndex].dependencyFileIds[index];
}

bytes DependencyGraph_getDependentCount(const DependencyGraph *self, u64 fileId) {
    if (self == nullptr || LanguageProject_findFileById(DependencyGraph_project(self), fileId) == nullptr)
        return 0u;
    bytes count = 0u;
    for (bytes row = 0u; row < DependencyGraph_rowCount(self); ++row)
        for (bytes edge = 0u; edge < DependencyGraph_rows(self)[row].dependencyCount; ++edge)
            if (DependencyGraph_rows(self)[row].dependencyFileIds[edge] == fileId)
                ++count;
    return count;
}

u64 DependencyGraph_getDependentAt(const DependencyGraph *self,
                                        u64 fileId, bytes targetIndex) {
    if (self == nullptr || LanguageProject_findFileById(DependencyGraph_project(self), fileId) == nullptr)
        return 0u;
    bytes found = 0u;
    for (bytes row = 0u; row < DependencyGraph_rowCount(self); ++row)
        for (bytes edge = 0u; edge < DependencyGraph_rows(self)[row].dependencyCount; ++edge)
            if (DependencyGraph_rows(self)[row].dependencyFileIds[edge] == fileId
                && found++ == targetIndex)
                return DependencyGraph_rows(self)[row].fileId;
    return 0u;
}

bool DependencyGraph_isAffectedBy(const DependencyGraph *self,
                                  u64 candidateFileId,
                                  u64 changedFileId) {
    if (self == nullptr || candidateFileId == 0u || changedFileId == 0u
        || LanguageProject_findFileById(DependencyGraph_project(self), candidateFileId) == nullptr
        || LanguageProject_findFileById(DependencyGraph_project(self), changedFileId) == nullptr)
        return false;
    if (candidateFileId == changedFileId)
        return true;
    bytes fileCount = LanguageProject_getFileCount(DependencyGraph_project(self));
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
        DependencyRow row = DependencyGraph_rows(self)[rowIndex];
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
    return self == nullptr ? 0u : LanguageProject_getId(DependencyGraph_project(self));
}
