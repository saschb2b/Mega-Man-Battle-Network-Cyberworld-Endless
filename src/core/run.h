/* State of the current run and the permanent profile. */
#ifndef RUN_H
#define RUN_H

#include <stdbool.h>
#include <stdint.h>

#include "jobs.h"

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
	/* what the player brought (docs/game/meta/): the net's length, the
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
	/* (new since the "CWE7" save) the Net's clock (docs/world/home/going-back.md, going
	 * back): the trips back taken, each a notch on the guardians ahead */
	uint8_t clock;
	uint8_t back_spent;      /* the older portals taken on this visit home, a bit each (pacing_older_acts) */
	uint16_t home_depth;     /* on a trip back, the run's own layer it comes home to (depth then the older act's), else 0 */
	/* (new since the "CWE8" save) the request taken at home (docs/world/home/jobs.md,
	 * piece 5: jobs.h), JOB_NONE for none */
	Job job;
	/* (new since the "CWE9" save) a dark layer's special route
	 * (docs/game/liberation/special-routes.md): the act whose board's route was
	 * found, 1-based in its cycle, 0 none, kept until the fork it turns
	 * (run_route_turns) is passed; the act that came by the route's way,
	 * the same, for its card */
	uint8_t route_found;
	uint8_t route_way;
} Run;

#define CYCLE_LAYERS 19      /* 6 acts of 3 layers, then the Cybeast Nest */
/* The short net: three acts, then the Nest on layer 10, whose fall wins
 * the run; the endless net repeats CYCLE_LAYERS, harder each time. */
enum { RUN_ENDLESS, RUN_SHORT };
#define SHORT_LAYERS 10
#define THREAT_MAX 10
/* The helpers (docs/game/meta/), a bit each in run.helpers: two more
 * HPMemory, a heal on every layer, gentler battles, every chip in *, and
 * BeastOut from the first battle (the one a profile earns: the endless
 * net's own Nest cleared, meta_beast_open; issue #99) */
enum { HELP_HEAD_START = 1, HELP_HEALS = 2, HELP_GENTLE = 4, HELP_ALL_STAR = 8, HELP_BEAST = 16 };
#define HELPERS 5
#define HELPER_BEAST 4   /* (its index: the last, shown once open) */

extern Run run;

/* Whether every chip of the run comes in * alone (the All * helper): in
 * the game, whose chip records then hold * alone, and in chip_info. */
static inline bool run_all_star(void) { return (run.helpers & HELP_ALL_STAR) != 0; }
/* Whether the run brought BeastOut (the BeastOut helper): BN6's own,
 * in the Custom screen from the first battle (powers_bring). */
static inline bool run_beast_start(void) { return (run.helpers & HELP_BEAST) != 0; }

void run_new(uint32_t seed);
/* The net area that draws BN6 area `biome` in this run: its own, or
 * another game's like it (docs/multirom/), where that game's ROM is
 * there and the run's seed picks it, in half the runs. */
int run_dress(int biome);
/* The guardian of the run's act in area `biome` (a normal layer's): one of
 * another game's Navis where his game dresses the area and guards it in
 * this run (run_xguardian_at), else the run's own pick, run.boss_order.
 * The navi numbers guardians.h names. */
int run_guardian(int biome);
/* ... and the guardian of the layer MegaMan stands on: a side layer's
 * (an Undernet detour, the Secret Area) is run.boss_order's, a trip back's
 * none (its guardian deleted, docs/world/home/) */
int run_layer_guardian(void);
/* Another game's Navi guarding act `act` (0-based) in area `biome`, 0 for
 * none (docs/game/bosses/bn5-navis.md), fought in BN6's engine where his
 * Navi is moved in: on a surface act where an area of his game dresses it
 * and names him (NetAreaDef.xguard), in half the runs; in the deep's acts
 * (the Undernet's, the Graveyard's) where his HP fits the act's band, by a
 * coin weighed by how many of each net's Navis fit it. From the seed and
 * the ROMs present: nothing is saved. */
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
 * it ends (docs/game/meta/phase-two.md, routes): an area of its tier the run has not
 * taken, and `navi` its guardian, fit for the act and no other act's; -1
 * for none (the first act, the short net's Nest, the Undernet and on). */
int run_route_alt(int act, int *navi);
/* The dark way the short net's last act before the Nest (act 2, 0-based)
 * may take once it is open (docs/game/meta/, branches): the Undernet, `navi`
 * its guardian, fit for the act, no other act's and not `avoid` (the other
 * way's); -1 for none. */
int run_route_dark(int act, int avoid, int *navi);
/* The area whose way at act `act`'s fork (0-based, from the second act:
 * the act's own area or the other way, run_route_alt) holds the act's dark
 * layer, its second (docs/game/liberation/): two acts in three that have a
 * fork, either way by a coin of the seed; -1 none. The fork as it was
 * offered, whichever way was taken since. */
int run_dark_area(int act);
/* The Darkloid of act `act`'s dark layer in area `biome`: one of BN5's own
 * Darkloids where its ROM brought them into BN6's engine, the act's of
 * BN5's missions' order the likeliest (guardians.h, navis 30-33;
 * docs/game/liberation/ stage 4); else a Navi of BN6's whose HP fits the act's
 * band, the area's own first, no act's guardian. */
int run_darkloid(int act, int biome);
/* The ways act `act` (0-based) may take once the act before it ends
 * (docs/world/home/: the town's ports): its own area, the other way's and the
 * dark way's where the act has them, each with the guardian the run will
 * set there (`navi`, run.boss_order's) and the one MegaMan meets (`meets`:
 * an older net's own where its game guards the area); how many are open,
 * the dark way only when `dark_open` (else `*sealed` where it would be).
 * The act after the net's last is the Nest's: the Nest its one way. A
 * special route found turns its fork (run_route_turns): a third way, the
 * sealed dark way's where the fork has one, else another area's of the
 * act's tier and its guardian; at the Nest, a second way to it, its
 * second guardian (run_nest_second). */
typedef struct { int biome, navi, meets; bool board, route; } RunWay;   /* (board: its act's dark layer is that way's, run_dark_area; route: the special route's way) */
int run_ways(int act, bool dark_open, RunWay way[3], bool *sealed);
/* The act (0-based) whose fork the special route found turns: the next
 * one after the board's act with a fork that has room for a third way
 * (not the short net's act 2, 0-based, with its dark way open), else the
 * Nest's; -1 none found. */
int run_route_turns(bool dark_open);
/* The route found on the dark layer of the act MegaMan leaves (`depth`'s):
 * kept for the run (run.route_found). */
void run_route_keep(int depth);
/* The endless net's Nest rebuilt past its guardian: the Cybeast's den
 * again, whoever a special route brought there. */
void run_nest_rebuilt(void);
/* The HP a Navi guarding the endless net's Nest at `depth` is fought at,
 * `hp` his own: a special route's last boss there at the Cybeast's HP at
 * least, as the Cybeast would have been (a different last boss, never an
 * easier one); the Cybeast's and the short net's own as they are. */
int run_nest_hp(int depth, int navi, int hp);
/* A guardian's HP more, in percent, each notch of the Net's clock
 * (pacing_clock_hp, docs/world/home/). */
#define RUN_CLOCK_PERCENT 10
/* The run's own depth: on a trip back, the layer it comes home to. */
static inline int run_reached(void) { return run.home_depth ? run.home_depth : run.depth; }
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
