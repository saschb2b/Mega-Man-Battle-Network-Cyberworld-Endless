/* The way on over a layer's floor (net_route.h). */
#include "net_route.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

int16_t route_walk[MAP_W * MAP_H];
int route_walk_len, route_walk_aim = -1;
static double way_eighths;   /* the last way, in eighths, not rounded */

static bool floor_at(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH; }

/* the panels a solid object stands on (a Mystery Data, a navi), but the
 * walk's two ends */
static uint8_t solid[MAP_H][MAP_W];
static bool open_at(int x, int y) { return floor_at(x, y) && !solid[y][x]; }

int route_grid_way(double dx, double dy) {
	/* the pad's eight ways (its RIGHT is grid +x -y, its DOWN +x +y), not
	 * the screen's eighths: a walkway (along x or y) runs in the middle of
	 * a diagonal way, where on the screen, 27 degrees off level, it lay 4
	 * from "left" or "right", and MegaMan a little aside on it tipped it */
	double right = dx - dy, down = dx + dy;
	way_eighths = atan2(down, right) / (3.14159265358979 / 4);
	int k = (int)lround(way_eighths);
	return (k % 8 + 8) % 8;
}

bool route_way_holds(int shown, double margin) {
	double d = fmod(way_eighths - shown + 16.0, 8.0);
	if (d > 4.0) d = 8.0 - d;
	return d <= margin;
}

/* Whether MegaMan walks from panel (sx, sy) to (ax, ay) in a straight line
 * over the floor: the line on floor panels throughout, where it crosses a
 * panel's side on either, and where it crosses a corner with three of the
 * four panels around it floor (a turn's corner cut; not between two drops),
 * and clear of what stands on the floor (MegaMan, walking at a Mystery Data
 * on the line, stopped against it). (Rounded, a line through a corner fell
 * on its empty panel, and the arrow showed a turn's first panel only:
 * followed, it ran past the turn.) */
static bool floor_line(int sx, int sy, int ax, int ay) {
	int n = 16 * (abs(ax - sx) + abs(ay - sy));
	for (int t = 1; t < n; ++t) {
		double fx = sx + (ax - sx) * (double)t / n, fy = sy + (ay - sy) * (double)t / n;
		bool ex = fabs(fx - floor(fx) - 0.5) < 1e-6, ey = fabs(fy - floor(fy) - 0.5) < 1e-6;
		int x0 = ex ? (int)floor(fx) : (int)lround(fx), y0 = ey ? (int)floor(fy) : (int)lround(fy);
		int x1 = x0 + ex, y1 = y0 + ey, floors = 0;
		for (int k = 0; k < 4; ++k) floors += open_at(k & 1 ? x1 : x0, k & 2 ? y1 : y0);
		if (ex && ey ? floors < 3 : !floors) return false;
	}
	return true;
}

bool route_floor_line(int sx, int sy, int ax, int ay) { return floor_line(sx, sy, ax, ay); }

/* A walkway the walk enters within its next four panels (a panel of floor
 * between void, entered along its line) while MegaMan stands off its
 * line: its mouth, the panel before it, where the arrow aims first, as BN6
 * lets him onto a walkway along it alone (aimed past it, the arrow's
 * diagonal stopped a playtester at the corner, five times a session); -1
 * for none, or where no straight line over the floor reaches the mouth.
 * On its line means within 12 of a panel's 32 units of it: 12 went in,
 * 13 was stopped at the walkway's edge (replays of two corners; a run
 * into a corner stops him 12 past its middle, and at 0.35 of a panel the
 * arrow pointed back from there at every turn, session 55). Where the
 * mouth is his own panel, *across is set: the arrow points across to the
 * walkway's line, not back to the panel's middle. */
static int mouth_aim(double px, double py, const int16_t *path, int n, bool *across) {
	int sx = (int)lround(px), sy = (int)lround(py), cx = sx, cy = sy;
	*across = false;
	for (int k = 1; k <= 4 && k <= n; ++k) {
		int x = path[n - k] % MAP_W, y = path[n - k] / MAP_W, dx = x - cx, dy = y - cy;
		if (floor_at(x + dy, y + dx) || floor_at(x - dy, y - dx)) { cx = x; cy = y; continue; }
		if (fabs(dx ? py - y : px - x) < 12.5 / 32 || !floor_line(sx, sy, cx, cy)) return -1;
		*across = k == 1;
		return cy * MAP_W + cx;
	}
	return -1;
}

/* Where the arrow aims on the walk (n panels, from the target back): a
 * walkway's mouth while MegaMan stands off its line; else along the
 * walk's first leg while it runs straight (on a walkway that is one of the
 * screen's diagonals: a flat arrow between two forking walkways said
 * neither), else at the farthest of the next four panels he can walk to
 * in a straight line over the floor (three along, as the crow flies, cut
 * corners over drops). */
static int walk_aim(double px, double py, int sx, int sy, const int16_t *path, int n, bool *across) {
	int mouth = mouth_aim(px, py, path, n, across);
	if (mouth >= 0) return mouth;
	int leg = 0;
	if (n) {
		int dx = path[n - 1] % MAP_W - sx, dy = path[n - 1] / MAP_W - sy;
		while (leg < 4 && leg < n && path[n - 1 - leg] % MAP_W == sx + dx * (leg + 1) && path[n - 1 - leg] / MAP_W == sy + dy * (leg + 1)) ++leg;
	}
	if (leg >= 2) return path[n - leg];
	for (int k = 4; k >= 2; --k)
		if (n >= k && floor_line(sx, sy, path[n - k] % MAP_W, path[n - k] / MAP_W)) return path[n - k];
	return n ? path[n - 1] : sy * MAP_W + sx;
}

int route_way(double px, double py, int tx, int ty, int *len) {
	static int16_t prev[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	int sx = (int)lround(px), sy = (int)lround(py);
	route_walk_len = 0;
	route_walk_aim = -1;
	if (!floor_at(sx, sy) || !floor_at(tx, ty)) return -1;
	/* around what stands on the floor (it keeps every panel in reach,
	 * test_core.c): the arrow led into a Mystery Data at a walkway's mouth */
	memset(solid, 0, sizeof solid);
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].solid && floor_at((int)layer.obj[i].x, (int)layer.obj[i].y)) solid[(int)layer.obj[i].y][(int)layer.obj[i].x] = 1;
	solid[ty][tx] = 0;
	/* MegaMan beside a Mystery Data, in its panel's corner: the walk from
	 * the free panel nearest him (from the object's own it led through it,
	 * and he pushed into it for good) */
	if (solid[sy][sx]) {
		double best = 1e9;
		int bx = sx, by = sy;
		for (int k = 0; k < 8; ++k) {
			static const int d[8][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };
			int x = sx + d[k][0], y = sy + d[k][1];
			if (!floor_at(x, y) || solid[y][x]) continue;
			double dist = (x - px) * (x - px) + (y - py) * (y - py);
			if (dist < best) { best = dist; bx = x; by = y; }
		}
		sx = bx;
		sy = by;
	}
	solid[sy][sx] = 0;
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
	bool across;
	int aim = walk_aim(px, py, sx, sy, path, n, &across);
	route_walk_len = n;
	route_walk_aim = aim;
	double ax = aim % MAP_W - px, ay = aim / MAP_W - py;
	/* (a walkway along x is lined up on y) */
	if (across && path[n - 1] / MAP_W == aim / MAP_W) ax = 0;
	else if (across) ay = 0;
	return route_grid_way(ax, ay);
}
