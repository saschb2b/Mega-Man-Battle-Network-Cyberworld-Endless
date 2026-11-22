/* A Soul's passives in BN6's engine (issue #82, the Souls track's S5;
 * docs/multirom/souls.md; docs/rom/souls.md, A Soul's
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
	SOULPASSIVE_BREAK_PLUS = 2,    /* a Soul's bonus to its chips left out: NumberSoul's +10 to plain attack chips, NapalmSoul's
	                                  +40 to Fire attack chips, BN4's GutsSoul's +30 (soulbn4.c) */
	SOULPASSIVE_BREAK_READY = 4,   /* a Soul's passives left out: as it unites (ShadowSoul's FloatShoe, GyroSoul's shoes), and
	                                  MagnetSoul's on the magnet panel (soulpassive_magnet_spared) */
};

/* The passives as the core starts, after the Souls' look and abilities:
 * the proxy's two passive entries pointed at hooked stubs, the hand's
 * hook. Once a core; false (nothing written) where a byte it stands on is
 * not as expected. */
bool soulpassive_install(int breaks);
/* (soulproxy.c's hook on the chips' bonuses by form, BN6's 0x0800EF34) a
 * chip's bonus where side stands in a Soul, `charged` the chip used
 * charged (its second argument): *bonus the damage added, *look the
 * bonus's mark (r7 there); false where side is in no Soul (the proxy
 * HeatCross's). BN5's 0x0800D0A0: NumberSoul +10 to an attack chip of no
 * kind; NapalmSoul +40 to a Fire attack chip used uncharged, but while A
 * is held to a full charge (0x0800D0CE); the other Souls none. */
bool soulpassive_bonus(uint32_t obj, int chip, bool charged, int *bonus, int *look);
/* (xpanels.c's magnet panel, as it would push the Navi `obj`) whether he is spared: a player's Navi (his AI data's first
 * byte 2) united with MagnetSoul, BN5's 0x08017222-0x08017230 */
bool soulpassive_magnet_spared(uint32_t obj);
/* the passives' break bits the install took (SOULPASSIVE_BREAK_*), 0 none: BN4's Souls' passives read them too */
int soulpassive_breaks(void);

#endif
