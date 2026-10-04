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
/* ... and stepped onto from (x, y): an arrow lane only the way it runs
 * (issue #43) */
static bool open_step(int x, int y, int nx, int ny) { return open_at(nx, ny) && layer_step_ok(x, y, nx, ny); }

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
 * four panels around it floor and none a solid object's (a turn's corner
 * cut; not between two drops; past a Mystery Data's corner it led MegaMan
 * into the data's panel, which the arrow leads him out of), and clear of
 * what stands on the floor (MegaMan, walking at a Mystery Data on the
 * line, stopped against it). (Rounded, a line through a corner fell on its
 * empty panel, and the arrow showed a turn's first panel only: followed,
 * it ran past the turn.) */
static bool floor_line(int sx, int sy, int ax, int ay) {
	int n = 16 * (abs(ax - sx) + abs(ay - sy));
	for (int t = 1; t < n; ++t) {
		double fx = sx + (ax - sx) * (double)t / n, fy = sy + (ay - sy) * (double)t / n;
		bool ex = fabs(fx - floor(fx) - 0.5) < 1e-6, ey = fabs(fy - floor(fy) - 0.5) < 1e-6;
		int x0 = ex ? (int)floor(fx) : (int)lround(fx), y0 = ey ? (int)floor(fy) : (int)lround(fy);
		int x1 = x0 + ex, y1 = y0 + ey, floors = 0, objects = 0;
		for (int k = 0; k < 4; ++k) {
			int x = k & 1 ? x1 : x0, y = k & 2 ? y1 : y0;
			floors += open_at(x, y);
			objects += floor_at(x, y) && solid[y][x];
		}
		if (ex && ey ? (floors < 3 || objects) : !floors) return false;
	}
	return true;
}

bool route_floor_line(int sx, int sy, int ax, int ay) { return floor_line(sx, sy, ax, ay); }

/* The lane the walk's step from panel (x0, y0) to its neighbour (x1, y1)
 * passes: the panels floor on both sides of their border, crosswise, in a
 * run through the walk's own; its width in panels, and its ends from the
 * walk's line (*lo, *hi) as MegaMan passes them, within 12 of a panel's 32
 * units of the end panels' middles (BN6 keeps him 12 past a panel's middle
 * against the void: 12 went in, 13 was stopped at a walkway's edge). */
static int step_lane(int x0, int y0, int x1, int y1, double *lo, double *hi) {
	int ux = y1 != y0, uy = x1 != x0, a = 0, b = 0;
	while (floor_at(x0 - (a + 1) * ux, y0 - (a + 1) * uy) && floor_at(x1 - (a + 1) * ux, y1 - (a + 1) * uy)) ++a;
	while (floor_at(x0 + (b + 1) * ux, y0 + (b + 1) * uy) && floor_at(x1 + (b + 1) * ux, y1 + (b + 1) * uy)) ++b;
	*lo = -a - 12.5 / 32;
	*hi = b + 12.5 / 32;
	return a + b + 1;
}

/* A lane the walk enters within its next four panels while MegaMan
 * stands off it: on the walk's first leg any (he stood in his panel's
 * edge, and the floor ahead did not reach there), past a turn a walkway's
 * or a band's (one or two panels wide, test_core.c's band, entered along
 * its line). Its mouth, the panel before it, is where the arrow aims
 * first, as BN6 lets him onto a walkway along it alone (aimed past it, the
 * arrow's diagonal stopped a playtester at the corner, five times a
 * session; a band two panels wide was left to the walk's line, and its
 * mouths stopped three playtesters at Green HP's crossing bands and the
 * Judge Tree's catwalks, session 63); -1 for none, or where no straight
 * line over the floor reaches the mouth. On a lane means within its ends
 * (step_lane; at 0.35 of a panel the arrow pointed back from a corner at
 * every turn, session 55). Where the mouth is his own panel, *across is
 * set: the arrow points across to the walk's line, not back to the
 * panel's middle. (sx, sy): the panel the walk sets off from. */
static int mouth_aim(double px, double py, int sx, int sy, const int16_t *path, int n, bool *across) {
	int cx = sx, cy = sy, fx = 0, fy = 0;
	bool ahead = true;
	*across = false;
	for (int k = 1; k <= 4 && k <= n; ++k) {
		int x = path[n - k] % MAP_W, y = path[n - k] / MAP_W, dx = x - cx, dy = y - cy;
		if (k == 1) { fx = dx; fy = dy; }
		ahead = ahead && dx == fx && dy == fy;
		double lo, hi, off = dx ? py - y : px - x;
		if ((step_lane(cx, cy, x, y, &lo, &hi) > 2 && !ahead) || (off > lo && off < hi)) { cx = x; cy = y; continue; }
		if (!floor_line(sx, sy, cx, cy)) return -1;
		*across = k == 1;
		return cy * MAP_W + cx;
	}
	return -1;
}

/* Whether panel (x, y) and the eight about it are floor: inside a
 * platform, where the arrow may point across the grid (one of the screen's
 * straight four) as the crow flies. On a walkway or band, at a junction or
 * a floor's edge it shows the grid's ways alone, as the walkways run on
 * the screen: aimed a few panels on, it pointed straight across the
 * screen on two-wide bands, against their diagonals, and flipped at their
 * junctions (session 63). */
static bool open_floor(int x, int y) {
	for (int j = -1; j <= 1; ++j)
		for (int i = -1; i <= 1; ++i)
			if (!floor_at(x + i, y + j)) return false;
	return true;
}

/* Whether MegaMan stands at (x, y) as BN6 keeps him on the floor: a
 * panel's floor reaches 12.5 of its 32 units past its middle towards the
 * void (12 went in, 13 was stopped), on into a floor panel beside it, and
 * into a corner where the three panels about it are floor; clear of what
 * stands on it, circles smaller than a panel (he stands in the corner of
 * a Mystery Data's panel). */
static bool stands(double x, double y) {
	const double m = 12.5 / 32;
	int px = (int)lround(x), py = (int)lround(y);
	double fx = x - px, fy = y - py;
	int ex = fx > m ? 1 : fx < -m ? -1 : 0, ey = fy > m ? 1 : fy < -m ? -1 : 0;
	if (!floor_at(px, py) || !floor_at(px + ex, py) || !floor_at(px, py + ey) || !floor_at(px + ex, py + ey)) return false;
	for (int j = -1; j <= 1; ++j)
		for (int i = -1; i <= 1; ++i)
			if (floor_at(px + i, py + j) && solid[py + j][px + i] && (fx - i) * (fx - i) + (fy - j) * (fy - j) < 0.45 * 0.45) return false;
	return true;
}

/* Whether MegaMan, holding the grid way (dx, dy) (each -1, 0 or 1, a unit
 * of a panel's 32 along each a step; held into an edge, along the other
 * alone, as BN6's wall shapes push him out square), walks from (x, y)
 * into panel (wx, wy) within `steps`. */
static bool held_into(double x, double y, int dx, int dy, int wx, int wy, int steps) {
	static const int part[3][2] = { { 1, 1 }, { 1, 0 }, { 0, 1 } };
	for (int f = 0; f < steps; ++f) {
		if (lround(x) == wx && lround(y) == wy) return true;
		int k = 0;
		while (k < 3 && ((dx * part[k][0] == 0 && dy * part[k][1] == 0) || !stands(x + dx * part[k][0] / 32.0, y + dy * part[k][1] / 32.0))) ++k;
		if (k == 3) return false;
		x += dx * part[k][0] / 32.0;
		y += dy * part[k][1] / 32.0;
	}
	return lround(x) == wx && lround(y) == wy;
}

/* The floor's width at panel (x, y) crosswise to a step (dx, dy) along
 * one axis: two panels or less is a walkway or band, whose way the arrow
 * shows (open_floor). */
static int width_across(int x, int y, int dx, int dy) {
	int ux = dy != 0, uy = dx != 0, w = 1;
	for (int s = -1; s <= 1; s += 2)
		for (int k = 1; floor_at(x + s * ux * k, y + s * uy * k); ++k) ++w;
	return w;
}

/* MegaMan off the lane of a mouth (mouth_aim's) that is his own panel or
 * the walk's next: the screen's straight way across onto the lane and
 * along it, where holding it walks him in (BN6 slides him along the edge
 * till he is on the lane, then in, and the lane's sides keep him there);
 * -1 for none. The way across alone took a hold just long enough: held
 * on, it carried him over the lane where the floor went on, and the arrow
 * flipped back; held short, it moved him a few units with the arrow
 * unchanged, and two playtesters took the lane's way too soon and stood
 * still at the mouth, where LEFT, RIGHT or UP walked them in (session 65).
 * Not on a walkway or band, where the arrow shows the grid's ways. */
static int slide_way(double px, double py, int sx, int sy, const int16_t *path, int n, int mouth) {
	int w = mouth == sy * MAP_W + sx && n >= 1 ? path[n - 1] : n >= 2 && mouth == path[n - 1] ? path[n - 2] : -1;
	if (w < 0 || width_across(sx, sy, path[n - 1] % MAP_W - sx, path[n - 1] / MAP_W - sy) <= 2) return -1;
	int mx = mouth % MAP_W, my = mouth / MAP_W, ax = w % MAP_W - mx, ay = w / MAP_W - my;
	/* (across: towards the lane, from the mouth's middle) */
	int t = (ax ? py - my : px - mx) > 0 ? -1 : 1, tx = ax ? 0 : t, ty = ax ? t : 0;
	if (!held_into(px, py, ax + tx, ay + ty, w % MAP_W, w / MAP_W, 96)) return -1;
	return route_grid_way(ax + tx, ay + ty);
}

/* Where the arrow aims on the walk (n panels, from the target back): a
 * walkway's or band's mouth while MegaMan stands off its lane (off open
 * floor, across to the walk's line where the mouth lies straight ahead);
 * else along the walk's first leg while it runs straight (on a walkway
 * that is one of the screen's diagonals: a flat arrow between two forking
 * walkways said neither), and off open floor always, its first step a
 * turn only once its lane is open; else at the farthest of the next four
 * panels he can walk to in a straight line over the floor (three along,
 * as the crow flies, cut corners over drops). `mouth` and *across are
 * mouth_aim's. */
static int walk_aim(int sx, int sy, const int16_t *path, int n, int mouth, bool axes, bool *across) {
	int leg = 0;
	if (n) {
		int dx = path[n - 1] % MAP_W - sx, dy = path[n - 1] / MAP_W - sy;
		while (leg < 4 && leg < n && path[n - 1 - leg] % MAP_W == sx + dx * (leg + 1) && path[n - 1 - leg] / MAP_W == sy + dy * (leg + 1)) ++leg;
	}
	if (axes && mouth >= 0) {
		for (int k = 1; k < leg && !*across; ++k) *across = mouth == path[n - k];
		return *across ? sy * MAP_W + sx : mouth;
	}
	if (mouth >= 0) return mouth;
	if (leg >= 2 || (axes && leg)) return path[n - leg];
	for (int k = 4; k >= 2; --k)
		if (n >= k && floor_line(sx, sy, path[n - k] % MAP_W, path[n - k] / MAP_W)) return path[n - k];
	return n ? path[n - 1] : sy * MAP_W + sx;
}

/* The walk from (sx, sy) to (tx, ty) into route_walk, backwards from the
 * target: the shortest, and of the shortest the one that turns least
 * (MegaMan walks as BN6 does, a direction held at a time: across a
 * lattice of walkways the shortest walk had turned at every crossing, a
 * stop and a new direction each, and a playtester nudged ten calls to a
 * tree; turning once, it runs along a walkway and on along another).
 * A state is a panel and the way it was entered (panel * 4 + way). */
enum { LT_STATES = MAP_W * MAP_H * 4 };
static int16_t lt_prev[LT_STATES], lt_dist[LT_STATES], lt_turns[LT_STATES], lt_q[LT_STATES];
static int lt_tail;
static const int lt_d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };

/* state s's step `k` onward, where it is open: a new state queued, or one
 * of the next level reached with fewer turns */
static void lt_relax(int s, int k, int sx, int sy) {
	int x = s / 4 % MAP_W, y = s / 4 / MAP_W, nx = x + lt_d[k][0], ny = y + lt_d[k][1];
	if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || (nx == sx && ny == sy) || !open_step(x, y, nx, ny)) return;
	int n = (ny * MAP_W + nx) * 4 + k, nt = lt_turns[s] + (k != s % 4);
	if (lt_dist[n] < 0) {
		lt_dist[n] = (int16_t)(lt_dist[s] + 1); lt_turns[n] = (int16_t)nt; lt_prev[n] = (int16_t)s;
		lt_q[lt_tail++] = (int16_t)n;
	} else if (lt_dist[n] == lt_dist[s] + 1 && nt < lt_turns[n]) { lt_turns[n] = (int16_t)nt; lt_prev[n] = (int16_t)s; }
}

/* Its length, -1 where none reaches it. */
static int least_turns(int sx, int sy, int tx, int ty) {
	memset(lt_dist, 0xFF, sizeof lt_dist);
	int h = 0, best = -1;
	lt_tail = 0;
	for (int k = 0; k < 4; ++k) {
		int nx = sx + lt_d[k][0], ny = sy + lt_d[k][1];
		if (!open_step(sx, sy, nx, ny)) continue;
		int s = (ny * MAP_W + nx) * 4 + k;
		lt_dist[s] = 1; lt_turns[s] = 0; lt_prev[s] = -1;
		lt_q[lt_tail++] = (int16_t)s;
	}
	while (h < lt_tail) {
		int s = lt_q[h++];
		if (best >= 0 && lt_dist[s] >= lt_dist[best]) break;   /* (the target's level done) */
		if (s / 4 == ty * MAP_W + tx) { if (best < 0 || lt_turns[s] < lt_turns[best]) best = s; continue; }
		for (int k = 0; k < 4; ++k) lt_relax(s, k, sx, sy);
	}
	/* (the target's other states of its level, reached after the first) */
	for (int k = 0; k < 4 && best >= 0; ++k) {
		int s = (ty * MAP_W + tx) * 4 + k;
		if (lt_dist[s] == lt_dist[best] && lt_turns[s] < lt_turns[best]) best = s;
	}
	if (best < 0) return -1;
	int n = 0;
	for (int s = best; s >= 0; s = lt_prev[s]) route_walk[n++] = (int16_t)(s / 4);
	return n;
}

/* What stands on the floor, into solid, but the target (tx, ty): the walk
 * goes round it (it keeps every panel in reach, test_core.c), as the
 * arrow led into a Mystery Data at a walkway's mouth. MegaMan beside one,
 * in its panel's corner: the walk from the free panel nearest him, *sx
 * and *sy (from the object's own it led through it, and he pushed into it
 * for good). */
static void floor_objects(double px, double py, int tx, int ty, int *sx, int *sy) {
	memset(solid, 0, sizeof solid);
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].solid && floor_at((int)layer.obj[i].x, (int)layer.obj[i].y)) solid[(int)layer.obj[i].y][(int)layer.obj[i].x] = 1;
	solid[ty][tx] = 0;
	if (solid[*sy][*sx]) {
		double best = 1e9;
		int bx = *sx, by = *sy;
		for (int k = 0; k < 8; ++k) {
			static const int d[8][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };
			int x = *sx + d[k][0], y = *sy + d[k][1];
			if (!floor_at(x, y) || solid[y][x]) continue;
			double dist = (x - px) * (x - px) + (y - py) * (y - py);
			if (dist < best) { best = dist; bx = x; by = y; }
		}
		*sx = bx;
		*sy = by;
	}
	solid[*sy][*sx] = 0;
}

int route_way(double px, double py, int tx, int ty, int *len) {
	int sx = (int)lround(px), sy = (int)lround(py);
	route_walk_len = 0;
	route_walk_aim = -1;
	if (!floor_at(sx, sy) || !floor_at(tx, ty)) return -1;
	floor_objects(px, py, tx, ty, &sx, &sy);
	/* the walk, backwards from the target */
	int n = sx == tx && sy == ty ? 0 : least_turns(sx, sy, tx, ty);
	if (n < 0) return -1;
	int16_t *path = route_walk;
	*len = n;
	/* (in a Mystery Data's panel, out to the one the walk sets off from:
	 * aimed along the walk from there, the arrow led into the data) */
	bool across = false, out = sx != (int)lround(px) || sy != (int)lround(py), axes = n && !out && !open_floor(sx, sy);
	int mouth = out ? -1 : mouth_aim(px, py, sx, sy, path, n, &across);
	int slide = axes && mouth >= 0 ? slide_way(px, py, sx, sy, path, n, mouth) : -1;
	int aim = out ? sy * MAP_W + sx : walk_aim(sx, sy, path, n, mouth, axes, &across);
	route_walk_len = n;
	route_walk_aim = aim;
	if (slide >= 0) return slide;
	double ax = aim % MAP_W - px, ay = aim / MAP_W - py;
	/* (a walkway along x is lined up on y) */
	if (across && path[n - 1] / MAP_W == aim / MAP_W) ax = 0;
	else if (across) ay = 0;
	/* (off open floor, the way of the first step itself: the line to a
	 * panel's middle tipped where MegaMan stood aside on a band) */
	else if (axes) { ax = path[n - 1] % MAP_W - sx; ay = path[n - 1] / MAP_W - sy; }
	return route_grid_way(ax, ay);
}
