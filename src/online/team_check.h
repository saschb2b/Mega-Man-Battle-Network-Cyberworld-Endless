/* Two MegaMen a side: a partner's messages checked on every GBA by the
 * rules his own Custom screen keeps (issue #181's review; docs/
 * docs/online/arena/two-a-side/messages-checked.md). His screen is BN6's own, on his
 * own client, so a message it makes passes; one no screen of his could
 * make (a pick past the hand dealt, two picks of neither one code nor one
 * chip, a Cross his MegaMan lacks, a Soul he does not hold or has united
 * this battle, a Program Advance its picks do not form) is refused on
 * every GBA alike: each check reads only what every game and spectator
 * holds the same (the deck in the core's RAM, his MegaMan as every game
 * was given it, the ROM's own lists). Numbers in, a reason out (NULL:
 * it passes), no state: the pair's code and the tests share it. */
#ifndef CW_TEAM_CHECK_H
#define CW_TEAM_CHECK_H

#include <stdbool.h>
#include <stdint.h>

#define TEAM_CHECK_PICKS 5    /* the picks a turn makes at most (BN6's Custom screen's five) */
#define TEAM_CHECK_DEALT 10   /* the chips a hand is dealt at most: BN6's Custom screen's ten slots, the top of its count
                                 (0x0802A408: his Custom level and the turn's extras, ten where it fills the screen) */
#define TEAM_CHECK_STAR  26   /* a folder entry's code (entry >> 9): '*', any code */
#define TEAM_CHECK_EMBLEM 11  /* a pick's slot that is no chip: the Beast Out emblem or UNITE under OK (BN6_CUSTOM_EMBLEM), which
                                 BN6's screen keeps among its picks as it is chosen; passed over, as the hand takes no chip of it */
#define TEAM_CHECK_SOULS 6    /* the Souls a MegaMan holds: bit k of his loadout's, BN5's flag BN5_FLAG_SOUL + k */

/* What his MegaMan holds, as every game was given it (his loadout): his
 * Crosses (bit c - 1 Gregar's Cross c), Beast Out, his Souls' bits */
typedef struct {
	uint8_t crosses, souls;
	bool beast;
} TeamAllowed;

/* `n` picks, slots of the hand dealt (`dealt` chips, their folder entries
 * `hand`): five at most, each a slot dealt and picked once (the emblem's
 * aside), all of one code ('*' any) or all one chip */
const char *team_check_picks(const uint16_t *hand, unsigned dealt, const uint8_t *slots, unsigned n);
/* the form his CROSSSELECT committed (BN6_BATTLE_FORM's numbers): a Cross
 * he holds, Beast Out (in a Cross he holds) where he has it, the Souls'
 * proxy form where he holds a Soul (his Soul's message checked apart) */
const char *team_check_form(unsigned form, const TeamAllowed *a);
/* the Soul his UNITE committed (BN5's 1-12; `bit` its held bit, -1 none
 * such), its Chaos Unison: one he holds, not united this battle (`used`,
 * BN6_SOUL_USED's bits: 1 << soul, << 16 a Chaos Unison) */
const char *team_check_soul(unsigned soul, unsigned chaos, int bit, const TeamAllowed *a, uint32_t used);
/* ColonelSoul's chip his button took: ColonelSoul his (`colonel`), the chip one of the hand dealt */
const char *team_check_soul_chip(unsigned chip, bool colonel, const uint16_t *hand, unsigned dealt);

/* The ROM read for the Program Advances' lists: `bytes` (1, 2 or 4) at a bus address */
typedef uint32_t (*TeamRead)(void *context, uint32_t address, unsigned bytes);
/* a Program Advance his OK made, chip `id`, from the `took` picks
 * `entries` (folder entries in pick order): a recipe of that id in one
 * of BN6's lists (`lists`, the words the Custom screen's check loads:
 * its own and the link battles') that those picks form: kind 4 its
 * chips in order, kind 0 its one chip `n` times in codes in a row, '*'
 * standing in for one */
const char *team_check_pa(TeamRead read, void *context, const uint32_t lists[2], unsigned id, const uint16_t *entries,
	unsigned took);

#endif
