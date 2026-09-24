/* State of the current run and the permanent profile. */
#ifndef RUN_H
#define RUN_H

#include <stdbool.h>
#include <stdint.h>

/* The areas a run visits (RomLayout.net_area has one per area). Areas added
 * later come after the Cybeast Nest; saves keep their numbers. */
enum {
	BIOME_CENTRAL, BIOME_SEASIDE, BIOME_SKY, BIOME_GREEN, BIOME_GRAVEYARD, BIOME_UNDERNET, BIOME_SECRET, BIOME_NEST,
	BIOME_COMP, BIOME_HOMEPAGE, BIOME_COMP_B,
	/* the story's comps and the other homepages */
	BIOME_ROBOT_COMP, BIOME_AQUARIUM_COMP, BIOME_JUDGE_COMP, BIOME_WEATHER_COMP, BIOME_COPYBOT_COMP,
	BIOME_ACDC_HP, BIOME_GREEN_HP, BIOME_SKY_HP,
	BIOME_COUNT
};
#define MAX_BIOMES 32   /* room in the run save */

/* What the engine decides about a run. MegaMan himself (HP, folder, pack,
 * zenny, BugFrags, key items) lives in the game's memory and its state. */
typedef struct {
	bool active;
	uint32_t seed;
	int depth;               /* 1-based layer */
	int biome;
	int side_kind;           /* LAYER_* while in an Undernet or Secret layer */
	uint32_t layer_seed;
	uint8_t biome_order[6];  /* act -> biome for this run */
	uint8_t boss_order[MAX_BIOMES];   /* biome -> navi for this run */
	int bosses_beaten;
	int viruses_deleted;
	int fragments;           /* ScrtData held (the game's key item), for generation */
	bool secret_cleared;
} Run;

#define CYCLE_LAYERS 19      /* 6 acts of 3 layers, then the Cybeast Nest */

extern Run run;

void run_new(uint32_t seed);
int biome_bg(int b);

#endif
