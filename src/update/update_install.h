/* How this copy of the game is updated (update.h), each system's own:
 * the AppImage puts the new file over itself and starts it; a PortMaster
 * handheld's launcher puts the port's zip in place as the game ends
 * (port/update.py); the Windows
 * installer's copy runs the new installer silently, which starts the game
 * again when it is done (windows/installer.nsi); a copy the itch app
 * installed is the itch app's; the Switch's .nro and the 3DS's .3dsx put
 * the new file in their own place as the game ends (update_swap.c), the
 * Switch's loader starting it; the 3DS's CIA and the Vita's VPK come from
 * the releases page, the PS5's app from Prospero Store; any other is
 * updated from the download page. The download is written to a file of
 * this module's, and placed only once update.c has checked it whole. */
#ifndef CW_UPDATE_INSTALL_H
#define CW_UPDATE_INSTALL_H

#include <stdbool.h>
#include <stdio.h>

#include "update.h"

/* This copy's way, and where it installs a file itself the key of that
 * file in update.json (NULL where it does not) */
UpdateWay update_install_way(const char **key);
/* A start's tidying: a download an earlier start left half written, or
 * the installer an earlier update ran, removed */
void update_install_tidy(void);
/* The file the download is written to, opened: NULL where it cannot be */
FILE *update_install_open(void);
/* The download (of `version`), checked whole, put where the next start runs it: false
 * where it could not be */
bool update_install_place(const char *version);
/* ... or dropped (a failed one) */
void update_install_drop(void);
/* As the game exits, the new version started with the game's own
 * arguments: returns only where it could not be */
void update_install_start(void);
/* Whether the new version starts by itself once the game has ended (the
 * 3DS's .3dsx is started again from the Homebrew Launcher) */
bool update_install_starts_again(void);
/* The download page, in the system's browser: false where none opened */
bool update_install_page(void);

#endif
