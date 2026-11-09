/* The layers' sizes (docs/engine/emulation/free-space.md, Moving regions):
 * every area's layers built as a run of each seed would build them, at
 * every depth of the net's cycle and past it and in each side layer's
 * kind, and the largest tile map and coordinate data each area made, beside
 * the regions that hold them (FREE_TILE_MAP, FREE_COORDS). The regions are
 * sized from it with a quarter to spare; this fails where one is outgrown. */
#include "layer_sizes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coords.h"
#include "free_space.h"
#include "net.h"
#include "net_layouts.h"
#include "netmap.h"
#include "rom.h"
#include "run.h"

/* the warp pads a layer adds to its coordinate data, at most (coords.c's
 * TRIGGERS_MAX cells, 8 bytes each, as literal blocks): the layers built
 * here have none */
#define PADS_BYTES (49 * 16 * 8 * 9 / 8)

typedef struct { size_t bytes; uint32_t seed; int depth, kind; } Peak;

static void peak(Peak *p, size_t bytes, uint32_t seed, int depth, int kind) {
	if (bytes <= p->bytes) return;
	*p = (Peak){ bytes, seed, depth, kind };
}

/* One area over `seeds` runs: each normal layer of depths 1 to the cycle's
 * length and one past it (every size class and guardian's layer), and
 * each side layer's kind past it, at the largest size. */
static void area_sizes(FILE *out, int area, int seeds, Peak *map, Peak *coord) {
	int b = area < NET_AREAS ? area : net_area_def(area)->like, built = 0, failed = 0;
	Peak m = { 0 }, c = { 0 };
	LayerKit kit;
	netmap_kit(area, &kit);
	for (int s = 1; s <= seeds; ++s)
		for (int i = 0; i < CYCLE_LAYERS + 1 + 4; ++i) {
			int depth = i <= CYCLE_LAYERS ? i + 1 : CYCLE_LAYERS + 1;
			int kind = i <= CYCLE_LAYERS ? LAYER_NORMAL : LAYER_UNDERNET + i - CYCLE_LAYERS - 1;
			run_new((uint32_t)s);
			run.depth = depth;
			run.biome = b;
			run.side_kind = kind;
			layout_forced = -1;
			uint32_t seed = run_layer_seed(depth, kind);
			layer_generate(seed, depth, b, kind, &kit);
			if (!netmap_build_layer(area, seed)) { ++failed; continue; }
			++built;
			peak(&m, netmap_tilemap_bytes(), (uint32_t)s, depth, kind);
			peak(&c, coords_layer_bytes() + PADS_BYTES, (uint32_t)s, depth, kind);
		}
	fprintf(out, "area %2d: %d layers built, %d not; tile map %zu bytes (seed %u depth %d kind %d), coordinates %zu with pads (seed %u depth %d kind %d)\n",
		area, built, failed, m.bytes, m.seed, m.depth, m.kind, c.bytes, c.seed, c.depth, c.kind);
	fflush(out);
	if (m.bytes > map->bytes) *map = m;
	if (c.bytes > coord->bytes) *coord = c;
}

int layer_sizes_run(const char *dir, const char *areas, int seeds) {
	xarea_review(XAREA_ALL);
	xrom_find_beside();
	bool want[NET_AREAS + XAREAS_MAX] = { false };
	if (*areas != '=') for (int a = 0; a < NET_AREAS + XAREAS_MAX; ++a) want[a] = true;
	else {
		char list[256];
		snprintf(list, sizeof list, "%s", areas + 1);
		for (char *t = strtok(list, ","); t; t = strtok(NULL, ",")) {
			int a = t[0] == 'x' ? NET_AREAS + atoi(t + 1) : atoi(t);
			if (a >= 0 && a < NET_AREAS + XAREAS_MAX) want[a] = true;
		}
	}
	char path[600];
	snprintf(path, sizeof path, "%s/sizes%s.txt", dir, areas);
	FILE *out = fopen(path, "w");
	if (!out) { fprintf(stderr, "layer sizes: cannot write %s\n", path); return 1; }
	Peak map = { 0 }, coord = { 0 };
	for (int a = 0; a < NET_AREAS + XAREAS_MAX; ++a)
		if (want[a] && (a < NET_AREAS || net_area_def(a))) area_sizes(out, a, seeds, &map, &coord);
	size_t map_room = FREE_TILE_MAP_END - FREE_TILE_MAP, coord_room = FREE_COORDS_END - FREE_COORDS;
	fprintf(out, "largest: tile map %zu of %zu, coordinates %zu of %zu\n", map.bytes, map_room, coord.bytes, coord_room);
	fclose(out);
	layout_forced = -1;
	bool fits = map.bytes <= map_room && coord.bytes <= coord_room;
	printf("layer sizes in %s: tile map %zu of %zu, coordinates %zu of %zu%s\n", path, map.bytes, map_room, coord.bytes, coord_room,
		fits ? "" : ": PAST THEIR REGION");
	return fits ? 0 : 1;
}
