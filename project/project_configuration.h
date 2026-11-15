#ifndef LANGUAGE_PROJECT_PROJECT_CONFIGURATION_H
#define LANGUAGE_PROJECT_PROJECT_CONFIGURATION_H

#include <stdbool.h>

typedef struct ProjectConfiguration ProjectConfiguration;

typedef enum ConfigurationOrigin {
    CONFIGURATION_ORIGIN_NONE = 0,
    CONFIGURATION_ORIGIN_DETECTED,
    CONFIGURATION_ORIGIN_USER
} ConfigurationOrigin;

ProjectConfiguration *ProjectConfiguration_0();
fn ProjectConfiguration_free(ProjectConfiguration *self);
bool ProjectConfiguration_setDetectedLanguage(ProjectConfiguration *self,
                                              const char *languageId);
bool ProjectConfiguration_setUserLanguage(ProjectConfiguration *self,
                                          const char *languageId);
fn ProjectConfiguration_clearUserLanguage(ProjectConfiguration *self);
const char *ProjectConfiguration_getLanguageId(const ProjectConfiguration *self);
ConfigurationOrigin ProjectConfiguration_getLanguageOrigin(const ProjectConfiguration *self);

#endif
