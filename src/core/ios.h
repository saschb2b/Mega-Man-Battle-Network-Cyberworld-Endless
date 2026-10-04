/* The iPhone's and the iPad's own (ios.m, Objective-C, built for
 * TARGET=ios alone): the Files pickers that bring the ROMs in, a tick under
 * the thumb, and the screen's safe area. `window` is the game's
 * SDL_Window. */
#ifndef CW_IOS_H
#define CW_IOS_H

#include <stddef.h>

/* The ROMs ios.m looks for: BN6's (needed) and BN5's (optional), as bits
 * of `want` and of what a look copied in. */
enum { IOS_ROM_BN6 = 1, IOS_ROM_BN5 = 2 };
/* What the ROM screen's picker asks for: the folder the ROMs are in (kept
 * as a bookmark, looked in again at each start), or the files themselves,
 * more than one at once. */
enum { IOS_PICK_FOLDER, IOS_PICK_FILES };

/* Opens the Files picker over the game. What is picked is looked over:
 * only .gba files are opened, and BN6's and BN5's ROMs, checked by their
 * SHA-1, alone are copied into `dir` (as bn6g.gba and bn5c.gba). Returns
 * at once: ios_pick_result says how it went. */
void ios_pick_roms(void *window, const char *dir, int what);
/* 1 once a pick has been looked over (then 0 again), -1 when it was
 * cancelled or failed (then 0 again), 0 otherwise. `msg` gets what the
 * pick found, and why each .gba was refused; "" when it has nothing to say. */
int ios_pick_result(char *msg, size_t msglen);
/* The folder picked earlier looked in again, the ROMs `want` names that
 * are there copied into `dir`: only a file not settled before (its name,
 * size and date) is opened, and one still in iCloud is sent for, not
 * waited for. The bits copied in; -1 when no folder was picked, or it can't
 * be opened any more (`msg` says so then). `msg` is left alone when the
 * look has nothing new to say. */
int ios_rom_folder_look(const char *dir, unsigned want, char *msg, size_t msglen);
/* A note a look left (a Battle Network ROM refused beside BN6's), shown
 * for a few seconds over the game once its window is up; nothing without one. */
void ios_rom_note(void *window);
/* A light tick, as a key of the system keyboard gives. */
void ios_haptic(void);
/* The window's safe area, in its points: what the notch, the rounded
 * corners and the home indicator leave free. */
void ios_safe_insets(void *window, float *top, float *left, float *bottom, float *right);

#endif
