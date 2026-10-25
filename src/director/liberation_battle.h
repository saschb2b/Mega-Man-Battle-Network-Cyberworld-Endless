/* A liberation's battle on BN6's own battle loop (docs/LIBERATION.md, The
 * liberation battle): BN5's rules, read in its ROM, answered at BN6's
 * loop by hooks (src/emu/bn6_battle_loop.h). Three turns, a Custom gauge
 * each: TURN 1 START!, TURN 2 START!, FINAL TURN! as each turn's fight
 * begins; the final turn's gauge full, TIME UP! and LIBERATE FAILED!, the
 * battle drawn (no reward, HP as it stands); won, LIBERATE SUCCESS!, or 1
 * TURN LIBERATE! in the first; below the final turn the Custom screen
 * opens by itself as the gauge fills. The Darkloid's battle the same.
 * What each battle stakes is shown before it starts (lib_battle_stakes);
 * its field is liberation_field.h's. */
#ifndef CW_LIBERATION_BATTLE_H
#define CW_LIBERATION_BATTLE_H

#include <stdbool.h>

#include "foes.h"

/* BN5's: the turns a liberation's battle runs (BN5_LIB_TURNS_UP) */
#define LIB_TURNS        3

/* How a liberation's battle ended */
enum { LIB_OUT_NONE, LIB_OUT_WON, LIB_OUT_TIME_UP, LIB_OUT_LOST };

/* The hooks on BN6's battle loop, once (encounter.c's install): they
 * answer only while a liberation's battle is armed. */
void lib_battle_install(void);
/* The next battle a liberation's: `turns` (LIB_TURNS; 0 none counted). */
void lib_battle_arm(int turns);
/* The foe's HP for the battle armed, the board's (BN5_LIB_FOE_HP: the
 * Darkloid's, 0 none), given him as he is first in play; and after it his
 * HP as the battle left it (BN5_LIB_FOE_HP_BACK), -1 unseen */
void lib_battle_foe(int hp);
int lib_battle_foe_left(void);
/* Each frame of it: the turn it is in, and the foe's HP. */
void lib_battle_watch(void);
/* Back on the map: how it ended (LIB_OUT_*) and in which turn; disarmed. */
int lib_battle_end(int *turn);
/* The battle for panel (x, y), faced as MegaMan faces it, set out on
 * BN5's field for the dark round it (liberation_field.h); its level */
int lib_battle_field(Encounter *e, int x, int y, bool darkloid);
/* No liberation's battle armed (the board begun or left). */
void lib_battle_disarm(void);

/* What liberating panel (x, y) with a command stakes, for the menu: the
 * turns, the panels freed won in any turn, those a win in the first turn
 * frees beside them, and whether the battle is Surrounded; false where
 * nothing is fought there. */
typedef struct {
	int turns;
	int freed;      /* won: the panels it frees (the hole's ring, the panel, LongSwrd's two) */
	int one_turn;   /* ... and, won in its first turn, the dark around freed too */
	bool hole;      /* a DarkHole closed */
	bool unit;      /* a guardian deleted */
	bool darkloid;  /* ... the Darkloid */
	bool surrounded;   /* its foes on both sides, L or R to turn round (liberation_field.h) */
} LibStakes;
bool lib_battle_stakes(int x, int y, int cmd, LibStakes *s);
/* Whether a win's sweep frees panel (x, y) beside the one fought for:
 * dark, an item's, the Darkloid's ring once every DarkHole is down, no
 * unit on it (liberation.c's one-turn win and a DarkHole's closing). */
bool lib_swept(int x, int y);

/* (drawing) The stakes of the command under the menu's cursor, under its
 * window: BN6's PET look, left of the window at (x, y) */
void lib_battle_draw_stakes(int right, int y);

#endif
