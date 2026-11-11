#ifndef LANGUAGE_PROJECT_PROJECT_FILE_H
#define LANGUAGE_PROJECT_PROJECT_FILE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct ProjectFile ProjectFile;

typedef enum ProjectFileKind {
    PROJECT_FILE_KIND_TEXT = 0,
    PROJECT_FILE_KIND_BINARY,
    PROJECT_FILE_KIND_ASSET
} ProjectFileKind;

typedef enum ProjectFileAnalysisStatus {
    PROJECT_FILE_ANALYSIS_PENDING = 0,
    PROJECT_FILE_ANALYSIS_CURRENT,
    PROJECT_FILE_ANALYSIS_STALE,
    PROJECT_FILE_ANALYSIS_FAILED,
    PROJECT_FILE_ANALYSIS_UNSUPPORTED
} ProjectFileAnalysisStatus;

ProjectFile *ProjectFile_4(u64 fileId, const char *relativePath,
                           ProjectFileKind kind, const char *languageId);
void ProjectFile_free(ProjectFile *self);
u64 ProjectFile_getId(const ProjectFile *self);
const char *ProjectFile_getRelativePath(const ProjectFile *self);
ProjectFileKind ProjectFile_getKind(const ProjectFile *self);
const char *ProjectFile_getLanguageId(const ProjectFile *self);
ProjectFileAnalysisStatus ProjectFile_getAnalysisStatus(const ProjectFile *self);
bool ProjectFile_setAnalysisStatus(ProjectFile *self, ProjectFileAnalysisStatus status);

#endif
