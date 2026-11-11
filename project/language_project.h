#ifndef LANGUAGE_PROJECT_LANGUAGE_PROJECT_H
#define LANGUAGE_PROJECT_LANGUAGE_PROJECT_H

#include "project/project_file.h"
#include "project/build_target.h"
#include "project/project_configuration.h"
#include <stddef.h>
#include <stdint.h>

typedef struct LanguageProject LanguageProject;

LanguageProject *LanguageProject_2(u64 projectId, const char *rootPath);
void LanguageProject_free(LanguageProject *self);
const ProjectFile *LanguageProject_addFile(LanguageProject *self,
                                           const char *relativePath,
                                           ProjectFileKind kind,
                                           const char *languageId);
const ProjectFile *LanguageProject_findFile(const LanguageProject *self,
                                             const char *relativePath);
const ProjectFile *LanguageProject_findFileById(const LanguageProject *self,
                                                 u64 fileId);
const ProjectFile *LanguageProject_getFileAt(const LanguageProject *self, bytes index);
BuildTarget *LanguageProject_addTarget(LanguageProject *self, const char *name,
                                       const char *languageId);
BuildTarget *LanguageProject_findTarget(const LanguageProject *self, const char *name);
bool LanguageProject_addSourceToTarget(LanguageProject *self, u64 targetId,
                                       u64 fileId);
bool LanguageProject_addTargetDependency(LanguageProject *self, u64 targetId,
                                         u64 dependencyId);
BuildTarget *LanguageProject_getTargetAt(const LanguageProject *self, bytes index);
ProjectConfiguration *LanguageProject_getConfiguration(LanguageProject *self);
u64 LanguageProject_getId(const LanguageProject *self);
const char *LanguageProject_getRootPath(const LanguageProject *self);
bytes LanguageProject_getFileCount(const LanguageProject *self);
bytes LanguageProject_getTargetCount(const LanguageProject *self);

#endif
