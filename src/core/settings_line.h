/* settings.ini's lines written one at a time, from the game's own screens
 * (the statistics' answer, Options' sound): the rest of the file kept as
 * the player left it. */
#ifndef CW_SETTINGS_LINE_H
#define CW_SETTINGS_LINE_H

#include <stdbool.h>

/* The file at `path` as it was with `key`'s line set to `value`, or, where
 * it has none, `note` (comment lines that say what it is, each ending in a
 * newline; a newline first sets it apart from the lines before it) and the
 * line added at its end. Written beside it and moved over it. */
bool settings_line_write(const char *path, const char *key, const char *value, const char *note);

#endif
