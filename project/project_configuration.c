#include "vex.h"
#include "project/project_configuration.h"
#include <stdlib.h>
#include <string.h>

;;DEFINITION
/** ProjectConfiguration retains detected and user-selected language identities
 * separately. The effective dialect is the explicit user choice when present,
 * otherwise the detected value; detected defaults never erase user intent. */

;;OVERVIEW
/** CLASS: ProjectConfiguration (project/project_configuration.c)
 * STRUCT FIELDS: owned detectedLanguageId and userLanguageId strings.
 * FUNCTION REGISTRY: constructor/free, setters, clear-user, effective id/origin getters.
 * CONTRACT: ids are explicit registry ids such as c23 or c99; no path suffix is
 * interpreted here; failed replacement preserves the previous selection. */

struct ProjectConfiguration {
    char *detectedLanguageId;
    char *userLanguageId;
};

static char *copyId(const char *languageId) {
    if (languageId == nullptr)
        return nullptr;
    bytes length = strlen(languageId);
    if (length == 0u)
        return nullptr;
    char *copy = malloc(length + 1u);
    if (copy != nullptr)
        memcpy(copy, languageId, length + 1u);
    return copy;
}

static bool replaceId(char **destination, const char *languageId) {
    if (languageId == nullptr || languageId[0] == '\0') {
        THROW("ProjectConfiguration: rejected empty language id");
        return false;
    }
    char *copy = copyId(languageId);
    if (copy == nullptr) {
        THROW("ProjectConfiguration: allocation failed copying language id");
        return false;
    }
    free(*destination);
    *destination = copy;
    return true;
}

;;GETTER
static inline char *ProjectConfiguration_detectedLanguageId(const ProjectConfiguration *self) { return (*self).detectedLanguageId; }
;;SETTER
static inline fn ProjectConfiguration_setDetectedLanguageId(ProjectConfiguration *self, char *v) { (*self).detectedLanguageId = v; }
;;GETTER
static inline char *ProjectConfiguration_userLanguageId(const ProjectConfiguration *self) { return (*self).userLanguageId; }
;;SETTER
static inline fn ProjectConfiguration_setUserLanguageId(ProjectConfiguration *self, char *v) { (*self).userLanguageId = v; }

ProjectConfiguration *ProjectConfiguration_0() {
    ProjectConfiguration *self = calloc(1u, sizeof *self);
    if (self == nullptr)
        THROW("ProjectConfiguration: allocation failed");
    return self;
}
fn ProjectConfiguration_free(ProjectConfiguration *self) {
    if (self != nullptr) {
        free(ProjectConfiguration_detectedLanguageId(self));
        ProjectConfiguration_setDetectedLanguageId(self, nullptr);
        free(ProjectConfiguration_userLanguageId(self));
        ProjectConfiguration_setUserLanguageId(self, nullptr);
        free(self);
    }
}
bool ProjectConfiguration_setDetectedLanguage(ProjectConfiguration *self,
                                              const char *languageId) {
    if (self == nullptr) {
        THROW("ProjectConfiguration: rejected null detected-language owner");
        return false;
    }
    char *value = ProjectConfiguration_detectedLanguageId(self);
    if (!replaceId(&value, languageId))
        return false;
    ProjectConfiguration_setDetectedLanguageId(self, value);
    return true;
}
bool ProjectConfiguration_setUserLanguage(ProjectConfiguration *self,
                                          const char *languageId) {
    if (self == nullptr) {
        THROW("ProjectConfiguration: rejected null user-language owner");
        return false;
    }
    char *value = ProjectConfiguration_userLanguageId(self);
    if (!replaceId(&value, languageId))
        return false;
    ProjectConfiguration_setUserLanguageId(self, value);
    return true;
}
fn ProjectConfiguration_clearUserLanguage(ProjectConfiguration *self) {
    if (self != nullptr) {
        free(ProjectConfiguration_userLanguageId(self));
        ProjectConfiguration_setUserLanguageId(self, nullptr);
    }
}
const char *ProjectConfiguration_getLanguageId(const ProjectConfiguration *self) {
    if (self == nullptr)
        return nullptr;
    return ProjectConfiguration_userLanguageId(self) != nullptr
        ? ProjectConfiguration_userLanguageId(self)
        : ProjectConfiguration_detectedLanguageId(self);
}
ConfigurationOrigin ProjectConfiguration_getLanguageOrigin(const ProjectConfiguration *self) {
    if (self == nullptr)
        return CONFIGURATION_ORIGIN_NONE;
    if (ProjectConfiguration_userLanguageId(self) != nullptr)
        return CONFIGURATION_ORIGIN_USER;
    return ProjectConfiguration_detectedLanguageId(self) != nullptr
        ? CONFIGURATION_ORIGIN_DETECTED : CONFIGURATION_ORIGIN_NONE;
}
