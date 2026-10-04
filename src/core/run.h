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
#define THREAT_MAX 10
/* The helpers (docs/META.md), a bit each in run.helpers: two more
 * HPMemory, a heal on every layer, gentler battles, every chip in * */
enum { HELP_HEAD_START = 1, HELP_HEALS = 2, HELP_GENTLE = 4, HELP_ALL_STAR = 8 };
#define HELPERS 4

extern Run run;

/* Whether every chip of the run comes in * alone (the All * helper): in
 * the game, whose chip records then hold * alone, and in chip_info. */
static inline bool run_all_star(void) { return (run.helpers & HELP_ALL_STAR) != 0; }

void run_new(uint32_t seed);
/* The net area that draws BN6 area `biome` in this run: its own, or
 * another game's like it (docs/MULTIROM.md), where that game's ROM is
 * there and the run's seed picks it, in half the runs. */
int run_dress(int biome);
/* The guardian of the run's act in area `biome` (a normal layer's): one of
 * another game's Navis where his game dresses the area and guards it in
 * this run (run_xguardian_at), else the run's own pick, run.boss_order.
 * The navi numbers guardians.h names. */
int run_guardian(int biome);
/* ... and the guardian of the layer MegaMan stands on: a side layer's
 * (an Undernet detour, the Secret Area) is run.boss_order's */
int run_layer_guardian(void);
/* Another game's Navi guarding act `act` (0-based) in area `biome`, 0 for
 * none (docs/BOSSES.md, BN5's Navis): where an area of his game dresses it
 * and names him (NetAreaDef.xguard), the build can fight its battles, and
 * the run's seed says so, in half the runs, never on the short net's last
 * act. From the seed and the ROMs present: nothing is saved. */
int run_xguardian_at(int act, int biome);
/* (dev: --guardian N with one of another game's Navis, who then guards
 * every area his game dresses; --net-biome xN's area, which draws its BN6
 * area whatever the seed says) */
extern int run_debug_xguardian, run_debug_area;
/* (dev: --guardian N, navi N guarding every area, or one of another
 * game's Navis every area his game dresses) */
void run_debug_guardian(int navi);
/* The setup chosen for the run just made: in the short net its Nest's
 * guardian is picked as a fourth act's. */
void run_setup(int mode, int folder, int threat, int helpers, int cross);
/* The other way act `act` (0-based) may take, offered where the act before
 * it ends (docs/META.md, routes): an area of its tier the run has not
 * taken, and `navi` its guardian, fit for the act and no other act's; -1
 * for none (the first act, the short net's Nest, the Undernet and on). */
int run_route_alt(int act, int *navi);
/* The dark way the short net's last act before the Nest (act 2, 0-based)
 * may take once it is open (docs/META.md, branches): the Undernet, `navi`
 * its guardian, fit for the act, no other act's and not `avoid` (the other
 * way's); -1 for none. */
int run_route_dark(int act, int avoid, int *navi);
/* Whether depth is the short net's Nest, the run's last layer (on threat
 * 10 its last two: the first guardian leads down to a second). */
static inline bool run_short_nest(int depth) { return run.mode == RUN_SHORT && depth >= SHORT_LAYERS; }
static inline bool run_short_last(int depth) { return run_short_nest(depth) && depth >= SHORT_LAYERS + (run.threat >= 10); }
/* The short net's second Nest guardian (threat 10): drawn from the run's
 * seed as the first is, none of the acts' guardians nor the first. */
int run_nest_second(void);
int biome_bg(int b);
/* A navi's HP at a version (0 V1, 1 EX, 2 SP) from the ROM; -1 unknown. */
int navi_hp(int navi, int version);
/* The area's first battle background, without a roll (the title's backdrop). */
int biome_backdrop(int b);

#endif
