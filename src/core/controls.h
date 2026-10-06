/* The controls screen (issue #37): which of the controller's buttons, and
 * on a desktop or in a browser which keys, are the GBA's A, B, L, R, START
 * and SELECT, with presets for A and B. Opened by SELECT on the title
 * screen and by CONTROLLER in the touch controls' menu, over whatever runs,
 * which waits under it (main.c). It reads the pads' own buttons
 * (pads_menu_*), never the map it edits, takes taps on its rows, and puts
 * a new map in play only once that map's A was pressed to keep it, so no
 * choice made on it can leave a player unable to work it. Where the build
 * can send the anonymous statistics (analytics.h), a row turns them on or
 * off, kept at once; and their question at the first start shows in this
 * screen's place (analytics_ask.h), its frame, picture, Back and taps
 * passed on from here. */
#ifndef CW_CONTROLS_H
#define CW_CONTROLS_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

void controls_open(void);
bool controls_shown(void);
/* Its frame (in the scene's place) and its picture over the canvas (after
 * the scene's: drawing changes nothing). */
void controls_update(void);
void controls_draw(void);
/* Escape or a phone's Back: its own back (true), else nothing of it. */
bool controls_back(void);
/* A finger while it is open (SDL_FINGERDOWN or UP), at canvas pixel (x, y). */
void controls_finger(uint32_t type, SDL_FingerID id, int x, int y);
/* The words for the button a GBA button (BTN_*) is on, on the device used
 * last: "R" on a keyboard, "Minus" on a Nintendo pad, "SELECT" by touch. */
const char *controls_word(uint32_t bit);

#endif
