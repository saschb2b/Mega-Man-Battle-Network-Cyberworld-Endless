/* The way on over a layer's floor (net_route.h). */
#include "net_route.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

int16_t route_walk[MAP_W * MAP_H];
int route_walk_len, route_walk_aim = -1;

static bool floor_at(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH; }

int route_grid_way(double dx, double dy) {
	/* the pad's eight ways (its RIGHT is grid +x -y, its DOWN +x +y), not
	 * the screen's eighths: a walkway (along x or y) runs in the middle of
	 * a diagonal way, where on the screen, 27 degrees off level, it lay 4
	 * from "left" or "right", and MegaMan a little aside on it tipped it */
	double right = dx - dy, down = dx + dy;
	int k = (int)lround(atan2(down, right) / (3.14159265358979 / 4));
	return (k % 8 + 8) % 8;
}

/* Whether MegaMan walks from panel (sx, sy) to (ax, ay) in a straight line
 * over the floor: the line on floor panels throughout, where it crosses a
 * panel's side on either, and where it crosses a corner with three of the
 * four panels around it floor (a turn's corner cut; not between two drops).
 * (Rounded, a line through a corner fell on its empty panel, and the arrow
 * showed a turn's first panel only: followed, it ran past the turn.) */
static bool floor_line(int sx, int sy, int ax, int ay) {
	int n = 16 * (abs(ax - sx) + abs(ay - sy));
	for (int t = 1; t < n; ++t) {
		double fx = sx + (ax - sx) * (double)t / n, fy = sy + (ay - sy) * (double)t / n;
		bool ex = fabs(fx - floor(fx) - 0.5) < 1e-6, ey = fabs(fy - floor(fy) - 0.5) < 1e-6;
		int x0 = ex ? (int)floor(fx) : (int)lround(fx), y0 = ey ? (int)floor(fy) : (int)lround(fy);
		int x1 = x0 + ex, y1 = y0 + ey, floors = 0;
		for (int k = 0; k < 4; ++k) floors += floor_at(k & 1 ? x1 : x0, k & 2 ? y1 : y0);
		if (ex && ey ? floors < 3 : !floors) return false;
	}
	return true;
}

int route_way(double px, double py, int tx, int ty, int *len) {
	static int16_t prev[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	static uint8_t solid[MAP_H][MAP_W];
	int sx = (int)lround(px), sy = (int)lround(py);
	route_walk_len = 0;
	route_walk_aim = -1;
	if (!floor_at(sx, sy) || !floor_at(tx, ty)) return -1;
	/* around what stands on the floor (it keeps every panel in reach,
	 * test_core.c): the arrow led into a Mystery Data at a walkway's mouth */
	memset(solid, 0, sizeof solid);
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].solid && floor_at((int)layer.obj[i].x, (int)layer.obj[i].y)) solid[(int)layer.obj[i].y][(int)layer.obj[i].x] = 1;
	solid[sy][sx] = solid[ty][tx] = 0;
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) prev[y][x] = -1;
	int h = 0, t = 0;
	qx[t] = (int16_t)sx; qy[t++] = (int16_t)sy;
	prev[sy][sx] = (int16_t)(sy * MAP_W + sx);
	while (h < t && prev[ty][tx] < 0) {
		int x = qx[h], y = qy[h++];
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int nx = x + d[k][0], ny = y + d[k][1];
			if (!floor_at(nx, ny) || prev[ny][nx] >= 0 || solid[ny][nx]) continue;
			prev[ny][nx] = (int16_t)(y * MAP_W + x);
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	if (prev[ty][tx] < 0) return -1;
	/* the walk, backwards from the target */
	int n = 0, cx = tx, cy = ty;
	int16_t *path = route_walk;
	while (!(cx == sx && cy == sy)) {
		path[n++] = (int16_t)(cy * MAP_W + cx);
		int p = prev[cy][cx];
		cx = p % MAP_W; cy = p / MAP_W;
	}
	*len = n;
	/* aim along the walk's first leg while it runs straight (on a walkway
	 * that is one of the screen's diagonals: a flat arrow between two
	 * forking walkways said neither), else at the farthest of the next four
	 * panels he can walk to in a straight line over the floor (three
	 * along, as the crow flies, cut corners over drops) */
	int aim = n ? path[n - 1] : sy * MAP_W + sx;
	int leg = 0;
	if (n) {
		int dx = path[n - 1] % MAP_W - sx, dy = path[n - 1] / MAP_W - sy;
		while (leg < 4 && leg < n && path[n - 1 - leg] % MAP_W == sx + dx * (leg + 1) && path[n - 1 - leg] / MAP_W == sy + dy * (leg + 1)) ++leg;
	}
	if (leg >= 2) aim = path[n - leg];
	else for (int k = 4; k >= 2; --k)
		if (n >= k && floor_line(sx, sy, path[n - k] % MAP_W, path[n - k] / MAP_W)) { aim = path[n - k]; break; }
	route_walk_len = n;
	route_walk_aim = aim;
	return route_grid_way(aim % MAP_W - px, aim / MAP_W - py);
}
