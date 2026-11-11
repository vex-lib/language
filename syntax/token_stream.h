#ifndef LANGUAGE_SYNTAX_TOKEN_STREAM_H
#define LANGUAGE_SYNTAX_TOKEN_STREAM_H

#include "lang/language_engine.h"
#include "token/token.h"
#include "text/bounded_text.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct TokenStream TokenStream;

// Collects one opened engine's lexical output. The stream retains its immutable
// snapshot and rejects mismatched, overlapping or out-of-bounds spans.
TokenStream *TokenStream_collect(LanguageEngine *engine, TextSnapshot *snapshot);
void TokenStream_free(TokenStream *self);
u32 TokenStream_getDocumentId(const TokenStream *self);
u64 TokenStream_getDocumentRevision(const TokenStream *self);
u64 TokenStream_getCompilationContextId(const TokenStream *self);
bytes TokenStream_getCount(const TokenStream *self);
Token TokenStream_getAt(const TokenStream *self, bytes index);
bool TokenStream_isComplete(const TokenStream *self);
BoundedText TokenStream_toString(const TokenStream *self);
BoundedText TokenStream_toStringStruct(const TokenStream *self);

#endif
