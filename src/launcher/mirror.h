/* The saves' copy in the folder the ROMs came from (mirror.c): on a phone,
 * where uninstalling the app deletes its files, a .cwsave (backup.h) beside
 * the ROMs in the folder the player chose, written a few seconds after the
 * game last saved and as the app goes away, so that a reinstall finds it
 * there and the launcher offers it back. Nothing elsewhere: a desktop's
 * saves outlive the program, the browser's page exports its own. */
#ifndef CW_MIRROR_H
#define CW_MIRROR_H

#include <stdbool.h>

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
/* While the launcher asks about saves the folder held: no copy over them */
void mirror_hold(bool on);
/* How the last copy went: 1 made, -1 refused (the folder's access was
 * read-only, or it is gone), 0 none made yet */
int mirror_last(void);

#endif
