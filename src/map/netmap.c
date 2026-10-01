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
#include "netmap.h"

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "bn6.h"
#include "bytes.h"
#include "coords.h"
#include "debug.h"
#include "decor.h"
#include "emu.h"
#include "legal.h"
#include "lz.h"
#include "props.h"
#include "stairs.h"
#include "tilemap.h"
#include "tiles.h"
#include "rom.h"

#define TILEMAP_AT  (EMU_FREE + 0x10000) /* generated tile map (LZ77) */
#define XGFX_AT     (EMU_FREE + 0x170000)   /* another game's area: its tile set and palette (docs/EMULATION.md) */
#define XGFX_SIZE   0x30000

#define MAX_BOOKS (4 * (1 + NET_MORE_MAPS))   /* maps, their mirrors, two heights each */

typedef struct {
	bool tried, ok;
	int ex, ey, tw, th;
	TileBook book[MAX_BOOKS]; /* each source map, then its mirror image */
	int nbooks;
	TileSeams seams;          /* the tiles the maps set side by side */
	DecorBook decor;          /* the scenery of the area's maps */
	uint32_t desc, coord_slot;   /* the map its layers take over (net_area.host) */
	uint32_t src_desc;        /* the learned map's, whose tile set and colours they draw in */
	bool other;               /* its maps are another game's (docs/MULTIROM.md): src_desc is that game's */
	const NetAreaDef *def;
	StairTemplate stairs[STAIR_DIRS];
	PropStamp counter[2];     /* the Net Dealer's counter, facing FACES_X and FACES_Y */
	int counter_dx[2], counter_dy[2];   /* the world offset that sets its tiles on the layer's lattice */
	PropStamp ornament[3];    /* pads' centrepieces: the red gem, the link ring, the cube on its base */
	PropStamp bush[2];        /* Green's potted bushes, plain and in flower */
	PropStamp emblem;         /* the emblem its floors carry (the Graveyard's crosses) */
	PropStamp pad;            /* a whole pad of its maps, for its layers' (Central's framed pads) */
	uint8_t rebank[2];        /* RomLayout.net_area[].rebank */
	uint8_t rebank_to[128];   /* the first-layer tiles its maps draw in rebank[1] (a bit each) */
	bool pads_seen;           /* its maps have pads, whose look its layers' pads take */
} Learned;

/* the centrepieces' tiles (the four surface areas share their tile set) */
static const int ornament_tile[3] = { 0x379, 0x372, 0x375 };

static Learned learned[NET_AREAS + XAREAS_MAX];

int netmap_scenery;
LegalStats netmap_legal;

/* the last tile map written, both layers (for the dev tools) */
static struct { uint16_t *map; uint8_t *seams; uint8_t *pasted; int tw, th; } last;   /* seams: bit 0 right, bit 1 below; pasted: NETMAP_PASTED_* */

/* the current layer's placement */
static struct { int gx0, gy0, ex, ey; } place;

/* ---- helpers ---- */

static int floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

/* Where map a's tiles fall in its panels (world units mod 32, of tile 0's
 * centre). */
static void lattice(const AreaSrc *a, int *fx, int *fy) {
	int u = 4 - a->tw * 4, v = 4 - a->th * 4;
	*fx = ((u - 2 * v) / 2 - a->ex) & 31;
	*fy = ((u + 2 * v) / 2 - a->ey) & 31;
}

/* Whether map b's tiles lie where map a's do in their panels: then a tile
 * of b drawn in a layer made on a's grid shows as it did in b. Tiles step
 * by (4, 4) and (-8, 8) world units. */
static bool aligned(const AreaSrc *a, const AreaSrc *b) {
	int ax, ay, bx, by;
	lattice(a, &ax, &ay);
	lattice(b, &bx, &by);
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 4; ++j)
			if (((ax + 4 * i - 8 * j - bx) & 31) == 0 && ((ay + 4 * i + 8 * j - by) & 31) == 0) return true;
	return false;
}

/* A map of the area's own game, BN6's or another's (docs/MULTIROM.md). */
static bool load_map(const NetAreaDef *na, int group, int number, AreaSrc *a) {
	return na->xrom ? area_src_load_x(na->xrom - 1, group, number, a) : area_src_load(group, number, a);
}

/* Learns the tiles of map `src` and its mirror image, where their tiles
 * line up with the layer's grid (`grid_of`). */
static void learn_view(const AreaSrc *src, const AreaSrc *grid_of, int area, Learned *L) {
	const NetAreaDef *na = net_area_def(area);
	if (L->nbooks < MAX_BOOKS && aligned(grid_of, src)) {
		tiles_learn(src, na->styles, na->walk_styles, na->skip_styles, na->bg_in_map, &L->book[L->nbooks++]);
		seams_add(&L->seams, src, na->bg_in_map);
	}
	AreaSrc m;
	if (!area_src_mirror(src, &m)) return;
	if (L->nbooks < MAX_BOOKS && aligned(grid_of, &m)) {
		tiles_learn(&m, na->styles, na->walk_styles, na->skip_styles, na->bg_in_map, &L->book[L->nbooks++]);
		seams_add(&L->seams, &m, na->bg_in_map);
	}
	area_src_free(&m);
}

#define LEVEL_MIN_CELLS 144   /* nine panels of floor at a height to learn it */

/* ... at each of its floor heights: raised floors are drawn higher, so each
 * is learned in a view that brings it down to the ground. */
static void learn_map(const AreaSrc *src, const AreaSrc *grid_of, int area, Learned *L) {
	int count[256] = { 0 };
	for (int i = 0; src->hz && i < src->hw * src->hh; ++i) count[src->hz[i]]++;
	learn_view(src, grid_of, area, L);
	for (int z = 8; z < HEIGHT_UNEVEN; z += 8) {
		if (count[z] < LEVEL_MIN_CELLS) continue;
		AreaSrc r;
		if (!area_src_raise(src, z, &r)) continue;
		learn_view(&r, grid_of, area, L);
		area_src_free(&r);
	}
}

/* The Net Dealer's counter, cut from its map facing world +Y (FACES_X) and
 * from that map's mirror image facing world -X (FACES_Y). Its tiles must
 * fall on the layer's lattice (`own`'s) at some small offset (paste). */
static void learn_counter(const int16_t *c, const AreaSrc *own, Learned *L) {
	if (!c[0]) return;
	AreaSrc src, m;
	if (!area_src_load(c[0], c[1], &src)) return;
	props_learn_counter(&src, c[2], c[3], FACES_X, &L->counter[FACES_X]);
	if (area_src_mirror(&src, &m)) {
		props_mirror_walls(&src, &m);
		props_learn_counter(&m, -c[3], -c[2], FACES_Y, &L->counter[FACES_Y]);
	}
	/* anchored on a panel corner of the layer (whose lattice is `own`'s:
	 * the same phase whatever the corner), the least move that sets its
	 * tiles on the lattice, along its run before in its depth (its back
	 * row stays on the panel edge the aisle's walls run along) */
	for (int f = 0; f < 2; ++f) {
		PropStamp *st = &L->counter[f];
		if (!st->ok) continue;
		int best = -1;
		for (int dx = -16; dx <= 16; dx += 4)
			for (int dy = -16; dy <= 16; dy += 4) {
				int X = own->ex + dx, Y = own->ey + dy;
				int px = area_px(own->tw, X, Y) + st->tiles[0].px, py = area_py(own->th, X, Y) + st->tiles[0].py;
				if ((px & 7) || (py & 7)) continue;
				int cost = f == FACES_X ? abs(dx) + 4 * abs(dy) : 4 * abs(dx) + abs(dy);
				if (best < 0 || cost < best) { best = cost; L->counter_dx[f] = dx; L->counter_dy[f] = dy; }
			}
		if (best < 0) props_free(st);
	}
	if (emu_debug_on())
		for (int f = 0; f < 2; ++f)
			fprintf(stderr, "counter faces %d ok %d len %d tiles %d walls %d navi %d,%d\n", f, L->counter[f].ok, L->counter[f].len,
				L->counter[f].ntiles, L->counter[f].nwalls, L->counter[f].navi_x, L->counter[f].navi_y);
	if (emu_debug_on())
		for (int f = 0; f < 2; ++f) fprintf(stderr, "counter faces %d moved %d,%d onto the lattice\n", f, L->counter_dx[f], L->counter_dy[f]);
	area_src_free(&m);
	area_src_free(&src);
}

/* Marks the first-layer tiles `a` draws in palette bank `to`. */
static void rebank_seen(const AreaSrc *a, int to, Learned *L) {
	if (!to) return;
	for (int i = 0; i < a->tw * a->th; ++i)
		if (a->tile[0][i] >> 12 == to && (a->tile[0][i] & 0x3FF)) L->rebank_to[(a->tile[0][i] & 0x3FF) >> 3] |= (uint8_t)(1u << (a->tile[0][i] & 7));
}

/* The area's other maps in the same tiles and colours, for the places its
 * own never shows, on its own map's grid. */
static void learn_more(const AreaSrc *grid, int area, Learned *L) {
	const NetAreaDef *na = net_area_def(area);
	for (int k = 0; k < NET_MORE_MAPS && na->more[k][0]; ++k) {
		AreaSrc b;
		if (!load_map(na, na->more[k][0], na->more[k][1], &b)) continue;
		learn_map(&b, grid, area, L);
		if (!(na->styles & TILES_NO_SCENERY)) decor_learn(&b, na->bg_in_map, &L->decor);
		rebank_seen(&b, na->rebank[1], L);
		for (int o = 0; o < 3 && !na->xrom; ++o)
			if (!L->ornament[o].ok && aligned(grid, &b)) props_learn_ornament(&b, ornament_tile[o], &L->ornament[o]);
		/* (its pads' tiles too, where its own map's all have a bridge
		 * beside them there) */
		if (na->pad_hues && aligned(grid, &b)) props_learn_pad(&b, na->pad_hues, &L->pad);
		area_src_free(&b);
	}
}

static bool learn(int area, Learned *L) {
	const NetAreaDef *na = net_area_def(area);
	AreaSrc a;
	if (!na || !load_map(na, na->group, na->number, &a)) return false;
	L->ex = a.ex; L->ey = a.ey; L->tw = a.tw; L->th = a.th;
	L->desc = L->src_desc = a.desc; L->coord_slot = a.coord_slot;
	/* (another game's area takes over a BN6 map: its slots are BN6's) */
	L->other = na->xrom != 0;
	L->def = na;
	if (L->other ? !area_src_slots(na->over[0], na->over[1], &L->desc, &L->coord_slot)
		: na->host && !area_src_slots(na->group, na->host - 1, &L->desc, &L->coord_slot)) { area_src_free(&a); return false; }
	L->nbooks = 0;
	learn_map(&a, &a, area, L);
	if (!(na->styles & TILES_NO_SCENERY)) decor_learn(&a, na->bg_in_map, &L->decor);
	stairs_learn(&a, L->stairs);
	learn_counter(na->counter, &a, L);
	/* (the pads' centrepieces are BN6's tiles) */
	for (int k = 0; k < 3 && !na->xrom; ++k) props_learn_ornament(&a, ornament_tile[k], &L->ornament[k]);
	if (na->emblem) props_learn_floor_emblem(&a, na->emblem, &L->emblem);
	if (na->pad_hues) props_learn_pad(&a, na->pad_hues, &L->pad);
	L->rebank[0] = na->rebank[0];
	L->rebank[1] = na->rebank[1];
	rebank_seen(&a, na->rebank[1], L);
	if (emu_debug_on()) fprintf(stderr, "emblem area %d ok %d tiles %d\n", area, L->emblem.ok, L->emblem.ntiles);
	if (na->looks & (1u << LOOK_TREE) && na->looks & (1u << LOOK_GIANT_TREE))   /* (Green's) */
		for (int k = 0; k < 2; ++k) props_learn_void_art(&a, k ? 0x361 : 0x292, &L->bush[k]);
	/* the colours its floors show (an area's maps share its palette) */
	static uint8_t seen[TILE_COLOURS];
	memset(seen, 0, sizeof seen);
	int own = L->nbooks;
	for (int k = 0; k < own; ++k) tiles_colours(&L->book[k], seen);
	/* pads in the look of the other maps' pads alone, where the own map's
	 * small platforms are something else (Robot Control Comp 2's conveyor) */
	if (na->styles & TILES_MORE_PADS)
		for (int k = 0; k < own; ++k) tiles_drop_pads(&L->book[k]);
	/* (its own map's grid alone from here, freed before the others load: a
	 * map, its mirror and the own map at once took a New 3DS's memory) */
	AreaSrc grid = { .tw = a.tw, .th = a.th, .ex = a.ex, .ey = a.ey };
	area_src_free(&a);
	learn_more(&grid, area, L);
	if (emu_debug_on()) fprintf(stderr, "pad stamp area %d ok %d tiles %d, %d panels beside its lower sides\n", area, L->pad.ok, L->pad.ntiles, L->pad.low);
	/* but not their pieces in colours this one's floors never show: another
	 * surface's (Central Area 2's raised plateau, yellow where Central's
	 * fields are green; an Undernet map's raised grey court), which fit some
	 * joins and dead ends better than the area's own tiles and stand out
	 * there, unless the area takes those colours too */
	for (int k = own; k < L->nbooks; ++k) tiles_other_colours(&L->book[k], seen, !(na->styles & TILES_MORE_COLOURS));
	L->pads_seen = false;
	for (int k = 0; k < L->nbooks; ++k)
		for (int i = 0; i < L->book[k].n && !L->pads_seen; ++i) L->pads_seen = L->book[k].cand[i].pad;
	if (emu_debug_on()) {
		fprintf(stderr, "tiles area %d floor %d px down, faces %d px, hanging %d px, %d books, %d pieces of scenery\n", area, L->book[0].dv, L->book[0].face, L->book[0].hang, L->nbooks, L->decor.n);
		for (int d = 0; d < STAIR_DIRS; ++d)
			fprintf(stderr, "stairs area %d dir %d ok %d rise %d ramp %d walls %d prio %d tiles %d\n", area, d, L->stairs[d].ok,
				L->stairs[d].rise, L->stairs[d].nramp, L->stairs[d].nwalls, L->stairs[d].nprio, L->stairs[d].ntiles);
	}
	return true;
}

/* ---- layout <-> world ---- */

/* grid x (down-right on screen) is world +Y, grid y (down-left) world -X */
static void grid_to_panel(int x, int y, int *A, int *B) { *A = -(y - place.gy0); *B = x - place.gx0; }

void netmap_world(int x, int y, int *wx, int *wy) {
	int A, B;
	grid_to_panel(x, y, &A, &B);
	*wx = place.ex + 16 + 32 * A;
	*wy = place.ey + 16 + 32 * B;
}

void netmap_grid(int wx, int wy, double *x, double *y) {
	*x = (wy - place.ey - 16) / 32.0 + place.gx0;
	*y = -(wx - place.ex - 16) / 32.0 + place.gy0;
}

bool netmap_panel(int wx, int wy, int *x, int *y) {
	int A = floordiv(wx - place.ex, 32), B = floordiv(wy - place.ey, 32);
	*x = B + place.gx0;
	*y = -A + place.gy0;
	return true;
}

static const NetLayout *cur;
static bool one_floor;        /* the area has no walkway floor: all is platform */
static bool by_shape;         /* its floors are told by shape: an arena is platform */
static bool arena_b;          /* the guardian's arena in the walkways' floor, to read apart (TILES_ARENA_FLOOR: not) */
static bool rimmed;           /* its platforms' edges are rims (TILES_RIMMED) */
static int apart;             /* what of its floor stands apart from the rest (NET_APART_*) */
static bool pad_look;         /* its originals' pads have a look for the layer's (RomLayout.net_area) */
static bool pads_walkway;     /* its framed pads are cut from its maps in their walkways' floor */
static uint32_t coord_slot;   /* the layer map's coordinate-data pointer */

/* K_SOLID: floor drawn, walled off (a counter's aisle, net.h C_SOLID) */
enum { K_VOID, K_FLOOR, K_RAISED, K_STAIR, K_SOLID };

/* What the layer has at grid cell (x, y). */
static int kind(int x, int y) {
	if (x < 0 || y < 0 || x >= cur->gw || y >= cur->gh || !cur->cell[y * cur->gw + x]) return K_VOID;
	for (int i = 0; i < cur->nstairs; ++i)
		if (x >= cur->stairs[i].x && x < cur->stairs[i].x + 2 && y >= cur->stairs[i].y && y < cur->stairs[i].y + 2) return K_STAIR;
	if (cur->cell[y * cur->gw + x] == C_SOLID) return K_SOLID;
	return cur->level && cur->level[y * cur->gw + x] ? K_RAISED : K_FLOOR;
}

/* The ground floor as drawn: walled-off panels look like the rest. */
static bool ground(int k) { return k == K_FLOOR || k == K_SOLID; }

static int kind_at(int A, int B) { return kind(B + place.gx0, -A + place.gy0); }


/* A walkway: a floor cell in no 2x2 block of floor, as the original areas
 * draw their 1-wide paths in their second floor. */
static bool walkway(int x, int y) {
	for (int dy = -1; dy <= 0; ++dy)
		for (int dx = -1; dx <= 0; ++dx)
			if (kind(x + dx, y + dy) && kind(x + dx + 1, y + dy) && kind(x + dx, y + dy + 1) && kind(x + dx + 1, y + dy + 1)) return false;
	return true;
}

/* The floor's material as the screen shows it: ground panels where they
 * are, raised ones where a flat panel `rise` world units up the screen
 * would be (stairs are drawn from their own pieces). */
/* A floor cell beside the void (8 ways): a platform's rim. */
static bool edge(int x, int y) {
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
			if (kind(x + dx, y + dy) == K_VOID) return true;
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

static bool floor_c(int x, int y) { return x >= 0 && y >= 0 && x < cur->gw && y < cur->gh && cur->cell[y * cur->gw + x] == C_PATH; }

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
	return (keep_pads && cur->pad && cur->pad[y * cur->gw + x]) || (cur->level && cur->level[y * cur->gw + x]);
}

static void make_stripes(bool keep_pads) {
	static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	memset(stripe, 0, sizeof stripe);
	for (int y = 0; y < cur->gh && y < MAP_H; ++y)
		for (int x = 0; x < cur->gw && x < MAP_W; ++x) {
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

/* (with a context, the layout's pads, whatever their look: the
 * neighbourhoods legal.c asks after) */
static int floor_raw(int A, int B, const void *ctx) {
	int k = cur->rise / 32, x = B + place.gx0, y = -A + place.gy0;
	if (!ground(kind(x, y)) && k) {
		x += k; y += k;   /* a raised panel drawn here */
		if (kind(x, y) != K_RAISED) return TILE_VOID;
	} else if (!ground(kind(x, y))) return TILE_VOID;
	int pad = cur->pad && cur->pad[y * cur->gw + x] && (pad_look || ctx) ? TILE_PAD : 0;
	/* the pieces drawn apart: the pads, or the platforms (all but the
	 * walkways) */
	if (apart == NET_APART_PADS ? pad : apart == NET_APART_PLATFORMS && !walkway(x, y)) pad |= TILE_APART;
	if (one_floor) return TILE_A | pad;
	/* by shape: walkways and platforms' rims one floor, their middles the other */
	if (by_shape) return (walkway(x, y) || edge(x, y) ? TILE_B : TILE_A) | pad;
	if (arena_b && x >= cur->ax && x < cur->ax + cur->aw && y >= cur->ay && y < cur->ay + cur->ah) return TILE_B | pad;
	if (striped && stripe[y][x]) return TILE_B | pad;
	/* (Central's, Seaside's and Sky's framed pads are islands of their
	 * walkways' floor: a walkway's last panel before one was drawn meeting
	 * a field, green at its sides) */
	if (pads_walkway && cur->pad && cur->pad[y * cur->gw + x]) return TILE_B | pad;
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
		int mx = B + place.gx0 + MEMO_EDGE, my = -A + place.gy0 + MEMO_EDGE;
		if (mx >= 0 && my >= 0 && mx < memo_w && my < memo_h) {
			m = &floor_memo[my * memo_w + mx];
			if (*m >= 0) return *m;
		}
	}
	int v = floor_raw(A, B, ctx);
	if (m) *m = v;
	return v;
}

int netmap_rise(void) { return cur ? cur->rise : 0; }

/* ---- output ---- */

/* A stair block's corner with the lowest panel indices, in world units. */
static void stair_origin(const Stair *st, int *X0, int *Y0) {
	int A0 = -(st->y + 1 - place.gy0), B0 = st->x - place.gx0;
	*X0 = place.ex + 32 * A0;
	*Y0 = place.ey + 32 * B0;
}

/* The stairs' own tiles over whatever the classes put there. */
static void paste_stairs(const Learned *L, uint16_t *map, int tw, int th) {
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < cur->nstairs; ++i) {
		const StairTemplate *t = &L->stairs[cur->stairs[i].dir];
		if (!t->ok) continue;
		int X0, Y0;
		stair_origin(&cur->stairs[i], &X0, &Y0);
		int px0 = X0 + Y0 + tw * 4, py0 = (Y0 - X0) / 2 + th * 4;
		for (int k = 0; k < t->ntiles; ++k) {
			int px = px0 + t->tiles[k].px, py = py0 + t->tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
			/* (not a tile three quarters empty: over void it floated as a
			 * speck, over floor it cut a notch into it) */
			if (t->tiles[k].opaque < 16) continue;
			size_t at = (size_t)(py / 8) * tw + px / 8;
			map[at] = t->tiles[k].e0;
			map[cells + at] = t->tiles[k].e1;
			if (last.pasted) last.pasted[at] |= NETMAP_PASTED_STAIR;
		}
	}
}

/* ---- props ---- */

/* The last layer's props as drawn: their anchors (world units, props.h). */
static struct { bool ok; int X, Y; const PropStamp *st; } prop_at[MAX_PROPS];

static void props_place(const Learned *L) {
	memset(prop_at, 0, sizeof prop_at);
	for (int i = 0; i < cur->nprops && i < MAX_PROPS; ++i) {
		const NetProp *p = &cur->props[i];
		const PropStamp *st = &L->counter[p->faces];
		if (p->kind != PROP_COUNTER || !st->ok || st->len != p->len) continue;
		/* the corner of its panels with the lowest world X and Y: FACES_X
		 * runs along grid y, whose highest cell is the lowest X */
		int A = p->faces == FACES_X ? -(p->y + p->len - 1 - place.gy0) : -(p->y - place.gy0), B = p->x - place.gx0;
		prop_at[i] = (__typeof__(prop_at[0])){ true, place.ex + 32 * A + L->counter_dx[p->faces], place.ey + 32 * B + L->counter_dy[p->faces], st };
		if (emu_debug_on()) fprintf(stderr, "counter faces %d at %d,%d (%d walls, %d layer priorities)\n", p->faces, prop_at[i].X, prop_at[i].Y, st->nwalls, st->nprio);
	}
}

/* Their tiles on the second layer, over the floor the classes drew. */
static void paste_props(uint16_t *map, int tw, int th) {
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < MAX_PROPS; ++i) {
		if (!prop_at[i].ok) continue;
		const PropStamp *st = prop_at[i].st;
		int px0 = area_px(tw, prop_at[i].X, prop_at[i].Y), py0 = area_py(th, prop_at[i].X, prop_at[i].Y);
		for (int k = 0; k < st->ntiles; ++k) {
			int px = px0 + st->tiles[k].px, py = py0 + st->tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
			map[cells + (size_t)(py / 8) * tw + px / 8] = st->tiles[k].e1;
		}
	}
}

/* The pads' centrepieces, walkable, on the middle panel of every pad at
 * ground level with nothing standing there, as the originals set theirs:
 * mostly the red gem, else the link ring or the cube (the layer's own
 * choice from its seed, the floor unchanged). */
static void paste_ornaments(const Learned *L, uint16_t *map, int tw, int th) {
	int have[3], n = 0;
	for (int k = 0; k < 3; ++k) if (L->ornament[k].ok) have[n++] = k;
	if (!n) return;
	uint32_t r = cur->seed ^ 0x0A7E0u;
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < layer.nrooms; ++i) {
		const Room *m = &layer.rooms[i];
		int x = m->x + m->w / 2, y = m->y + m->h / 2;
		if (m->kind != ROOM_PAD || layer.cell[y][x] != C_PATH || layer.level[y][x]) continue;
		bool taken = false;
		for (int o = 0; o < layer.nobj; ++o) taken |= (int)layer.obj[o].x == x && (int)layer.obj[o].y == y;
		if (taken) continue;
		r = r * 1103515245u + 12345u;
		int roll = (int)((r >> 16) % 5);
		const PropStamp *st = &L->ornament[have[roll < 3 ? 0 : (roll - 2) % n]];
		int A, B;
		grid_to_panel(x, y, &A, &B);
		int px0 = area_px(tw, place.ex + 32 * A, place.ey + 32 * B), py0 = area_py(th, place.ex + 32 * A, place.ey + 32 * B);
		for (int k = 0; k < st->ntiles; ++k) {
			int px = px0 + st->tiles[k].px, py = py0 + st->tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
			map[cells + (size_t)(py / 8) * tw + px / 8] = st->tiles[k].e1;
		}
	}
}

/* The area's own pad, whole, on every 3 x 3 pad at ground level away from
 * the stairs, in place of the floor the classes drew there (the classes
 * drew Central's pads green, where its maps frame them); their
 * centrepieces go on after. */
/* Whether the tile at map pixel (px, py) lies over pad `m`'s own panels or
 * the void alone (at the floor's height, drawn dv below the panel edges). */
static bool pad_tile_ok(const Learned *L, const Room *m, int tw, int th, int px, int py) {
	for (int y = 1; y < 8; y += 2)
		for (int x = 1; x < 8; x += 2) {
			int u = px + x - tw * 4, v = 2 * (py + y - L->book[0].dv - th * 4), X = (u - v) / 2, Y = (u + v) / 2;
			int A = floordiv(X - place.ex, 32), B = floordiv(Y - place.ey, 32);
			int gx = B + place.gx0, gy = -A + place.gy0;
			if (gx >= m->x && gx < m->x + 3 && gy >= m->y && gy < m->y + 3) continue;
			if (gx >= 0 && gy >= 0 && gx < MAP_W && gy < MAP_H && layer.cell[gy][gx] != C_VOID) return false;
		}
	return true;
}

static void paste_pads(const Learned *L, uint16_t *map, int tw, int th) {
	if (!L->pad.ok) return;
	for (int i = 0; i < layer.nrooms; ++i) {
		const Room *m = &layer.rooms[i];
		if (m->kind != ROOM_PAD || m->w != 3 || m->h != 3) continue;
		bool flat = true;
		for (int y = m->y; y < m->y + 3; ++y)
			for (int x = m->x; x < m->x + 3; ++x) flat &= layer.cell[y][x] == C_PATH && !layer.level[y][x];
		for (int s = 0; s < layer.nstairs; ++s)
			flat &= layer.stair[s].x + 2 < m->x - 1 || layer.stair[s].x > m->x + 3 || layer.stair[s].y + 2 < m->y - 1 || layer.stair[s].y > m->y + 3;
		if (!flat) continue;
		int A, B;
		grid_to_panel(m->x, m->y + 2, &A, &B);
		int X0 = place.ex + 32 * A, Y0 = place.ey + 32 * B;
		int px0 = area_px(tw, X0, Y0), py0 = area_py(th, X0, Y0);
		/* the stamp only over the pad's own panels or the void: its edges
		 * and faces would cut into a walkway or a platform it touches (a
		 * walkway's last panel before the pad came out cut) */
		for (int k = 0; k < L->pad.ntiles; ++k) {
			int px = px0 + L->pad.tiles[k].px, py = py0 + L->pad.tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th || !pad_tile_ok(L, m, tw, th, px, py)) continue;
			map[(size_t)(py / 8) * tw + px / 8] = L->pad.tiles[k].e0;
			if (last.pasted) last.pasted[(size_t)(py / 8) * tw + px / 8] |= NETMAP_PASTED_PAD;
		}
	}
}

/* The emblems the generator set in the floor (PROP_EMBLEM), in place of
 * the floor the classes drew there. */
static void paste_emblems(const Learned *L, uint16_t *map, int tw, int th) {
	if (!L->emblem.ok) return;
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < cur->nprops && i < MAX_PROPS; ++i) {
		const NetProp *p = &cur->props[i];
		if (p->kind != PROP_EMBLEM) continue;
		int A, B;
		grid_to_panel(p->x, p->y, &A, &B);
		int px0 = area_px(tw, place.ex + 32 * A, place.ey + 32 * B), py0 = area_py(th, place.ex + 32 * A, place.ey + 32 * B);
		for (int k = 0; k < L->emblem.ntiles; ++k) {
			int px = px0 + L->emblem.tiles[k].px, py = py0 + L->emblem.tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
			size_t at = (size_t)(py / 8) * tw + px / 8;
			if (L->emblem.tiles[k].e0) map[at] = L->emblem.tiles[k].e0;
			if (L->emblem.tiles[k].e1) map[cells + at] = L->emblem.tiles[k].e1;
			if (last.pasted) last.pasted[at] |= NETMAP_PASTED_EMBLEM;
		}
	}
}

/* Green's potted bushes in the gaps between parallel planks, as its maps
 * set them: in a void panel with floor on both sides along one axis and
 * void on the other two, every second panel along the gap, the plain and
 * the flowering pot in turn. */
static void paste_bushes(const Learned *L, uint16_t *map, int tw, int th) {
	if (!L->bush[0].ok) return;
	size_t cells = (size_t)tw * th;
	for (int y = 1; y < MAP_H - 1; ++y)
		for (int x = 1; x < MAP_W - 1; ++x) {
			if (layer.cell[y][x] != C_VOID) continue;
			bool across = layer.cell[y][x - 1] == C_PATH && layer.cell[y][x + 1] == C_PATH && !layer.cell[y - 1][x] && !layer.cell[y + 1][x];
			bool down = layer.cell[y - 1][x] == C_PATH && layer.cell[y + 1][x] == C_PATH && !layer.cell[y][x - 1] && !layer.cell[y][x + 1];
			/* (every second one along the gap) */
			if (!(across && (y & 1) == 0) && !(down && (x & 1) == 0)) continue;
			const PropStamp *st = &L->bush[L->bush[1].ok && ((x + y) / 2 & 1)];
			int A, B;
			grid_to_panel(x, y, &A, &B);
			int px0 = area_px(tw, place.ex + 32 * A, place.ey + 32 * B), py0 = area_py(th, place.ex + 32 * A, place.ey + 32 * B);
			for (int k = 0; k < st->ntiles; ++k) {
				int px = px0 + st->tiles[k].px, py = py0 + st->tiles[k].py;
				if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
				size_t at = (size_t)(py / 8) * tw + px / 8;
				if (!map[cells + at]) map[cells + at] = st->tiles[k].e1;   /* (never over the floor's own art) */
			}
		}
}

bool netmap_prop_navi(int i, int *wx, int *wy, int *tx, int *ty) {
	if (i < 0 || i >= MAX_PROPS || !prop_at[i].ok) return false;
	*wx = prop_at[i].X + prop_at[i].st->navi_x;
	*wy = prop_at[i].Y + prop_at[i].st->navi_y;
	*tx = prop_at[i].st->talk_x;
	*ty = prop_at[i].st->talk_y;
	return true;
}

/* One shade of floor: tiles of the other bank in this one's colours, where
 * its maps draw the same tile in it. */
static void rebank_map(const Learned *L, uint16_t *map, size_t cells) {
	const uint8_t *rb = L->rebank;
	if (!rb[0] || !rb[1]) return;
	for (size_t i = 0; i < cells; ++i) {
		uint16_t e = map[i];
		if (e >> 12 == rb[0] && (e & 0x3FF) && L->rebank_to[(e & 0x3FF) >> 3] >> (e & 7) & 1) map[i] = (uint16_t)((e & 0x0FFF) | rb[1] << 12);
	}
}

/* Another game's area: its tile set and colours copied into the core's
 * ROM, where BN6 loads them from the map the layers take over. */
static bool install_gfx(const Learned *L) {
	uint8_t *ts, *pal;
	size_t nts, npal;
	if (!area_src_gfx(L->def->xrom, L->def->group, L->def->number, &ts, &nts, &pal, &npal)) return false;
	uint32_t pal_at = XGFX_AT + (uint32_t)nts;
	bool ok = pal_at + npal <= XGFX_AT + XGFX_SIZE;
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

static bool write_tilemap(const Learned *L) {
	/* extent of the floor around the world origin */
	int umax = 0, vmax = 0;
	for (int y = 0; y < cur->gh; ++y)
		for (int x = 0; x < cur->gw; ++x) {
			if (!cur->cell[y * cur->gw + x]) continue;
			int X, Y;
			netmap_world(x, y, &X, &Y);
			int u = abs(X + Y), v = abs((Y - X) / 2);
			if (u > umax) umax = u;
			if (v > vmax) vmax = v;
		}
	int tw = 2 * ((umax + 64 + 7) / 8), th = 2 * ((vmax + 48 + cur->rise + 7) / 8);
	if ((tw & 1) != (L->tw & 1)) ++tw;   /* same tile phase as the source */
	if ((th & 1) != (L->th & 1)) ++th;
	if (tw > 255 || th > 255 || (size_t)tw * th * 4 > BN6_TILEMAP_MAX) return false;
	size_t cells = (size_t)tw * th;
	uint16_t *map = calloc(cells * 2, 2);
	TileGrid grid = { tw, th, place.ex, place.ey, L->book[0].dv, L->book[0].face, L->book[0].hang, by_shape || rimmed, apart != NET_APART_NONE };
	free(last.seams);
	last.seams = calloc(cells, 1);
	free(last.pasted);
	last.pasted = calloc(cells, 1);
	memo_w = cur->gw + 2 * MEMO_EDGE;
	memo_h = cur->gh + 2 * MEMO_EDGE;
	floor_memo = malloc((size_t)memo_w * memo_h * sizeof *floor_memo);
	if (floor_memo) memset(floor_memo, 0xFF, (size_t)memo_w * memo_h * sizeof *floor_memo);
	tilemap_pick(L->book, L->nbooks, &L->seams, &grid, floor_cb, NULL, map, last.seams);
	free(floor_memo);
	floor_memo = NULL;
	size_t raw = cells * 4;
	uint8_t *out = malloc(16 + raw + raw / 8 + 16);
	paste_stairs(L, map, tw, th);
	paste_props(map, tw, th);
	paste_pads(L, map, tw, th);
	paste_ornaments(L, map, tw, th);
	paste_emblems(L, map, tw, th);
	paste_bushes(L, map, tw, th);
	/* how the classes' picks went, where they are drawn: not under what
	 * was set whole over them */
	for (size_t i = 0; i < cells; ++i) {
		if (last.pasted[i] & (NETMAP_PASTED_PAD | NETMAP_PASTED_STAIR)) continue;
		tiles_stats.seams += (last.seams[i] & 1) + (last.seams[i] >> 1 & 1);
		tiles_stats.off_near += (last.seams[i] >> 2 & 3) == TILE_OFF_NEAR;
		tiles_stats.off_edge += (last.seams[i] >> 2 & 3) == TILE_OFF_EDGE;
		tiles_stats.other += last.seams[i] >> 7;
	}
	rebank_map(L, map, cells);
	netmap_scenery = decor_place(&L->decor, map, tw, th, cur->seed);
	size_t lz = lz_literal((const uint8_t *)map, raw, out + 12);
	out[0] = (uint8_t)tw; out[1] = (uint8_t)th; out[2] = out[3] = 0;
	put32(out + 4, 12);
	put32(out + 8, (uint32_t)(12 + cells * 2)); /* the second layer, in the decompressed buffer */
	emu_write(TILEMAP_AT, out, 12 + lz);
	if (emu_debug_on()) {
		FILE *f = emu_debug_file("gen_tilemap.bin");
		if (f) { fwrite(out, 1, 12, f); fwrite(map, 2, cells * 2, f); fclose(f); }
	}
	/* (a host of the same group shares the tile set and colours: set them
	 * anyway; another game's are set with its own copies) */
	if (L->desc != L->src_desc && !L->other)
		for (uint32_t k = 0; k < 8; k += 4) emu_write32(0x08000000u + L->desc + k, emu_read32(0x08000000u + L->src_desc + k));
	emu_write32(0x08000000u + L->desc + 8, TILEMAP_AT);
	if (L->other && !install_gfx(L)) { free(out); return false; }
	free(out);
	free(last.map);
	last.map = map;
	last.tw = tw;
	last.th = th;
	return true;
}

/* the layer's floor at `level` in world panels: its own level and stairs */
static bool floor_level(int A, int B, int level) {
	int k = kind_at(A, B);
	return k == K_STAIR || k == (level ? K_RAISED : K_FLOOR);
}

bool netmap_floor_cell(int cx, int cy, int level) {
	/* the cell centre, relative to the panel edges */
	int X = cx * 8 + 4 - place.ex, Y = cy * 8 + 4 - place.ey;
	int A = floordiv(X, 32), B = floordiv(Y, 32);
	bool onx = (X & 31) == 0, ony = (Y & 31) == 0;
	if (!floor_level(A, B, level)) return false;
	if (onx && !floor_level(A - 1, B, level)) return false;
	if (ony && !floor_level(A, B - 1, level)) return false;
	if (onx && ony && !floor_level(A - 1, B - 1, level)) return false;
	return true;
}

bool netmap_stair_cell(int cx, int cy) {
	int A = floordiv(cx * 8 + 4 - place.ex, 32), B = floordiv(cy * 8 + 4 - place.ey, 32);
	return kind_at(A, B) == K_STAIR;
}

/* Coordinate cells beyond the walls: raised floor heights and the stairs'
 * own ramps, walls and layer priorities, placed at their blocks. */
static CoordExtra extra;
static CoordCell extra_cells[4][4096];
static CoordCell over_cells[1024];   /* walls in place of the floor's own (props') */

static void add_extra(int s, CoordCell c) {
	if (extra.n[s] < 4096) extra_cells[s][extra.n[s]++] = c;
}

/* The row past each stair's top a step, as the originals' is: a cell from
 * the ramp's foot as high as the rise (z 0, height 32), where the raised
 * floor's own cells start at the rise (z 32, height 0). The game takes a
 * floor cell only within its heights, and coming up the ramp MegaMan is
 * under the rise until he has crossed that row: with the raised floor's
 * cells there, he fell to the ground and walked on under the room (issue
 * #22). */
static void stair_steps(const Learned *L) {
	for (int i = 0; i < cur->nstairs; ++i) {
		const StairTemplate *t = &L->stairs[cur->stairs[i].dir];
		if (!t->ok) continue;
		int X0, Y0;
		stair_origin(&cur->stairs[i], &X0, &Y0);
		for (int k = 0; k < t->nramp; ++k) {
			const CoordCell *r = &t->ramp[k];
			if ((r->type != 0x13 && r->type != 0x14) || r->height != t->rise) continue;
			/* (0x13 climbs towards +X, 0x14 towards -Y) */
			CoordCell step = { (int16_t)(r->x + X0 + (r->type == 0x13 ? 8 : 0)), (int16_t)(r->y + Y0 - (r->type == 0x14 ? 8 : 0)), r->z, 0, (uint8_t)t->rise, 0x11 };
			bool found = false;
			for (int c = 0; c < extra.n[1]; ++c) {
				CoordCell *e = &extra_cells[1][c];
				if (e->type != 0x11 || e->x >> 3 != step.x >> 3 || e->y >> 3 != step.y >> 3) continue;
				*e = step;
				found = true;
			}
			if (!found) add_extra(1, step);
		}
	}
}

static void build_extra(const Learned *L) {
	memset(&extra, 0, sizeof extra);
	for (int s = 0; s < 4; ++s) extra.cells[s] = extra_cells[s];
	/* the props' own walls: a counter's ring, whose back row the walls of
	 * its walled-off aisle would duplicate */
	extra.over = over_cells;
	props_place(L);
	for (int i = 0; i < MAX_PROPS; ++i) {
		if (!prop_at[i].ok) continue;
		for (int k = 0; k < prop_at[i].st->nwalls && extra.nover < 1024; ++k) {
			CoordCell c = prop_at[i].st->walls[k];
			c.x = (int16_t)(c.x + prop_at[i].X);
			c.y = (int16_t)(c.y + prop_at[i].Y);
			over_cells[extra.nover++] = c;
		}
		/* (and its layer priorities, as a stair's are laid) */
		for (int k = 0; k < prop_at[i].st->nprio; ++k) {
			CoordCell c = prop_at[i].st->prio[k];
			c.x = (int16_t)(c.x + prop_at[i].X);
			c.y = (int16_t)(c.y + prop_at[i].Y);
			add_extra(2, c);
		}
	}
	for (int y = 0; y < cur->gh; ++y)
		for (int x = 0; x < cur->gw; ++x) {
			if (kind(x, y) != K_RAISED) continue;
			int A, B;
			grid_to_panel(x, y, &A, &B);
			int X0 = place.ex + 32 * A, Y0 = place.ey + 32 * B;
			/* the 8 x 8 cells whose centres lie on the panel */
			for (int cy = floordiv(Y0 - 4 + 7, 8); cy * 8 + 4 < Y0 + 32; ++cy)
				for (int cx = floordiv(X0 - 4 + 7, 8); cx * 8 + 4 < X0 + 32; ++cx) {
					CoordCell c = { (int16_t)(cx * 8), (int16_t)(cy * 8), (int8_t)cur->rise, 0, 0, 0x11 };
					add_extra(1, c);
				}
		}
	for (int i = 0; i < cur->nstairs; ++i) {
		const StairTemplate *t = &L->stairs[cur->stairs[i].dir];
		if (!t->ok) continue;
		int X0, Y0;
		stair_origin(&cur->stairs[i], &X0, &Y0);
		const CoordCell *src[3] = { t->walls, t->ramp, t->prio };
		int n[3] = { t->nwalls, t->nramp, t->nprio };
		for (int s = 0; s < 3; ++s)
			for (int k = 0; k < n[s]; ++k) {
				CoordCell c = src[s][k];
				c.x = (int16_t)(c.x + X0);
				c.y = (int16_t)(c.y + Y0);
				add_extra(s, c);
			}
	}
	stair_steps(L);
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
	place.gx0 = (cu + cv) / 2;
	place.gy0 = (cv - cu) / 2;
	return true;
}

/* ---- a floor the tiles can draw ---- */

#define LEGAL_BUDGET 400   /* cells changed at most */

/* Whether the panel of grid cell (x, y) has a neighbourhood the original
 * maps show. */
static bool legal_clean(int x, int y, void *ctx) {
	const Learned *L = ctx;
	int A, B;
	grid_to_panel(x, y, &A, &B);
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

bool netmap_build(int area, const NetLayout *lay) {
	const NetAreaDef *na = net_area_def(area);
	if (!na) return false;
	Learned *L = &learned[area];
	if (!L->tried) { L->tried = true; L->ok = learn(area, L); }
	if (!L->ok) return false;
	cur = lay;
	one_floor = !na->walk_styles;
	by_shape = na->styles & TILES_BY_SHAPE;
	arena_b = !by_shape && !(na->styles & TILES_ARENA_FLOOR);
	rimmed = na->styles & TILES_RIMMED;
	pad_look = !(na->styles & TILES_NO_PAD_LOOK);
	pads_walkway = L->pad.ok && (na->pad_hues & na->walk_styles);
	apart = na->apart;
	place.ex = L->ex;
	place.ey = L->ey;
	if (!centre(lay)) return false;
	netmap_legal = (LegalStats){ 0, 0 };
	/* (the stripes as the floor is, and again as the legalizer left it) */
	striped = na->styles & TILES_CROSSING;
	bool keep_pads = pad_look && L->pads_seen;
	if (striped) make_stripes(keep_pads);
	if (lay->locked) {
		legalize(L, lay);
		centre(lay);
	}
	if (striped) make_stripes(keep_pads);
	coord_slot = L->coord_slot;
	build_extra(L);
	return write_tilemap(L) && coords_write(coord_slot, NULL, 0, &extra);
}

bool netmap_set_pads(const CoordPad *pads, int n) { return coords_write(coord_slot, pads, n, &extra); }

/* The stairs area `area` can draw (bit per STAIR_UP_*) and their rise. */
static unsigned netmap_stair_dirs(int area, int *rise) {
	if (!net_area_def(area)) return 0;
	Learned *L = &learned[area];
	if (!L->tried) { L->tried = true; L->ok = learn(area, L); }
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
	kit->emblem = learned[area].emblem.ok;
}

/* Locks the w x h cells from (x, y) and `margin` around them. */
static void lock(uint8_t locked[MAP_H][MAP_W], int x, int y, int w, int h, int margin) {
	for (int j = y - margin; j < y + h + margin; ++j)
		for (int i = x - margin; i < x + w + margin; ++i)
			if (i >= 0 && j >= 0 && i < MAP_W && j < MAP_H) locked[j][i] = 1;
}

bool netmap_build_layer(int area, uint32_t seed) {
	/* the pads, in their own look (and where the area's platforms are pads,
	 * the small platforms and the guardian's arena: the Aquarium's water
	 * never widens into one) */
	static uint8_t pads[MAP_H][MAP_W];
	memset(pads, 0, sizeof pads);
	for (int r = 0; r < layer.nrooms; ++r) {
		const Room *m = &layer.rooms[r];
		int small = net_area_def(area) ? net_area_def(area)->pad_rooms : 0;
		bool pad = m->kind == ROOM_PAD || (small && ((m->kind == ROOM_PLATFORM && m->w * m->h <= small) || r == layer.arena));
		if (!pad) continue;
		for (int y = m->y; y < m->y + m->h; ++y)
			for (int x = m->x; x < m->x + m->w; ++x) pads[y][x] = 1;
	}
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
		layer.props, layer.nprops };
	if (layer.arena >= 0) {
		const Room *a = &layer.rooms[layer.arena];
		lay.ax = a->x; lay.ay = a->y; lay.aw = a->w; lay.ah = a->h;
	}
	return netmap_build(area, &lay);
}

const uint16_t *netmap_last_tiles(int *tw, int *th) {
	*tw = last.tw;
	*th = last.th;
	return last.map;
}

const uint8_t *netmap_last_seams(void) { return last.seams; }

const uint8_t *netmap_last_pasted(void) { return last.pasted; }

void netmap_last_cells(char out[MAP_H][MAP_W + 1]) {
	for (int y = 0; y < MAP_H; ++y) {
		for (int x = 0; x < MAP_W; ++x) {
			int A, B;
			grid_to_panel(x, y, &A, &B);
			int m = cur ? floor_cb(A, B, NULL) : TILE_VOID;
			char c = ".ab"[TILE_MATERIAL(m)];
			if (c == 'b' && striped && stripe[y][x]) c = 's';
			if (m & TILE_PAD) c = 'p';
			out[y][x] = c;
		}
		out[y][MAP_W] = 0;
	}
	if (!last.seams) return;
	TileGrid grid = { last.tw, last.th, place.ex, place.ey, 0, 0, 0, false, false, false, 0 };
	for (int ty = 0; ty < last.th; ++ty)
		for (int tx = 0; tx < last.tw; ++tx) {
			if ((last.seams[ty * last.tw + tx] >> 2 & 3) != TILE_OFF_NEAR || last.pasted[ty * last.tw + tx] & (NETMAP_PASTED_PAD | NETMAP_PASTED_STAIR)) continue;
			int phase, A, B;
			tile_class(&grid, tx, ty, &phase, &A, &B);
			int x = B + place.gx0, y = -A + place.gy0;
			if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) continue;
			char *c = &out[y][x];
			*c = *c == '.' ? '*' : *c >= 'a' ? (char)(*c - 'a' + 'A') : *c;
		}
}
