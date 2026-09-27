/* A whole tile map: every tile's pair in reading order, each knowing the
 * tiles picked before it; then again where a tile meets its neighbours badly
 * (tiles_trouble), now knowing all four, with the neighbours it meets badly,
 * until none changes. Where the pads stand apart, each pad and the rest of
 * the floor are picked so, each as if the others were not there, and laid
 * over each other on the two tile layers. */
#include "tilemap.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define PASSES 3

/* The tiles picked so far: how each looks and what it draws. */
typedef struct {
	const TileBook *books;
	int nbooks;
	const TileSeams *seams;
	const TileGrid *g;
	TileFloor floor;
	const void *ctx;
	uint16_t *map;
	uint32_t *look;
	uint64_t *mask;
	bool *on;       /* on the floor: has a pair */
} Picked;

/* Tile (tx, ty)'s four neighbours: left, above, right, below. */
static void neighbours(const Picked *p, int tx, int ty, TileNeighbours *n) {
	static const int d[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	for (int k = 0; k < 4; ++k) {
		int x = tx + d[k][0], y = ty + d[k][1];
		bool off = x < 0 || y < 0 || x >= p->g->tw || y >= p->g->th;
		n->look[k] = off ? SEAM_VOID : p->look[y * p->g->tw + x];
		n->mask[k] = off ? 0 : p->mask[y * p->g->tw + x];
	}
}

static bool pick_one(Picked *p, int tx, int ty) {
	size_t i = (size_t)ty * p->g->tw + tx, cells = (size_t)p->g->tw * p->g->th;
	TileNeighbours n;
	neighbours(p, tx, ty, &n);
	p->look[i] = SEAM_VOID;
	p->mask[i] = 0;
	return tiles_pick(p->books, p->nbooks, p->g, tx, ty, p->floor, p->ctx, p->seams, &n,
		&p->map[i], &p->map[cells + i], &p->look[i], &p->mask[i]);
}

/* The sides (bit k: left, above, right, below) where tile (tx, ty) meets its
 * neighbour badly. */
static unsigned trouble(const Picked *p, int tx, int ty) {
	size_t i = (size_t)ty * p->g->tw + tx;
	TileNeighbours n;
	neighbours(p, tx, ty, &n);
	unsigned sides = 0;
	for (int k = 0; k < 4; ++k) {
		TileNeighbours one = { { SEAM_ANY, SEAM_ANY, SEAM_ANY, SEAM_ANY }, { 0 } };
		one.look[k] = n.look[k];
		one.mask[k] = n.mask[k];
		if (tiles_trouble(p->seams, p->look[i], p->mask[i], &one)) sides |= 1u << k;
	}
	return sides;
}

static void pick_all(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g,
	TileFloor floor, const void *ctx, uint16_t *map, uint8_t *left) {
	int tw = g->tw, th = g->th;
	size_t cells = (size_t)tw * th;
	Picked p = { books, nbooks, seams, g, floor, ctx, map,
		malloc(cells * sizeof *p.look), calloc(cells, sizeof *p.mask), calloc(cells, 1) };
	memset(map, 0, cells * 2 * sizeof *map);   /* (off the floor a tile keeps nothing) */
	for (size_t i = 0; i < cells; ++i) p.look[i] = SEAM_ANY;
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) p.on[ty * tw + tx] = pick_one(&p, tx, ty);
	TileStats first = tiles_stats;   /* (the passes below would count twice) */
	/* a tile is looked at again once a neighbour has changed */
	uint8_t *dirty = malloc(cells);
	memset(dirty, 1, cells);
	static const int d[5][2] = { { 0, 0 }, { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	for (int pass = 0; pass < PASSES; ++pass) {
		int changed = 0;
		for (int ty = 0; ty < th; ++ty)
			for (int tx = 0; tx < tw; ++tx) {
				size_t i = (size_t)ty * tw + tx;
				if (!dirty[i] || !p.on[i]) continue;
				dirty[i] = 0;
				unsigned sides = trouble(&p, tx, ty);
				/* the tile, then the neighbours it meets badly */
				for (int k = 0; sides && k < 5; ++k) {
					if (k && !(sides >> (k - 1) & 1)) continue;
					int x = tx + d[k][0], y = ty + d[k][1];
					size_t j = (size_t)y * tw + x;
					if (x < 0 || y < 0 || x >= tw || y >= th || !p.on[j]) continue;
					uint32_t was = p.look[j];
					pick_one(&p, x, y);
					if (p.look[j] == was) continue;
					++changed;
					for (int m = 1; m < 5; ++m) {
						int u = x + d[m][0], v = y + d[m][1];
						if (u >= 0 && v >= 0 && u < tw && v < th) dirty[(size_t)v * tw + u] = 1;
					}
				}
			}
		if (!changed) break;
	}
	tiles_stats = first;
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) {
			unsigned sides = trouble(&p, tx, ty);
			left[ty * tw + tx] = (uint8_t)((sides >> 2 & 1) | (sides >> 3 & 1) << 1);
		}
	free(dirty);
	free(p.look);
	free(p.mask);
	free(p.on);
}

/* Where the pads stand apart, the floor in pieces: each pad (its panels
 * joined side by side) and the rest of the floor. `label` covers the
 * panels the map shows: 0 off the floor, 1 the rest, 2 and up the pads. */
typedef struct {
	TileFloor floor;
	const void *ctx;
	int A0, B0, w, h;
	uint8_t *label;
	int lo, hi;   /* the pieces a view shows */
} Pieces;

#define REST 1
#define FIRST_PAD 2

static int piece(const Pieces *p, int A, int B) {
	if (A < p->A0 || B < p->B0 || A >= p->A0 + p->w || B >= p->B0 + p->h) return 0;
	return p->label[(size_t)(B - p->B0) * p->w + A - p->A0];
}

/* The floor of pieces lo..hi alone, the rest taken for void. */
static int pieces_floor(int A, int B, const void *ctx) {
	const Pieces *p = ctx;
	int l = piece(p, A, B);
	return l && l >= p->lo && l <= p->hi ? p->floor(A, B, p->ctx) : TILE_VOID;
}

/* Labels the floor's pieces; how many pads it found. */
static int label_pieces(const TileGrid *g, Pieces *p) {
	int phase, A, B, A1 = INT_MIN, B1 = INT_MIN;
	p->A0 = p->B0 = INT_MAX;
	for (int k = 0; k < 4; ++k) {   /* the map's corners reach furthest */
		tile_class(g, k & 1 ? g->tw - 1 : 0, k & 2 ? g->th - 1 : 0, &phase, &A, &B);
		if (A < p->A0) p->A0 = A;
		if (B < p->B0) p->B0 = B;
		if (A > A1) A1 = A;
		if (B > B1) B1 = B;
	}
	p->A0 -= 1; p->B0 -= 1;   /* (a tile looks a panel further) */
	p->w = A1 + 2 - p->A0;
	p->h = B1 + 2 - p->B0;
	size_t n = (size_t)p->w * p->h;
	p->label = calloc(n, 1);
	int *queue = malloc(n * sizeof *queue), pads = 0;
	for (size_t i = 0; i < n; ++i) {
		int m = p->floor(p->A0 + (int)(i % p->w), p->B0 + (int)(i / p->w), p->ctx);
		if (TILE_MATERIAL(m)) p->label[i] = m & TILE_PAD ? 255 : REST;
	}
	for (size_t i = 0; i < n; ++i) {
		if (p->label[i] != 255) continue;
		/* (past 253 pads, the last takes them all) */
		uint8_t l = (uint8_t)(FIRST_PAD + (pads < 253 ? pads++ : pads - 1));
		int head = 0, tail = 0;
		queue[tail++] = (int)i;
		p->label[i] = l;
		while (head < tail) {
			int c = queue[head++], x = c % p->w, y = c / p->w;
			static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
			for (int k = 0; k < 4; ++k) {
				int nx = x + d[k][0], ny = y + d[k][1];
				if (nx < 0 || ny < 0 || nx >= p->w || ny >= p->h || p->label[ny * p->w + nx] != 255) continue;
				p->label[ny * p->w + nx] = l;
				queue[tail++] = ny * p->w + nx;
			}
		}
	}
	free(queue);
	return pads;
}

static bool drawn(uint16_t e) { return e & 0x3FF; }

/* The entry of a tile's pair the game shows in front: the second layer's
 * (BG2 at priority 2, over the first's BG1 at priority 3). */
static uint16_t front_of(uint16_t e0, uint16_t e1) { return drawn(e1) ? e1 : e0; }

void tilemap_pick(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g,
	TileFloor floor, const void *ctx, uint16_t *map, uint8_t *left) {
	if (!g->pads_apart) {
		pick_all(books, nbooks, seams, g, floor, ctx, map, left);
		return;
	}
	/* Each pad as if no other floor were there, then the other floor as if
	 * there were no pads, laid over each other: where two draw a tile, the
	 * one whose top covers the other's side faces on the second layer, which
	 * the game shows in front. A pad keeps its whole rim, and a walkway
	 * ends at it. */
	Pieces all = { floor, ctx, 0, 0, 0, 0, NULL, 0, 0 };
	int pads = label_pieces(g, &all);
	size_t cells = (size_t)g->tw * g->th;
	uint16_t *one = calloc(cells * 2, sizeof *one);
	uint8_t *one_left = calloc(cells, 1), *stacked = calloc(cells, 1);
	memset(map, 0, cells * 2 * sizeof *map);
	memset(left, 0, cells);
	/* (each piece in one layer's tiles: two can always share a tile, and
	 * none brings what the original drew over its floor on the other) */
	TileGrid one_layer = *g;
	one_layer.single = true;
	Pieces laid = all, view = all;   /* the pieces laid so far, the next one */
	laid.lo = FIRST_PAD; laid.hi = FIRST_PAD - 1;
	for (int k = 0; k <= pads; ++k) {
		int l = k < pads ? FIRST_PAD + k : REST;
		view.lo = view.hi = l;
		pick_all(books, nbooks, seams, &one_layer, pieces_floor, &view, one, one_left);
		for (int ty = 0; ty < g->th; ++ty)
			for (int tx = 0; tx < g->tw; ++tx) {
				size_t i = (size_t)ty * g->tw + tx;
				uint16_t *e0 = &map[i], *e1 = &map[cells + i], n0 = one[i], n1 = one[cells + i];
				left[i] |= one_left[i];
				if (!drawn(n0) && !drawn(n1)) continue;
				if (!drawn(*e0) && !drawn(*e1)) { *e0 = n0; *e1 = n1; continue; }
				/* (a tile holds two pieces: of three, the one found behind
				 * goes) */
				uint16_t mine = front_of(n0, n1), theirs = front_of(*e0, *e1);
				if (tiles_in_front(g, tx, ty, pieces_floor, &view, pieces_floor, &laid)) { *e0 = theirs; *e1 = mine; }
				else if (!drawn(*e0) || !drawn(*e1)) { *e0 = mine; *e1 = theirs; }
				stacked[i] = 1;
			}
		if (l != REST) laid.hi = l;
		else laid.lo = REST;
	}
	/* two tiles over each other: no original to compare them with */
	for (int ty = 0; ty < g->th; ++ty)
		for (int tx = 0; tx < g->tw; ++tx) {
			size_t i = (size_t)ty * g->tw + tx;
			if (stacked[i] || (tx + 1 < g->tw && stacked[i + 1])) left[i] &= (uint8_t)~1u;
			if (stacked[i] || (ty + 1 < g->th && stacked[i + g->tw])) left[i] &= (uint8_t)~2u;
		}
	free(one);
	free(one_left);
	free(stacked);
	free(all.label);
}
