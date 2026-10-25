/* MegaMan's own sounds felt (feel.h; docs/GUIDE.md, Rumble and triggers):
 * a hook on BN6's PlaySoundEffect tells the sounds his side's objects
 * start (his buster, a chip's attack, its hit landing) from the enemies'
 * and the menus', and after each frame the channels those sounds play on
 * are read, their envelopes and velocities, so the rumble follows each
 * sound as it plays: every shot of a Vulcan, a sword's swing, a bomb's
 * blast, each its own: a sample by its own bytes' loudness, the PSG by its
 * envelope. A sample and the PSG's noise move the heavy motor most, its
 * squares and wave the light one. */
#ifndef CW_FEEL_SOUND_H
#define CW_FEEL_SOUND_H

#include <stdbool.h>
#include <stdint.h>

/* After a frame of the run's own game (feel_game.c): the hook put in
 * once, the sounds started since taken in, and their level now, each
 * motor 0-255 (0 outside the fight); true where one of his began (a
 * charged shot's kick lands with its sound). The buster's charging hum
 * is left to the charge's own feel. */
bool feel_sound_frame(bool fight, uint8_t *low, uint8_t *high);

#endif
