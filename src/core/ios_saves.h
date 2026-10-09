/* The SAVES screen's Files picker and kept-folder reads. */
#ifndef CW_IOS_SAVES_H
#define CW_IOS_SAVES_H

#include <stdbool.h>
#include <stddef.h>

/* kind: import 0, export 1, auto-export folder 2. */
bool ios_saves_pick(void *window, const char *dir, int kind, const char *from);
int ios_saves_result(char *out, size_t n);
bool ios_saves_get(const char *to);
bool ios_saves_folder_name(char *out, size_t n);
bool ios_saves_folder_scope(char *out, size_t n);
const char *ios_saves_device(void);

#endif
