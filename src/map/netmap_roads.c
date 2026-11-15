/* The way's walkways drawn as roads on a generated map (netmap.c;
 * docs/world/level-design/roads.md): the way from the layer's arrival to
 * its exit marked again over its floor as the legalizer left it, and the
 * runs of it too short to draw as road left walkway. */
#include "netmap_roads.h"

#include <stdint.h>
#include <string.h>

#include "netmap_parts.h"

/* The way over the floor as it now stands: the generator's way
 * (NetLayout.way) walked again between its two ends, as the legalizer
 * takes cells of it (of a way widened to two, the lane it ran on); the
 * shortest walk, and of those the one that keeps most to the generator's. */
static uint8_t road[MAP_H][MAP_W];

#define STEP 64   /* a step's cost, one off the generator's way 1 more: the shortest first */

static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

static bool on_way(int x, int y) { return nm_cur->way[y * nm_cur->gw + x]; }

/* The generator's way's two ends still floor (cells as y * MAP_W + x); false none. */
static bool way_ends(int gw, int gh, int end[2]) {
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

/* Whether cell c (y * MAP_W + x) of the way is a walkway on the ground, drawn as road. */
static bool road_cell(int c) {
	int x = c % MAP_W, y = c / MAP_W, k = nm_kind(x, y);
	return (k == K_FLOOR || k == K_SOLID) && nm_walkway(x, y);
}

/* The bit of a panel's neighbourhood (TILE_SHAPE) for the grid cell (dx,
 * dy) from it: panel (A - dy, B + dx), nm_grid_to_panel. */
static int shape_bit(int dx, int dy) { return (dx + 1) * 3 + 1 - dy; }

/* Whether the area's roads' art has a bend between the grid cells (ax, ay)
 * and (bx, by) from it: a panel of road with road on those two sides
 * alone, and none at the corner between them, in its map or its mirror
 * image. */
static bool bend_seen(const struct TileBook *books, int nbooks, int ax, int ay, int bx, int by) {
	unsigned need = 1u << 4 | 1u << shape_bit(ax, ay) | 1u << shape_bit(bx, by);
	unsigned none = 1u << shape_bit(-ax, -ay) | 1u << shape_bit(-bx, -by) | 1u << shape_bit(ax + bx, ay + by);
	for (int k = 0; k < nbooks; ++k)
		for (int i = 0; i < books[k].nshapes; ++i) {
			unsigned o = (books[k].shapes[i] >> 10 | books[k].shapes[i] >> 1) & 0x1FFu;
			if ((o & need) == need && !(o & none)) return true;
		}
	return false;
}

/* A road turns only where its map shows the bend: one it lacks was laid in
 * pieces, the void through its inner corner or past its tip (BN4's ACDC
 * Area draws the bends whose inner corner faces the screen's left or
 * right, and not those facing up or down, where a player saw navy
 * squares in the cyan). There the bend's panel is walkway, and the road
 * ends on either side of it, under it (tilemap.c). The way's cells in its
 * order: `path`, n of them. */
static void cut_unseen_bends(const struct TileBook *books, int nbooks, const int16_t *path, int n) {
	for (int i = 1; i + 1 < n; ++i) {
		int x = path[i] % MAP_W, y = path[i] / MAP_W;
		int ax = path[i - 1] % MAP_W - x, ay = path[i - 1] / MAP_W - y, bx = path[i + 1] % MAP_W - x, by = path[i + 1] / MAP_W - y;
		if (ax == -bx && ay == -by) continue;   /* (straight on) */
		if (!road_cell(path[i - 1]) || !road_cell(path[i]) || !road_cell(path[i + 1])) continue;
		if (!bend_seen(books, nbooks, ax, ay, bx, by)) road[y][x] = 0;
	}
}

/* A run of road shorter than ROAD_MIN panels is walkway: what the way left
 * between two stretches of wider floor (a junction widened to a block, a
 * room) was all ends, and drew as a block of the road's hue with no rim,
 * in steps where it met the walkway's floor. A road of three panels
 * between two squares draws whole, its ends under their rims (tilemap.c,
 * runs_under). The way's cells in its order: `path`, n of them. */
#define ROAD_MIN 3
static void drop_short_runs(const int16_t *path, int n) {
	for (int i = 0, j; i < n; i = j + 1) {
		j = i;
		while (j < n && road_cell(path[j]) && road[path[j] / MAP_W][path[j] % MAP_W]) ++j;
		for (int k = i; j - i < ROAD_MIN && k < j; ++k) road[path[k] / MAP_W][path[k] % MAP_W] = 0;
	}
}

void nm_mark_roads(const struct TileBook *books, int nbooks) {
	static int32_t cost[MAP_H][MAP_W];
	static int16_t path[MAP_H * MAP_W];
	int gw = nm_cur->gw < MAP_W ? nm_cur->gw : MAP_W, gh = nm_cur->gh < MAP_H ? nm_cur->gh : MAP_H, end[2], n = 0;
	memset(road, 0, sizeof road);
	if (!nm_cur->way || nbooks <= 0 || !way_ends(gw, gh, end)) return;
	walk_costs(gw, gh, end[1], cost);
	/* (back from the other end along falling cost: the first way found) */
	for (int c = end[0], next; c >= 0 && cost[c / MAP_W][c % MAP_W] != INT32_MAX && n < MAP_H * MAP_W; c = next) {
		int x = c % MAP_W, y = c / MAP_W;
		road[y][x] = 1;
		path[n++] = (int16_t)c;
		next = -1;
		for (int d = 0; d < 4 && next < 0; ++d) {
			int nx = x + d4[d][0], ny = y + d4[d][1];
			if (nx < 0 || ny < 0 || nx >= gw || ny >= gh || cost[ny][nx] == INT32_MAX) continue;
			if (cost[ny][nx] + STEP + !on_way(x, y) == cost[y][x]) next = ny * MAP_W + nx;
		}
	}
	cut_unseen_bends(books, nbooks, path, n);
	drop_short_runs(path, n);
}

bool nm_road(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && road[y][x]; }
