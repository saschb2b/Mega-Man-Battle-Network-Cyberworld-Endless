/* The atlas's Glitch looks (docs/game/glitch-layer.md, Checking an area's
 * look): a layer built as the game builds it, then picked again in another
 * area's tiles as a Glitch layer's flip shows it (netmap_look), both drawn
 * side by side without running the game. "intoAREA" draws a layer of every
 * other area in AREA's look, for the area's own pieces, scenery and stairs
 * on floors not its own; "fromAREA" one of AREA's in every other area's. A
 * line of DIR/report.txt each: the layer's stairs and rise, what each pick
 * set (scenery, side pieces) and the holes its safety net left. */
#include "atlas_looks.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "net.h"
#include "net_layouts.h"
#include "netmap.h"
#include "rom.h"
#include "run.h"
#include "tiles.h"
#include "voids.h"

#define VOID_ARGB 0xFF282830u
#define GAP 16   /* pixels between the two pictures */

/* (AREA as --net-biome names it: N a BN6 area, xN another game's) */
static int area_named(const char *s) { return s[0] == 'x' ? NET_AREAS + atoi(s + 1) : atoi(s); }

static int holes_left(void) {
	int tw, th, left = 0;
	const uint8_t *v = voids_last(&tw, &th);
	for (int i = 0; v && i < tw * th; ++i) left += v[i] && !(v[i] & VOID_MENDED);
	return left;
}

/* Area `area`'s picture of tiles `map` (tw x th), void filled */
static uint32_t *drawn(int area, const uint16_t *map, int tw, int th) {
	const NetAreaDef *a = net_area_def(area);
	uint32_t *px = a ? area_src_render(a->xrom, a->group, a->number, map, tw, th) : NULL;
	for (int i = 0; px && i < tw * th * 64; ++i) if (!(px[i] >> 24)) px[i] = VOID_ARGB;
	return px;
}

/* The two pictures side by side, saved as DIR/NAME.bmp */
static void save_pair(const char *dir, const char *name, const uint32_t *own, const uint32_t *look, int tw, int th) {
	int w = tw * 8, W = 2 * w + GAP, H = th * 8;
	uint32_t *px = malloc((size_t)W * H * 4);
	if (!px) return;
	for (int y = 0; y < H; ++y) {
		for (int x = 0; x < GAP; ++x) px[(size_t)y * W + w + x] = 0xFF000000u;
		memcpy(px + (size_t)y * W, own + (size_t)y * w, (size_t)w * 4);
		memcpy(px + (size_t)y * W + w + GAP, look + (size_t)y * w, (size_t)w * 4);
	}
	char path[1200];
	snprintf(path, sizeof path, "%s/%s.bmp", dir, name);
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(px, W, H, 32, W * 4, SDL_PIXELFORMAT_ARGB8888);
	if (s) { SDL_SaveBMP(s, path); SDL_FreeSurface(s); }
	free(px);
}

/* A layer of `from` at `depth` of `seed`, as the game builds it: false
 * where it was not built */
static bool built(int from, int depth, uint32_t seed) {
	int b = from < NET_AREAS ? from : net_area_def(from)->like;
	LayerKit kit;
	netmap_kit(from, &kit);
	run_new(seed);
	run.depth = depth;
	run.biome = b;
	layout_forced = -1;
	layer_generate(seed, depth, b, LAYER_NORMAL, &kit);
	return netmap_build_layer(from, seed);
}

/* The seed of a layer of `from` that climbs a stair, where its kit has
 * one (the first of 300 tried from `s`); 0 where none */
static uint32_t stair_seed(int from, int s) {
	LayerKit kit;
	netmap_kit(from, &kit);
	if (!(kit.stair_dirs | kit.stair_narrow)) return 0;
	int b = from < NET_AREAS ? from : net_area_def(from)->like;
	for (int k = 0; k < 300; ++k) {
		uint32_t seed = (uint32_t)((s * 300 + k) * 7919 + from * 131 + 17);
		run_new(seed);
		run.depth = 5;
		run.biome = b;
		layout_forced = -1;
		layer_generate(seed, 5, b, LAYER_NORMAL, &kit);
		if (layer.nstairs) return seed;
	}
	return 0;
}

/* One layer of `from` and its look in `into`: drawn, reported */
static void pair(const char *dir, FILE *report, int from, int into, int depth, uint32_t seed) {
	if (!built(from, depth, seed)) { fprintf(report, "look from %d into %d depth %d seed %u: the layer NOT BUILT\n", from, into, depth, seed); return; }
	int tw, th, stairs = layer.nstairs, rise = layer.rise, scenery = netmap_scenery, sides = netmap_sides(), holes = holes_left();
	const uint16_t *tiles = netmap_last_tiles(&tw, &th);
	uint32_t *own = drawn(from, tiles, tw, th);
	NetmapLook look;
	if (!netmap_look(into, &look)) {
		fprintf(report, "look from %d into %d depth %d seed %u: NO LOOK (none learned, or 256 colours)\n", from, into, depth, seed);
		free(own);
		return;
	}
	uint32_t *px = drawn(into, look.map, look.tw, look.th);
	LayerKit kit;
	netmap_kit(into, &kit);
	char name[200];
	snprintf(name, sizeof name, "look_into%02d_from%02d_d%d_s%u", into, from, depth, seed);
	if (own && px) save_pair(dir, name, own, px, tw, th);
	fprintf(report, "look from %d into %d depth %d seed %u: stairs %d rise %d (its stairs 0x%x, ladders 0x%x, rise %d), scenery %d -> %d, sides %d -> %d, holes left %d -> %d\n",
		from, into, depth, seed, stairs, rise, kit.stair_dirs, kit.stair_narrow, kit.rise, scenery, look.scenery, sides, look.sides, holes, holes_left());
	free(own);
	free(px);
	netmap_look_free(&look);
}

/* One layer (a plain one, and one up a stair where `from` has stairs) per
 * seed */
static void pairs(const char *dir, FILE *report, int from, int into, int seeds) {
	if (from == into || !net_area_def(from) || !net_area_def(into)) return;
	for (int s = 1; s <= seeds; ++s) {
		pair(dir, report, from, into, 2, (uint32_t)(s * 7919 + from * 131));
		uint32_t up = stair_seed(from, s);
		if (up) pair(dir, report, from, into, 5, up);
	}
}

bool atlas_looks_run(const char *dir, const char *spec, int seeds, int *code) {
	bool into = !strncmp(spec, "into", 4);
	if (!into && strncmp(spec, "from", 4)) return false;
	int area = area_named(spec + 4);
	xarea_review(XAREA_ALL);
	xrom_find_beside();
	*code = 1;
	if (!net_area_def(area)) { fprintf(stderr, "atlas: no area %s\n", spec + 4); return true; }
	char path[600];
	snprintf(path, sizeof path, "%s/report.txt", dir);
	FILE *report = fopen(path, "w");
	if (!report) return true;
	/* (the picks as the game makes them: unmeasured, but for the holes) */
	tiles_measure = false;
	voids_measure = true;
	for (int k = 0; k < NET_AREAS + XAREAS_MAX; ++k) {
		if (into) pairs(dir, report, k, area, seeds);
		else pairs(dir, report, area, k, seeds);
	}
	fclose(report);
	layout_forced = -1;
	printf("looks written to %s\n", dir);
	*code = 0;
	return true;
}
