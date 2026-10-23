/* Options: the game's settings, a PET panel of our own (BN6 has no
 * options screen to drive: bn6f searched for config, options and stereo).
 * Opened by SELECT on the title on every system and by OPTIONS in the
 * touch controls' menu, over whatever runs, which waits under it (main.c).
 * Its lines open a section each: Controls (the controls screen,
 * controls.h; not on the 3DS, whose buttons are the console's own), Display
 * (the fullscreen window, the scale and smooth motion, where the system
 * has them: display.h), Audio (the sound, gba or soft: sound_filter.h),
 * each kept in settings.ini at once, and Statistics where the build can
 * send them (their question,
 * analytics_ask.h); Done closes it. It is the overlay's root: the
 * statistics' question at the first start shows in its place, and the
 * controls screen over it, each passed its frame, picture, Back and taps
 * from here. */
#ifndef CW_OPTIONS_H
#define CW_OPTIONS_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "sound_filter.h"

void options_open(void);
/* The statistics' question, once: the first start where the build can
 * send (analytics_ask_offer), in Options' place */
void options_first_start(void);
bool options_shown(void);
/* Its frame (in the scene's place) and its picture over the canvas (after
 * the scene's: drawing changes nothing). */
void options_update(void);
void options_draw(void);
/* Escape or a phone's Back: its own back (true), else nothing of it. */
bool options_back(void);
/* A finger while it is shown (SDL_FINGERDOWN or UP), at canvas pixel (x, y). */
void options_finger(uint32_t type, SDL_FingerID id, int x, int y);
/* The statistics' question at the first start answers the touch controls'
 * buttons too, as it answers keys: their fingers go to them (touch_takes). */
bool options_takes_buttons(void);
/* The sound set and kept in settings.ini; `tell_page` tells the browser
 * player's menu (its Sound button) as well. */
void options_sound_keep(SoundFilter f, bool tell_page);

#endif
