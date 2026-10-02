/* The iPhone's and the iPad's own (ios.m, Objective-C, built for
 * TARGET=ios alone): the Files picker that brings the ROM in, a tick under
 * the thumb, and the screen's safe area. `window` is the game's
 * SDL_Window. */
#ifndef CW_IOS_H
#define CW_IOS_H

/* Opens the Files picker over the game; the file picked is copied into
 * `dir` under its own name. Returns at once: ios_pick_result says how it
 * went. */
void ios_pick_rom(void *window, const char *dir);
/* 1 once a pick has been copied (then 0 again), -1 when it was cancelled
 * or failed (then 0 again), 0 otherwise. */
int ios_pick_result(void);
/* A light tick, as a key of the system keyboard gives. */
void ios_haptic(void);
/* The window's safe area, in its points: what the notch, the rounded
 * corners and the home indicator leave free. */
void ios_safe_insets(void *window, float *top, float *left, float *bottom, float *right);

#endif
