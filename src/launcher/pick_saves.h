/* Shared device-local preferences and ordinary-file transfer helpers. */
#ifndef CW_PICK_SAVES_H
#define CW_PICK_SAVES_H

#include <stdbool.h>
#include <stddef.h>

bool saves_folder_keep(const char *path);
bool saves_copy(const char *from, const char *to);
/* A present file that cannot be read stages a refusal marker, so an
 * automatic write never destroys an unreadable incoming file. */
bool saves_stage(const char *from, const char *to);
/* Phones keep a transfer folder separately, with the ROM folder as the
 * initial default. Choosing one never changes where ROMs are looked for. */
bool saves_phone_place(char *out, size_t n);

#endif
