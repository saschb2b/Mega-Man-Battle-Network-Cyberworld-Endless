/* The atlas (docs/DEVTOOLS.md): every area's layers built and drawn without
 * running the game, for looking over many at once. Each area in each of
 * its layouts, on a normal layer and a guardian's, drawn with the area's
 * own tiles and colours, with its objects marked; a report line per layer
 * says how the tile picks went. */
#include "atlas.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "emu.h"
#include "net.h"
#include "net_layouts.h"
#include "netmap.h"
#include "rom.h"
#include "run.h"
#include "tiles.h"

#define VOID_ARGB 0xFF282830u
#define SEAM_ARGB 0xFFFF2040u

static uint32_t marker(int type) {
	switch (type) {
	case OBJ_WARP_IN: return 0xFF3080FFu;   /* arrival: blue */
	case OBJ_EXIT: case OBJ_RETURN: return 0xFF30E060u;   /* exit: green */
	case OBJ_BOSS: return 0xFFFF3030u;      /* guardian: red */
	case OBJ_SHOP: case OBJ_PROGRAMS: return 0xFFFFD020u;
	case OBJ_HEAL: return 0xFFFF80C0u;
	case OBJ_MYSTERY: return 0xFFFFFFFFu;
	default: return 0xFFC0C0C0u;
	}
}

static void dot(uint32_t *px, int W, int H, int x, int y, int r, uint32_t c) {
	for (int dy = -r; dy <= r; ++dy)
		for (int dx = -r; dx <= r; ++dx) {
			int X = x + dx, Y = y + dy;
			if (X < 0 || Y < 0 || X >= W || Y >= H) continue;
			bool edge = abs(dx) == r || abs(dy) == r;
			px[(size_t)Y * W + X] = edge ? 0xFF000000u : c;
		}
}

static bool save_bmp(const char *path, uint32_t *px, int W, int H) {
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(px, W, H, 32, W * 4, SDL_PIXELFORMAT_ARGB8888);
	if (!s) return false;
	bool ok = SDL_SaveBMP(s, path) == 0;
	SDL_FreeSurface(s);
	return ok;
}

/* One layer: built, drawn, reported. */
static void one(const char *dir, FILE *report, int biome, int layout, int depth, uint32_t seed) {
	run_new(seed);
	run.depth = depth;
	run.biome = biome;
	layout_forced = layout;
	int rise;
	unsigned stairs = netmap_stair_dirs(biome, &rise);
	layer_generate(seed, depth, biome, LAYER_NORMAL, stairs, rise);
	memset(&tiles_stats, 0, sizeof tiles_stats);
	if (!netmap_build_layer(biome, seed)) {
		fprintf(report, "biome %2d layout %d depth %d seed %u: NOT BUILT\n", biome, layout, depth, seed);
		return;
	}
	int tw, th;
	const uint16_t *tiles = netmap_last_tiles(&tw, &th);
	const __typeof__(R.layout->net_area[0]) *a = &R.layout->net_area[biome];
	uint32_t *px = area_src_render(a->group, a->number, tiles, tw, th);
	if (!px) return;
	int W = tw * 8, H = th * 8;
	for (int i = 0; i < W * H; ++i) if (!(px[i] >> 24)) px[i] = VOID_ARGB;
	/* seams, marked in a copy */
	uint32_t *sp = malloc((size_t)W * H * 4);
	memcpy(sp, px, (size_t)W * H * 4);
	const uint8_t *seams = netmap_last_seams();
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx)
			for (int k = 0; k < 8; ++k) {
				if (seams[ty * tw + tx] & 1 && tx + 1 < tw) sp[(size_t)(ty * 8 + k) * W + tx * 8 + 7] = sp[(size_t)(ty * 8 + k) * W + tx * 8 + 8] = SEAM_ARGB;
				if (seams[ty * tw + tx] & 2 && ty + 1 < th) sp[(size_t)(ty * 8 + 7) * W + tx * 8 + k] = sp[(size_t)(ty * 8 + 8) * W + tx * 8 + k] = SEAM_ARGB;
			}
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int X, Y;
		netmap_world((int)o->x, (int)o->y, &X, &Y);
		int z = layer.level[(int)o->y][(int)o->x] ? layer.rise : 0;
		dot(px, W, H, area_px(tw, X, Y), area_py(th, X, Y) - z, o->type == OBJ_MYSTERY ? 2 : 4, marker(o->type));
	}
	char path[600];
	snprintf(path, sizeof path, "%s/b%02d_l%d_d%d_s%u.bmp", dir, biome, layout, depth, seed);
	save_bmp(path, px, W, H);
	snprintf(path, sizeof path, "%s/seams_b%02d_l%d_d%d_s%u.bmp", dir, biome, layout, depth, seed);
	save_bmp(path, sp, W, H);
	free(sp);
	free(px);
	int floor = 0;
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) floor += layer.cell[y][x] == C_PATH;
	int picks = tiles_stats.picks ? tiles_stats.picks : 1;
	fprintf(report, "biome %2d layout %d (%s) depth %d seed %u: %d panels, %d rooms, near %.1f%%, fallback %.2f%%, seams %d, cells changed %d, panels not exact %d, scenery %d, arena %s, stairs %d\n",
		biome, layout, layout_names[layer.layout], depth, seed, floor, layer.nrooms,
		100.0 * tiles_stats.near / picks, 100.0 * tiles_stats.fallbacks / picks, tiles_stats.seams, netmap_legal.edits, netmap_legal.left, netmap_scenery,
		layer.arena >= 0 ? "yes" : layer.boss_layer ? "NO" : "-", layer.nstairs);
}

/* The area's own maps as the game draws them, to hold the layers against. */
static void sources(const char *dir, int biome) {
	const __typeof__(R.layout->net_area[0]) *na = &R.layout->net_area[biome];
	for (int k = -1; k < NET_MORE_MAPS; ++k) {
		int group = k < 0 ? na->group : na->more[k][0], number = k < 0 ? na->number : na->more[k][1];
		if (k >= 0 && !group) break;
		AreaSrc a;
		if (!area_src_load(group, number, &a)) continue;
		int W = a.tw * 8, H = a.th * 8;
		for (int i = 0; i < W * H; ++i) if (!(a.px[i] >> 24)) a.px[i] = VOID_ARGB;
		char path[600];
		snprintf(path, sizeof path, "%s/src_b%02d_%02x_%d.bmp", dir, biome, group, number);
		save_bmp(path, a.px, W, H);
		area_src_free(&a);
	}
}

int atlas_run(const char *spec) {
	/* DIR[:BIOMES[:SEEDS]]: BIOMES "all" or a comma list, SEEDS per layout */
	char dir[512] = ".build/atlas", biomes[256] = "all";
	int seeds = 1;
	sscanf(spec, "%511[^:]:%255[^:]:%d", dir, biomes, &seeds);
	if (!emu_init(R.data, ROM_SIZE)) { fprintf(stderr, "atlas: no core\n"); return 1; }
	bool want[BIOME_COUNT] = { false };
	if (!strcmp(biomes, "all")) for (int b = 0; b < BIOME_COUNT; ++b) want[b] = true;
	else for (char *t = strtok(biomes, ","); t; t = strtok(NULL, ",")) { int b = atoi(t); if (b >= 0 && b < BIOME_COUNT) want[b] = true; }
	char path[600];
	snprintf(path, sizeof path, "%s/report.txt", dir);
	FILE *report = fopen(path, "w");
	if (!report) { fprintf(stderr, "atlas: cannot write %s\n", path); return 1; }
	for (int b = 0; b < BIOME_COUNT; ++b) {
		if (!want[b]) continue;
		sources(dir, b);
		for (int l = 0; l < LAYOUT_COUNT; ++l) {
			if (!layout_weight(b, l)) continue;
			for (int s = 1; s <= seeds; ++s) one(dir, report, b, l, 2, (uint32_t)(s * 7919 + b * 131));
		}
		/* and its guardian's layer, in the layout its act plans */
		for (int s = 1; s <= seeds; ++s) one(dir, report, b, -1, 3, (uint32_t)(s * 104729 + b));
	}
	fclose(report);
	layout_forced = -1;
	printf("atlas written to %s\n", dir);
	return 0;
}
