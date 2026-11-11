#ifndef LANGUAGE_DOCUMENT_TEXT_DOCUMENT_H
#define LANGUAGE_DOCUMENT_TEXT_DOCUMENT_H

#include "document/text_snapshot.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct TextDocument TextDocument;

TextDocument *TextDocument_3(u32 documentId, const char *bytes, bytes length);
#define TextDocument_0 TextDocument_invalid_arity
#define TextDocument_1 TextDocument_invalid_arity
#define TextDocument_2 TextDocument_invalid_arity
#define TextDocument_4 TextDocument_invalid_arity
#define TextDocument_CAT_INNER(left, right) left##right
#define TextDocument_CAT(left, right) TextDocument_CAT_INNER(left, right)
#define TextDocument_NARG_INNER(_0, _1, _2, _3, _4, count, ...) count
#define TextDocument_NARG(...) \
    TextDocument_NARG_INNER(_ __VA_OPT__(,) __VA_ARGS__, 4, 3, 2, 1, 0)
#define TextDocument(...) \
    TextDocument_CAT(TextDocument_, TextDocument_NARG(__VA_ARGS__))(__VA_ARGS__)
void TextDocument_free(TextDocument *self);
bool TextDocument_update(TextDocument *self, const char *bytes, bytes length);
TextSnapshot *TextDocument_snapshot(const TextDocument *self);
u32 TextDocument_getDocumentId(const TextDocument *self);
u64 TextDocument_getRevision(const TextDocument *self);

#endif
