/* The ground of an original real-world map, learned as the source of a
 * generated town's tiles (docs/OVERWORLD.md). */
#ifndef CW_TOWNSRC_H
#define CW_TOWNSRC_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"

/* The ground's materials, per 8-unit world cell. Edge is anything off the
 * walkable ground that is not a planter or the sky: faces, buildings. */
enum {
	TM_VOID, TM_ROAD, TM_SIDE, TM_COBB, TM_BRICK, TM_GRASS, TM_MARK, TM_EDGE,
	TM_COUNT
};

/* Along axis o (0: world x, 1: y) the source shows a run of material a,
 * then nmid cells of mid, then a run of b, where a ends at `ph` (mod 32). */
typedef struct {
	uint8_t o, a, b, nmid, mid[3], ph;
	uint8_t mirrored;   /* seen only in the mirror image */
	int count;
} TownProfile;

typedef struct TownBook TownBook;

/* A source's material of each colour index. */
typedef int (*TownMatOf)(int colour);

/* Learns map (group, number) and its mirror image. */
TownBook *townsrc_learn(int group, int number, TownMatOf mat_of);
void townsrc_free(TownBook *b);

/* The source's own size in tiles: a generated map keeps its parity. */
void townsrc_size(const TownBook *b, int *tw, int *th);
/* ROM offsets of the source's MapBGDescriptor and coordinate-data pointer. */
void townsrc_slots(const TownBook *b, uint32_t *desc, uint32_t *coord_slot);


/* The original map (not its mirror): decoded, and per 8-unit cell its
 * material and whether Lan walks there (floor that no wall covers; walls
 * that an event flag lifts do not count). */
const AreaSrc *townsrc_area(const TownBook *b);
int townsrc_mat(const TownBook *b, int cx, int cy);
bool townsrc_walk(const TownBook *b, int cx, int cy);

/* The front layer's art in pieces: 8-connected runs of its tiles, each
 * standing on the cell under its lowest pixels (its foot). part[] gives
 * each tile's piece (-1 none), foot[] each piece's cell. */
typedef struct { int16_t cx, cy; } TownFoot;
int townsrc_parts(const TownBook *b, const int **part, const TownFoot **foot);

/* The original's map objects (its 20-byte spawn records, as read from the
 * ROM), up to `max`. */
int townsrc_objects(const TownBook *b, uint8_t (*rec)[20], int max);

/* The materials of a plan, by 8-unit world cell. */
typedef int (*TownCell)(int cx, int cy);

typedef struct {
	int picks, exact, coherent, hinted, near;
} TownSynthStats;

/* A tile taken from the original instead of picked: its layer-0 entry from
 * source tile (x0, y0), its layer-1 entry from (x1, y1); -1 where a layer
 * is not taken (then picked), x1 -2 where layer 1 stays empty. A picked
 * tile is source tile (hx, hy) when that one's key is the tile's (-1:
 * no such hint): stretched ground repeats the original's own tiles. */
typedef struct { int16_t x0, y0, x1, y1, hx, hy; } TownPin;

/* Picks the tiles of a tw x th map (layer 0 then layer 1, tw * th entries
 * each) for the plan `cell`; world (0, 0) is the map's middle, as the game
 * draws it. `pins` (tw * th, or NULL) are the tiles taken whole; picks
 * next to them continue them. `miss`, when given, gets 1 where no source
 * tile matched, 2 where the mirror's served, 4 where a hint did, 8 where
 * the tile was taken whole. */
void townsrc_synth(const TownBook *b, TownCell cell, int tw, int th, uint32_t seed, const TownPin *pins, uint16_t *map, uint8_t *miss, TownSynthStats *st);

#endif
