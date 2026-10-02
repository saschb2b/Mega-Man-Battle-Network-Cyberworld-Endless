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

/* ... and every one-wide walkway it may cross, whatever its platforms, as
 * the originals' way across does (src/dev/navstudy.c, the walk between a
 * map's two farthest panels: Central 2, 6, 2; Seaside 0, 3, 0; Sky 5, 4,
 * 2; Green 2, 1; the Undernet 3, 3, 1; the Secret Area 1; the Nest's
 * Underground 3, 3; Robot Control 3, 3; the Aquarium 5, 1, 4; the Judge
 * Tree 6, 2, 5; Mr. Weather 1, 1, 0; CopyBot's 4, 4, 1, 5, 1). 0: none
 * (the comps' and homepages' originals are single small rooms, nothing to
 * hold a layer to; the Graveyard's eight run over its slabs). */
static const uint8_t narrow_cap[BIOME_COUNT] = {
	[BIOME_CENTRAL] = 3, [BIOME_SEASIDE] = 1, [BIOME_SKY] = 4, [BIOME_GREEN] = 2,
	[BIOME_UNDERNET] = 2, [BIOME_SECRET] = 1, [BIOME_NEST] = 3,
	[BIOME_ROBOT_COMP] = 3, [BIOME_AQUARIUM_COMP] = 3, [BIOME_JUDGE_COMP] = 4, [BIOME_WEATHER_COMP] = 1, [BIOME_COPYBOT_COMP] = 3,
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

/* The walkways the way crosses between two big platforms (`any`: between
 * any platform floor): runs of its cells in no block (their first and last
 * index in s0, s1). */
#define MAX_RUNS 32
static int runs_of(int s0[MAX_RUNS], int s1[MAX_RUNS], bool any) {
	int n = 0;
	for (int i = 1; i + 1 < way_n && n < MAX_RUNS; ++i) {
		if (in_block(way_x[i], way_y[i]) || !in_block(way_x[i - 1], way_y[i - 1])) continue;
		int j = i;
		while (j + 1 < way_n && !in_block(way_x[j + 1], way_y[j + 1])) ++j;
		if (j + 1 >= way_n) break;   /* (the goal on a walkway: no platform past it) */
		if (any || (big(way_x[i - 1], way_y[i - 1]) && big(way_x[j + 1], way_y[j + 1]))) { s0[n] = i; s1[n++] = j; }
		i = j;
	}
	return n;
}

static int runs(int s0[MAX_RUNS], int s1[MAX_RUNS]) { return runs_of(s0, s1, false); }

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

int layer_way_narrows(int sx, int sy, int gx, int gy) {
	int s0[MAX_RUNS], s1[MAX_RUNS];
	return walk(sx, sy, gx, gy) ? runs_of(s0, s1, true) : 0;
}

int layer_narrow_cap(int biome) { return biome >= 0 && biome < BIOME_COUNT ? narrow_cap[biome] : 0; }

int layer_way_runs(int sx, int sy, int gx, int gy) {
	int s0[MAX_RUNS], s1[MAX_RUNS];
	return walk(sx, sy, gx, gy) ? runs(s0, s1) : 0;
}

/* The way's walkways past a cap widened, the shortest first (`any`: every
 * one-wide walkway against narrow_cap, else those between big platforms
 * against way_cap) */
static void widen_past(int biome, int gx, int gy, bool any) {
	int sx = layer.rooms[0].ax, sy = layer.rooms[0].ay, cap = any ? narrow_cap[biome] : way_cap[biome];
	if (any && !cap) return;
	for (int tries = 0; tries < MAX_RUNS; ++tries) {
		int s0[MAX_RUNS], s1[MAX_RUNS];
		if (!walk(sx, sy, gx, gy)) return;
		int n = runs_of(s0, s1, any);
		if (n <= cap) return;
		/* the shortest first: the long bridges are an area's own */
		bool done = false;
		for (int len = 1; len < MAP_W * 2 && !done; ++len)
			for (int k = 0; k < n && !done; ++k)
				if (s1[k] - s0[k] + 1 == len) done = widen(s0[k], s1[k], 1) || widen(s0[k], s1[k], -1);
		if (!done) return;
	}
}

void layer_widen_way(int biome, int gx, int gy) {
	/* (every walkway first: one widened joins its platforms, and the
	 * walkways between big platforms are counted on what that makes) */
	widen_past(biome, gx, gy, true);
	widen_past(biome, gx, gy, false);
}

/* ---- The same count on any floor: BN6's own maps too (src/dev/navstudy.c) ----
 * The way across a floor: the walk between its two farthest panels (the
 * largest piece of it; from the farthest panel of any, the farthest from
 * that), and the one-wide walkways it crosses between platform floor, of
 * any size, and between big platforms. */
#define GRID_MAX (256 * 256)
static const uint8_t *g_cells;
static int g_w, g_h;
static int32_t g_from[GRID_MAX];
static uint8_t g_comp[GRID_MAX];
static int32_t g_q[GRID_MAX];

static bool g_at(int x, int y) { return x >= 0 && y >= 0 && x < g_w && y < g_h && g_cells[y * g_w + x]; }
static bool g_block(int x, int y) {
	for (int dy = -1; dy <= 0; ++dy)
		for (int dx = -1; dx <= 0; ++dx)
			if (g_at(x + dx, y + dy) && g_at(x + dx + 1, y + dy) && g_at(x + dx, y + dy + 1) && g_at(x + dx + 1, y + dy + 1)) return true;
	return false;
}

/* A walk over the floor from cell s, every cell's predecessor in g_from;
 * the last cell reached (the farthest), and how many. */
static int g_walk(int s, int *count) {
	static const int d4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	memset(g_from, 0xFF, sizeof g_from[0] * (size_t)(g_w * g_h));
	int h = 0, t = 0, last = s;
	g_from[s] = s;
	g_q[t++] = s;
	while (h < t) {
		int c = g_q[h++], x = c % g_w, y = c / g_w;
		last = c;
		for (int k = 0; k < 4; ++k) {
			int nx = x + d4[k][0], ny = y + d4[k][1], n = ny * g_w + nx;
			if (!g_at(nx, ny) || g_from[n] >= 0) continue;
			g_from[n] = c;
			g_q[t++] = n;
		}
	}
	*count = t;
	return last;
}

/* Whether the platform floor at cell c is big: BIG_PLATFORM panels of 2 x 2 floor joined */
static bool g_big(int c) {
	static uint8_t seen[GRID_MAX];
	memset(seen, 0, (size_t)(g_w * g_h));
	int h = 0, t = 0;
	seen[c] = 1;
	g_q[t++] = c;
	while (h < t && t < BIG_PLATFORM) {
		int x = g_q[h] % g_w, y = g_q[h++] / g_w;
		for (int d = 0; d < 4; ++d) {
			int nx = x + dir_dx[d], ny = y + dir_dy[d];
			if (!g_at(nx, ny) || seen[ny * g_w + nx] || !g_block(nx, ny)) continue;
			seen[ny * g_w + nx] = 1;
			g_q[t++] = ny * g_w + nx;
		}
	}
	return t >= BIG_PLATFORM;
}

int grid_way_narrows(const uint8_t *cells, int w, int h, int *big_links, int *way_len) {
	*big_links = *way_len = 0;
	if (w <= 0 || h <= 0 || w * h > GRID_MAX) return 0;
	g_cells = cells; g_w = w; g_h = h;
	/* the largest piece of floor */
	int best = -1, most = 0, n;
	memset(g_comp, 0, sizeof g_comp[0] * (size_t)(w * h));
	for (int c = 0; c < w * h; ++c) {
		if (!cells[c] || g_comp[c]) continue;
		g_walk(c, &n);
		for (int k = 0; k < n; ++k) g_comp[g_q[k]] = 1;
		if (n > most) { most = n; best = c; }
	}
	if (best < 0) return 0;
	int u = g_walk(best, &n), v = g_walk(u, &n);
	/* the walk from v back to u, as a list of cells */
	static int32_t path[GRID_MAX];
	int len = 0;
	for (int c = v;; c = g_from[c]) { path[len++] = c; if (c == u) break; }
	*way_len = len;
	int narrows = 0;
	for (int i = 1; i + 1 < len; ++i) {
		int x = path[i] % w, y = path[i] / w, px = path[i - 1] % w, py = path[i - 1] / w;
		if (g_block(x, y) || !g_block(px, py)) continue;
		int j = i;
		while (j + 1 < len && !g_block(path[j + 1] % w, path[j + 1] / w)) ++j;
		if (j + 1 >= len) break;
		++narrows;
		*big_links += g_big(path[i - 1]) && g_big(path[j + 1]);
		i = j;
	}
	return narrows;
}
