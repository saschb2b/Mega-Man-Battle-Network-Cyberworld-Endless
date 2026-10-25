/* The feel sent to the pad (feel.h's rules; docs/GUIDE.md, Rumble and
 * triggers): the beats the game raises, mixed once a frame into the
 * rumble of the pad played on; a DualSense's triggers as the game asks,
 * each by what the map makes it (L or R, FAST); Options' Vibration and
 * Trigger effects (settings.ini); and --dev feel's trace of everything
 * sent. Output only, as sound is: nothing here changes the game. */
#ifndef CW_FEEL_OUT_H
#define CW_FEEL_OUT_H

#include <stdbool.h>
#include <stdint.h>

#include "feel.h"

/* Options' Vibration (FEEL_OFF ... FEEL_STRONG) and Trigger effects */
void feel_set_level(int level);
int feel_level(void);
void feel_set_triggers(bool on);
bool feel_triggers(void);

/* A beat raised by the game (feel_game.c), felt from the next frame */
void feel_beat(FeelCue cue, uint8_t amount);
/* This frame's level of MegaMan's own sounds (feel_sound.h), each motor
 * 0-255; unset, none */
void feel_sound_level(uint8_t low, uint8_t high);
/* This frame's triggers, asked by the game: the click of the trigger
 * that is L or R (the Custom gauge full), and of FAST's (fast forward
 * can run); unasked, both are free */
void feel_want_triggers(bool gauge, bool fast);
/* Once a frame, after the scene's: the rumble and the triggers sent */
void feel_frame(void);
/* At the end: the motors stilled, a DualSense's triggers freed (an effect
 * stays in the controller till another replaces it) */
void feel_close(void);

/* The system rumbles a controller through SDL (not Android's, the
 * browser's or the 3DS's): Options shows Vibration */
bool feel_here(void);
/* The pad played on is a DualSense whose triggers SDL can set (not on
 * iOS): Options shows Trigger effects */
bool feel_dualsense(void);
/* --dev feel: each beat, each rumble and each trigger effect sent */
void feel_trace(bool on);
bool feel_tracing(void);

#endif
