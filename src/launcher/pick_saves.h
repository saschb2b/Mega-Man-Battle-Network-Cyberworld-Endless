/* What the systems' pickers share for SAVES (pick_saves.c): the folder
 * kept in saves.ini, device-local. */
#ifndef CW_PICK_SAVES_H
#define CW_PICK_SAVES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The folder chosen for the automatic copy (a desktop's): kept */
bool saves_folder_keep(const char *path);

#if defined(__ANDROID__) || defined(CW_IOS)
/* (phones) The provider's file read through a file of the data folder's
 * own, gone again once read: `get` copies it there (1), finds none (0) or
 * cannot read it (-1) */
int saves_read_through(int (*get)(const char *to), uint8_t **bytes, size_t *n);
/* ... and written through one: `put` copies it into the place */
bool saves_put_through(bool (*put)(const char *from), const uint8_t *bytes, size_t n);
#endif

#endif
