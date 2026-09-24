#include "run.h"

#include <string.h>

#include "game.h"

Run run;

void run_new(uint32_t seed) {
	memset(&run, 0, sizeof run);
	run.active = true;
	run.seed = seed;
	run.depth = 1;
	rng_seed(seed);
	/* Acts 1-4 visit four of the surface areas, in a random order. */
	uint8_t surface[] = { BIOME_CENTRAL, BIOME_SEASIDE, BIOME_SKY, BIOME_GREEN, BIOME_COMP, BIOME_HOMEPAGE, BIOME_COMP_B };
	enum { NSURFACE = sizeof surface };
	for (int i = NSURFACE - 1; i > 0; --i) { int j = rng_range(0, i); uint8_t t = surface[i]; surface[i] = surface[j]; surface[j] = t; }
	for (int i = 0; i < 4; ++i) run.biome_order[i] = surface[i];
	run.biome_order[4] = BIOME_GRAVEYARD;
	run.biome_order[5] = BIOME_UNDERNET;
	/* Each area's guardian, drawn from navis that suit it. */
	static const uint8_t pools[BIOME_COUNT][4] = {
		{ 12, 2, 5, 12 },   /* Central: BlastMan, ElecMan, ChargeMan */
		{ 13, 6, 2, 1 },    /* Seaside: DiveMan, SpoutMan, ElecMan, HeatMan */
		{ 16, 15, 8, 2 },   /* Sky: ElementMan, JudgeMan, TenguMan, ElecMan */
		{ 7, 9, 5, 7 },     /* Green: TomahawkMan, GroundMan, ChargeMan */
		{ 4, 10, 3, 4 },    /* Graveyard: EraseMan, DustMan, SlashMan */
		{ 11, 3, 1, 11 },   /* Undernet: ProtoMan, SlashMan, HeatMan */
		{ 11, 11, 11, 11 }, /* Secret Area: ProtoMan SP */
		{ 3, 4, 11, 3 },    /* Cybeast Nest */
		{ 14, 12, 17, 14 }, /* Comp: CircusMan, BlastMan, Colonel */
		{ 14, 16, 13, 16 }, /* Homepage: CircusMan, ElementMan, DiveMan */
		{ 17, 15, 12, 17 }, /* Comp (second): Colonel, JudgeMan, BlastMan */
	};
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = pools[b][rng_range(0, 3)];
}

int biome_bg(int b) {
	/* the BattleSettings background (0x00-0x15) of each area's battles, two
	 * where the game has a second that suits it */
	static const uint8_t bg[BIOME_COUNT][2] = {
		{ 0x07, 0x09 }, { 0x0B, 0x0A }, { 0x04, 0x10 }, { 0x0D, 0x0C }, { 0x14, 0x12 }, { 0x0F, 0x11 },
		{ 0x13, 0x13 }, { 0x15, 0x15 }, { 0x06, 0x06 }, { 0x03, 0x01 }, { 0x08, 0x00 },
	};
	return b >= 0 && b < BIOME_COUNT ? bg[b][rng_range(0, 1)] : 0;
}
