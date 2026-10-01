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

/* Gregar's Crosses by navi index: HeatMan 1 .. ChargeMan 5, what MegaMan
 * feels of each, the attacks that hit it twice as hard and what it gives
 * (BN6's own Cross tutorials, CompText86D0614: its chips' +50 is theirs
 * that don't dim the screen; a playtester chose a Cross by its weakness
 * alone, the setup naming no strength) */
static const struct { int navi, flag; const char *name, *feel, *weak, *strong, *navis; } crosses[] = {
	{ 1, BN6_FLAG_HEAT_CROSS, "HeatCross", "HeatMan's Cross data! My chest is burning up, Lan!", "Aqua", "Fire chips +50, buster +1" },
	{ 2, BN6_FLAG_ELEC_CROSS, "ElecCross", "ElecMan's Cross data! It's crackling all through me!", "Wood", "Elec chips +50" },
	{ 3, BN6_FLAG_SLASH_CROSS, "SlashCross", "SlashMan's Cross data! I feel faster already!", "Breaker", "Sword chips +50" },
	/* (BN6's EraseCross: an elementless chip that doesn't dim the screen
	 * deletes a virus whose HP has a 4 in it, and bugs a Navi, whose HP
	 * then drains; two playtesters had found it by chance and taken it for
	 * counters, and a third saw DiveMan's HP fall with nothing hitting;
	 * a fourth, told only of viruses, met a Navi's bug unexplained) */
	{ 4, BN6_FLAG_ERASE_CROSS, "EraseCross", "EraseMan's Cross data... It's cold, Lan. But it's power.", "Wind", "A 4 in HP: plain chips erase",
	  "Navis: a bug drains their HP" },
	{ 5, BN6_FLAG_CHARGE_CROSS, "ChargeCross", "ChargeMan's Cross data! Full steam ahead, Lan!", "Aqua", "One more chip each turn" },
};

/* Whether the run has beaten `navi` as an earlier act's guardian (its
 * Cross is MegaMan's already). */
static bool beaten_before(int navi, int depth) {
	for (int d = 1; d < depth; ++d)
		if (is_boss_depth(d) && run.boss_order[biome_for_depth(d)] == navi) return true;
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

void powers_bring(int navi) {
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i)
		if (crosses[i].navi == navi) flag_set(crosses[i].flag);
}

const char *powers_reward_text(int navi, int biome, int depth) {
	static char text[512];
	int k = 0;
	#define ADD(...) (k += snprintf(text + k, k < (int)sizeof text ? sizeof text - (size_t)k : 0, __VA_ARGS__))
	/* (a run that brought a Cross keeps it alone: the choice's cost) */
	const char *brought = powers_cross_name(run.cross);
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i) {
		if (crosses[i].navi != navi || beaten_before(navi, depth)) continue;
		if (!brought) ADD("%sMegaMan got:\n\"%s\"!!|@M %s|@M But %s attacks hit me twice as hard in it, Lan.", k ? "|" : "", crosses[i].name, crosses[i].feel,
			crosses[i].weak);
		else if (run.cross == navi) ADD("%s@M %s's Cross data... We brought his Cross along already, Lan!", k ? "|" : "", guardian(navi)->name);
		/* (said as the net's fact, not a rule's: "One Cross a run!" put
		 * the game's word in MegaMan's mouth; a first win opens its start
		 * for good, said here, where a playtester read "won't fit" as
		 * lost, and found the start in the setup) */
		else ADD("%s@M %s's Cross data... It won't fit beside our %s, Lan. We can only carry one Cross down here!%s", k ? "|" : "",
			guardian(navi)->name, brought, profile.crosses_open >> navi & 1 ? "" : "|@M But it's ours now. Next dive, we can start with it!");
	}
	/* the Graveyard sits over the Nest: its call wakes the Cybeast in
	 * MegaMan, and Dad lets him use it (once a run) */
	if (biome == BIOME_GRAVEYARD && depth <= CYCLE_LAYERS)
		ADD("%s@B Grrrr...!|@M Lan... The Nest is calling to the Cybeast inside me!|"
			"@D Lan, it's Dad! I'm unlocking the Cybeast Button in your PET.|"
			"@D BeastOut is strong, but don't let the beast take over!|@N MegaMan can now BeastOut!", k ? "|" : "");
	#undef ADD
	return k ? text : NULL;
}

void powers_after_boss(int navi, int biome) {
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i)
		if (crosses[i].navi == navi && !run.cross) flag_set(crosses[i].flag);
	/* the Graveyard's guardian wakes the Cybeast */
	if (biome == BIOME_GRAVEYARD) flag_set(BN6_FLAG_BEAST_OUT);
}
