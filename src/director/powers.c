/* Flags found by setting them in a battle and opening CROSSSELECT (see
 * docs/ROM_DATA.md). */
#include "powers.h"

#include <stdio.h>

#include "bn6.h"
#include "flags.h"
#include "guardians.h"
#include "net.h"
#include "run.h"
#include "save.h"
#include "souls.h"
#include "super_boss.h"

/* Gregar's Crosses by navi index: HeatMan 1 .. ChargeMan 5 (what MegaMan
 * feels of each: powers_words.c), the attacks that hit it twice as hard and what it gives
 * (BN6's own Cross tutorials, CompText86D0614: its chips' +50 is theirs
 * that don't dim the screen; a playtester chose a Cross by its weakness
 * alone, the setup naming no strength) */
static const struct { int navi, flag; const char *name, *weak, *strong, *navis; } crosses[] = {
	{ 1, BN6_FLAG_HEAT_CROSS, "HeatCross", "Aqua", "Fire chips +50, Buster +1" },
	{ 2, BN6_FLAG_ELEC_CROSS, "ElecCross", "Wood", "Elec chips +50" },
	{ 3, BN6_FLAG_SLASH_CROSS, "SlashCross", "Breaker", "Sword chips +50" },
	/* (BN6's EraseCross: an elementless chip that doesn't dim the screen
	 * deletes a virus whose HP has a 4 in it, and bugs a Navi, whose HP
	 * then drains; two playtesters had found it by chance and taken it for
	 * counters, and a third saw DiveMan's HP fall with nothing hitting;
	 * a fourth, told only of viruses, met a Navi's bug unexplained) */
	{ 4, BN6_FLAG_ERASE_CROSS, "EraseCross", "Wind", "A 4 in HP: plain chips erase",
	  "Navis: a bug drains their HP" },
	{ 5, BN6_FLAG_CHARGE_CROSS, "ChargeCross", "Aqua", "One more chip each turn" },
};

/* Whether the run has beaten `navi` as an earlier act's guardian (its
 * Cross is MegaMan's already). */
static bool beaten_before(int navi, int depth) {
	for (int d = 1; d < depth; ++d)
		if (is_boss_depth(d) && run_guardian(biome_for_depth(d)) == navi) return true;
	return false;
}

const char *powers_cross_name(int navi) {
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i)
		if (crosses[i].navi == navi) return crosses[i].name;
	return NULL;
}

const char *powers_cross_strength(int navi) {
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i)
		if (crosses[i].navi == navi) return crosses[i].strong;
	return NULL;
}

const char *powers_cross_on_navis(int navi) {
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i)
		if (crosses[i].navi == navi) return crosses[i].navis;
	return NULL;
}

const char *powers_cross_weakness(int navi) {
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i)
		if (crosses[i].navi == navi) return crosses[i].weak;
	return NULL;
}

bool powers_cross_owned(int navi) {
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i)
		if (crosses[i].navi == navi) return flag_get(crosses[i].flag);
	return false;
}

void powers_bring(void) {
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i)
		if (crosses[i].navi == run.cross) flag_set(crosses[i].flag);
	/* (BN6's own BeastOut: the emblem under OK, its three turns a battle,
	 * as the Cybeast's fall gives it, docs/ROM_DATA.md; a headless start
	 * past the first Net's Nest has it too, as a run has it there) */
	if (run_beast_start() || run.depth > CYCLE_LAYERS) flag_set(BN6_FLAG_BEAST_OUT);
}

const char *powers_reward_text(int navi, int biome, int depth) {
	/* (a run that brought a Cross keeps it alone: the choice's cost; the
	 * Graveyard sits over the Nest: its call stirs the Cybeast in MegaMan,
	 * once a run, and unlocks nothing, the Cybeast's fall does: issue
	 * #109) */
	return powers_reward_words(navi, powers_cross_name(navi) && !beaten_before(navi, depth), biome == BIOME_GRAVEYARD && depth <= CYCLE_LAYERS);
}

const char *powers_den_text(int depth) {
	/* (beat the beast, then its power: the first Net's Cybeast, in a run
	 * that did not bring BeastOut; on the Nets after, the run has it) */
	return depth <= CYCLE_LAYERS && !run_beast_start() ? powers_beast_words() : NULL;
}

void powers_after_boss(int navi) {
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i)
		if (crosses[i].navi == navi && !run.cross) flag_set(crosses[i].flag);
	/* BN5's guardians their Soul, for the run's BN5 battles (docs/META.md,
	 * Souls in BN5 territory) */
	soul_give(navi);
	/* the Cybeast's fall gives BeastOut for the rest of the run (issue
	 * #109: beat the beast, then its power), as Dad says in his call
	 * (powers_den_text) */
	if (navi == SUPER_CYBEAST) flag_set(BN6_FLAG_BEAST_OUT);
}
