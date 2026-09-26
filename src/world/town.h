/* The town: one of BN6's towns cut into pieces and set out again, where
 * Lan starts a run and jacks MegaMan in (docs/OVERWORLD.md). */
#ifndef CW_TOWN_H
#define CW_TOWN_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"   /* CoordCell */

#define TOWN_SPOTS 6

typedef struct {
	int group, number;      /* the real-world map it is cut from and takes over */
	int start_x, start_y;   /* where Lan arrives (world units) */
	int start_face;         /* ... facing (the game's warp facing) */
	int port_x, port_y;     /* the middle of the jack-in point */
	int tw, th;             /* the tile map's size */
	int misses;             /* picked tiles no source tile matched */
	int picks;
	int spots[TOWN_SPOTS][2];   /* places worth a look (the dev tour) */
	uint32_t talk_archive;      /* once installed: the townsfolk's text archive */
	int intro;                  /* ... and in it, what Lan and MegaMan say on arriving */
} TownInfo;

/* The town's seed for a run's seed (apart from the layers'). */
static inline uint32_t town_seed(uint32_t run_seed) { return run_seed ^ 0x70776E00u; }

/* Plans the town for `seed` and picks its tiles (no core needed). */
bool town_plan(uint32_t seed);
const TownInfo *town_info(void);
/* The planned town's two tile layers (tw * th entries each), and where
 * no source tile matched. */
const uint16_t *town_tiles(void);
const uint8_t *town_misses(void);

/* Whether Lan can stand on 8-unit world cell (cx, cy). */
bool town_walkable(int cx, int cy);
/* Where to head from world (x, y) to reach the jack-in on foot. */
bool town_route(int x, int y, int *wx, int *wy);
/* The town's trigger cells (section 3: the jack-in 0x40, checks 0xF0 +). */
int town_triggers(const CoordCell **cells);
/* The run before this one was left unfinished (NEW GAME over CONTINUE):
 * the town's first words do not speak of how it ended. */
extern bool town_after_abandon;
/* Whether world position (x, y) is a jack-in cell (R jacks in there). */
bool town_on_port(int x, int y);
/* The town's map objects (id) and people (id -1), in world units. */
void town_objects(void (*fn)(int id, int x, int y, void *ctx), void *ctx);

/* Installs the planned town in its original's map: tiles, walls, the
 * jack-in cells and checks and what they say, trees and statues, people
 * and their words, the music; jacking in takes MegaMan to world (x, y) of
 * map (group, number). */
bool town_install(int to_group, int to_number, int x, int y);

/* The planned town drawn with the source's tiles (the dev atlas): ARGB,
 * w x h, to free. */
uint32_t *town_render(int *w, int *h);

#endif
