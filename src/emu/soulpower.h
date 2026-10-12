/* A Soul's abilities in BN6's engine (issue #82, the Souls track's S4;
 * docs/MULTIROM.md, Transplanted Souls; docs/ROM_DATA.md, A Soul's
 * abilities): the proxy form fights as BN5's Soul. BN6's power attacks and
 * their charge times grown with the Souls' (their routines BN5's, moved:
 * soulcode.h), MegaMan's loadout in the proxy the Soul's own row of BN5's
 * in BN6's ids, his element the Soul's (TomahawkSoul's Wood, ToadSoul's
 * Aqua), no weakness and no buster bonus of HeatCross's; the songs its
 * code plays that Gregar has otherwise copied into BN6's empty song slots,
 * each sound at the slot issue #82's allocations give it whatever track
 * copies it (xsong.h's xsong_put; tools/xsoul.py's SONG_SLOTS). */
#ifndef CW_SOULPOWER_H
#define CW_SOULPOWER_H

#include <stdbool.h>

/* Who the Souls' abilities' bytes are (claims.h) */
#define SOULPOWER_CLAIMS "the Souls' abilities (soulpower.c)"

/* (soulbreak's abilities, soulunite.h's SOULBREAK_POWER: the differential's own test, tests/xdiff/mutations.json) */
enum {
	SOULPOWER_BREAK_CHARGE = 1,   /* a Soul's charged shot charged 10 frames longer than BN5's */
	SOULPOWER_BREAK_FAMILY = 2,   /* a Soul's charged shot run as the next Soul's family */
	SOULPOWER_BREAK_ELEMENT = 4,  /* a Soul's element none */
};

/* The abilities as the core starts, after the Souls' code: the tables
 * grown, their words pointed at them, the songs copied, the hooks. Once a
 * core; false (nothing written) where a byte it stands on is not as
 * expected. */
bool soulpower_install(int breaks);

#endif
