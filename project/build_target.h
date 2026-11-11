#ifndef LANGUAGE_PROJECT_BUILD_TARGET_H
#define LANGUAGE_PROJECT_BUILD_TARGET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct BuildTarget BuildTarget;

BuildTarget *BuildTarget_3(u64 targetId, const char *name,
                           const char *languageId);
void BuildTarget_free(BuildTarget *self);
bool BuildTarget_addSourceFile(BuildTarget *self, u64 fileId);
bool BuildTarget_addDependency(BuildTarget *self, u64 targetId);
bool BuildTarget_addCompilerOption(BuildTarget *self, const char *option);
u64 BuildTarget_getId(const BuildTarget *self);
const char *BuildTarget_getName(const BuildTarget *self);
const char *BuildTarget_getLanguageId(const BuildTarget *self);
bytes BuildTarget_getSourceCount(const BuildTarget *self);
u64 BuildTarget_getSourceAt(const BuildTarget *self, bytes index);
bytes BuildTarget_getDependencyCount(const BuildTarget *self);
u64 BuildTarget_getDependencyAt(const BuildTarget *self, bytes index);
bytes BuildTarget_getCompilerOptionCount(const BuildTarget *self);
const char *BuildTarget_getCompilerOptionAt(const BuildTarget *self, bytes index);

#endif
