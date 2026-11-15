/* The floor's holes, found in a finished tile map and mended (voids.h).
 * A tile's entries draw the pixels their tile set's tiles draw (colour 0
 * none), so what the map shows of the floor's top is known without
 * drawing it: each tile's two entries are two lookups. Only a tile that
 * draws less than all of its pixels and lies by the floor is looked at
 * closer, and only one with a hole is mended: a map's pass costs a few
 * lookups a tile, its holes a look through their phase's pairs each. */
#include "voids.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "tiles_parts.h"

bool voids_on = true, voids_measure;

static uint8_t *last;
static int last_tw, last_th;

const uint8_t *voids_last(int *tw, int *th) {
	*tw = last_tw;
	*th = last_th;
	return last;
}

const uint64_t *voids_opaque(int rom, int group, int number) {
	static uint64_t kept[VOID_TILES];
	static int krom = -1, kgroup, knumber;
	if (krom == rom && kgroup == group && knumber == number) return kept;
	krom = -1;
	if (!area_src_opaque(rom, group, number, kept)) return NULL;
	krom = rom;
	kgroup = group;
	knumber = number;
	return kept;
}

/* A tile's pixels mirrored left to right (each row's byte reversed), and
 * top to bottom (the rows in reverse). */
static uint64_t flip_x(uint64_t m) {
	m = (m & 0xF0F0F0F0F0F0F0F0ull) >> 4 | (m & 0x0F0F0F0F0F0F0F0Full) << 4;
	m = (m & 0xCCCCCCCCCCCCCCCCull) >> 2 | (m & 0x3333333333333333ull) << 2;
	return (m & 0xAAAAAAAAAAAAAAAAull) >> 1 | (m & 0x5555555555555555ull) << 1;
}

static uint64_t flip_y(uint64_t m) {
	uint64_t r = 0;
	for (int y = 0; y < 8; ++y) r |= (m >> (8 * y) & 0xFF) << (8 * (7 - y));
	return r;
}

uint64_t voids_entry_px(const uint64_t *opaque, uint16_t e) {
	if (!(e & 0x3FF)) return 0;
	uint64_t m = opaque[e & 0x3FF];
	if (e & 0x400) m = flip_x(m);
	return e & 0x800 ? flip_y(m) : m;
}

/* The floor as the net reads it, its materials by panel asked once each:
 * a tile's top asks after a hundred pixels, most of them in the same few
 * panels. */
typedef struct {
	const VoidFloor *f;
	const TileGrid *g;
	TileFloor floor;
	const void *ctx;
	int A0, B0, w, h;
	int8_t *mat;   /* (-1: not asked yet) */
} Panels;

static int mat_at(Panels *p, int A, int B) {
	unsigned x = (unsigned)(A - p->A0), y = (unsigned)(B - p->B0);
	if (!p->mat || x >= (unsigned)p->w || y >= (unsigned)p->h) return TILE_MATERIAL(p->floor(A, B, p->ctx));
	int8_t *m = &p->mat[y * (unsigned)p->w + x];
	if (*m < 0) *m = (int8_t)TILE_MATERIAL(p->floor(A, B, p->ctx));
	return *m;
}

static bool panels_open(Panels *p) {
	int phase, A, B, A1 = INT_MIN, B1 = INT_MIN;
	p->A0 = p->B0 = INT_MAX;
	for (int k = 0; k < 4; ++k) {   /* (the map's corners reach furthest) */
		tile_class(p->g, k & 1 ? p->g->tw - 1 : 0, k & 2 ? p->g->th - 1 : 0, &phase, &A, &B);
		if (A < p->A0) p->A0 = A;
		if (B < p->B0) p->B0 = B;
		if (A > A1) A1 = A;
		if (B > B1) B1 = B;
	}
	p->A0 -= 2;
	p->B0 -= 2;
	p->w = A1 + 3 - p->A0;
	p->h = B1 + 3 - p->B0;
	p->mat = malloc((size_t)p->w * p->h);
	if (p->mat) memset(p->mat, 0xFF, (size_t)p->w * p->h);
	return p->mat != NULL;
}

/* Whether any of the 3 x 3 panels round tile (tx, ty)'s is floor. */
static bool by_floor(Panels *p, int tx, int ty) {
	int phase, A, B;
	tile_class(p->g, tx, ty, &phase, &A, &B);
	for (int k = 0; k < 9; ++k)
		if (mat_at(p, A + k % 3 - 1, B + k / 3 - 1)) return true;
	return false;
}

/* The tile's top: its pixels on the floor with floor IN_X pixels to either
 * side and IN_Y above and below. The floor's outline is not its top: a
 * tile's own edge may fall short of it, and a rim's corner be cut off or
 * notched (Robot Control's platforms' corners, a red stud in a notch). */
#define IN_X 4
#define IN_Y 2
static uint64_t top_of(Panels *p, int tx, int ty) {
	bool on[8 + 2 * IN_Y][8 + 2 * IN_X];
	for (int y = 0; y < 8 + 2 * IN_Y; ++y)
		for (int x = 0; x < 8 + 2 * IN_X; ++x) {
			int A, B;
			tl_pixel_panel(p->g, tx * 8 - IN_X + x, ty * 8 - IN_Y + y, &A, &B);
			on[y][x] = mat_at(p, A, B) != 0;
		}
	uint64_t m = 0;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x)
			if (on[y + IN_Y][x + IN_X] && on[y + IN_Y][x] && on[y + IN_Y][x + 2 * IN_X] && on[y][x + IN_X] && on[y + 2 * IN_Y][x + IN_X])
				m |= 1ull << (y * 8 + x);
	return m;
}

/* A hole's tile as it stands, and what mends it best so far. */
typedef struct {
	const uint64_t *opaque;
	uint64_t top, drawn, never;
	uint16_t back, front;   /* its entries: the front one, and the one behind it (0 none) */
	unsigned oa, ob;        /* the floors of the panels round its own (tl_occupancy) */
	uint16_t e0, e1;        /* the best mend found */
	int cost;
} Hole;

/* How far a pair's class lies from the tile's own neighbourhood: each panel
 * round it with floor where the other has none two, of the other floor one,
 * the tile's own panel twice that. */
static int class_distance(uint32_t key, unsigned oa, unsigned ob) {
	unsigned ka = KEY_A(key), kb = KEY_B(key), floors = ((ka | kb) ^ (oa | ob)) & 0x1FF, other = (ka ^ oa) & (ka | kb) & (oa | ob) & 0x1FF;
	int d = 2 * tl_popcount64(floors) + tl_popcount64(other);
	return d + (floors & 0x10 ? 4 : other & 0x10 ? 2 : 0);
}

/* The tile as entries (back, front), at `cost` and EXTRA_COST for each
 * pixel it shows past the hole where the tile showed nothing: kept where
 * it leaves no hole, draws nothing more where the floor must not be, and
 * costs less. (A rim's tile laid behind Robot Control's stud stood out
 * beside it, where the platform's own corner without one closes it.) */
#define EXTRA_COST 2
static void consider(Hole *h, uint16_t back, uint16_t front, int cost) {
	if (cost >= h->cost) return;
	uint64_t drawn = voids_entry_px(h->opaque, back) | voids_entry_px(h->opaque, front);
	if (tl_popcount64(h->top & ~drawn) > VOID_SLACK) return;
	if (tl_popcount64(drawn & ~h->drawn & h->never) > SLACK) return;
	cost += EXTRA_COST * tl_popcount64(drawn & ~h->drawn & ~h->top);
	if (cost >= h->cost) return;
	h->e0 = back;
	h->e1 = front;
	h->cost = cost;
}

/* What a mend costs beside its class's distance (four a step): a tile of
 * the books laid behind the tile's own nothing more, but the pixels of the
 * one it puts aside that showed; a pair laid in its stead, all of its own
 * art gone, more. */
#define STEP_COST 4
#define REPLACE_COST 12

/* The pairs of `books` at the tile's phase, each as a tile laid behind its
 * front one where it is one tile, and as the tile's pair in its stead. */
static void try_books(Hole *h, const TileBook *books, int nbooks, int phase) {
	int aside = tl_popcount64(voids_entry_px(h->opaque, h->back) & ~voids_entry_px(h->opaque, h->front));
	for (int k = 0; k < nbooks; ++k) {
		const TileBook *b = &books[k];
		for (int i = tl_first_of(b, KEY(phase, 0, 0)); i < b->n && KEY_PHASE(b->cand[i].key) == phase; ++i) {
			const TileCand *c = &b->cand[i];
			int d = STEP_COST * class_distance(c->key, h->oa, h->ob);
			bool d0 = (c->e0 & 0x3FF) != 0, d1 = (c->e1 & 0x3FF) != 0;
			if (d0 != d1) consider(h, d0 ? c->e0 : c->e1, h->front, d + aside);
			if (d0 || d1) consider(h, c->e0, c->e1, d + REPLACE_COST);
		}
	}
}

/* Whether the area's own maps leave the hole undrawn where they set a tile
 * in the tile's very neighbourhood (its class): a pair of `books` at that
 * key drawing at most a few of its pixels. Then it is their art, no hole:
 * Robot Control's platforms set a red stud in a notch at each corner. */
static bool art_of(const TileBook *books, int nbooks, uint32_t key, uint64_t hole) {
	for (int k = 0; k < nbooks; ++k) {
		const TileBook *b = &books[k];
		for (int i = tl_first_of(b, key); i < b->n && b->cand[i].key == key; ++i)
			if (tl_popcount64(hole & b->cand[i].mask) <= SLACK) return true;
	}
	return false;
}

/* Whether tile (tx, ty)'s undrawn pixels of its top, `hole`, are their art
 * (art_of, in the books of the floor under it). */
static bool hole_is_art(Panels *p, int tx, int ty, uint64_t hole) {
	int phase, A, B, HA, HB, first = __builtin_ctzll(hole);
	unsigned oa, ob;
	tile_class(p->g, tx, ty, &phase, &A, &B);
	tl_occupancy(p->floor, p->ctx, A, B, &oa, &ob);
	tl_pixel_panel(p->g, tx * 8 + first % 8, ty * 8 + first / 8, &HA, &HB);
	bool road = p->floor(HA, HB, p->ctx) & TILE_ROAD && p->g->nroads;
	return art_of(road ? p->g->roads : p->f->books, road ? p->g->nroads : p->f->nbooks, KEY(phase, oa, ob), hole);
}

/* Mends tile (tx, ty), whose entries are *e0 and *e1, with hole pixels
 * `top` & ~`drawn`: false where nothing the books hold covers it. */
static bool mend(Panels *p, int tx, int ty, uint64_t top, uint64_t drawn, uint16_t *e0, uint16_t *e1) {
	const TileGrid *g = p->g;
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	Hole h = { p->f->opaque, top, drawn, 0, (*e1 & 0x3FF) ? *e0 : 0, (*e1 & 0x3FF) ? *e1 : *e0, 0, 0, 0, 0, INT_MAX };
	tl_occupancy(p->floor, p->ctx, A, B, &h.oa, &h.ob);
	/* (the floor under the hole: its material, and whether a road's) */
	int first = __builtin_ctzll(top & ~drawn), HA, HB;
	tl_pixel_panel(g, tx * 8 + first % 8, ty * 8 + first / 8, &HA, &HB);
	int under = p->floor(HA, HB, p->ctx);
	uint64_t must, deep;
	tl_expect(g, tx, ty, p->floor, p->ctx, TILE_MATERIAL(under) ? TILE_MATERIAL(under) : TILE_A, &must, &h.never, &deep);
	if (under & TILE_ROAD && g->nroads) try_books(&h, g->roads, g->nroads, phase);
	if (h.cost == INT_MAX) try_books(&h, p->f->books, p->f->nbooks, phase);
	if (h.cost == INT_MAX) return false;
	*e0 = h.e0;
	*e1 = h.e1;
	return true;
}

int voids_mend(const VoidFloor *f, uint16_t *map, int *left) {
	const TileGrid *g = f->g;
	const uint64_t *opaque = f->opaque;
	size_t cells = (size_t)g->tw * g->th;
	*left = 0;
	free(last);
	last = voids_measure ? calloc(cells, 1) : NULL;   /* (the atlas's marks: a game never asks after them) */
	last_tw = g->tw;
	last_th = g->th;
	Panels p = { f, g, f->floor, f->ctx, 0, 0, 0, 0, NULL };
	if (!opaque || !panels_open(&p)) { free(p.mat); return 0; }
	int mended = 0;
	for (int ty = 0; ty < g->th; ++ty)
		for (int tx = 0; tx < g->tw; ++tx) {
			size_t i = (size_t)ty * g->tw + tx;
			uint64_t drawn = voids_entry_px(opaque, map[i]) | voids_entry_px(opaque, map[cells + i]);
			if (drawn == ~0ull || !by_floor(&p, tx, ty)) continue;
			uint64_t top = top_of(&p, tx, ty);
			if (tl_popcount64(top & ~drawn) <= VOID_SLACK || hole_is_art(&p, tx, ty, top & ~drawn)) continue;
			if (last) last[i] = VOID_FOUND;
			if (voids_on && mend(&p, tx, ty, top, drawn, &map[i], &map[cells + i])) {
				++mended;
				if (last) last[i] |= VOID_MENDED;
			} else ++*left;
		}
	free(p.mat);
	return mended;
}
