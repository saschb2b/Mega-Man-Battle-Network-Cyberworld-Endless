/* The SAVES screen's side on the iPhone and iPad (ios_saves.m,
 * Objective-C, built for TARGET=ios alone): the saves folder, kept as a
 * bookmark (the ROM folder's, ios.m, until one is chosen), its copy read
 * and written, and Files' pickers to import, export and choose it. */
#ifndef CW_IOS_SAVES_H
#define CW_IOS_SAVES_H

#include <stdbool.h>
#include <stddef.h>

/* A Files picker over the game (`window`, the SDL_Window). kind 0
 * imports: the file picked copied to `dir`/saves-import.cwsave; 1 exports
 * `from`, offered as BACKUP_NAME; 2 chooses the saves folder. Returns at
 * once, false where no picker could be offered; ios_saves_result says how
 * it went. */
bool ios_saves_pick(void *window, const char *dir, int kind, const char *from);
/* 1 once a pick is done (`out`: the imported file's path, the exported
 * copy's, or the folder's name), -1 cancelled, -2 failed (`out` says why),
 * then 0 again; 0 while the picker is up */
int ios_saves_result(char *out, size_t n);
/* The saves folder's BACKUP_NAME copied whole to `to`: 1 copied; 0 no
 * folder kept, or no such file in it; -1 there but not readable, or in
 * iCloud and not on the phone as its latest (sent for). Nothing is left
 * at `to` but on 1. */
int ios_saves_get(const char *to);
/* (ios_saves_put is declared in ios.h and lives in ios_saves.m, beside
 * this: both use the one kept folder) */
/* The saves folder's name; false where none is kept */
bool ios_saves_folder_name(char *out, size_t n);
/* The saves folder as a URL, which tells one folder from another; false
 * where none is kept or it cannot be opened */
bool ios_saves_folder_scope(char *out, size_t n);

#endif
