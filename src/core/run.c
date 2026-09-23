#include "run.h"

#include <string.h>

#include "data.h"
#include "game.h"
#include "rom.h"

Run run;

void folder_add(int id, char code) {
	if (run.folder_n >= FOLDER_MAX) return;
	run.folder[run.folder_n].id = (uint16_t)id;
	run.folder[run.folder_n].code = code;
	run.folder_n++;
}

void run_new(uint32_t seed) {
	memset(&run, 0, sizeof run);
	run.active = true;
	run.seed = seed;
	run.hp = run.max_hp = 200;
	run.atk = 1;
	run.rapid = 1;
	run.charge = 1;
	run.custom_size = 5;
	run.depth = 1;
	run.zenny = 300;
	/* A starter folder in the spirit of the games' default folders. */
	static const struct { int id; char code; } start[] = {
		{ 1, 'A' }, { 1, 'A' }, { 1, 'B' }, { 1, 'B' },  /* Cannon */
		{ 4, '*' }, { 4, '*' },                          /* AirShot */
		{ 71, 'A' }, { 71, 'A' }, { 72, 'H' },           /* Sword, WideSwrd */
		{ 5, 'H' }, { 5, 'H' },                          /* Vulcan1 */
		{ 54, 'A' }, { 54, 'G' },                        /* MiniBomb */
		{ 154, 'A' }, { 155, 'A' },                      /* Recov10, Recov30 */
		{ 162, '*' },                                    /* PanlGrab */
		{ 40, 'E' }, { 9, 'B' },                         /* RlngLog1, Spreadr1 */
	};
	for (size_t i = 0; i < sizeof start / sizeof *start; ++i) folder_add(start[i].id, start[i].code);
	rng_seed(seed);
	/* Acts 1-4 visit the four surface areas in a random order. */
	uint8_t surface[4] = { BIOME_CENTRAL, BIOME_SEASIDE, BIOME_SKY, BIOME_GREEN };
	for (int i = 3; i > 0; --i) { int j = rng_range(0, i); uint8_t t = surface[i]; surface[i] = surface[j]; surface[j] = t; }
	for (int i = 0; i < 4; ++i) run.biome_order[i] = surface[i];
	run.biome_order[4] = BIOME_GRAVEYARD;
	run.biome_order[5] = BIOME_UNDERNET;
	/* Each area's guardian, drawn from navis that suit it. */
	static const uint8_t pools[BIOME_COUNT][3] = {
		{ 12, 2, 5 },   /* Central: BlastMan, ElecMan, ChargeMan */
		{ 13, 2, 1 },   /* Seaside: DiveMan, ElecMan, HeatMan */
		{ 16, 15, 2 },  /* Sky: ElementMan, JudgeMan, ElecMan */
		{ 7, 9, 5 },    /* Green: TomahawkMan, GroundMan, ChargeMan */
		{ 4, 10, 3 },   /* Graveyard: EraseMan, DustMan, SlashMan */
		{ 11, 3, 1 },   /* Undernet: ProtoMan, SlashMan, HeatMan */
		{ 11, 11, 11 }, /* Secret Area: ProtoMan SP */
		{ 3, 4, 11 },   /* Cybeast Nest */
	};
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = pools[b][rng_range(0, 2)];
}

int biome_bg(int b) {
	/* the BattleSettings background byte (0x00-0x15) for each biome's battles */
	static const int bg[BIOME_COUNT] = { 0x07, 0x0B, 0x04, 0x0D, 0x14, 0x0F, 0x13, 0x15 };
	return b >= 0 && b < BIOME_COUNT ? bg[b] : 0;
}

