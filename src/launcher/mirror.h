/* Auto-export to this device's transfer folder: its own choice, a phone's
 * ROM folder by default, or the known transfer place without a picker.
 * After the saves rest, one .cwsave is refreshed there; an incoming file
 * is held for SAVES to compare first. The browser exports by download. */
#ifndef CW_MIRROR_H
#define CW_MIRROR_H

#include <stdbool.h>
#include <stddef.h>

/* Files were written (platform_persist) */
void mirror_note(void);
/* Each frame: the copy made once the saves have rested a few seconds */
void mirror_tick(void);
/* The app going away (quit, sent to the background): the copy made now,
 * where the saves changed */
void mirror_flush(void);
/* A folder newly chosen (or chosen again after a copy was refused): the
 * copy made there at the next frame */
void mirror_new_folder(void);
/* While SAVES asks about an incoming file: no export over it */
void mirror_hold(bool on);
/* How the last copy went: 1 made, -1 refused (the folder's access was
 * read-only, or it is gone), 0 none made yet */
int mirror_last(void);
/* At the title/start and before each auto-export: stage a different file
 * from the transfer places and hold exports until the player's decision.
 * Repeated calls return the pending path; only mirror_hold(false), after
 * Import or Keep this device, releases it. */
bool mirror_scan(char *path, size_t n);

#endif
