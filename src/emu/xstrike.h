/* BN5's obstacle strike in BN6's engine (issue #82; docs/MULTIROM.md,
 * Transplanted Souls; docs/ROM_DATA.md, ColonelSoul's button, chip shot
 * and obstacle strike): BN5's obstacles' states (0x08018002-0x0801802E, a
 * block BN6's lack) turn an obstacle on the side opposite a Colonel's,
 * with an enemy of his before it or one of his side behind it, into BN5's
 * soldier (the third kind's 0x30, moved with the Souls' code: soulcode.h),
 * which fires at the enemy. Both Colonels arm it through one field, the
 * side's bit of BN6_COLONEL_SIDES (BN5's BattleState +0x5C 0x10 or 0x20):
 * a Navi Colonel's spawn (his moved code's write, XN_STUB_SIDES) and
 * ColonelSoul's ready (BN5's 0x080CAD06, the Souls' stub
 * SOUL_STUB_STRIKE); BN5 clears the side's as a form is set up (soullife.c).
 * Installed with the core where BN5's Navis are (xinstall.c), after every
 * other track, and by the Souls' install (soulcolonel.c) where it was not. */
#ifndef CW_XSTRIKE_H
#define CW_XSTRIKE_H

#include <stdbool.h>

/* Who the strike's bytes are (claims.h) */
#define XSTRIKE_CLAIMS "the obstacle strike (xstrike.c)"

/* The Souls' moved code first (soulcode_install: the soldier among it),
 * then BN5's block, the test's and the flag's stubs and their hooks, once
 * a core: true once it is there (now or before); false, nothing of the
 * strike written, where a byte it stands on is not Gregar's as expected or
 * the soldier is not moved. */
bool xstrike_install(void);
/* (the differential's own test, soulbreak's SOULCOLONEL_BREAK_STRIKE) no
 * obstacle strikes while on */
void xstrike_break(bool on);

#endif
