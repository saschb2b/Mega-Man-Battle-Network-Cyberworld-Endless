/* settings.ini's lines written one at a time, from the game's own screens
 * (the statistics' answer, Options' sound and display): the rest of the
 * file kept as the player left it; and one line read before the rest are
 * (the window the start opens). */
#ifndef CW_SETTINGS_LINE_H
#define CW_SETTINGS_LINE_H

#include <stdbool.h>
#include <stddef.h>

/* The file at `path` as it was with `key`'s line set to `value`, or, where
 * it has none, `note` (comment lines that say what it is, each ending in a
 * newline; a newline first sets it apart from the lines before it) and the
 * line added at its end. Written beside it and moved over it. */
bool settings_line_write(const char *path, const char *key, const char *value, const char *note);
/* `key`'s value in the file at `path` into `value` (n bytes): false where
 * it has no such line, or no file */
bool settings_line_read(const char *path, const char *key, char *value, size_t n);

#endif
