/* Generated net layers in the game's own map formats (docs/EMULATION.md). */
#ifndef CW_NETMAP_H
#define CW_NETMAP_H

#include <stdbool.h>
#include <stdint.h>

/* A layer: panels on a gw x gh grid (1 = floor). Grid x runs down-right on
 * screen, y down-left, as in the layer generator. */
typedef struct {
	int gw, gh;
	const uint8_t *cell;
} NetLayout;

/* Builds the layer from biome `area`'s original map (RomLayout.net_area),
 * writes its tile map and walls into the free ROM space and points that map
 * at them. The map is (group, number) of the area, entered with emu_warp. */
bool netmap_build(int area, const NetLayout *lay);

/* World position of the centre of grid panel (x, y) in the last layer. */
void netmap_world(int x, int y, int *wx, int *wy);
/* The grid panel under world (wx, wy), or false outside the grid. */
bool netmap_panel(int wx, int wy, int *x, int *y);

#endif
