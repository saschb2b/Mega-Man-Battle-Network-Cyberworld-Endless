/* MegaMan's own sounds felt (feel.h; docs/GUIDE.md, Rumble and triggers):
 * a hook on BN6's PlaySoundEffect tells the sounds his side's objects
 * start (his buster, a chip's attack, its hit landing) from the enemies'
 * and the menus', and after each frame the channels those sounds play on
 * are read, their envelopes and velocities, so the rumble follows each
 * sound as it plays: every shot of a Vulcan, a sword's swing, a bomb's
 * blast, each its own. The PSG's noise and a sample's low notes move the
 * heavy motor, its squares, wave and high notes the light one. */
#ifndef CW_FEEL_SOUND_H
#define CW_FEEL_SOUND_H

#include <stdbool.h>
#include <stdint.h>

/* After a frame of the run's own game (feel_game.c): the hook put in
 * once, the sounds started since taken in, and their level now, each
 * motor 0-255 (0 outside the fight) */
void feel_sound_frame(bool fight, uint8_t *low, uint8_t *high);

#endif
