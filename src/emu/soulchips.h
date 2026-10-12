/* What a Soul does to the chips MegaMan uses in BN6's engine (issue #82,
 * the Souls track's S5; docs/MULTIROM.md, Transplanted Souls; docs/
 * ROM_DATA.md, A Soul's chips), by BN5's rule where BN6's engine reads the
 * form for its Crosses: the chip charge (A held) of a chip of the Soul's
 * kind (BN5 0x08010902: ShadowSoul's swords, KnightSoul's Break chips,
 * ToadSoul's Aqua chips), a charged chip doubled for KnightSoul and ToadSoul
 * (0x080103C8), ShadowSoul's charged sword run by BN5's sword family (its
 * paths in front of an enemy), TomahawkSoul's Wood chip doubled on grass
 * and the panel plain after (0x08010322), KnightSoul's 50 frames unharmed
 * after a chip used from his front column (0x0801038A), and ShadowSoul's
 * missing shadow (0x0800E0B6). Hooks only on BN6's code, each answering
 * BN6's own way for any other form; the chip use's end carries BN5's
 * DarkChip use and its dark gate too, in BN5's order (soulmood.c). */
#ifndef CW_SOULCHIPS_H
#define CW_SOULCHIPS_H

#include <stdbool.h>

/* Who the Souls' chips' bytes are (claims.h) */
#define SOULCHIPS_CLAIMS "the Souls' chips (soulchips.c)"

/* (soulbreak's chips, soulunite.h's SOULBREAK_CHIPS: the differential's own test, tests/xdiff/mutations.json) */
enum {
	SOULCHIPS_BREAK_CHARGE = 1,   /* no chip charged in a Soul (BN6's rule for form 8) */
	SOULCHIPS_BREAK_DOUBLE = 2,   /* a charged chip not doubled */
	SOULCHIPS_BREAK_GRASS = 4,    /* TomahawkSoul's Wood chip not doubled on grass */
	SOULCHIPS_BREAK_FRONT = 8,    /* KnightSoul's front column gives no unharmed frames */
	SOULCHIPS_BREAK_SWORD = 16,   /* ShadowSoul's charged sword run by BN6's sword family */
	SOULCHIPS_BREAK_SHADOW = 32,  /* ShadowSoul keeps his shadow */
};

/* The hooks and the one stub of Thumb they jump to (soul_room.h's
 * SOULS_THUMB), after the Souls' code (soulcode_install: the moved sword
 * family). Once a core; false (nothing written) where a byte it stands on
 * is not Gregar's as expected. */
bool soulchips_install(int breaks);

#endif
