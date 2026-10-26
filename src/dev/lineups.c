/* The lining up a layer's floor asks of MegaMan (lineups.h). BN6 lets him
 * into a one-wide walkway only from its own lane: from wider floor, or past
 * a junction he overshot, he must stop on its line first, the cost a
 * playtester counted in calls (docs/world/level-design/generator.md,
 * Navigation; issue #164). A corner's wall stops him on the line itself. */
#include "lineups.h"

#include <string.h>

#include "net.h"

static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

static bool floor_at(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH; }

/* (x, y) one panel wide across direction d: void on both its sides */
static bool lane(int x, int y, int d) {
	int px = d4[(d + 1) & 3][0], py = d4[(d + 1) & 3][1];
	return !floor_at(x + px, y + py) && !floor_at(x - px, y - py);
}

/* a walkways' junction: floor on three sides or four, in no 2 x 2 block */
static bool junction(int x, int y) {
	int n = 0;
	for (int d = 0; d < 4; ++d) n += floor_at(x + d4[d][0], y + d4[d][1]);
	if (n < 3) return false;
	for (int k = 0; k < 4; ++k) {
		int bx = x - (k & 1), by = y - (k >> 1);
		if (floor_at(bx, by) && floor_at(bx + 1, by) && floor_at(bx, by + 1) && floor_at(bx + 1, by + 1)) return false;
	}
	return true;
}

int layer_lineups(int sx, int sy, int gx, int gy, int *len, int *junctions) {
	static int16_t from[MAP_H][MAP_W];
	static int16_t q[MAP_W * MAP_H], path[MAP_W * MAP_H];
	*len = *junctions = 0;
	if (!floor_at(sx, sy) || !floor_at(gx, gy)) return -1;
	memset(from, 0xFF, sizeof from);
	int head = 0, tail = 0;
	from[sy][sx] = (int16_t)(sy * MAP_W + sx);
	q[tail++] = (int16_t)(sy * MAP_W + sx);
	while (head < tail && from[gy][gx] < 0) {
		int c = q[head++], x = c % MAP_W, y = c / MAP_W;
		for (int d = 0; d < 4; ++d) {
			int nx = x + d4[d][0], ny = y + d4[d][1];
			if (!floor_at(nx, ny) || from[ny][nx] >= 0) continue;
			from[ny][nx] = (int16_t)c;
			q[tail++] = (int16_t)(ny * MAP_W + nx);
		}
	}
	if (from[gy][gx] < 0) return -1;
	int n = 0;
	for (int c = gy * MAP_W + gx;; c = from[c / MAP_W][c % MAP_W]) {
		path[n++] = (int16_t)c;
		if (c == sy * MAP_W + sx) break;
	}
	*len = n - 1;
	/* walked from the start: aligned once on a lane's line, until he turns
	 * out onto wider floor */
	int count = 0, prev = -1;
	bool aligned = false;
	for (int i = n - 1; i > 0; --i) {
		int x = path[i] % MAP_W, y = path[i] / MAP_W, nx = path[i - 1] % MAP_W, ny = path[i - 1] / MAP_W, d = 0;
		while (d4[d][0] != nx - x || d4[d][1] != ny - y) ++d;
		bool into_lane = lane(nx, ny, d);
		*junctions += junction(nx, ny);
		if (prev >= 0 && d != prev) {
			if (!floor_at(x + d4[prev][0], y + d4[prev][1])) aligned = true;   /* a corner: its wall stops him */
			else if (into_lane) { ++count; aligned = true; }
			else aligned = false;
		} else if (!aligned && into_lane) { ++count; aligned = true; }
		prev = d;
	}
	return count;
}
