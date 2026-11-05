#include "index/dependency_graph.h"
#include <assert.h>

int main(void) {
    LanguageProject *project = LanguageProject_2(71u, "/work/graph");
    assert(project != nullptr);
    const ProjectFile *first = LanguageProject_addFile(
        project, "src/first.c", PROJECT_FILE_KIND_TEXT, "c23");
    const ProjectFile *second = LanguageProject_addFile(
        project, "include/second.h", PROJECT_FILE_KIND_TEXT, "c23");
    const ProjectFile *third = LanguageProject_addFile(
        project, "include/third.h", PROJECT_FILE_KIND_TEXT, "c23");
    const ProjectFile *fourth = LanguageProject_addFile(
        project, "src/fourth.c", PROJECT_FILE_KIND_TEXT, "c23");
    assert(first != nullptr && second != nullptr && third != nullptr && fourth != nullptr);

    DependencyGraph *graph = DependencyGraph_1(project);
    assert(graph != nullptr && DependencyGraph_getProjectId(graph) == 71u);
    assert(DependencyGraph_getDependencyCount(nullptr, 1u) == 0u);
    assert(DependencyGraph_getDependentCount(graph, 4u) == 0u);
    uint64_t firstDeps[] = {2u, 3u};
    uint64_t secondDeps[] = {4u};
    uint64_t thirdDeps[] = {4u};
    uint64_t fourthDeps[] = {1u};
    assert(DependencyGraph_setDependencies(graph, 1u, firstDeps, 2u));
    assert(DependencyGraph_setDependencies(graph, 2u, secondDeps, 1u));
    assert(DependencyGraph_setDependencies(graph, 3u, thirdDeps, 1u));
    assert(DependencyGraph_setDependencies(graph, 4u, fourthDeps, 1u));
    assert(DependencyGraph_getDependencyCount(graph, 1u) == 2u);
    assert(DependencyGraph_getDependencyAt(graph, 1u, 0u) == 2u);
    assert(DependencyGraph_getDependencyAt(graph, 1u, 1u) == 3u);
    assert(DependencyGraph_getDependencyAt(graph, 1u, 2u) == 0u);
    assert(DependencyGraph_getDependentCount(graph, 4u) == 2u);
    assert(DependencyGraph_getDependentAt(graph, 4u, 0u) == 2u);
    assert(DependencyGraph_getDependentAt(graph, 4u, 1u) == 3u);
    assert(DependencyGraph_isAffectedBy(graph, 1u, 4u));
    assert(DependencyGraph_isAffectedBy(graph, 4u, 3u));
    assert(DependencyGraph_isAffectedBy(graph, 2u, 2u));
    assert(!DependencyGraph_isAffectedBy(graph, 1u, 99u));

    uint64_t duplicateDeps[] = {2u, 2u};
    assert(!DependencyGraph_setDependencies(graph, 1u, duplicateDeps, 2u));
    assert(DependencyGraph_getDependencyCount(graph, 1u) == 2u);
    uint64_t unknownDeps[] = {99u};
    assert(!DependencyGraph_setDependencies(graph, 1u, unknownDeps, 1u));
    assert(DependencyGraph_getDependencyAt(graph, 1u, 1u) == 3u);
    assert(!DependencyGraph_setDependencies(graph, 1u, nullptr, 1u));
    assert(!DependencyGraph_setDependencies(graph, 99u, nullptr, 0u));
    assert(DependencyGraph_setDependencies(graph, 1u, nullptr, 0u));
    assert(DependencyGraph_getDependencyCount(graph, 1u) == 0u);
    assert(!DependencyGraph_isAffectedBy(graph, 1u, 4u));

    DependencyGraph_free(graph);
    DependencyGraph_free(nullptr);
    LanguageProject_free(project);
    return 0;
}
