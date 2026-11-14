/* Generated layers as game maps.
 *
 * Tiles: each tile takes a pair of layer entries the area's original map
 * uses at the same place in a panel with the same or the nearest floor
 * around it, checked pixel by pixel (tiles.c).
 *
 * Walls: the game's collision is a list of wall cells (8x8 world units) keyed
 * by row * 254 + column (both offset by 127), each pointing to a 4-byte shape
 * (lowest z, flag, height, type). Panel edges run through cell centres; a
 * cell is floor when its centre is inside floor panels, and the cells around
 * the floor get the wall types the original maps use: 1 NE, 2 SW, 3 SE, 4 NW
 * edges and 5 E, 6 S, 7 N, 8 W outer corners.
 *
 * Here the panels' kinds, the floor's tiles and the map written and built;
 * an area's maps learned is netmap_learn.c's, their scenery pasted
 * netmap_paste.c's, the set pieces' cells netmap_extra.c's. */
#include "netmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bn6_map.h"
#include "bytes.h"
#include "debug.h"
#include "emu.h"
#include "lz.h"
#include "net_shapes.h"
#include "netmap_extra.h"
#include "netmap_learn.h"
#include "netmap_paste.h"
#include "tilemap.h"
#include "xbackdrop.h"

/* Generated layers as game maps.
 *
 * Tiles: each tile takes a pair of layer entries the area's original map
 * uses at the same place in a panel with the same or the nearest floor
 * around it, checked pixel by pixel (tiles.c).
 *
 * Walls: the game's collision is a list of wall cells (8x8 world units) keyed
 * by row * 254 + column (both offset by 127), each pointing to a 4-byte shape
 * (lowest z, flag, height, type). Panel edges run through cell centres; a
 * cell is floor when its centre is inside floor panels, and the cells around
 * the floor get the wall types the original maps use: 1 NE, 2 SW, 3 SE, 4 NW
 * edges and 5 E, 6 S, 7 N, 8 W outer corners. */

#define TILEMAP_AT  FREE_TILE_MAP        /* generated tile map (LZ77) */
#define XGFX_AT     FREE_XAREA              /* another game's area: its tile set and palette (docs/engine/emulation/) */
#define XGFX_SIZE   (FREE_XAREA_END - FREE_XAREA)   /* (docs/engine/emulation/free-space.md, The measured pass) */

/* (the bytes the last layer's tile map took: the layers' sizes) */
static size_t tilemap_bytes;
size_t netmap_tilemap_bytes(void) { return tilemap_bytes; }

/* the centrepieces' tiles (the four surface areas share their tile set) */
const int nm_ornament_tile[3] = { 0x379, 0x372, 0x375 };

static Learned learned[NET_AREAS + XAREAS_MAX];

/* The areas whose learned maps are kept: all of them on a computer, the
 * layer's own alone on the 3DS. Each area keeps 2 to 7 MB once learned,
 * and in a long run the areas kept, with a new one's learning, filled the
 * 3DS's 96 MB heap: allocations refused in most long runs; with two kept,
 * still in most where BN5's ROM was read (docs/engine/emulation/3ds-memory.md, The 3DS's
 * memory; CW_HEAP_3DS builds the host so, for tools/heap3ds.py, and
 * AREAS_KEPT sets another number there). An area left behind is learned
 * again, the same, where a run comes back to it: the endless net's acts
 * once round, a detour's way back. */
#ifndef AREAS_KEPT
#if defined(__3DS__) || defined(CW_HEAP_3DS)
#define AREAS_KEPT 1
#else
#define AREAS_KEPT 0   /* (all) */
#endif
#endif
static unsigned area_used[NET_AREAS + XAREAS_MAX], area_clock;
static bool area_forgotten[NET_AREAS + XAREAS_MAX];

/* What `L` learned, freed: the area is learned afresh where it is asked
 * for again */
static void forget(Learned *L) {
	for (int k = 0; k < MAX_BOOKS; ++k) tiles_free(&L->book[k]);
	for (int k = 0; k < 2; ++k) tiles_free(&L->road[k]);
	seams_free(&L->seams);
	breaks_add(&L->breaks, NULL);
	decor_free(&L->decor);
	for (int k = 0; k < L->sides.n; ++k) { free(L->sides.piece[k].tiles); free(L->sides.piece[k].prio); }
	stairs_free(L->stairs);
	PropStamp *stamps[] = { &L->counter[0], &L->counter[1], &L->ornament[0], &L->ornament[1], &L->ornament[2], &L->bush[0],
		&L->bush[1], &L->emblem, &L->pad, &L->arrow[0], &L->arrow[1], &L->arrow[2], &L->arrow[3] };
	for (size_t i = 0; i < sizeof stamps / sizeof *stamps; ++i) props_free(stamps[i]);
	memset(L, 0, sizeof *L);
}

/* (before area `area` is learned: the least lately used others forgotten
 * past AREAS_KEPT) */
static void forget_areas(int area) {
	while (AREAS_KEPT) {
		int oldest = -1, held = 0;
		for (int a = 0; a < NET_AREAS + XAREAS_MAX; ++a) {
			if (a == area || !learned[a].tried) continue;
			++held;
			if (oldest < 0 || area_used[a] < area_used[oldest]) oldest = a;
		}
		if (held < AREAS_KEPT || oldest < 0) return;
		printf("netmap: area %d's learned maps forgotten for area %d's (%d kept)\n", oldest, area, AREAS_KEPT);
		forget(&learned[oldest]);
		area_forgotten[oldest] = true;
	}
}

/* Area `area`'s maps learned, once while it is kept */
static Learned *learn_area(int area) {
	Learned *L = &learned[area];
	area_used[area] = ++area_clock;
	if (!L->tried) {
		forget_areas(area);
		L->tried = true;
		L->ok = nm_learn(area, L);
		if (area_forgotten[area]) printf("netmap: area %d learned again\n", area);
	}
	return L;
}

unsigned netmap_road_hues(int area) { return net_area_def(area) && learn_area(area)->ok ? learn_area(area)->road_hues : 0; }

int netmap_scenery;
LegalStats netmap_legal;
LastMap nm_last;

MapPlace nm_place;

/* ---- layout <-> world ---- */

/* grid x (down-right on screen) is world +Y, grid y (down-left) world -X */
void nm_grid_to_panel(int x, int y, int *A, int *B) { *A = -(y - nm_place.gy0); *B = x - nm_place.gx0; }

void netmap_world(int x, int y, int *wx, int *wy) {
	int A, B;
	nm_grid_to_panel(x, y, &A, &B);
	*wx = nm_place.ex + 16 + 32 * A;
	*wy = nm_place.ey + 16 + 32 * B;
}

void netmap_grid(int wx, int wy, double *x, double *y) {
	*x = (wy - nm_place.ey - 16) / 32.0 + nm_place.gx0;
	*y = -(wx - nm_place.ex - 16) / 32.0 + nm_place.gy0;
}

bool netmap_panel(int wx, int wy, int *x, int *y) {
	int A = nm_floordiv(wx - nm_place.ex, 32), B = nm_floordiv(wy - nm_place.ey, 32);
	*x = B + nm_place.gx0;
	*y = -A + nm_place.gy0;
	return true;
}

const NetLayout *nm_cur;
static bool one_floor;        /* the area has no walkway floor: all is platform */
static bool by_shape;         /* its floors are told by shape: an arena is platform */
static bool arena_b;          /* the guardian's arena in the walkways' floor, to read apart (TILES_ARENA_FLOOR: not) */
static bool rimmed;           /* its platforms' edges are rims (TILES_RIMMED) */
static int apart;             /* what of its floor stands apart from the rest (NET_APART_*) */
static bool pad_look;         /* its originals' pads have a look for the layer's (RomLayout.net_area) */
static bool pads_walkway;     /* its framed pads are cut from its maps in their walkways' floor */
static bool arena_drawn;      /* its arena drawn in its platforms' floor, laid out as walkway (TILES_ARENA_DRAWN) */
static bool joints;           /* its walkways' ends, turns and crossings are joints (NetAreaDef.joint_hues) */
static bool roads;            /* its maps have a second look of walkway, which the way's walkways take (netmap_learn.c, road_hues) */
static bool picking;          /* the tiles are picked: the floor as they draw it, not as the legalizer reads it */
static uint32_t coord_slot;   /* the layer map's coordinate-data pointer */

/* What the layer has at grid cell (x, y). */
int nm_kind(int x, int y) {
	if (x < 0 || y < 0 || x >= nm_cur->gw || y >= nm_cur->gh || !nm_cur->cell[y * nm_cur->gw + x]) return K_VOID;
	for (int i = 0; i < nm_cur->nstairs; ++i)
		if (x >= nm_cur->stairs[i].x && x < nm_cur->stairs[i].x + 2 && y >= nm_cur->stairs[i].y && y < nm_cur->stairs[i].y + 2) return K_STAIR;
	if (nm_cur->cell[y * nm_cur->gw + x] == C_SOLID) return K_SOLID;
	return nm_cur->level && nm_cur->level[y * nm_cur->gw + x] ? K_RAISED : K_FLOOR;
}

/* The ground floor as drawn: walled-off panels look like the rest. */
static bool ground(int k) { return k == K_FLOOR || k == K_SOLID; }

int nm_kind_at(int A, int B) { return nm_kind(B + nm_place.gx0, -A + nm_place.gy0); }

/* A walkway: a floor cell in no 2x2 block of floor, as the original areas
 * draw their 1-wide paths in their second floor. */
static bool walkway(int x, int y) {
	for (int dy = -1; dy <= 0; ++dy)
		for (int dx = -1; dx <= 0; ++dx)
			if (nm_kind(x + dx, y + dy) && nm_kind(x + dx + 1, y + dy) && nm_kind(x + dx, y + dy + 1) && nm_kind(x + dx + 1, y + dy + 1)) return false;
	return true;
}

/* The floor's material as the screen shows it: ground panels where they
 * are, raised ones where a flat panel `rise` world units up the screen
 * would be (stairs are drawn from their own pieces). */
/* A floor cell beside the void (8 ways): a platform's rim. */
static bool edge(int x, int y) {
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
			if (nm_kind(x + dx, y + dy) == K_VOID) return true;
	return false;
}

/* Whether platform cell (x, y) is a middle or lies beside one (8 ways), where
 * the floors are told by shape: a room's panel. Floor two wide has no
 * middle and is all rim, the walkways' floor: BN4's countries' ring road,
 * labelled a piece apart with the square it passes, was picked as one
 * floor with it, and where no map sets the two so the square's rim took
 * the road's plain tiles, squares bitten out of it. */
static bool has_middle(int x, int y) {
	if (!by_shape) return true;
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
			if (nm_kind(x + dx, y + dy) && !walkway(x + dx, y + dy) && !edge(x + dx, y + dy)) return true;
	return false;
}

/* Where the walkways' floor runs on across the platforms they meet
 * (TILES_CROSSING), as the comps' and homepages' maps cross their fields
 * with stripes of it: a walkway entering a platform square on, in the
 * middle of a side, goes on straight in its own floor as far as floor lies
 * on both sides of it, out the other side only into a walkway: their
 * stripes end a panel inside a field's edge or leave it as a walkway. A
 * walkway stopped at a field's edge is a join of the two floors their maps
 * never draw (green cut square over orange), and so is a stripe ending on
 * the edge. */
static bool striped;
static uint8_t stripe[MAP_H][MAP_W];

static bool floor_c(int x, int y) { return x >= 0 && y >= 0 && x < nm_cur->gw && y < nm_cur->gh && nm_cur->cell[y * nm_cur->gw + x] == C_PATH; }

/* platform floor: in a 2 x 2 block of floor (a walkway is in none) */
static bool platform_c(int x, int y) {
	if (!floor_c(x, y)) return false;
	for (int dy = -1; dy <= 0; ++dy)
		for (int dx = -1; dx <= 0; ++dx)
			if (floor_c(x + dx, y + dy) && floor_c(x + dx + 1, y + dy) && floor_c(x + dx, y + dy + 1) && floor_c(x + dx + 1, y + dy + 1)) return true;
	return false;
}

/* raised floor, and pads where the area's maps give them a look of their
 * own, keep theirs */
static bool kept(int x, int y, bool keep_pads) {
	return (keep_pads && nm_cur->pad && nm_cur->pad[y * nm_cur->gw + x]) || (nm_cur->level && nm_cur->level[y * nm_cur->gw + x]);
}

static void make_stripes(bool keep_pads) {
	static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	memset(stripe, 0, sizeof stripe);
	for (int y = 0; y < nm_cur->gh && y < MAP_H; ++y)
		for (int x = 0; x < nm_cur->gw && x < MAP_W; ++x) {
			if (!floor_c(x, y) || platform_c(x, y) || kept(x, y, keep_pads)) continue;
			for (int d = 0; d < 4; ++d) {
				int dx = d4[d][0], dy = d4[d][1];
				/* straight on from the walkway behind */
				if (!floor_c(x - dx, y - dy) || platform_c(x - dx, y - dy)) continue;
				int k = 1;
				for (;; ++k) {
					int cx = x + dx * k, cy = y + dy * k;
					if (!platform_c(cx, cy) || kept(cx, cy, keep_pads) || !floor_c(cx + dy, cy + dx) || !floor_c(cx - dy, cy - dx)) break;
				}
				/* (on the edge, a panel short) */
				if (!floor_c(x + dx * k, y + dy * k)) --k;
				for (int j = 1; j < k; ++j) stripe[y + dy * j][x + dx * j] = 1;
			}
		}
}

/* The way from the layer's arrival to its exit over the floor as it now
 * stands, for its walkways there to be drawn as roads
 * (docs/world/level-design/roads.md): the generator's way
 * (NetLayout.way) walked again between its two ends, as the legalizer
 * takes cells of it (of a way widened to two, the lane it ran on); the
 * shortest walk, and of those the one that keeps most to the generator's. */
static uint8_t road[MAP_H][MAP_W];

#define STEP 64   /* a step's cost, one off the generator's way 1 more: the shortest first */

static bool on_way(int x, int y) { return nm_cur->way[y * nm_cur->gw + x]; }

/* The generator's way's two ends still floor (cells as y * MAP_W + x); false none. */
static bool way_ends(int gw, int gh, int end[2]) {
	static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	int ends = 0;
	for (int i = 0; i < gw * gh; ++i) {
		int x = i % gw, y = i / gw, n = 0;
		if (!on_way(x, y) || !nm_kind(x, y)) continue;
		for (int d = 0; d < 4; ++d) {
			int nx = x + d4[d][0], ny = y + d4[d][1];
			n += nx >= 0 && ny >= 0 && nx < gw && ny < gh && on_way(nx, ny);
		}
		if (n == 1 && ends++ < 2) end[ends - 1] = y * MAP_W + x;
	}
	return ends == 2;
}

/* What the walk from cell `from` over the floor costs to each cell (INT32_MAX: not reached). */
static void walk_costs(int gw, int gh, int from, int32_t cost[MAP_H][MAP_W]) {
	static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	static int16_t queue[MAP_H * MAP_W * 8];
	size_t head = 0, tail = 0, size = sizeof queue / sizeof *queue;
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) cost[y][x] = INT32_MAX;
	cost[from / MAP_W][from % MAP_W] = 0;
	queue[tail++ % size] = (int16_t)from;
	while (head != tail) {
		int c = queue[head++ % size], x = c % MAP_W, y = c / MAP_W;
		for (int d = 0; d < 4; ++d) {
			int nx = x + d4[d][0], ny = y + d4[d][1];
			if (nx < 0 || ny < 0 || nx >= gw || ny >= gh || !nm_kind(nx, ny)) continue;
			int32_t to = cost[y][x] + STEP + !on_way(nx, ny);
			if (to >= cost[ny][nx] || tail - head >= size) continue;
			cost[ny][nx] = to;
			queue[tail++ % size] = (int16_t)(ny * MAP_W + nx);
		}
	}
}

static void mark_roads(void) {
	static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	static int32_t cost[MAP_H][MAP_W];
	int gw = nm_cur->gw < MAP_W ? nm_cur->gw : MAP_W, gh = nm_cur->gh < MAP_H ? nm_cur->gh : MAP_H, end[2];
	memset(road, 0, sizeof road);
	if (!nm_cur->way || !roads || !way_ends(gw, gh, end)) return;
	walk_costs(gw, gh, end[1], cost);
	/* (back from the other end along falling cost: the first way found) */
	for (int c = end[0], next; c >= 0 && cost[c / MAP_W][c % MAP_W] != INT32_MAX; c = next) {
		int x = c % MAP_W, y = c / MAP_W;
		road[y][x] = 1;
		next = -1;
		for (int d = 0; d < 4 && next < 0; ++d) {
			int nx = x + d4[d][0], ny = y + d4[d][1];
			if (nx < 0 || ny < 0 || nx >= gw || ny >= gh || cost[ny][nx] == INT32_MAX) continue;
			if (cost[ny][nx] + STEP + !on_way(x, y) == cost[y][x]) next = ny * MAP_W + nx;
		}
	}
}

/* A walkway cell where the area's originals set a joint (the Undernet's
 * gems): its ends, turns and crossings, and where it meets other floor;
 * not in a straight run, a walkway cell on each side along one line and
 * nothing on the other two. */
static bool joint_at(int x, int y) {
	static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	bool walk[4];
	int n = 0;
	for (int d = 0; d < 4; ++d) {
		int nx = x + d4[d][0], ny = y + d4[d][1];
		n += nm_kind(nx, ny) != K_VOID;
		walk[d] = ground(nm_kind(nx, ny)) && walkway(nx, ny);
	}
	return !(n == 2 && ((walk[0] && walk[2]) || (walk[1] && walk[3])));
}

/* TILE_PAD for a joint while the tiles are picked: its tiles' look, as
 * tiles.c learns it (the legalizer reads plain walkway floor, as before) */
static int joint_look(int x, int y) {
	return picking && joints && ground(nm_kind(x, y)) && walkway(x, y) && joint_at(x, y) ? TILE_PAD : 0;
}

/* The guardian's arena in walkway floor, to read apart from the platforms;
 * with TILES_ARENA_DRAWN, while the tiles are picked, not (the legalizer
 * reads it so, as before). */
static bool arena_walkway(int x, int y) {
	return arena_b && !(picking && arena_drawn) && x >= nm_cur->ax && x < nm_cur->ax + nm_cur->aw && y >= nm_cur->ay && y < nm_cur->ay + nm_cur->ah;
}

/* TILE_ROAD for the way's walkways while the tiles are picked: a piece
 * apart in the area's roads' look (the legalizer reads walkway floor, as
 * before) */
static int road_look(int x, int y) {
	return picking && roads && x < MAP_W && y < MAP_H && road[y][x] && ground(nm_kind(x, y)) && walkway(x, y) ? TILE_ROAD : 0;
}

/* (with a context, the layout's pads, whatever their look: the
 * neighbourhoods legal.c asks after) */
static int floor_raw(int A, int B, const void *ctx) {
	int k = nm_cur->rise / 32, x = B + nm_place.gx0, y = -A + nm_place.gy0;
	if (!ground(nm_kind(x, y)) && k) {
		x += k; y += k;   /* a raised panel drawn here */
		if (nm_kind(x, y) != K_RAISED) return TILE_VOID;
	} else if (!ground(nm_kind(x, y))) return TILE_VOID;
	int pad = nm_cur->pad && nm_cur->pad[y * nm_cur->gw + x] && (pad_look || ctx) ? TILE_PAD : 0;
	/* the pieces drawn apart: the pads, or the platforms (all but the
	 * walkways) */
	if (apart == NET_APART_PADS ? pad : apart == NET_APART_PLATFORMS && !walkway(x, y) && has_middle(x, y)) pad |= TILE_APART;
	pad |= joint_look(x, y);
	if (!pad) pad |= road_look(x, y);
	if (one_floor) return TILE_A | pad;
	/* by shape: walkways and platforms' rims one floor, their middles the other */
	if (by_shape) return (walkway(x, y) || edge(x, y) ? TILE_B : TILE_A) | pad;
	if (arena_walkway(x, y)) return TILE_B | pad;
	if (striped && stripe[y][x]) return TILE_B | pad;
	/* (Central's, Seaside's and Sky's framed pads are islands of their
	 * walkways' floor: a walkway's last panel before one was drawn meeting
	 * a field, green at its sides) */
	if (pads_walkway && nm_cur->pad && nm_cur->pad[y * nm_cur->gw + x]) return TILE_B | pad;
	return (walkway(x, y) ? TILE_B : TILE_A) | pad;
}

/* floor_raw's answers while the tiles are picked, per panel: the layout
 * no longer changes then, and the pick asks after each panel for every
 * pixel near it, four fifths of making a layer (a 3DS took seconds) */
#define MEMO_EDGE 8
static int32_t *floor_memo;   /* (-1: not asked yet) */
static int memo_w, memo_h;

static int floor_cb(int A, int B, const void *ctx) {
	int32_t *m = NULL;
	if (floor_memo && !ctx) {
		int mx = B + nm_place.gx0 + MEMO_EDGE, my = -A + nm_place.gy0 + MEMO_EDGE;
		if (mx >= 0 && my >= 0 && mx < memo_w && my < memo_h) {
			m = &floor_memo[my * memo_w + mx];
			if (*m >= 0) return *m;
		}
	}
	int v = floor_raw(A, B, ctx);
	if (m) *m = v;
	return v;
}

int netmap_rise(void) { return nm_cur ? nm_cur->rise : 0; }

static bool in_256;   /* (the layer's tiles 256-colour: BN4's) */
static int map_256;   /* (... and the map its layers take over, group and number as BN6's MapId reads them) */

void netmap_colours(void) {
	/* (on the layer's own map alone: at home the next layer is built already, and Lan's HP, an internet map with the
	 * net's very controls, is BN6's own 16-colour tiles, which the bit drew as noise, docs/rom/bn4.md) */
	if (!in_256 || emu_read16(BN6_MAP_ID) != map_256) return;
	/* (the map's alone: BN6's menus, main modes of their own, set BG1 and BG2 to the net's very controls for their
	 * 16-colour tiles, the folder's screens as they open, or leave them as the map had them, a shop's and the Chip
	 * Trader's: there the bit comes off again) */
	int mode = emu_read8(emu_read32(BN6_TOOLKIT));
	bool map = mode == BN6_MODE_GAME, menu = mode == BN6_MODE_SUBMENU || mode == BN6_MODE_SHOP || mode == BN6_MODE_TRADER || mode == BN6_MODE_MAIL;
	if (!map && !menu) return;
	uint32_t render = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_RENDER) + BN6_RENDER_BGCNT;
	static const uint16_t net[2] = { BN6_BGCNT_NET_BG1, BN6_BGCNT_NET_BG2 };
	for (uint32_t bg = 1; bg <= 2; ++bg) {
		uint16_t from = (uint16_t)(net[bg - 1] | (map ? 0 : BN6_BGCNT_256)), to = (uint16_t)(net[bg - 1] | (map ? BN6_BGCNT_256 : 0));
		if (emu_read16(render + 2 * bg) == from) {
			uint8_t v[2] = { (uint8_t)to, (uint8_t)(to >> 8) };
			emu_write(render + 2 * bg, v, sizeof v);
		}
	}
}

/* Where the layer's tiles lie in VRAM, [tiles_lo, tiles_hi) bus addresses:
 * its tile set's two blocks as their header places them (a word count, the
 * LZ77's offset, an offset into VRAM), read from the core's ROM copy as
 * BN6 will */
#define VRAM_BUS 0x06000000u
static uint32_t tiles_lo, tiles_hi;

static void tiles_vram(const Learned *L) {
	uint32_t h = emu_read32(0x08000000u + L->desc);
	tiles_lo = UINT32_MAX;
	tiles_hi = 0;
	for (uint32_t k = 0; k < 2; ++k) {
		uint32_t words = emu_read32(h + 12 * k), vram = VRAM_BUS | emu_read32(h + 12 * k + 8);
		if (!words) continue;
		if (vram < tiles_lo) tiles_lo = vram;
		if (vram + 4 * words > tiles_hi) tiles_hi = vram + 4 * words;
	}
}

bool netmap_backdrop_over(int area, uint32_t *lo, uint32_t *hi) {
	const NetAreaDef *a = net_area_def(area);
	if (!a) return false;
	int group = a->xrom ? a->over[0] : a->group, number = a->xrom ? a->over[1] : a->host ? a->host - 1 : a->number;
	return xbackdrop_map_vram(group, number, lo, hi) && *hi > tiles_lo && *lo < tiles_hi;
}

/* Another game's area: its tile set and colours copied into the core's
 * ROM, where BN6 loads them from the map the layers take over. */
static bool install_gfx(const Learned *L) {
	uint8_t *ts, *pal;
	size_t nts, npal;
	if (!area_src_gfx(L->def->xrom, L->def->group, L->def->number, &ts, &nts, &pal, &npal)) return false;
	uint32_t pal_at = XGFX_AT + (uint32_t)nts;
	bool ok = pal_at + npal <= XGFX_AT + XGFX_SIZE;
	/* (its region holds a tile set that fills the BGs' 64 KB of VRAM, its
	 * two blocks as literal blocks after their header, and 256 colours; the
	 * largest there is, BN4's Undernet's with its colours, is 59,876 bytes) */
	_Static_assert(24 + 2 * LZ_LITERAL_MAX(0x8000) + 4 + 0x200 <= XGFX_SIZE, "a tile set of the BGs' VRAM past its region");
	if (!ok) fprintf(stderr, "netmap: an area's tile set and colours, %u bytes, past FREE_XAREA's %u: not written\n", (unsigned)(nts + npal), (unsigned)XGFX_SIZE);
	if (ok) {
		emu_write(XGFX_AT, ts, nts);
		emu_write(pal_at, pal, npal);
		emu_write32(0x08000000u + L->desc, XGFX_AT);
		emu_write32(0x08000000u + L->desc + 4, pal_at);
	}
	free(ts);
	free(pal);
	return ok;
}

/* The layer's lead: one of the area's maps, by the layer's seed, whose
 * books come before the others', so its looks win where two maps' tiles
 * fit alike (the first book's panel, the first book's pair at the same
 * distance: panels_lay.c, tiles_pick.c). An area's own map led every
 * layer, and its layers all took the one look; now its own or one of those
 * its definition allows (NetAreaDef.leads: the maps that as the lead drew
 * its layers no worse, docs/world/level-design/lead-map.md). The books in
 * that order into `led`; how many. */
int netmap_lead_forced = -1;
bool netmap_whole_forced;
unsigned netmap_rules_forced;   /* (dev) every layer led by this map, whatever is allowed */

static int lead_books(const Learned *L, TileBook led[MAX_BOOKS]) {
	int may[1 + NET_MORE_MAPS] = { 0 }, n = 1, nled = 0;
	for (int m = 1; m <= NET_MORE_MAPS; ++m)
		if (L->def->leads >> (m - 1) & 1) may[n++] = m;
	int lead = netmap_lead_forced >= 0 ? netmap_lead_forced : may[nm_cur->seed % (uint32_t)n];
	for (int pass = 0; pass < 2; ++pass)
		for (int k = 0; k < L->nbooks; ++k)
			if ((L->book_map[k] == lead) == !pass) led[nled++] = L->book[k];
	return nled;
}

/* Both layers of the current layout's tile map in `L`'s tiles, tw x th
 * (freed by the caller): the classes' picks, then what is set whole over
 * them (the props' with `props`: their stamps the layer's own area's), the
 * palette banks and the scenery */
static uint16_t *pick_map(const Learned *L, int tw, int th, bool props) {
	size_t cells = (size_t)tw * th;
	uint16_t *map = calloc(cells * 2, 2);
	if (!map) return NULL;
	TileGrid grid = { tw, th, nm_place.ex, nm_place.ey, L->book[0].dv, L->book[0].face, L->book[0].hang, by_shape || rimmed, apart != NET_APART_NONE };
	/* (a bridge of the Undernets ends at a plate in its joint, a piece of its
	 * own; where an area has none, a walkway's end at a room is no piece of
	 * its maps: TILE_JOINS) */
	grid.plain_ends = !joints;
	grid.rules = netmap_rules_forced | (L->def->lay & (LAY_RULES | LAY_RIM_OWN | LAY_PADS_WHOLE | LAY_ROOMS_OVER));
	grid.roads = L->road;
	grid.nroads = L->nroads;
	/* (where its pads' panels alone are laid whole, LAY_PADS_WHOLE, the rest is picked as before) */
	for (int k = 0; k < L->nbooks && !(((L->def->lay | netmap_rules_forced) & LAY_PADS_WHOLE) && !(L->def->lay & LAY_WHOLE) && !(L->def->styles & TILES_WHOLE)); ++k) grid.wall = L->book[k].panels.n ? 8 : grid.wall;
	free(nm_last.seams);
	nm_last.seams = calloc(cells, 1);
	free(nm_last.pasted);
	nm_last.pasted = calloc(cells, 1);
	memo_w = nm_cur->gw + 2 * MEMO_EDGE;
	memo_h = nm_cur->gh + 2 * MEMO_EDGE;
	floor_memo = malloc((size_t)memo_w * memo_h * sizeof *floor_memo);
	if (floor_memo) memset(floor_memo, 0xFF, (size_t)memo_w * memo_h * sizeof *floor_memo);
	picking = true;
	static TileBook led[MAX_BOOKS];
	int nled = lead_books(L, led);
	tilemap_pick(led, nled, &L->seams, &grid, floor_cb, NULL, map, nm_last.seams);
	uint16_t *beside = nm_pick_beside_pads(L, &grid, led, nled, floor_cb);
	picking = false;
	free(floor_memo);
	floor_memo = NULL;
	nm_paste_stairs(L, map, tw, th);
	if (props) nm_paste_props(map, tw, th);
	nm_paste_pads(L, map, beside, tw, th);
	free(beside);
	nm_paste_ornaments(L, map, tw, th);
	nm_paste_emblems(L, map, tw, th);
	nm_paste_arrows(L, map, tw, th);
	nm_paste_bushes(L, map, tw, th);
	nm_paste_sides(L, map, tw, th);
	/* how the classes' picks went, where they are drawn: not under what
	 * was set whole over them */
	for (size_t i = 0; i < cells; ++i) {
		if (nm_last.pasted[i] & (NETMAP_PASTED_PAD | NETMAP_PASTED_STAIR)) continue;
		tiles_stats.seams += (nm_last.seams[i] & 1) + (nm_last.seams[i] >> 1 & 1);
		tiles_stats.off_near += (nm_last.seams[i] >> 2 & 3) == TILE_OFF_NEAR;
		tiles_stats.off_edge += (nm_last.seams[i] >> 2 & 3) == TILE_OFF_EDGE;
		tiles_stats.other += nm_last.seams[i] >> 7;
	}
	nm_rebank_map(L, map, cells);
	/* (measuring) the breaks in the floor's picture as it is drawn, what was
	 * set whole over the picks with it, before the scenery beside it */
	free(nm_last.breaks);
	nm_last.breaks = NULL;
	uint32_t *px = L->breaks && tiles_measure ? area_src_render(L->def->xrom, L->def->group, L->def->number, map, tw, th) : NULL;
	if (px && (nm_last.breaks = calloc(cells, 1))) breaks_count(L->breaks, px, tw, th, nm_last.breaks, &tiles_stats.breaks, &tiles_stats.nicks);
	free(px);
	netmap_scenery = decor_place(&L->decor, map, tw, th, nm_cur->seed);
	return map;
}

static bool write_tilemap(const Learned *L) {
	/* extent of the floor around the world origin */
	int umax = 0, vmax = 0;
	for (int y = 0; y < nm_cur->gh; ++y)
		for (int x = 0; x < nm_cur->gw; ++x) {
			if (!nm_cur->cell[y * nm_cur->gw + x]) continue;
			int X, Y;
			netmap_world(x, y, &X, &Y);
			int u = abs(X + Y), v = abs((Y - X) / 2);
			if (u > umax) umax = u;
			if (v > vmax) vmax = v;
		}
	int tw = 2 * ((umax + 64 + 7) / 8), th = 2 * ((vmax + 48 + nm_cur->rise + 7) / 8);
	if ((tw & 1) != (L->tw & 1)) ++tw;   /* same tile phase as the source */
	if ((th & 1) != (L->th & 1)) ++th;
	if (tw > 255 || th > 255 || (size_t)tw * th * 4 > BN6_TILEMAP_MAX) return false;
	size_t cells = (size_t)tw * th;
	uint16_t *map = pick_map(L, tw, th, true);
	if (!map) return false;
	size_t raw = cells * 4;
	uint8_t *out = malloc(16 + raw + raw / 8 + 16);
	size_t lz = lz_literal((const uint8_t *)map, raw, out + 12);
	out[0] = (uint8_t)tw; out[1] = (uint8_t)th; out[2] = out[3] = 0;
	put32(out + 4, 12);
	put32(out + 8, (uint32_t)(12 + cells * 2)); /* the second layer, in the decompressed buffer */
	/* (its region, and the town's, hold the largest map BN6's buffer takes
	 * as literal blocks, and say so where one would not:
	 * docs/engine/emulation/free-space.md) */
	_Static_assert(12 + LZ_LITERAL_MAX(BN6_TILEMAP_MAX) <= FREE_TILE_MAP_END - FREE_TILE_MAP, "the largest tile map past its region");
	_Static_assert(12 + LZ_LITERAL_MAX(BN6_TILEMAP_MAX) <= FREE_TOWN_TILE_MAP_END - FREE_TOWN_TILE_MAP, "the largest town map past its region");
	tilemap_bytes = 12 + lz;
	if (12 + lz > FREE_TILE_MAP_END - FREE_TILE_MAP) {
		fprintf(stderr, "netmap: a tile map of %u bytes, past its region's %u: not written\n", (unsigned)(12 + lz), (unsigned)(FREE_TILE_MAP_END - FREE_TILE_MAP));
		free(out);
		free(map);
		return false;
	}
	emu_write(TILEMAP_AT, out, 12 + lz);
	if (emu_debug_on()) {
		FILE *f = emu_debug_file("gen_tilemap.bin");
		if (f) { fwrite(out, 1, 12, f); fwrite(map, 2, cells * 2, f); fclose(f); }
	}
	/* (the learned map's tile set and colours as BN6's ROM has them: a host
	 * of the same group shares them, and another game's area that took the
	 * map over in an earlier run of the session left its own there; another
	 * game's are set with its own copies) */
	if (!L->other)
		for (uint32_t k = 0; k < 8; k += 4) emu_write32(0x08000000u + L->desc + k, rom_u32(L->src_desc + k));
	emu_write32(0x08000000u + L->desc + 8, TILEMAP_AT);
	if (L->other && !install_gfx(L)) { free(out); return false; }
	in_256 = L->other && area_src_bpp8(L->def->xrom, L->def->group, L->def->number);
	map_256 = L->def->over[0] | L->def->over[1] << 8;
	tiles_vram(L);
	free(out);
	free(nm_last.map);
	nm_last.map = map;
	nm_last.tw = tw;
	nm_last.th = th;
	return true;
}

/* Centres the floor on the world origin, across (x - y) and up and down
 * (x + y) the screen; false without floor. */
static bool centre(const NetLayout *lay) {
	int u0 = 1 << 30, u1 = -(1 << 30), v0 = 1 << 30, v1 = -(1 << 30);
	for (int y = 0; y < lay->gh; ++y)
		for (int x = 0; x < lay->gw; ++x)
			if (lay->cell[y * lay->gw + x]) {
				if (x - y < u0) u0 = x - y;
				if (x - y > u1) u1 = x - y;
				if (x + y < v0) v0 = x + y;
				if (x + y > v1) v1 = x + y;
			}
	if (u1 < u0) return false;
	int cu = (u0 + u1) / 2, cv = (v0 + v1) / 2;
	nm_place.gx0 = (cu + cv) / 2;
	nm_place.gy0 = (cv - cu) / 2;
	return true;
}

/* ---- a floor the tiles can draw ---- */

#define LEGAL_BUDGET 400   /* cells changed at most */

/* Whether the panel of grid cell (x, y) has a neighbourhood the original
 * maps show. */
static bool legal_clean(int x, int y, void *ctx) {
	const Learned *L = ctx;
	int A, B;
	nm_grid_to_panel(x, y, &A, &B);
	unsigned oa = 0, ob = 0;
	for (int k = 0; k < 9; ++k) {
		int m = TILE_MATERIAL(floor_cb(A + k % 3 - 1, B + k / 3 - 1, NULL));
		if (m == TILE_A) oa |= 1u << k;
		if (m == TILE_B) ob |= 1u << k;
	}
	return tiles_shape_seen(L->book, L->nbooks, TILE_SHAPE(oa, ob, floor_cb(A, B, L) & TILE_PAD));
}

static void legalize(Learned *L, const NetLayout *lay) {
	LegalGrid g = { lay->gw, lay->gh, lay->cell, lay->locked, legal_clean, L };
	netmap_legal = legal_fix(&g, LEGAL_BUDGET);
}

/* Area `area`'s settings for the tiles' pick, its maps learned; NULL where
 * it has none */
static Learned *use_area(int area) {
	const NetAreaDef *na = net_area_def(area);
	if (!na) return NULL;
	Learned *L = learn_area(area);
	if (!L->ok) return NULL;
	one_floor = !na->walk_styles;
	by_shape = na->styles & TILES_BY_SHAPE;
	arena_b = !by_shape && !(na->styles & TILES_ARENA_FLOOR);
	rimmed = na->styles & TILES_RIMMED;
	pad_look = !(na->styles & TILES_NO_PAD_LOOK);
	pads_walkway = L->pad.ok && (na->pad_hues & na->walk_styles);
	arena_drawn = na->styles & TILES_ARENA_DRAWN;
	joints = na->joint_hues;
	apart = na->apart;
	striped = na->styles & TILES_CROSSING;
	roads = L->nroads > 0;
	return L;
}

bool netmap_build(int area, const NetLayout *lay) {
	Learned *L = use_area(area);
	if (!L) return false;
	nm_cur = lay;
	nm_place.ex = L->ex;
	nm_place.ey = L->ey;
	if (!centre(lay)) return false;
	netmap_legal = (LegalStats){ 0, 0 };
	/* (the stripes as the floor is, and again as the legalizer left it) */
	bool keep_pads = pad_look && L->pads_seen;
	if (striped) make_stripes(keep_pads);
	if (lay->locked) {
		legalize(L, lay);
		centre(lay);
	}
	if (striped) make_stripes(keep_pads);
	mark_roads();
	coord_slot = L->coord_slot;
	nm_build_extra(L);
	nm_gaps_place();
	nm_blocks_place();
	nm_lanes_place();
	if (!write_tilemap(L)) return false;
	/* (the side pieces the tiles set, their layer priorities with them) */
	nm_sides_extra();
	return coords_write(coord_slot, NULL, 0, &nm_extra);
}

bool netmap_set_pads(const CoordPad *pads, int n) { return coords_write(coord_slot, pads, n, &nm_extra); }

/* (the area settings the pick reads, kept round another look's) */
typedef struct { bool one_floor, by_shape, arena_b, rimmed, pad_look, pads_walkway, arena_drawn, joints, striped, roads; int apart; } AreaFlags;

static AreaFlags flags_now(void) {
	return (AreaFlags){ one_floor, by_shape, arena_b, rimmed, pad_look, pads_walkway, arena_drawn, joints, striped, roads, apart };
}

static void flags_set(const AreaFlags *f) {
	one_floor = f->one_floor; by_shape = f->by_shape; arena_b = f->arena_b; rimmed = f->rimmed; pad_look = f->pad_look;
	pads_walkway = f->pads_walkway; arena_drawn = f->arena_drawn; joints = f->joints; striped = f->striped; roads = f->roads; apart = f->apart;
}

/* The look's tile map: the layer's floor as it stands (its legalizer's
 * cells kept, nothing locked or changed), in the place it was built */
static uint16_t *look_map(Learned *L, int tw, int th) {
	NetLayout dry = *nm_cur;
	dry.locked = NULL;
	nm_cur = &dry;
	if (striped) make_stripes(pad_look && L->pads_seen);
	uint16_t *map = pick_map(L, tw, th, false);
	nm_cur = NULL;
	return map;
}

bool netmap_look(int area, NetmapLook *out) {
	memset(out, 0, sizeof *out);
	if (!nm_cur || !nm_last.map || in_256) return false;
	const NetLayout *cur = nm_cur;
	MapPlace place = nm_place;
	AreaFlags flags = flags_now();
	LastMap last = nm_last;
	int scenery = netmap_scenery;
	static uint8_t stripes[MAP_H][MAP_W];
	memcpy(stripes, stripe, sizeof stripe);
	nm_last.seams = nm_last.pasted = nm_last.breaks = NULL;
	Learned *L = use_area(area);
	bool ok = L && !area_src_bpp8(L->def->xrom, L->def->group, L->def->number);
	if (ok) {
		nm_cur = cur;
		out->map = look_map(L, last.tw, last.th);
		out->tw = last.tw;
		out->th = last.th;
		ok = out->map && area_src_vram(L->def->xrom, L->def->group, L->def->number, &out->gfx);
	}
	free(nm_last.seams);
	free(nm_last.pasted);
	free(nm_last.breaks);
	nm_last = last;
	nm_cur = cur;
	nm_place = place;
	flags_set(&flags);
	memcpy(stripe, stripes, sizeof stripe);
	netmap_scenery = scenery;
	if (!ok) netmap_look_free(out);
	return ok;
}

void netmap_look_free(NetmapLook *l) {
	free(l->map);
	area_vram_free(&l->gfx);
	memset(l, 0, sizeof *l);
}

/* The stairs area `area` can draw (bit per STAIR_UP_*) and their rise. */
static unsigned netmap_stair_dirs(int area, int *rise) {
	if (!net_area_def(area)) return 0;
	Learned *L = learn_area(area);
	unsigned dirs = 0;
	*rise = 0;
	for (int d = 0; d < STAIR_DIRS; ++d)
		if (L->ok && L->stairs[d].ok) { dirs |= 1u << d; *rise = L->stairs[d].rise; }
	return dirs;
}

void netmap_kit(int area, LayerKit *kit) {
	memset(kit, 0, sizeof *kit);
	kit->stair_dirs = netmap_stair_dirs(area, &kit->rise);
	const NetAreaDef *na = net_area_def(area);
	if (!na || !learned[area].ok) return;
	for (int f = 0; f < 2; ++f) kit->counter_len[f] = learned[area].counter[f].ok ? learned[area].counter[f].len : 0;
	kit->looks = na->looks;
	/* (where every room is drawn as the art's own small hubs, BN5's
	 * Science Labs and Oran Isle, no signature: a bigger room came out
	 * flat and in pieces of other surfaces, and one of seven a side drew
	 * its tiles off on a fifth more of the floor, over 18 layers each) */
	kit->hub_rooms = na->pad_rooms >= 25;
	kit->one_wide = na->styles & TILES_ONE_WIDE;
	kit->emblem = learned[area].emblem.ok;
	kit->gem = learned[area].ornament[0].ok;
	/* (a lane towards grid DIR_* g runs towards BN6's way g + 1: grid x is
	 * world +Y, grid y world -X) */
	for (int g = 0; g < 4; ++g) kit->arrows |= (unsigned)learned[area].arrow[(g + 1) & 3].ok << g;
}

/* Locks the w x h cells from (x, y) and `margin` around them. */
static void lock(uint8_t locked[MAP_H][MAP_W], int x, int y, int w, int h, int margin) {
	for (int j = y - margin; j < y + h + margin; ++j)
		for (int i = x - margin; i < x + w + margin; ++i)
			if (i >= 0 && j >= 0 && i < MAP_W && j < MAP_H) locked[j][i] = 1;
}

/* The set pieces as generated: a teleport's panels with the floor round
 * them (the gem's), a Rush gap's stand and the floor behind it, and its
 * void panels with the void beside them; an obstacle's or a cube's mouth,
 * one panel wide: the void beside it and the floor before and after it
 * (widened, MegaMan walked round the cube: issue #45; the corners left to
 * the tiles). */
static void lock_pieces(uint8_t locked[MAP_H][MAP_W]) {
	for (int k = 0; k < 2 && layer.nteleports; ++k) lock(locked, layer.teleport_x[k] - 1, layer.teleport_y[k] - 1, 3, 3, 0);
	/* (an arrow lane, its ends and the void beside it as generated) */
	for (int i = 0; i < layer.nlanes; ++i)
		for (int k = 0; k <= layer.lane[i].len + 1; ++k)
			lock(locked, layer.lane[i].x + dir_dx[layer.lane[i].dir] * k, layer.lane[i].y + dir_dy[layer.lane[i].dir] * k, 1, 1, 1);
	for (int k = 0; k < layer.nblocks; ++k) {
		const NetBlock *b = &layer.block[k];
		bool along_x = !(b->dir & 1);
		lock(locked, b->x - along_x, b->y - !along_x, 1 + 2 * along_x, 1 + 2 * !along_x, 0);
		lock(locked, b->x - !along_x, b->y - along_x, 1 + 2 * !along_x, 1 + 2 * along_x, 0);
	}
	for (int g = 0; g < layer.ngaps; ++g) {
		const NetGap *p = &layer.gap[g];
		for (int k = -1; k <= p->len; ++k) lock(locked, p->x + dir_dx[p->dir] * k, p->y + dir_dy[p->dir] * k, 1, 1, 1);
	}
	/* (an invisible path's tip, its void panels and the void beside them) */
	for (int i = 0; i < layer.npaths; ++i) {
		const NetGap *p = &layer.path[i];
		for (int k = -1; k <= p->len; ++k) lock(locked, p->x + dir_dx[p->dir] * k, p->y + dir_dy[p->dir] * k, 1, 1, 1);
	}
}

/* The way from the layer's arrival to its exit, as the generator marked it. */
static const uint8_t *layer_way(void) {
	static uint8_t way[MAP_H][MAP_W];
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) way[y][x] = layer_on_way(x, y);
	return &way[0][0];
}

/* The pads, in their own look (and where the area's platforms are pads,
 * the small platforms, the guardian's arena and the layer's signature:
 * the Aquarium's water never widens into one). */
static void mark_pads(int area, uint8_t pads[MAP_H][MAP_W]) {
	const NetAreaDef *na = net_area_def(area);
	int small = na ? na->pad_rooms : 0;
	bool every = na && (na->styles & TILES_ONE_WIDE);
	memset(pads, 0, sizeof(uint8_t[MAP_H][MAP_W]));
	for (int r = 0; r < layer.nrooms; ++r) {
		const Room *m = &layer.rooms[r];
		bool pad = m->kind == ROOM_PAD || every || (small && ((m->kind == ROOM_PLATFORM && m->w * m->h <= small) || r == layer.arena || r == layer.sig_room));
		/* (a pad of another size where the area's pads are cut whole from
		 * its maps, 3 x 3: a teleport's island of 4 x 4 is a platform there,
		 * and so is a small platform that is not 3 x 3; picked tile by tile
		 * in the pads' look it was a jumble of frames) */
		if (pad && na && na->pad_hues && (m->w != 3 || m->h != 3) && r != layer.arena && r != layer.sig_room) pad = false;
		if (!pad) continue;
		for (int y = m->y; y < m->y + m->h; ++y)
			for (int x = m->x; x < m->x + m->w; ++x) pads[y][x] = 1;
	}
}

bool netmap_build_layer(int area, uint32_t seed) {
	static uint8_t pads[MAP_H][MAP_W];
	mark_pads(area, pads);
	/* what the floor must keep: the cells of objects, rooms' anchors, pads
	 * and the guardian's arena, and the stairs and raised floors with the
	 * cells beside them */
	static uint8_t locked[MAP_H][MAP_W];
	memset(locked, 0, sizeof locked);
	/* (and the floor around them: the rules that kept a service off a
	 * walkway's mouth held for the floor as generated, and a cell changed
	 * beside one made it a mouth, the way on through a Chip Trader) */
	for (int i = 0; i < layer.nobj; ++i) lock(locked, (int)layer.obj[i].x, (int)layer.obj[i].y, 1, 1, 1);
	for (int r = 0; r < layer.nrooms; ++r) {
		const Room *m = &layer.rooms[r];
		lock(locked, m->ax, m->ay, 1, 1, 0);
		if (m->kind == ROOM_PAD || r == layer.arena) lock(locked, m->x, m->y, m->w, m->h, 0);
	}
	for (int i = 0; i < layer.nstairs; ++i) lock(locked, layer.stair[i].x, layer.stair[i].y, 2, 2, 2);
	lock_pieces(locked);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (layer.level[y][x]) lock(locked, x, y, 1, 1, 2);
	/* the props with their aisles and the floor before them */
	for (int i = 0; i < layer.nprops; ++i) {
		const NetProp *p = &layer.props[i];
		if (p->kind == PROP_EMBLEM) lock(locked, p->x, p->y, 1, 1, 1);   /* (its floor all round kept) */
		else if (p->faces == FACES_X) lock(locked, p->x - 1, p->y, 3, p->len, 1);
		else lock(locked, p->x, p->y - 1, p->len, 3, 1);
	}
	static NetLayout lay;
	lay = (NetLayout){ MAP_W, MAP_H, &layer.cell[0][0], &locked[0][0], &layer.level[0][0], layer.rise, layer.stair, layer.nstairs, 0, 0, 0, 0, seed, &pads[0][0],
		layer.props, layer.nprops, .way = layer_way() };
	if (layer.arena >= 0) {
		const Room *a = &layer.rooms[layer.arena];
		lay.ax = a->x; lay.ay = a->y; lay.aw = a->w; lay.ah = a->h;
	}
	return netmap_build(area, &lay);
}

const uint16_t *netmap_last_tiles(int *tw, int *th) {
	*tw = nm_last.tw;
	*th = nm_last.th;
	return nm_last.map;
}

const uint8_t *netmap_last_seams(void) { return nm_last.seams; }

const uint8_t *netmap_last_pasted(void) { return nm_last.pasted; }

const uint8_t *netmap_last_breaks(void) { return nm_last.breaks; }

void netmap_last_cells(char out[MAP_H][MAP_W + 1]) {
	for (int y = 0; y < MAP_H; ++y) {
		for (int x = 0; x < MAP_W; ++x) {
			int A, B;
			nm_grid_to_panel(x, y, &A, &B);
			int m = nm_cur ? floor_cb(A, B, NULL) : TILE_VOID;
			char c = ".ab"[TILE_MATERIAL(m)];
			if (c == 'b' && striped && stripe[y][x]) c = 's';
			if (m & TILE_PAD) c = 'p';
			out[y][x] = c;
		}
		out[y][MAP_W] = 0;
	}
	if (!nm_last.seams) return;
	TileGrid grid = { nm_last.tw, nm_last.th, nm_place.ex, nm_place.ey, 0, 0, 0, false, false, false, 0, false, 0, NULL, 0, false };
	for (int ty = 0; ty < nm_last.th; ++ty)
		for (int tx = 0; tx < nm_last.tw; ++tx) {
			if ((nm_last.seams[ty * nm_last.tw + tx] >> 2 & 3) != TILE_OFF_NEAR || nm_last.pasted[ty * nm_last.tw + tx] & (NETMAP_PASTED_PAD | NETMAP_PASTED_STAIR)) continue;
			int phase, A, B;
			tile_class(&grid, tx, ty, &phase, &A, &B);
			int x = B + nm_place.gx0, y = -A + nm_place.gy0;
			if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) continue;
			char *c = &out[y][x];
			*c = *c == '.' ? '*' : *c >= 'a' ? (char)(*c - 'a' + 'A') : *c;
		}
}
