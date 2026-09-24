#include "run.h"

#include <string.h>

#include "data.h"
#include "game.h"
#include "pacing.h"

Run run;

void run_new(uint32_t seed) {
	memset(&run, 0, sizeof run);
	run.active = true;
	run.seed = seed;
	run.depth = 1;
	rng_seed(seed);
	/* Acts 1-4 visit four of the surface areas, easier ones first; then the
	 * Undernet and the Graveyard, which BN6 keeps for after its story. */
	pacing_area_order(run.biome_order);
	run.biome_order[4] = BIOME_UNDERNET;
	run.biome_order[5] = BIOME_GRAVEYARD;
	/* Each area's guardian, drawn from navis that suit it; in the areas the
	 * run visits, from those whose HP suits the act. */
	static const uint8_t pools[BIOME_COUNT][4] = {
		{ 12, 2, 5, 12 },   /* Central: BlastMan, ElecMan, ChargeMan */
		{ 13, 6, 2, 1 },    /* Seaside: DiveMan, SpoutMan, ElecMan, HeatMan */
		{ 16, 15, 8, 2 },   /* Sky: ElementMan, JudgeMan, TenguMan, ElecMan */
		{ 7, 9, 5, 7 },     /* Green: TomahawkMan, GroundMan, ChargeMan */
		{ 4, 10, 3, 4 },    /* Graveyard: EraseMan, DustMan, SlashMan */
		{ 11, 3, 1, 11 },   /* Undernet: ProtoMan, SlashMan, HeatMan */
		{ 11, 11, 11, 11 }, /* Secret Area: ProtoMan SP */
		{ 3, 4, 11, 3 },    /* Cybeast Nest */
		{ 14, 12, 18, 14 }, /* Comp: CircusMan, BlastMan, Colonel */
		{ 14, 16, 13, 16 }, /* Homepage: CircusMan, ElementMan, DiveMan */
		{ 18, 15, 12, 18 }, /* Comp (second): Colonel, JudgeMan, BlastMan */
		{ 1, 12, 5, 1 },    /* Robot Control Comp: HeatMan, BlastMan, ChargeMan */
		{ 2, 6, 13, 2 },    /* Aquarium Comp: ElecMan, SpoutMan, DiveMan */
		{ 3, 7, 15, 3 },    /* Judge Tree Comp: SlashMan, TomahawkMan, JudgeMan */
		{ 4, 8, 16, 4 },    /* Mr. Weather Comp: EraseMan, TenguMan, ElementMan */
		{ 18, 14, 11, 18 }, /* CopyBot's comp: Colonel, CircusMan, ProtoMan */
		{ 1, 5, 12, 1 },    /* ACDC HP: HeatMan, ChargeMan, BlastMan */
		{ 3, 9, 7, 3 },     /* Green HP: SlashMan, GroundMan, TomahawkMan */
		{ 4, 10, 16, 4 },   /* Sky HP: EraseMan, DustMan, ElementMan */
	};
	static const uint8_t navis[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18 };
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = pools[b][rng_range(0, 3)];
	for (int act = 0; act < 6; ++act) {
		int b = run.biome_order[act];
		run.boss_order[b] = (uint8_t)pacing_guardian_pick(pools[b], navis, (int)sizeof navis, act, 0, false, navi_hp);
	}
}

int navi_hp(int navi, int version) {
	int hp, damage, id = enemy_id(1, navi, version);
	return id >= 0 && enemy_stats(id, &hp, &damage) ? hp : -1;
}

/* the BattleSettings background (0x00-0x15) of each area's battles, two
 * where the game has a second that suits it */
static const uint8_t biome_bgs[BIOME_COUNT][2] = {
	{ 0x07, 0x09 }, { 0x0B, 0x0A }, { 0x04, 0x10 }, { 0x0D, 0x0C }, { 0x14, 0x12 }, { 0x0F, 0x11 },
	{ 0x13, 0x13 }, { 0x15, 0x15 }, { 0x06, 0x06 }, { 0x03, 0x01 }, { 0x08, 0x00 },
	{ 0x0E, 0x06 }, { 0x0A, 0x0B }, { 0x05, 0x0C }, { 0x10, 0x04 }, { 0x06, 0x0E },
	{ 0x00, 0x00 }, { 0x01, 0x0D }, { 0x04, 0x10 },
};

int biome_bg(int b) { return b >= 0 && b < BIOME_COUNT ? biome_bgs[b][rng_range(0, 1)] : 0; }

int biome_backdrop(int b) { return b >= 0 && b < BIOME_COUNT ? biome_bgs[b][0] : 0; }
