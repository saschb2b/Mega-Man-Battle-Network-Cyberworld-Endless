/* The town: a generated square in Central Town's style, where Lan starts a
 * run and jacks MegaMan in (docs/OVERWORLD.md). */
#ifndef CW_TOWN_H
#define CW_TOWN_H

#include <stdbool.h>
#include <stdint.h>

/* The real-world map the town takes over, and whose tiles it is made of
 * (Central Town). */
#define TOWN_GROUP  0x01
#define TOWN_NUMBER 0x00
#define TOWN_SONG   0x03

typedef struct {
	int start_x, start_y;   /* where Lan arrives (world units) */
	int port_x, port_y;     /* the jack-in port */
	int tw, th;             /* the tile map's size */
	int misses;             /* tiles no source tile matched */
	int picks;
} TownInfo;

/* Plans the town for `seed` and picks its tiles (no core needed). */
bool town_plan(uint32_t seed);
const TownInfo *town_info(void);
/* The planned town's two tile layers (tw * th entries each), and where
 * no source tile matched. */
const uint16_t *town_tiles(void);
const uint8_t *town_misses(void);

/* Installs the planned town in its map: tiles, walls, the port's jack-in
 * trigger, trees, people and their words, the music; jacking in takes
 * MegaMan to world (x, y) of map (group, number). */
bool town_install(int to_group, int to_number, int x, int y);

/* The planned town drawn with the source's tiles (the dev atlas): ARGB,
 * w x h, to free. */
uint32_t *town_render(int *w, int *h);

#endif
