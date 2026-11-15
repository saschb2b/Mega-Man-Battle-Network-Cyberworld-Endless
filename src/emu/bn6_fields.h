/* The game's fields in the engine's units, where bn6.h's name a raw format:
 * MegaMan's (or Lan's) place on the map, in whole world units (16.16), and
 * whether the game's fade covers the screen. */
#ifndef CW_BN6_FIELDS_H
#define CW_BN6_FIELDS_H

#include "bn6.h"
#include "emu.h"

static inline int bn6_player_x(void) { return (int)emu_read32(BN6_PLAYER_X) >> 16; }
static inline int bn6_player_y(void) { return (int)emu_read32(BN6_PLAYER_Y) >> 16; }
static inline int bn6_player_z(void) { return (int)emu_read32(BN6_PLAYER_Z) >> 16; }

/* BN6's own fade of the screen under way, or the white or dark it holds
 * after one (eScreenFade's first byte): a jack-in's flash, a warp's, a
 * jack-out's, and the fade back in after each */
static inline bool bn6_screen_faded(void) { return emu_read8(BN6_SCREEN_FADE) == 1; }

#endif
