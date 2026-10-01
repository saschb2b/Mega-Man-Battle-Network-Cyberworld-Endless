/* The way across a layer (docs/LEVEL_DESIGN.md, Navigation). MegaMan walks
 * as in BN6: a one-wide walkway is entered only from its own lane, so each
 * one the way from the arrival to the goal crosses costs a lining up.
 * Capcom's net maps carry their way across over wide floor, their one-wide
 * walkways mostly spurs off it; past the area's cap, the walkways on the
 * way are widened to two panels, which draw in its platforms' floor. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "net.h"
#include "net_shapes.h"
#include "run.h"

/* The one-wide walkways the way may cross between big platforms, per
 * area, as its originals' do (BN6's net maps: none in Central 2, the
 * Seasides, Sky and Green, one in the Undernets, three in Central 1 and
 * the Graveyard, one and seven in the Underground's; Robot Control's
 * comps up to two, the Judge Tree's catwalks up to four, the other comps
 * and homepages none or one). */
static const uint8_t way_cap[BIOME_COUNT] = {
	[BIOME_CENTRAL] = 1, [BIOME_SEASIDE] = 1, [BIOME_SKY] = 1, [BIOME_GREEN] = 1,
	[BIOME_GRAVEYARD] = 3, [BIOME_UNDERNET] = 1, [BIOME_SECRET] = 1, [BIOME_NEST] = 2,
	[BIOME_COMP] = 1, [BIOME_HOMEPAGE] = 1, [BIOME_COMP_B] = 1,
	[BIOME_ROBOT_COMP] = 2, [BIOME_AQUARIUM_COMP] = 1, [BIOME_JUDGE_COMP] = 3, [BIOME_WEATHER_COMP] = 1, [BIOME_COPYBOT_COMP] = 1,
	[BIOME_ACDC_HP] = 1, [BIOME_GREEN_HP] = 1, [BIOME_SKY_HP] = 1,
};

/* The way: the shortest walk over the floor, its cells from start to goal. */
static int16_t way_x[MAP_W * MAP_H], way_y[MAP_W * MAP_H];
static int way_n;

static bool walk(int sx, int sy, int gx, int gy) {
	static int16_t from[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(from, 0xFF, sizeof from);
	int head = 0, tail = 0;
	from[sy][sx] = (int16_t)(sy * MAP_W + sx);
	qx[tail] = (int16_t)sx; qy[tail++] = (int16_t)sy;
	while (head < tail && from[gy][gx] < 0) {
		int x = qx[head], y = qy[head++];
		for (int d = 0; d < 4; ++d) {
			int nx = x + dir_dx[d], ny = y + dir_dy[d];
			if (!floor_at(nx, ny) || from[ny][nx] >= 0) continue;
			from[ny][nx] = (int16_t)(y * MAP_W + x);
			qx[tail] = (int16_t)nx; qy[tail++] = (int16_t)ny;
		}
	}
	way_n = 0;
	if (from[gy][gx] < 0) return false;
	for (int x = gx, y = gy;; ++way_n) {
		way_x[way_n] = (int16_t)x; way_y[way_n] = (int16_t)y;
		if (x == sx && y == sy) break;
		int f = from[y][x];
		x = f % MAP_W; y = f / MAP_W;
	}
	++way_n;
	/* (from the start) */
	for (int i = 0; i < way_n / 2; ++i) {
		int16_t t = way_x[i]; way_x[i] = way_x[way_n - 1 - i]; way_x[way_n - 1 - i] = t;
		t = way_y[i]; way_y[i] = way_y[way_n - 1 - i]; way_y[way_n - 1 - i] = t;
	}
	return true;
}

/* A floor cell in a 2 x 2 block of floor: platform floor, not a walkway. */
static bool in_block(int x, int y) {
	for (int dy = -1; dy <= 0; ++dy)
		for (int dx = -1; dx <= 0; ++dx)
			if (floor_at(x + dx, y + dy) && floor_at(x + dx + 1, y + dy) && floor_at(x + dx, y + dy + 1) && floor_at(x + dx + 1, y + dy + 1)) return true;
	return false;
}

/* Whether platform floor (x, y) is part of a big platform, 12 panels or
 * more: a pad's spur leaves from the middle of its side, where MegaMan
 * stands already, and the originals' counts are between big platforms. */
#define BIG_PLATFORM 12
static bool big(int x, int y) {
	static uint8_t seen[MAP_H][MAP_W];
	static int16_t qx[BIG_PLATFORM * 4], qy[BIG_PLATFORM * 4];
	memset(seen, 0, sizeof seen);
	int head = 0, tail = 0;
	seen[y][x] = 1;
	qx[tail] = (int16_t)x; qy[tail++] = (int16_t)y;
	while (head < tail && tail < BIG_PLATFORM) {
		int cx = qx[head], cy = qy[head++];
		for (int d = 0; d < 4 && tail < BIG_PLATFORM * 4; ++d) {
			int nx = cx + dir_dx[d], ny = cy + dir_dy[d];
			if (!floor_at(nx, ny) || seen[ny][nx] || !in_block(nx, ny)) continue;
			seen[ny][nx] = 1;
			qx[tail] = (int16_t)nx; qy[tail++] = (int16_t)ny;
		}
	}
	return tail >= BIG_PLATFORM;
}

/* The walkways the way crosses between two big platforms: runs of its
 * cells in no block (their first and last index in s0, s1). */
#define MAX_RUNS 32
static int runs(int s0[MAX_RUNS], int s1[MAX_RUNS]) {
	int n = 0;
	for (int i = 1; i + 1 < way_n && n < MAX_RUNS; ++i) {
		if (in_block(way_x[i], way_y[i]) || !in_block(way_x[i - 1], way_y[i - 1])) continue;
		int j = i;
		while (j + 1 < way_n && !in_block(way_x[j + 1], way_y[j + 1])) ++j;
		if (j + 1 >= way_n) break;   /* (the goal on a walkway: no platform past it) */
		if (big(way_x[i - 1], way_y[i - 1]) && big(way_x[j + 1], way_y[j + 1])) { s0[n] = i; s1[n++] = j; }
		i = j;
	}
	return n;
}

/* The cells beside the run [i0, i1] on one side (`sgn` of the normal to
 * each step: a bend's outer corner too), into nx, ny; how many. */
#define MAX_NEW 160
static int beside(int i0, int i1, int sgn, int16_t nx[MAX_NEW], int16_t ny[MAX_NEW]) {
	int n = 0;
	for (int i = i0; i <= i1 && n + 3 <= MAX_NEW; ++i) {
		int ix = way_x[i] - way_x[i - 1], iy = way_y[i] - way_y[i - 1];
		int ox = way_x[i + 1] - way_x[i], oy = way_y[i + 1] - way_y[i];
		/* the normal of a step (dx, dy): (-dy, dx), on the side of sgn */
		int ax = -iy * sgn, ay = ix * sgn, bx = -oy * sgn, by = ox * sgn;
		nx[n] = (int16_t)(way_x[i] + ax); ny[n++] = (int16_t)(way_y[i] + ay);
		if (ax == bx && ay == by) continue;
		nx[n] = (int16_t)(way_x[i] + bx); ny[n++] = (int16_t)(way_y[i] + by);
		nx[n] = (int16_t)(way_x[i] + ax + bx); ny[n++] = (int16_t)(way_y[i] + ay + by);
	}
	return n;
}

/* The platform floor at the run's ends a new cell may touch: what of it
 * lies within three panels of the panel the run leaves or enters, joined
 * to that panel (not a neighbouring platform a void panel away). */
static uint8_t near_end[MAP_H][MAP_W];

static void mark_end(int ex, int ey) {
	static int16_t qx[49], qy[49];
	int head = 0, tail = 0;
	near_end[ey][ex] = 1;
	qx[tail] = (int16_t)ex; qy[tail++] = (int16_t)ey;
	while (head < tail) {
		int x = qx[head], y = qy[head++];
		for (int d = 0; d < 4; ++d) {
			int nx = x + dir_dx[d], ny = y + dir_dy[d];
			if (abs(nx - ex) > 3 || abs(ny - ey) > 3 || !floor_at(nx, ny) || !in_block(nx, ny) || near_end[ny][nx]) continue;
			near_end[ny][nx] = 1;
			qx[tail] = (int16_t)nx; qy[tail++] = (int16_t)ny;
		}
	}
}

/* Whether floor (x, y) is the run's own: its cells, or its ends' platforms. */
static bool own(int x, int y, int i0, int i1) {
	for (int i = i0 - 1; i <= i1 + 1; ++i)
		if (x == way_x[i] && y == way_y[i]) return true;
	return near_end[y][x];
}

static bool is_new(const int16_t nx[], const int16_t ny[], int n, int x, int y) {
	for (int m = 0; m < n; ++m)
		if (nx[m] == x && ny[m] == y) return true;
	return false;
}

/* Widens the run [i0, i1] to two panels on one side: only where each new
 * cell lies in the window on the void, touches no floor but the run's and
 * its ends' platforms (corner to corner neither), and the new side meets
 * both platforms whole. */
static bool widen(int i0, int i1, int sgn) {
	int16_t nx[MAX_NEW], ny[MAX_NEW];
	int n = beside(i0, i1, sgn, nx, ny);
	/* (the new side's ends against platform floor: no notch at a mouth) */
	int ix = way_x[i0] - way_x[i0 - 1], iy = way_y[i0] - way_y[i0 - 1];
	int ox = way_x[i1 + 1] - way_x[i1], oy = way_y[i1 + 1] - way_y[i1];
	if (!floor_at(way_x[i0 - 1] - iy * sgn, way_y[i0 - 1] + ix * sgn) || !floor_at(way_x[i1 + 1] - oy * sgn, way_y[i1 + 1] + ox * sgn)) return false;
	memset(near_end, 0, sizeof near_end);
	mark_end(way_x[i0 - 1], way_y[i0 - 1]);
	mark_end(way_x[i1 + 1], way_y[i1 + 1]);
	for (int k = 0; k < n; ++k) {
		int x = nx[k], y = ny[k];
		if (floor_at(x, y)) { if (!own(x, y, i0, i1)) return false; continue; }
		if (!win_in(x, y)) return false;
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx)
				if (floor_at(x + dx, y + dy) && !own(x + dx, y + dy, i0, i1) && !is_new(nx, ny, n, x + dx, y + dy)) return false;
	}
	for (int k = 0; k < n; ++k) put(nx[k], ny[k]);
	return true;
}

int layer_way_cap(int biome) { return biome >= 0 && biome < BIOME_COUNT ? way_cap[biome] : 1; }

int layer_way_runs(int sx, int sy, int gx, int gy) {
	int s0[MAX_RUNS], s1[MAX_RUNS];
	return walk(sx, sy, gx, gy) ? runs(s0, s1) : 0;
}

void layer_widen_way(int biome, int gx, int gy) {
	int sx = layer.rooms[0].ax, sy = layer.rooms[0].ay;
	for (int tries = 0; tries < MAX_RUNS; ++tries) {
		int s0[MAX_RUNS], s1[MAX_RUNS];
		if (!walk(sx, sy, gx, gy)) return;
		int n = runs(s0, s1);
		if (n <= way_cap[biome]) return;
		/* the shortest first: the long bridges are an area's own */
		bool done = false;
		for (int len = 1; len < MAP_W * 2 && !done; ++len)
			for (int k = 0; k < n && !done; ++k)
				if (s1[k] - s0[k] + 1 == len) done = widen(s0[k], s1[k], 1) || widen(s0[k], s1[k], -1);
		if (!done) return;
	}
}
