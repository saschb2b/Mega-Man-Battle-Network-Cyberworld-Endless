/* ColonelSoul's own in BN6's engine (issue #82, the Souls track's S5;
 * docs/multirom/souls.md; docs/rom/souls.md, ColonelSoul's
 * button, chip shot and obstacle strike): the Custom screen's button under
 * the picks (BN5's elements 8 and 9, which Gregar keeps for DustCross: its
 * set-up and its A the same, BN5's rule that lights it, BN5's mode 0x38
 * that loads the last chip picked into it, moved, BN5's draw of the chip
 * on it, BN5's art copied, BN5's B that gives the chip back and BN5's OK
 * that spends it), OK keeping the chip and the turn's start giving it to
 * his charged shot (BN5's 0x13, his chip shot, in place of his
 * ScreenDivide); and his obstacle strike, BN5's rule for either Colonel
 * (xstrike.h): while he is united his ready arms his side (BN6_COLONEL_SIDES,
 * the field a Navi Colonel's spawn arms too, never BattleState +0x5C's bits,
 * which are BN6's own). */
#ifndef CW_SOULCOLONEL_H
#define CW_SOULCOLONEL_H

#include <stdbool.h>

/* Who ColonelSoul's bytes are (claims.h) */
#define SOULCOLONEL_CLAIMS "ColonelSoul's button (soulcolonel.c)"

/* (soulbreak's passives byte, soulunite.h's SOULBREAK_PASSIVE, from bit 3: the differential's own test) */
enum {
	SOULCOLONEL_BREAK_RULE = 0x08,   /* his button lit for any chip picked (DustCross's rule) */
	SOULCOLONEL_BREAK_STRIKE = 0x10, /* no obstacle strikes */
	SOULCOLONEL_BREAK_UNDO = 0x20,   /* B leaves the button's chip in it */
	SOULCOLONEL_BREAK_SPENT = 0x40,  /* the button's chip left in the deck at OK */
	SOULCOLONEL_BREAK_TURN = 0x80,   /* his charged shot ScreenDivide with a chip in the button */
};

/* The hooks and the Thumb they jump to (soul_room.h's SOULS_COLONEL), the
 * button's art (SOULS_COLONEL_ART), after the Souls' code (soulcode_install:
 * BN5's button's phases, its draw, his ready moved) and the obstacle strike
 * (xstrike_install, its break SOULCOLONEL_BREAK_STRIKE). Once a core; false
 * (nothing written) where a byte it stands on is not Gregar's or BN5's as
 * expected. */
bool soulcolonel_install(int breaks);

#endif
