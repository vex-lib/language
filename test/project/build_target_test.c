#include "project/build_target.h"
#include <assert.h>
#include <string.h>

int main(void) {
    char name[] = "app";
    BuildTarget *target = BuildTarget_3(4u, name, "c23");
    assert(target != nullptr);
    name[0] = 'X';
    assert(BuildTarget_getId(target) == 4u);
    assert(strcmp(BuildTarget_getName(target), "app") == 0);
    assert(strcmp(BuildTarget_getLanguageId(target), "c23") == 0);
    assert(BuildTarget_addSourceFile(target, 2u));
    assert(BuildTarget_addSourceFile(target, 9u));
    assert(!BuildTarget_addSourceFile(target, 2u));
    assert(BuildTarget_getSourceCount(target) == 2u);
    assert(BuildTarget_getSourceAt(target, 1u) == 9u);
    assert(BuildTarget_getSourceAt(target, 2u) == 0u);
    assert(BuildTarget_addDependency(target, 7u));
    assert(!BuildTarget_addDependency(target, 0u));
    assert(BuildTarget_getDependencyCount(target) == 1u);
    assert(BuildTarget_addCompilerOption(target, "-std=c23"));
    assert(BuildTarget_addCompilerOption(target, "-Iinclude"));
    assert(BuildTarget_getCompilerOptionCount(target) == 2u);
    assert(strcmp(BuildTarget_getCompilerOptionAt(target, 0u), "-std=c23") == 0);
    assert(BuildTarget_getCompilerOptionAt(target, 2u) == nullptr);
    assert(!BuildTarget_addCompilerOption(target, ""));
    assert(BuildTarget_getCompilerOptionCount(target) == 2u);
    BuildTarget_free(target);
    return 0;
}
