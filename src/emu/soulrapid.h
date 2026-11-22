/* GutsSoul's rapid buster in BN6's engine (docs/multirom/bn4-souls.md,
 * GutsSoul in BN6's engine): in his form, B pressed six times, each within
 * ten frames of the last, asks his state 0x10, eight shots of his buster,
 * as BN4's key handler counts them (BN4_GUTS_RAPID_FORM). The state is the
 * chips' unit 'GutsRapid' (tools/xchiprecipe_bn4.py), BN4's own moved; the
 * count and its window are kept in his side's Souls block
 * (BN6_SOUL_PRESSES, BN6_SOUL_WINDOW), where the unit's end clears them
 * (XC4_SOULS_SIDE), since BN6's AI data holds others at BN4's places. The
 * request is BN6's charged B's (2), which BN6's requests answer as BN4's
 * answer its 0x20, with the unit's family in place of the charged shot's
 * while the count stands. */
#ifndef CW_SOULRAPID_H
#define CW_SOULRAPID_H

#include <stdbool.h>

/* Who the rapid buster's hooks are (claims.h) */
#define SOULRAPID_CLAIMS "GutsSoul's rapid buster (soulrapid.c)"

/* (soulunite.h's SOULBREAK_BN4: the differential's own test, tests/xdiff/mutations/) */
enum {
	SOULRAPID_BREAK_SEVEN = 1,   /* the rapid buster asked at seven presses where BN4 asks at six */
};

/* The hooks, once a core: true once they stand, or where BN4's ROM is not
 * read or its units are not installed (no rapid buster, said); false where
 * an instruction of BN6's or BN4's they stand on is not as read */
bool soulrapid_install(int breaks);

#endif
