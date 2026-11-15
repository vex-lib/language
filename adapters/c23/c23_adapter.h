#ifndef LANGUAGE_ADAPTERS_C23_C23_ADAPTER_H
#define LANGUAGE_ADAPTERS_C23_C23_ADAPTER_H

#include "vex.h"
#include "lang/language_adapter.h"

/* C23 is a distinct explicit dialect id. The adapter does not claim C99, C++,
 * preprocessing, parsing or semantic resolution support. */
const LanguageAdapter *C23Adapter_adapter();

#endif
