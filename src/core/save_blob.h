/* Checksummed save files in savedata/, shared by save.c and the readers of
 * older formats in save_legacy.c. */
#ifndef CW_SAVE_BLOB_H
#define CW_SAVE_BLOB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void save_path(char *out, size_t n, const char *name);
bool save_write_blob(const char *name, uint32_t magic, const void *data, size_t n);
bool save_read_blob(const char *name, uint32_t magic, void *data, size_t n);

/* Older formats, turned into the current ones (save_legacy.c). */
bool legacy_load_run(void);
void legacy_move_state(void);

#endif
