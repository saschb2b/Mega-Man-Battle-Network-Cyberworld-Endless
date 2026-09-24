/* A whole tile map: every tile's pair in reading order, each knowing the
 * tiles picked before it; then again where a tile meets its neighbours badly
 * (tiles_trouble), now knowing all four, with the neighbours it meets badly,
 * until none changes. */
#include "tilemap.h"

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

void tilemap_pick(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g,
	TileFloor floor, const void *ctx, uint16_t *map, uint8_t *left) {
	int tw = g->tw, th = g->th;
	size_t cells = (size_t)tw * th;
	Picked p = { books, nbooks, seams, g, floor, ctx, map,
		malloc(cells * sizeof *p.look), calloc(cells, sizeof *p.mask), calloc(cells, 1) };
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
