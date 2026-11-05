#include "project/project_configuration.h"
#include <assert.h>
#include <string.h>

int main(void) {
    ProjectConfiguration *configuration = ProjectConfiguration_0();
    assert(configuration != nullptr);
    assert(ProjectConfiguration_getLanguageId(configuration) == nullptr);
    assert(ProjectConfiguration_getLanguageOrigin(configuration) == CONFIGURATION_ORIGIN_NONE);
    assert(ProjectConfiguration_setDetectedLanguage(configuration, "c99"));
    assert(strcmp(ProjectConfiguration_getLanguageId(configuration), "c99") == 0);
    assert(ProjectConfiguration_getLanguageOrigin(configuration) == CONFIGURATION_ORIGIN_DETECTED);
    assert(ProjectConfiguration_setUserLanguage(configuration, "c23"));
    assert(strcmp(ProjectConfiguration_getLanguageId(configuration), "c23") == 0);
    assert(ProjectConfiguration_getLanguageOrigin(configuration) == CONFIGURATION_ORIGIN_USER);
    assert(ProjectConfiguration_setDetectedLanguage(configuration, "c11"));
    assert(strcmp(ProjectConfiguration_getLanguageId(configuration), "c23") == 0);
    assert(!ProjectConfiguration_setUserLanguage(configuration, ""));
    assert(strcmp(ProjectConfiguration_getLanguageId(configuration), "c23") == 0);
    ProjectConfiguration_clearUserLanguage(configuration);
    assert(strcmp(ProjectConfiguration_getLanguageId(configuration), "c11") == 0);
    assert(ProjectConfiguration_getLanguageOrigin(configuration) == CONFIGURATION_ORIGIN_DETECTED);
    ProjectConfiguration_free(configuration);
    return 0;
}
