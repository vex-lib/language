#ifndef LANGUAGE_ADAPTERS_DIFF_DIFF_ADAPTER_H
#define LANGUAGE_ADAPTERS_DIFF_DIFF_ADAPTER_H

#include "token/token.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Classify one line from a unified diff. inHunk belongs to one parser stream:
// callers keep a separate state per document and reset it before a new patch.
// The line is a byte span without its LF delimiter; the optional final CR is
// accepted for CRLF input. outToken receives one token covering that line.
// On rejection, both inHunk and outToken are preserved and a diagnostic is
// emitted through THROW. This adapter classifies patch structure only; it does
// not tokenize the program text carried by added/deleted lines.
bool DiffAdapter_classifyLine(bool *inHunk, u32 documentId,
                              u64 lineStartByte, const char *line,
                              bytes lineLength, Token *outToken);

#endif
