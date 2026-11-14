/* The atlas (docs/devtools/): every area's layers built and drawn without
 * running the game, for looking over many at once. Each area in each of
 * its layouts, on a normal layer and a guardian's, drawn with the area's
 * own tiles and colours, with its objects marked; a report line per layer
 * says how the tile picks went. */
#include "atlas.h"
#include "layer_sizes.h"
#include "break_places.h"
#include "lent.h"
#include "navstudy.h"
#include "town.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "area_src.h"
#include "breaks.h"
#include "lineups.h"
#include "emu.h"
#include "net.h"
#include "net_layouts.h"
#include "net_signature.h"
#include "netmap.h"
#include "rom.h"
#include "run.h"
#include "tiles.h"
#include "xbackdrop.h"

#define VOID_ARGB 0xFF282830u
#define SEAM_ARGB 0xFFFF2040u
#define BREAK_ARGB 0xFFFFE000u   /* a tile edge across which the picture shows a strip no original map shows */
#define OFF_NEAR_ARGB 0xFFFF40FFu   /* a tile whose pair was seen with other floors where it shows them */
#define OFF_EDGE_ARGB 0xFFFFE040u   /* ... at a few pixels */

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

/* The layer `name`'s picture with its seams marked (`sp`, tw x th tiles),
 * saved: the breaks in the picture (what no original map shows across a
 * tile's edge, breaks.h) marked over them and listed, the nicks listed
 * too, for build.py. */
static void save_marked(const char *dir, const char *name, uint32_t *sp, int tw, int th) {
	const uint8_t *breaks = netmap_last_breaks();
	int W = tw * 8;
	char path[1300];
	snprintf(path, sizeof path, "%s/breaks_%s.txt", dir, name);
	FILE *bf = fopen(path, "w");
	for (int i = 0; breaks && i < tw * th; ++i) {
		int tx = i % tw, ty = i / tw;
		uint8_t b = breaks[i];
		if (!b) continue;
		if (bf) fprintf(bf, "%d %d %d\n", tx * 8, ty * 8, b);
		for (int k = 0; k < 8; ++k) {
			if (b & 1 && tx + 1 < tw) sp[(size_t)(ty * 8 + k) * W + tx * 8 + 7] = sp[(size_t)(ty * 8 + k) * W + tx * 8 + 8] = BREAK_ARGB;
			if (b & 2 && ty + 1 < th) sp[(size_t)(ty * 8 + 7) * W + tx * 8 + k] = sp[(size_t)(ty * 8 + 8) * W + tx * 8 + k] = BREAK_ARGB;
		}
	}
	if (bf) fclose(bf);
	snprintf(path, sizeof path, "%s/seams_%s.bmp", dir, name);
	save_bmp(path, sp, W, th * 8);
}

/* One layer: built, drawn, reported. */
/* The layer's floor as text, cut to where it is. */
static void write_cells(const char *dir, int biome, int layout, int depth, uint32_t seed) {
	static char cells[MAP_H][MAP_W + 1];
	netmap_last_cells(cells);
	int x0 = MAP_W, x1 = -1, y0 = MAP_H, y1 = -1;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (cells[y][x] != '.') {
				if (x < x0) x0 = x;
				if (x > x1) x1 = x;
				if (y < y0) y0 = y;
				if (y > y1) y1 = y;
			}
	char path[600];
	snprintf(path, sizeof path, "%s/cells_b%02d_l%d_d%d_s%u.txt", dir, biome, layout, depth, seed);
	FILE *cf = fopen(path, "w");
	if (cf) {
		for (int y = y0; y <= y1; ++y) fprintf(cf, "%3d %.*s\n", y, x1 - x0 + 1, cells[y] + x0);
		fprintf(cf, "(x from %d)\n", x0);
		fclose(cf);
	}
}

/* (the report's: the side of the screen a layer arrives at, SIDE_*, and
 * BN6's facings, UP first) */
static const char *const side_names[SIDE_COUNT] = { "top", "bottom", "left", "right" };
static const char *const face_names[8] = { "up", "up-right", "right", "down-right", "down", "down-left", "left", "up-left" };

/* Services and navis beside a panel-wide stretch of floor, as the floor
 * was drawn (made drawable after they were placed). */
static int at_mouths(void) {
	int mouths = 0;
	for (int i = 1; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		bool stands = o->type == OBJ_SHOP || o->type == OBJ_HEAL || o->type == OBJ_TRADER || o->type == OBJ_BUGTRADER ||
			o->type == OBJ_NPC || o->type == OBJ_CHALLENGE || o->type == OBJ_PROGRAMS || o->type == OBJ_GIFT;
		if (!stands || o->prop >= 0) continue;
		static const int d4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int nx = (int)o->x + d4[k][0], ny = (int)o->y + d4[k][1];
			if (nx < 1 || ny < 1 || nx >= MAP_W - 1 || ny >= MAP_H - 1 || layer.cell[ny][nx] != C_PATH) continue;
			int ax = d4[k][1], ay = d4[k][0];
			if (layer.cell[ny + ay][nx + ax] != C_PATH && layer.cell[ny - ay][nx - ax] != C_PATH) { ++mouths; break; }
		}
	}
	return mouths;
}

/* The lining up the floor as drawn asks on the walks a player takes: from
 * the arrival to the exit (the arena's antechamber) and to each Net
 * Dealer's talk spot (lineups.h; issue #164). */
static void walk_counts(int *lineups, int *walked, int *junctions) {
	*lineups = *walked = *junctions = 0;
	const Room *goal = &layer.rooms[layer.arena >= 0 ? layer.ante : layer.exit_room];
	int sx = layer.rooms[0].ax, sy = layer.rooms[0].ay, len, jn;
	int n = layer_lineups(sx, sy, goal->ax, goal->ay, &len, &jn);
	if (n > 0) *lineups += n;
	*walked += len;
	*junctions += jn;
	static const int d5[5][2] = { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int i = 1; i < layer.nobj; ++i) {
		if (layer.obj[i].type != OBJ_SHOP) continue;
		int best = -1, best_len = 0, best_jn = 0;
		for (int k = 0; k < 5; ++k) {
			n = layer_lineups(sx, sy, (int)layer.obj[i].x + d5[k][0], (int)layer.obj[i].y + d5[k][1], &len, &jn);
			if (n >= 0 && (best < 0 || len < best_len)) { best = n; best_len = len; best_jn = jn; }
		}
		if (best > 0) *lineups += best;
		*walked += best_len;
		*junctions += best_jn;
	}
}

/* A layer not built, reported so; one built, its backdrop as the game sets
 * it, clear of its tiles: BN6 would draw one over them in the game, which
 * no picture here shows (docs/rom/bn4.md, Net maps in 256 colours). False
 * where it was not built. */
static bool layer_checked(FILE *report, bool built, int area, int layout, int depth, uint32_t seed) {
	if (!built) {
		fprintf(report, "biome %2d layout %d depth %d seed %u: NOT BUILT\n", area, layout, depth, seed);
		return false;
	}
	uint32_t lo, hi;
	xbackdrop_map(area);
	if (netmap_backdrop_over(area, &lo, &hi))
		fprintf(report, "biome %2d layout %d depth %d seed %u: BACKDROP OVER TILES at %08x-%08x\n", area, layout, depth, seed, (unsigned)lo, (unsigned)hi);
	return true;
}

/* A layer of `area` (a BN6 biome's own, or another game's), laid out by
 * `biome`'s rules. */
/* A layer's tiles drawn with its area's own (free it), and their pictures
 * kept for what the layers take from the area's maps (lent.c). */
static uint32_t *layer_drawn(int area, const uint16_t *tiles, int tw, int th) {
	const NetAreaDef *a = net_area_def(area);
	lent_layer(a->xrom, a->group, a->number, tiles, tw, th);
	return area_src_render(a->xrom, a->group, a->number, tiles, tw, th);
}

static void one(const char *dir, FILE *report, int biome, int area, int layout, int depth, uint32_t seed) {
	run_new(seed);
	run.depth = depth;
	run.biome = biome;
	layout_forced = layout;
	LayerKit kit;
	netmap_kit(area, &kit);
	/* timed as the game builds it, then again measured */
	layer_generate(seed, depth, biome, LAYER_NORMAL, &kit);
	tiles_measure = false;
	clock_t t0 = clock();
	netmap_build_layer(area, seed);
	int ms = (int)((clock() - t0) * 1000 / CLOCKS_PER_SEC);
	tiles_measure = true;
	if (getenv("CYBERWORLD_TILE_AT")) fprintf(stderr, "layer b%02d_l%d_d%d_s%u\n", area, layout, depth, seed);
	layer_generate(seed, depth, biome, LAYER_NORMAL, &kit);
	memset(&tiles_stats, 0, sizeof tiles_stats);
	bool built = netmap_build_layer(area, seed);
	if (!layer_checked(report, built, area, layout, depth, seed)) return;
	int tw, th;
	const uint16_t *tiles = netmap_last_tiles(&tw, &th);
	uint32_t *px = layer_drawn(area, tiles, tw, th);
	if (!px) return;
	int W = tw * 8, H = th * 8;
	for (int i = 0; i < W * H; ++i) if (!(px[i] >> 24)) px[i] = VOID_ARGB;
	/* seams, marked in a copy */
	uint32_t *sp = malloc((size_t)W * H * 4);
	if (!sp) { free(px); return; }
	memcpy(sp, px, (size_t)W * H * 4);
	const uint8_t *seams = netmap_last_seams();
	const uint8_t *pasted = netmap_last_pasted();
	/* the tiles whose pairs were seen with other floors where they show
	 * them, framed and listed (build.py cuts close-ups of them), but under
	 * a pad or a stair set whole over them */
	char path[600];
	snprintf(path, sizeof path, "%s/offs_b%02d_l%d_d%d_s%u.txt", dir, area, layout, depth, seed);
	FILE *offs = fopen(path, "w");
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) {
			int off = seams[ty * tw + tx] >> 2 & 3, why = seams[ty * tw + tx] >> 4 & 7;
			if (off < TILE_OFF_EDGE || (pasted && pasted[ty * tw + tx] & (NETMAP_PASTED_PAD | NETMAP_PASTED_STAIR))) continue;
			if (offs) { if (off == TILE_OFF_NEAR) fprintf(offs, "%d %d %d\n", tx * 8, ty * 8, why); }
			uint32_t c = off == TILE_OFF_NEAR ? OFF_NEAR_ARGB : OFF_EDGE_ARGB;
			for (int k = 1; k < 7; ++k) {
				sp[(size_t)(ty * 8 + 1) * W + tx * 8 + k] = sp[(size_t)(ty * 8 + 6) * W + tx * 8 + k] = c;
				sp[(size_t)(ty * 8 + k) * W + tx * 8 + 1] = sp[(size_t)(ty * 8 + k) * W + tx * 8 + 6] = c;
			}
		}
	if (offs) fclose(offs);
	/* the tiles set whole over the classes' picks (pads, emblems, stairs): a
	 * cyan dot in their corner */
	for (int ty = 0; ty < th && pasted; ++ty)
		for (int tx = 0; tx < tw; ++tx)
			if (pasted[ty * tw + tx]) sp[(size_t)(ty * 8) * W + tx * 8] = sp[(size_t)(ty * 8) * W + tx * 8 + 1] = sp[(size_t)(ty * 8 + 1) * W + tx * 8] = 0xFF00FFFFu;
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx)
			for (int k = 0; k < 8; ++k) {
				if (pasted && pasted[ty * tw + tx] & (NETMAP_PASTED_PAD | NETMAP_PASTED_STAIR)) continue;
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
	snprintf(path, sizeof path, "%s/b%02d_l%d_d%d_s%u.bmp", dir, area, layout, depth, seed);
	save_bmp(path, px, W, H);
	snprintf(path, sizeof path, "b%02d_l%d_d%d_s%u", area, layout, depth, seed);
	save_marked(dir, path, sp, tw, th);
	write_cells(dir, area, layout, depth, seed);
	free(sp);
	free(px);
	int floor = 0;
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) floor += layer.cell[y][x] == C_PATH;
	int mouths = at_mouths();
	int picks = tiles_stats.picks ? tiles_stats.picks : 1, lineups, walked, junctions;
	walk_counts(&lineups, &walked, &junctions);
	fprintf(report, "biome %2d layout %d (%s) depth %d seed %u: %d panels, %d rooms, near %.1f%%, whole %.1f%%, fallback %.2f%%, seams %d, breaks %d, nicks %d, off near %d, off edge %d, cells changed %d, panels not exact %d, other colours %d, scenery %d, arena %s, stairs %d, built in %d ms, at mouths %d, signature %s, arrives %s facing %s, lineups %d and junctions %d over %d panels\n",
		area, layout, layout_names[layer.layout], depth, seed, floor, layer.nrooms,
		100.0 * tiles_stats.near / picks, 100.0 * tiles_stats.whole / picks, 100.0 * tiles_stats.fallbacks / picks, tiles_stats.seams, tiles_stats.breaks, tiles_stats.nicks, tiles_stats.off_near, tiles_stats.off_edge,
		netmap_legal.edits, netmap_legal.left, tiles_stats.other, netmap_scenery,
		layer.arena >= 0 ? "yes" : layer.boss_layer ? "NO" : "-", layer.nstairs, ms, mouths, sig_names[layer.sig], side_names[layer.arrive_side & 3], face_names[layer.arrive_face & 7],
		lineups, junctions, walked);
}

/* A view of an area's map: its panels as the tiles learn them (as text,
 * read before the void is filled in: the panels' test reads its pixels),
 * then its picture. */
static void source_view(const char *dir, const NetAreaDef *na, int biome, int group, int number, AreaSrc *v) {
	char path[600], tag[16] = "";
	if (v->level) snprintf(tag, sizeof tag, "_z%d", v->level);
	snprintf(path, sizeof path, "%s/src_b%02d_%02x_%d%s.txt", dir, biome, group, number, tag);
	FILE *f = fopen(path, "w");
	if (f) {
		tiles_src_text(v, na->styles, na->walk_styles, na->skip_styles, na->joint_hues, na->bg_in_map, f);
		fclose(f);
	}
	/* (... and as its roads are learned, netmap_learn.c) */
	snprintf(path, sizeof path, "%s/roads_b%02d_%02x_%d%s.txt", dir, biome, group, number, tag);
	unsigned roads = netmap_road_hues(biome);
	f = roads ? fopen(path, "w") : NULL;
	if (f) {
		tiles_src_text(v, na->styles & ~(0x1FFFu | TILES_PADS_ALONE), (uint16_t)roads, na->walk_styles & 0x1FFFu, 0, na->bg_in_map, f);
		fclose(f);
	}
	int W = v->tw * 8, H = v->th * 8;
	uint32_t *pic = malloc((size_t)W * H * sizeof *pic);
	if (!pic) return;
	for (int i = 0; i < W * H; ++i) pic[i] = v->idx[i] ? area_src_px(v, (size_t)i) : VOID_ARGB;
	snprintf(path, sizeof path, "%s/src_b%02d_%02x_%d%s.bmp", dir, biome, group, number, tag);
	save_bmp(path, pic, W, H);
	/* (and its first layer alone: what the tiles are learned from where the
	 * second sets something over the floor) */
	for (int i = 0; i < W * H; ++i) pic[i] = v->idx0[i] ? area_src_px0(v, (size_t)i) : VOID_ARGB;
	snprintf(path, sizeof path, "%s/src1_b%02d_%02x_%d%s.bmp", dir, biome, group, number, tag);
	save_bmp(path, pic, W, H);
	free(pic);
}

/* The area's own maps as the game draws them, to hold the layers against,
 * each floor height in a view of its own, as the tiles learn them. */
static void sources(const char *dir, int biome) {
	const NetAreaDef *na = net_area_def(biome);
	for (int k = -1; na && k < NET_MORE_MAPS; ++k) {
		int group = k < 0 ? na->group : na->more[k][0], number = k < 0 ? na->number : na->more[k][1];
		if (k >= 0 && !group) break;
		AreaSrc a;
		if (!area_src_load_as(na->xrom, group, number, na->recolour, &a)) continue;
		int count[256] = { 0 };
		for (int i = 0; a.hz && i < a.hw * a.hh; ++i) count[a.hz[i]]++;
		for (int z = 8; z < HEIGHT_UNEVEN; z += 8) {
			AreaSrc r;
			if (count[z] < NETMAP_LEVEL_MIN_CELLS || !area_src_raise(&a, z, &r)) continue;
			source_view(dir, na, biome, group, number, &r);
			area_src_free(&r);
		}
		source_view(dir, na, biome, group, number, &a);
		area_src_free(&a);
	}
}

/* Every map of a group as the game draws them (up to four missing in a
 * row), for choosing the maps an area learns from. */
static void group_sources(const char *dir, int group) {
	for (int n = 0, miss = 0; n < 64 && miss < 4; ++n) {
		AreaSrc a;
		if (!area_src_load(group, n, &a)) { ++miss; continue; }
		miss = 0;
		int W = a.tw * 8, H = a.th * 8;
		uint32_t *px = area_src_argb(&a, VOID_ARGB);
		char path[600];
		snprintf(path, sizeof path, "%s/src_g%02x_%d.bmp", dir, group, n);
		if (px) save_bmp(path, px, W, H);
		free(px);
		area_src_free(&a);
	}
}

/* The real world's maps (groups 0x00-0x06) as the game draws them, and
 * again with their coordinate data marked: walls red, raised floor blue,
 * triggers yellow. */
static void world_sources(const char *dir) {
	static const int counts[7] = { 2, 5, 11, 5, 5, 4, 5 };
	for (int g = 0; g < 7; ++g)
		for (int n = 0; n < counts[g]; ++n) {
			AreaSrc a;
			if (!area_src_load(g, n, &a)) { printf("world %02x:%d: no map\n", g, n); continue; }
			int W = a.tw * 8, H = a.th * 8;
			uint32_t *px = area_src_argb(&a, VOID_ARGB);
			if (!px) { area_src_free(&a); continue; }
			char path[600];
			snprintf(path, sizeof path, "%s/world_%02x_%d.bmp", dir, g, n);
			save_bmp(path, px, W, H);
			static const uint32_t mark[4] = { 0xFFFF3030u, 0xFF3070FFu, 0xFF30FF30u, 0xFFFFE020u };
			for (int s = 0; s < 4; ++s)
				for (int i = 0; i < a.nsec[s]; ++i) {
					const CoordCell *c = &a.sec[s][i];
					int x = area_px(a.tw, c->x + 4, c->y + 4), y = area_py(a.th, c->x + 4, c->y + 4) - (c->z > 0 ? c->z : 0);
					if (s == 2) continue;
					dot(px, W, H, x, y, s == 3 ? 2 : 1, mark[s]);
				}
			snprintf(path, sizeof path, "%s/world_%02x_%d_coords.bmp", dir, g, n);
			save_bmp(path, px, W, H);
			free(px);
			printf("world %02x:%d: %dx%d tiles, %d layers, walls %d, heights %d, priority %d, triggers %d\n", g, n, a.tw, a.th,
				a.layers, a.nsec[0], a.nsec[1], a.nsec[2], a.nsec[3]);
			area_src_free(&a);
		}
}

/* Every map of an extra game (docs/multirom/) as it draws them, its real
 * world's groups and its internet's, read from beside the BN6 ROM: what
 * its areas could lend a run. */
static void xrom_sources(const char *dir, int xrom) {
	xrom_research = true;
	if (xrom >= 0 && xrom < XROM_COUNT) xrom_find_beside();
	if (xrom < 0 || xrom >= XROM_COUNT || !XR[xrom].data) { printf("extra ROM %d: not found beside %s\n", xrom, R.path); return; }
	const XRomLayout *x = XR[xrom].layout;
	printf("%s\n", x->name);
	for (int k = 0; k < x->rw_groups + x->net_groups; ++k) {
		int g = k < x->rw_groups ? k : 0x80 + k - x->rw_groups;
		for (int n = 0, miss = 0; n < 64 && miss < 4; ++n) {
			AreaSrc a;
			if (!area_src_load_x(xrom, g, n, &a)) { ++miss; continue; }
			miss = 0;
			int W = a.tw * 8, H = a.th * 8;
			uint32_t *px = area_src_argb(&a, VOID_ARGB);
			char path[600];
			snprintf(path, sizeof path, "%s/x%d_%02x_%d.bmp", dir, xrom, g, n);
			if (px) save_bmp(path, px, W, H);
			free(px);
			printf("x%d %02x:%d: %dx%d tiles, %d layers, walls %d, heights %d, priority %d, triggers %d\n", xrom, g, n, a.tw, a.th,
				a.layers, a.nsec[0], a.nsec[1], a.nsec[2], a.nsec[3]);
			area_src_free(&a);
		}
	}
}

static void stair_layers(const char *dir, FILE *report, int biome, int area, int seeds);

/* (dev) A floor drawn by hand in `area`'s tiles, as it stands (no cell
 * changed): the rows of the file CYBERWORLD_FLOOR names, '.' void, '#' floor,
 * 'P' a pad's floor; its picture hand_bNN.bmp, its breaks marked in
 * seams_hand_bNN.bmp. What shapes an area's art draws whole. */
static void hand_floor(const char *dir, int area) {
	static uint8_t cell[MAP_H][MAP_W], pad[MAP_H][MAP_W];
	if (area >= NET_AREAS) { xarea_review(area); xrom_find_beside(); }
	if (!net_area_def(area)) { printf("atlas: no area %d\n", area); return; }
	const char *file = getenv("CYBERWORLD_FLOOR");
	FILE *f = file ? fopen(file, "r") : NULL;
	if (!f) { printf("atlas: no CYBERWORLD_FLOOR to read\n"); return; }
	char row[256];
	for (int y = 4; y < MAP_H - 4 && fgets(row, sizeof row, f); ++y)
		for (int x = 0; row[x] && row[x] != '\n' && x + 4 < MAP_W - 4; ++x) {
			cell[y][x + 4] = row[x] != '.' && row[x] != ' ';
			pad[y][x + 4] = row[x] == 'P';
		}
	fclose(f);
	run_new(1);
	memset(&layer, 0, sizeof layer);
	layer.arena = layer.sig_room = layer.exit_room = -1;
	NetLayout lay = { MAP_W, MAP_H, &cell[0][0], NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 1, &pad[0][0], NULL, 0, NULL, 0, NULL };
	memset(&tiles_stats, 0, sizeof tiles_stats);
	if (!netmap_build(area, &lay)) { printf("atlas: the floor was not built\n"); return; }
	int tw, th;
	const uint16_t *tiles = netmap_last_tiles(&tw, &th);
	const NetAreaDef *a = net_area_def(area);
	uint32_t *px = area_src_render(a->xrom, a->group, a->number, tiles, tw, th);
	if (!px) return;
	for (int i = 0; i < tw * th * 64; ++i) if (!(px[i] >> 24)) px[i] = VOID_ARGB;
	char path[600];
	snprintf(path, sizeof path, "%s/hand_b%02d.bmp", dir, area);
	save_bmp(path, px, tw * 8, th * 8);
	snprintf(path, sizeof path, "hand_b%02d", area);
	save_marked(dir, path, px, tw, th);
	free(px);
	printf("hand floor in area %d: breaks %d, nicks %d, seams %d, whole %d of %d\n", area, tiles_stats.breaks, tiles_stats.nicks, tiles_stats.seams, tiles_stats.whole, tiles_stats.picks);
}

/* An area's layers, a BN6 biome's or another game's (laid out by the rules
 * of the BN6 area it is like, docs/multirom/): every layout at depth 2,
 * its guardian's layer in the layout its act plans and layers that climb a
 * stair where it has one, its original maps beside them. */
/* A layer drawn (one), and where its breaks lie: a line in DIR/places.txt
 * (begun anew by lent_fresh), its breaks and its panels by place
 * (break_places.c). */
static void one_placed(const char *dir, FILE *report, int biome, int area, int layout, int depth, uint32_t seed) {
	one(dir, report, biome, area, layout, depth, seed);
	char path[600];
	snprintf(path, sizeof path, "%s/places.txt", dir);
	FILE *f = fopen(path, "a");
	if (!f) return;
	BreakPlaces p;
	break_places(&p);
	fprintf(f, "places %2d layout %d depth %d seed %u:", area, layout, depth, seed);
	for (int k = 0; k < BP_PLACES; ++k) fprintf(f, " %s %d/%d,", break_place_names[k], p.breaks[k], p.cells[k]);
	fprintf(f, "\n");
	fclose(f);
}

/* What the area's layers built since lent_begin took from each of its
 * maps: a line each in DIR/lent.txt (begun anew by lent_fresh), and the map
 * with the tiles no layer used tinted (lent.c). */
static void lent_fresh(const char *dir) {
	char path[600];
	snprintf(path, sizeof path, "%s/lent.txt", dir);
	remove(path);
	snprintf(path, sizeof path, "%s/places.txt", dir);
	remove(path);
}

static void lent_maps(const char *dir, int area) {
	const NetAreaDef *na = net_area_def(area);
	char list[600];
	snprintf(list, sizeof list, "%s/lent.txt", dir);
	FILE *report = fopen(list, "a");
	if (!report) return;
	for (int k = -1; na && k < NET_MORE_MAPS; ++k) {
		int group = k < 0 ? na->group : na->more[k][0], number = k < 0 ? na->number : na->more[k][1];
		if (k >= 0 && !group) break;
		AreaSrc a;
		Lent n;
		if (!area_src_load_as(na->xrom, group, number, na->recolour, &a)) continue;
		uint8_t *panels = tiles_src_panels(&a, na->styles, na->walk_styles, na->skip_styles, na->joint_hues, na->bg_in_map);
		uint32_t *px = lent_map(&a, panels, &n);
		free(panels);
		char path[600];
		snprintf(path, sizeof path, "%s/lent_b%02d_%02x_%d.bmp", dir, area, group, number);
		for (int i = 0; px && i < a.tw * a.th * 64; ++i) if (!(px[i] >> 24)) px[i] = VOID_ARGB;
		if (px) save_bmp(path, px, a.tw * 8, a.th * 8);
		fprintf(report, "lent %2d map %02x:%d: %d tiles drawn, %d as a layer draws one (%.1f%%); %d different, %d of them used (%.1f%%); left on its floor %d, on its pads %d, on other floor %d, on raised floor %d, on ramps %d, beside its floor %d, beside other floor %d, away %d\n",
			area, group, number, n.drawn, n.used, 100.0 * n.used / (n.drawn ? n.drawn : 1), n.kinds, n.kinds_used, 100.0 * n.kinds_used / (n.kinds ? n.kinds : 1),
			n.left[LENT_OWN], n.left[LENT_PAD], n.left[LENT_OTHER], n.left[LENT_RAISED], n.left[LENT_RAMP], n.left[LENT_BY_OWN], n.left[LENT_BY_OTHER], n.left[LENT_AWAY]);
		free(px);
		area_src_free(&a);
	}
	fclose(report);
}

static void area_layers(const char *dir, FILE *report, int area, int seeds) {
	int b = area < NET_AREAS ? area : net_area_def(area)->like;
	sources(dir, area);
	lent_begin();
	for (int l = 0; l < LAYOUT_COUNT; ++l) {
		if (!layout_weight(b, l)) continue;
		for (int s = 1; s <= seeds; ++s) one_placed(dir, report, b, area, l, 2, (uint32_t)(s * 7919 + area * 131));
	}
	for (int s = 1; s <= seeds; ++s) one_placed(dir, report, b, area, -1, 3, (uint32_t)(s * 104729 + area));
	stair_layers(dir, report, b, area, seeds);
	lent_maps(dir, area);
}

/* Another game's net area N alone. */
static void xrom_area(const char *dir, int n, int seeds) {
	int area = NET_AREAS + n;
	xarea_review(area);
	xrom_find_beside();
	if (!net_area_def(area)) { printf("area %d: its game's ROM is not beside %s\n", area, R.path); return; }
	char path[600];
	snprintf(path, sizeof path, "%s/report.txt", dir);
	FILE *report = fopen(path, "w");
	if (!report) return;
	lent_fresh(dir);
	area_layers(dir, report, area, seeds);
	fclose(report);
}

typedef struct { uint32_t *px; int W, H, tw, th; } TownDots;
/* trees green, other objects blue, people yellow, at their feet */
static void town_dot(int id, int x, int y, void *ctx) {
	TownDots *d = ctx;
	dot(d->px, d->W, d->H, area_px(d->tw, x, y), area_py(d->th, x, y), 2, id < 0 ? 0xFFFFE020u : id == 0x7D || id == 0x7E ? 0xFF20C020u : 0xFF3060FFu);
}

/* The town's ports (docs/world/home/): how many cells each, and how many Lan
 * reaches on foot from the landmark's */
static void port_feet(char *out, size_t size) {
	const CoordCell *trig;
	int nt = town_triggers(&trig), cells[TOWN_PORTS] = { 0 }, feet[TOWN_PORTS] = { 0 };
	for (int i = 0; i < nt; ++i) {
		int x = trig[i].x + 4, y = trig[i].y + 4, k = town_port_at(x, y), wx, wy;
		if ((trig[i].value & 0x7F) != 0x40 || k < 0) continue;
		++cells[k];
		feet[k] += town_walk(x, y, 0, &wx, &wy, NULL);
	}
	snprintf(out, size, "ports %d/%d, %d/%d, %d/%d on foot", feet[0], cells[0], feet[1], cells[1], feet[2], cells[2]);
}

/* The towns (src/world/town.c) runs of a few seeds start in, drawn with
 * their original's tiles; tiles no source tile matched are marked red, the
 * town's objects and people dotted. */
static void towns(const char *dir, int seeds) {
	for (int s = 1; s <= seeds; ++s) {
		/* (the town a run of seed s starts in) */
		if (!town_plan(town_seed((uint32_t)s))) { printf("town seed %d: not planned\n", s); continue; }
		int W, H;
		uint32_t *px = town_render(&W, &H);
		if (!px) continue;
		const TownInfo *ti = town_info();
		const uint8_t *miss = town_misses();
		for (int i = 0; i < W * H; ++i) if (!(px[i] >> 24)) px[i] = 0xFF5AFFEFu;
		for (int ty = 0; ty < ti->th; ++ty)
			for (int tx = 0; tx < ti->tw; ++tx)
				if (miss[ty * ti->tw + tx] & 1)
					for (int k = 0; k < 8; ++k) { px[(ty * 8) * W + tx * 8 + k] = 0xFFFF0000u; px[(ty * 8 + k) * W + tx * 8] = 0xFFFF0000u; }
				else if (getenv("CYBERWORLD_TOWN_DEBUG") && (miss[ty * ti->tw + tx] & 6))
					/* blue: the mirror's tile; green: the hinted one */
					for (int k = 0; k < 8; ++k) px[(ty * 8 + k) * W + tx * 8 + k] = miss[ty * ti->tw + tx] & 2 ? 0xFF0000FFu : 0xFF00A000u;
		int sx = area_px(ti->tw, ti->start_x, ti->start_y), sy = area_py(ti->th, ti->start_x, ti->start_y);
		dot(px, W, H, sx, sy, 3, 0xFF3080FFu);
		dot(px, W, H, area_px(ti->tw, ti->port_x, ti->port_y), area_py(ti->th, ti->port_x, ti->port_y), 3, 0xFF30FF30u);
		TownDots dots = { px, W, H, ti->tw, ti->th };
		town_objects(town_dot, &dots);
		/* the jack-in cells green, the checks yellow */
		const CoordCell *trig;
		int nt = town_triggers(&trig);
		for (int i = 0; i < nt && getenv("CYBERWORLD_TOWN_DEBUG"); ++i)
			dot(px, W, H, area_px(ti->tw, trig[i].x + 4, trig[i].y + 4), area_py(ti->th, trig[i].x + 4, trig[i].y + 4), 1,
				trig[i].value == 0x40 ? 0xFF00FF00u : 0xFFFFE000u);
		char path[600];
		snprintf(path, sizeof path, "%s/town_s%02d.bmp", dir, s);
		save_bmp(path, px, W, H);
		free(px);
		char feet[64];
		port_feet(feet, sizeof feet);
		printf("town seed %d: %dx%d tiles, %d picks, %d misses, %s\n", s, ti->tw, ti->th, ti->picks, ti->misses, feet);
	}
}

/* Layers of `area` that climb a stair, where it has one (`seeds` of them),
 * laid out by biome `b`'s rules: none of the layers above happened to raise
 * a room, and players saw the Undernet's ramps broken. */
static void stair_layers(const char *dir, FILE *report, int b, int area, int seeds) {
	LayerKit kit;
	netmap_kit(area, &kit);
	for (int s = 1, found = 0; kit.stair_dirs && s <= 300 && found < seeds; ++s) {
		uint32_t seed = (uint32_t)(s * 7919 + area * 131 + 17);
		run_new(seed);
		run.depth = 5;
		run.biome = b;
		layout_forced = -1;
		layer_generate(seed, 5, b, LAYER_NORMAL, &kit);
		if (!layer.nstairs) continue;
		one(dir, report, b, area, -1, 5, seed);
		++found;
	}
}

static int env_int(const char *name, int none) {
	const char *v = getenv(name);
	return v ? atoi(v) : none;
}

int atlas_run(const char *spec) {
	/* ("nav": the way across BN6's own maps instead, navstudy.c; "arrivals"
	 * where MegaMan arrives on them) */
	if (!strcmp(spec, "nav")) return navstudy_run();
	if (!strcmp(spec, "arrivals")) return navstudy_arrivals();
	/* DIR[:BIOMES[:SEEDS]]: BIOMES "all" or a comma list, SEEDS per layout */
	char dir[512] = ".build/atlas", biomes[256] = "all";
	int seeds = 1;
	sscanf(spec, "%511[^:]:%255[^:]:%d", dir, biomes, &seeds);
	if (!emu_init(R.data, ROM_SIZE)) { fprintf(stderr, "atlas: no core\n"); return 1; }
	tiles_measure = true;
	breaks_measure = true;
	/* (CYBERWORLD_LEAD=N: every layer led by its area's map N, to measure a
	 * map as the lead: docs/world/level-design/lead-map.md) */
	netmap_lead_forced = env_int("CYBERWORLD_LEAD", -1);
	netmap_whole_forced = env_int("CYBERWORLD_WHOLE", 0);
	netmap_rules_forced = (unsigned)env_int("CYBERWORLD_RULES", 0);
	if (!strcmp(biomes, "world")) { world_sources(dir); return 0; }
	if (!strcmp(biomes, "town")) { towns(dir, seeds); return 0; }
	if (!strncmp(biomes, "sizes", 5)) return layer_sizes_run(dir, biomes + 5, seeds);
	if (biomes[0] == 'g') { group_sources(dir, (int)strtol(biomes + 1, NULL, 16)); return 0; }
	if (biomes[0] == 'x') { xrom_sources(dir, atoi(biomes + 1)); return 0; }
	if (biomes[0] == 'a' && biomes[1] >= '0' && biomes[1] <= '9') { xrom_area(dir, atoi(biomes + 1), seeds); return 0; }
	if (biomes[0] == 'h') { hand_floor(dir, atoi(biomes + 1)); return 0; }
	bool want[BIOME_COUNT] = { false };
	if (!strcmp(biomes, "all")) for (int b = 0; b < BIOME_COUNT; ++b) want[b] = true;
	else for (char *t = strtok(biomes, ","); t; t = strtok(NULL, ",")) { int b = atoi(t); if (b >= 0 && b < BIOME_COUNT) want[b] = true; }
	char path[600];
	snprintf(path, sizeof path, "%s/report.txt", dir);
	FILE *report = fopen(path, "w");
	if (!report) { fprintf(stderr, "atlas: cannot write %s\n", path); return 1; }
	lent_fresh(dir);
	for (int b = 0; b < BIOME_COUNT; ++b)
		if (want[b]) area_layers(dir, report, b, seeds);
	/* (and every area another game beside BN6's lends a run, numbered after
	 * BN6's, held ones and those of a game read for research too:
	 * docs/multirom/) */
	if (!strcmp(biomes, "all")) { xarea_review(XAREA_ALL); xrom_find_beside(); }
	for (int k = 0; k < XAREAS_MAX && !strcmp(biomes, "all"); ++k)
		if (net_area_def(NET_AREAS + k)) area_layers(dir, report, NET_AREAS + k, seeds);
	fclose(report);
	layout_forced = -1;
	printf("atlas written to %s\n", dir);
	return 0;
}
