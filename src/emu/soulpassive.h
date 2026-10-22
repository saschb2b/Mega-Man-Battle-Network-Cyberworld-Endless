/* A Soul's passives in BN6's engine (issue #82, the Souls track's S5;
 * docs/MULTIROM.md, Transplanted Souls; docs/ROM_DATA.md, A Soul's
 * passives): what BN5's engine does for a Soul at the places BN6's engine
 * descends from, each a hook there where MegaMan stands in the proxy form
 * with a Soul (BN6's engine reads the proxy as HeatCross's otherwise):
 * the passives a Soul gives as it unites and as his form's state is set
 * again, BN5's routines by the Soul moved (soulcode.h; ShadowSoul's
 * FloatShoe, TomahawkSoul's grass, ToadSoul's flag), in place of the
 * proxy's entries of BN6's tables by form; NumberSoul's hand of ten and
 * its +10 to an attack chip of no kind. */
#ifndef CW_SOULPASSIVE_H
#define CW_SOULPASSIVE_H

#include <stdbool.h>
#include <stdint.h>

/* Who the Souls' passives' bytes are (claims.h) */
#define SOULPASSIVE_CLAIMS "the Souls' passives (soulpassive.c)"

/* (soulbreak's passives, tests/xdiff/mutations.json; from 0x08 ColonelSoul's, soulcolonel.h) */
enum {
	SOULPASSIVE_BREAK_HAND = 1,    /* NumberSoul's hand of 9 where BN5 deals 10 */
	SOULPASSIVE_BREAK_PLUS = 2,    /* NumberSoul's +10 to plain attack chips left out */
	SOULPASSIVE_BREAK_READY = 4,   /* a Soul's passives as it unites left out (ShadowSoul's FloatShoe) */
};

/* The passives as the core starts, after the Souls' look and abilities:
 * the proxy's two passive entries pointed at hooked stubs, the hand's
 * hook. Once a core; false (nothing written) where a byte it stands on is
 * not as expected. */
bool soulpassive_install(int breaks);
/* (soulproxy.c's hook on the chips' bonuses by form, BN6's 0x0800EF34) a
 * chip's bonus where side stands in a Soul: *bonus the damage added, *look
 * the bonus's mark (r7 there); false where side is in no Soul (the proxy
 * HeatCross's). BN5's 0x0800D0A0: NumberSoul +10 to an attack chip of no
 * kind; the other Souls none. */
bool soulpassive_bonus(uint32_t obj, int chip, int *bonus, int *look);

#endif
