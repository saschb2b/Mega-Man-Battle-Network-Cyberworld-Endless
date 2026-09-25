/* The ground of an original real-world map, learned as the source of a
 * generated town's tiles (docs/OVERWORLD.md). */
#ifndef CW_TOWNSRC_H
#define CW_TOWNSRC_H

#include <stdbool.h>
#include <stdint.h>

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

/* Learns map (group, number) and its mirror image. */
TownBook *townsrc_learn(int group, int number);
void townsrc_free(TownBook *b);

/* The source's own size in tiles: a generated map keeps its parity. */
void townsrc_size(const TownBook *b, int *tw, int *th);
/* ROM offsets of the source's MapBGDescriptor and coordinate-data pointer. */
void townsrc_slots(const TownBook *b, uint32_t *desc, uint32_t *coord_slot);

int townsrc_profiles(const TownBook *b, const TownProfile **out);

/* The materials of a plan, by 8-unit world cell. */
typedef int (*TownCell)(int cx, int cy);

typedef struct {
	int picks, exact, coherent, near;
} TownSynthStats;

/* Picks the tiles of a tw x th map (layer 0 then layer 1, tw * th entries
 * each) for the plan `cell`; world (0, 0) is the map's middle, as the game
 * draws it. `miss`, when given, gets 1 where no source tile matched. */
void townsrc_synth(const TownBook *b, TownCell cell, int tw, int th, uint32_t seed, uint16_t *map, uint8_t *miss, TownSynthStats *st);

#endif
