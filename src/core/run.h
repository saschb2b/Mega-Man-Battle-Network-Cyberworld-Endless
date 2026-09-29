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
	/* what the player brought (docs/META.md): the net's length, the
	 * starting folder, the threat rung, the helpers switched on */
	uint8_t mode;            /* RUN_ENDLESS (0, as runs were), RUN_SHORT */
	uint8_t folder;          /* FOLDER_* (folders.h) */
	uint8_t threat;          /* 0-THREAT_MAX */
	uint8_t helpers;         /* HELP_* bits */
	/* (new since the "CWE5" save, which reads them as 0) */
	uint8_t cross;           /* the Cross brought from the start: its navi (1-5, powers.c), 0 none */
	uint8_t codes[3];        /* the folder's codes as the layer was made, most held first: code + 1 (loot_fit_code) */
	/* (new since the "CWE6" save) */
	uint8_t programs[8];     /* the NaviCust programs on MegaMan's board as the layer was made (variant: program * 4 + v), 0 ends */
} Run;

#define CYCLE_LAYERS 19      /* 6 acts of 3 layers, then the Cybeast Nest */
/* The short net: three acts, then the Nest on layer 10, whose fall wins
 * the run; the endless net repeats CYCLE_LAYERS, harder each time. */
enum { RUN_ENDLESS, RUN_SHORT };
#define SHORT_LAYERS 10
#define THREAT_MAX 9
enum { HELP_HEAD_START = 1, HELP_HEALS = 2, HELP_GENTLE = 4 };

extern Run run;

void run_new(uint32_t seed);
/* The setup chosen for the run just made: in the short net its Nest's
 * guardian is picked as a fourth act's. */
void run_setup(int mode, int folder, int threat, int helpers, int cross);
/* The other way act `act` (0-based) may take, offered where the act before
 * it ends (docs/META.md, routes): an area of its tier the run has not
 * taken, and `navi` its guardian, fit for the act and no other act's; -1
 * for none (the first act, the short net's Nest, the Undernet and on). */
int run_route_alt(int act, int *navi);
/* Whether depth is the short net's Nest, the run's last layer. */
static inline bool run_short_nest(int depth) { return run.mode == RUN_SHORT && depth >= SHORT_LAYERS; }
int biome_bg(int b);
/* A navi's HP at a version (0 V1, 1 EX, 2 SP) from the ROM; -1 unknown. */
int navi_hp(int navi, int version);
/* The area's first battle background, without a roll (the title's backdrop). */
int biome_backdrop(int b);

#endif
