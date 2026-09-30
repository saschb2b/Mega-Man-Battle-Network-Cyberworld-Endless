/* On-screen controls for touch screens: a phone or tablet (the browser on
 * one, the Android build) or a Steam Deck's screen. A D-pad, A and B, L
 * and R, START and SELECT on the canvas around the game's picture
 * (touch_layout.c), shown from the first touch until a key or a
 * controller's button is used, or all along with --touch. */
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
/* The same for a finger at canvas (x, y): `type` SDL_FINGERDOWN, MOTION or
 * UP (touch_event's own, and --taps' in tests). */
bool touch_finger(uint32_t type, SDL_FingerID id, int x, int y);
/* The buttons fingers hold now, and those pressed since the last call
 * (a tap shorter than a frame counts). */
uint32_t touch_held(void);
uint32_t touch_taken(void);
/* Places them for the canvas's layout (platform.c calls it). */
void touch_relayout(void);
/* The player's arrangement: read from touch.ini at `path`, and written
 * there when the editor is done. */
void touch_load(const char *path);
/* The editor (the title screen's EDIT CONTROLS, while the controls show):
 * drag a control to move it, - and + size the one last touched, RESET
 * puts this screen shape's back, DONE keeps them. The scene that offers
 * it says so every frame; the fingers press nothing while it is open. */
void touch_offer_edit(bool on);
bool touch_editing(void);
/* Draws them on the canvas, over the scene. */
void touch_draw(void);

#endif
