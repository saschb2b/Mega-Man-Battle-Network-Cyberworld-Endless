/* The game's music as its core holds it, for the engine's MP2K to take and
 * give back (music_carry.c, under the engine's screens; music_fast.c, under
 * fast forward): its RAM read and written, its PSG's registers. */
#ifndef CW_MUSIC_GAME_H
#define CW_MUSIC_GAME_H

#include "mp2k_carry.h"

/* The game's music as its RAM holds it */
void music_game_read(Mp2kGameMusic *g);
/* ... written back, and the words of other tracks' lists besides */
void music_game_write(const Mp2kGameMusic *g, const Mp2kWrite *writes, int n);
/* The game's own PSG registers */
extern const Mp2kRegs music_game_regs;

#endif
