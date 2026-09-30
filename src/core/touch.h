/* On-screen controls for touch screens: a phone or tablet (the browser on
 * one, the Android build) or a Steam Deck's screen. A D-pad, A and B, L
 * and R, START and SELECT round the game's picture, drawn at the screen's
 * own resolution (touch_layout.c places them, touch_art.c draws them),
 * shown from the first touch until a key or a controller's button is used,
 * or all along with --touch. Their MENU button opens their menu over the
 * paused game: their size and opacity, haptics, and the editor that moves
 * and sizes each one (touch.ini keeps it all). */
#ifndef CW_TOUCH_H
#define CW_TOUCH_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "touch_layout.h"

bool touch_shown(void);
/* Shows or hides them (a finger shows them, a key or a button hides them);
 * true when that changed the canvas's layout. */
bool touch_show(bool on);
/* --touch: shown from the start and never hidden. */
void touch_always(void);
/* Every finger let go: the app lost the screen (a notification shade, the
 * home screen), whose fingers never send their lifting. */
void touch_release(void);
/* A finger event (SDL_FINGERDOWN, MOTION or UP) from platform_poll; true
 * when it showed the controls (the first touch only shows them). */
bool touch_event(const SDL_Event *e);
/* The same for a finger at screen pixel (x, y) (touch_event's own, and
 * --taps' in tests). */
bool touch_finger(uint32_t type, SDL_FingerID id, float x, float y);
/* The buttons fingers hold now, and those pressed since the last call
 * (a tap shorter than a frame counts). */
uint32_t touch_held(void);
uint32_t touch_taken(void);
/* Places them for the screen and the picture on it (platform.c's layout). */
void touch_relayout(void);
/* The player's arrangement: read from touch.ini at `path`, written there
 * when their menu closes. */
void touch_load(const char *path);
/* Their menu or its editor is open: the game waits. */
bool touch_paused(void);
/* Back or Escape: closes their menu (true), else it is the game's. */
bool touch_back(void);
/* Draws them on the screen over the picture, before it is shown. */
void touch_draw(void);
/* The renderer lost its textures (they are drawn again). */
void touch_reset_art(void);

#endif
