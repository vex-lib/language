#ifndef LANGUAGE_DOCUMENT_TEXT_SNAPSHOT_H
#define LANGUAGE_DOCUMENT_TEXT_SNAPSHOT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct TextSnapshot TextSnapshot;

TextSnapshot *TextSnapshot_4(u32 documentId, u64 revision,
                            const char *bytes, bytes length);
#define TextSnapshot_0 TextSnapshot_invalid_arity
#define TextSnapshot_1 TextSnapshot_invalid_arity
#define TextSnapshot_2 TextSnapshot_invalid_arity
#define TextSnapshot_3 TextSnapshot_invalid_arity
#define TextSnapshot_5 TextSnapshot_invalid_arity
#define TEXT_SNAPSHOT_CAT_INNER(left, right) left##right
#define TEXT_SNAPSHOT_CAT(left, right) TEXT_SNAPSHOT_CAT_INNER(left, right)
#define TEXT_SNAPSHOT_NARG_INNER(_0, _1, _2, _3, _4, _5, count, ...) count
#define TEXT_SNAPSHOT_NARG(...) \
    TEXT_SNAPSHOT_NARG_INNER(_ __VA_OPT__(,) __VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define TextSnapshot(...) \
    TEXT_SNAPSHOT_CAT(TextSnapshot_, TEXT_SNAPSHOT_NARG(__VA_ARGS__))(__VA_ARGS__)
void TextSnapshot_free(TextSnapshot *self);
bool TextSnapshot_retain(TextSnapshot *self);
void TextSnapshot_release(TextSnapshot *self);
u32 TextSnapshot_getDocumentId(const TextSnapshot *self);
u64 TextSnapshot_getRevision(const TextSnapshot *self);
const char *TextSnapshot_getBytes(const TextSnapshot *self);
bytes TextSnapshot_getLength(const TextSnapshot *self);
u64 TextSnapshot_getReferenceCount(const TextSnapshot *self);

#endif
