/* ColonelSoul's own in BN6's engine (issue #82, the Souls track's S5;
 * docs/MULTIROM.md, Transplanted Souls; docs/ROM_DATA.md, ColonelSoul's
 * button, chip shot and obstacle strike): the Custom screen's button under
 * the picks (BN5's elements 8 and 9, which Gregar keeps for DustCross: its
 * set-up and its A the same, BN5's rule that lights it, BN5's mode 0x38
 * that loads the last chip picked into it, moved, BN5's draw of the chip
 * on it, BN5's art copied, BN5's B that gives the chip back and BN5's OK
 * that spends it), OK keeping the chip and the turn's start giving it to
 * his charged shot (BN5's 0x13, his chip shot, in place of his
 * ScreenDivide); and his obstacle strike: while he is united (his ready
 * sets the side's flag, in the Souls' block, never BattleState +0x5C's
 * bits, which are BN6's own) an obstacle on the enemy's area with an
 * enemy before it, or one of his side behind it, turns into BN5's soldier
 * (the third kind's 0x30, moved through the second-level dispatch), which
 * fires at the enemy, as BN5's obstacles' states do (0x08018002-
 * 0x0801802E, a block BN6's lack). */
#ifndef CW_SOULCOLONEL_H
#define CW_SOULCOLONEL_H

#include <stdbool.h>

/* Who ColonelSoul's bytes are (claims.h) */
#define SOULCOLONEL_CLAIMS "ColonelSoul's button and strike (soulcolonel.c)"

/* (soulbreak's passives byte, soulunite.h's SOULBREAK_PASSIVE, from bit 3: the differential's own test) */
enum {
	SOULCOLONEL_BREAK_RULE = 0x08,   /* his button lit for any chip picked (DustCross's rule) */
	SOULCOLONEL_BREAK_STRIKE = 0x10, /* no obstacle strikes */
	SOULCOLONEL_BREAK_UNDO = 0x20,   /* B leaves the button's chip in it */
	SOULCOLONEL_BREAK_SPENT = 0x40,  /* the button's chip left in the deck at OK */
	SOULCOLONEL_BREAK_TURN = 0x80,   /* his charged shot ScreenDivide with a chip in the button */
};

/* The hooks, the stubs and the Thumb they jump to (soul_room.h's
 * SOULS_COLONEL), the button's art (SOULS_COLONEL_ART), after the Souls'
 * code (soulcode_install: BN5's button's phases, its draw, his ready and
 * the soldier moved). Once a core; false (nothing written) where a byte it
 * stands on is not Gregar's or BN5's as expected. */
bool soulcolonel_install(int breaks);

#endif
