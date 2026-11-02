/* Options' Display scenes: BN6's own battle, staged on a plain core of
 * its own and played in a loop, for the panel's picture of what a display
 * setting does (docs/engine/settings-screens.md; the panel's look in
 * docs/engine/ui.md, Ours). The core is the player's ROM as it is, no hook
 * of the game's (the Arena's, arena_pair.h), booted as core_boot.h takes
 * one to the net, its battles forced from a record of ours: a scene each
 * for what a setting changes (colour, flicker and motion), its keys
 * scripted, run from a state kept in memory where its loop begins. It is
 * heard by no one and runs only while Options' Display is shown, the game
 * paused under it. Not on the 3DS, whose heap holds no second core: there
 * the panel shows the paused game. */
#ifndef CW_SHOWCASE_H
#define CW_SHOWCASE_H

#include <stdbool.h>
#include <stdint.h>

/* What a scene is staged to show */
typedef enum {
	SCENE_COLOUR,   /* many colours at once: the field's two sides, a fire and an aqua virus */
	SCENE_FLICKER,  /* MegaMan hit and flickering, shots crossing: Frame blending's */
	SCENE_MOTION,   /* steady movement across the field: Smooth motion's */
	SCENE_COUNT
} ShowScene;

/* Whether this system can (every one but the 3DS) */
bool showcase_here(void);
/* The scene wanted (its core made and booted on the first, a frame at a
 * time); showcase_close lets the core and its states go */
void showcase_want(ShowScene s);
void showcase_close(void);
/* A frame of it, in the game's frame (options_update): the boot's or the
 * staging's frames, or the scene's next */
void showcase_frame(void);
/* The scene's last picture (240x160, as emu_video's), NULL while none is
 * ready; *made counts the pictures made (a new one since the last read is
 * blended once, lcd_frame.h) */
const uint32_t *showcase_picture(uint32_t *made);
/* Where in its picture the scene's action is: the part the panel shows */
void showcase_focus(int *x, int *y);

#endif
