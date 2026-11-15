/* The guardian's arena (docs/world/level-design/, Guardians): a square platform
 * at the end of a single bridge, as far from MegaMan's arrival as the layer
 * allows. The room the bridge leaves from is the antechamber, where the
 * last services before the fight stand. */
#include "net_arena.h"

#include <stdlib.h>
#include <string.h>

#include "net_gen.h"
#include "net_shapes.h"

/* (a gate the walk meets on its line beats one this many panels' walk farther) */
#define GATE_LINED 1000

/* The arena's box for a bridge of `len` leaving (ex, ey) in direction d:
 * the bridge meets the middle of its near side. */
static void arena_box(int ex, int ey, int d, int len, int n, int *x0, int *y0) {
	int bx = ex + dir_dx[d] * len, by = ey + dir_dy[d] * len;   /* the bridge's last cell */
	*x0 = d == DIR_E ? bx + 1 : d == DIR_W ? bx - n : bx - n / 2;
	*y0 = d == DIR_S ? by + 1 : d == DIR_N ? by - n : by - n / 2;
}

/* Whether a bridge of `len` from (ex, ey) in direction d runs through the
 * window with nothing beside it. */
static bool bridge_clear(int ex, int ey, int d, int len) {
	for (int k = 1; k <= len; ++k) {
		int x = ex + dir_dx[d] * k, y = ey + dir_dy[d] * k;
		if (!win_in(x, y) || floor_at(x, y)) return false;
		int s1 = (d + 1) % 4, s2 = (d + 3) % 4;
		if (floor_at(x + dir_dx[s1], y + dir_dy[s1]) || floor_at(x + dir_dx[s2], y + dir_dy[s2])) return false;
	}
	return true;
}

/* Whether the walk from the arrival (dist) meets the bridge leaving (x, y)
 * in direction d on its line: each step it may take into (x, y) runs on
 * into the bridge, or runs along the room's side to where the side ends,
 * its edge stopping MegaMan on the bridge's line. A gate in the middle of
 * a side the walk runs along is a mouth he walks past (BN6's walk enters
 * a one-wide walkway only from its lane: session 77's playtesters stood
 * at the antechamber's side holding the bridge's way). */
static bool gate_lined(int16_t dist[MAP_H][MAP_W], int x, int y, int d) {
	int steps = 0;
	for (int k = 0; k < 4; ++k) {
		int px = x - dir_dx[k], py = y - dir_dy[k];
		if (!win_in(px, py) || dist[py][px] < 0 || dist[py][px] != dist[y][x] - 1) continue;
		++steps;
		if (k != d && floor_at(x + dir_dx[k], y + dir_dy[k])) return false;
	}
	return steps > 0;
}

/* Where the bridge leaves room r's side d: the middle of that side
 * (room_edge), or where the walk meets it on its line, the side's
 * outermost panel nearest the middle (gate_lined); *lined whether it
 * does. */
static bool gate_start(int16_t dist[MAP_H][MAP_W], int r, int d, int *ex, int *ey, bool *lined) {
	if (!room_edge(r, d, ex, ey) || dist[*ey][*ex] < 0) return false;
	*lined = gate_lined(dist, *ex, *ey, d);
	if (*lined) return true;
	const Room *m = &layer.rooms[r];
	int best = 1 << 30;
	for (int j = m->y; j < m->y + m->h; ++j)
		for (int i = m->x; i < m->x + m->w; ++i) {
			bool side = d == DIR_E || d == DIR_W ? i == *ex : j == *ey;
			if (!side || !floor_at(i, j) || floor_at(i + dir_dx[d], j + dir_dy[d]) || dist[j][i] < 0 || !gate_lined(dist, i, j, d)) continue;
			int off = abs(i - *ex) + abs(j - *ey);
			if (off < best) { best = off; *ex = i; *ey = j; }
		}
	*lined = best < (1 << 30);
	return true;
}

/* (as far from the arrival as there is room for, past the layer's
 * signature where it can be: the way to the guardian runs through it,
 * docs/world/level-design/identity.md; a gate the walk meets on its line
 * first, wherever one is) */
int arena_attach(int n, ArenaInfo *out, bool lined_first, int rise) {
	static int16_t dist[MAP_H][MAP_W];
	ng_walk_past_signature(dist);
	int best = -1, best_r = -1, best_d = 0, best_len = 0, best_x = 0, best_y = 0;
	for (int r = 1; r < layer.nrooms; ++r)
		for (int d = 0; d < 4; ++d) {
			int ex, ey;
			bool lined = false;
			if (lined_first ? !gate_start(dist, r, d, &ex, &ey, &lined) : !room_edge(r, d, &ex, &ey) || dist[ey][ex] < 0) continue;
			for (int len = 3; len <= 5; ++len) {
				int x0, y0;
				arena_box(ex, ey, d, len, n, &x0, &y0);
				if (!bridge_clear(ex, ey, d, len) || !box_free(x0, y0, n, n, 1)) continue;
				bool lines = lined && lined_first && ng_fits_with(rise, x0, y0, x0 + n - 1, y0 + n - 1);
				int score = dist[ey][ex] + len + (lines ? GATE_LINED : 0);
				if (score > best) { best = score; best_r = r; best_d = d; best_len = len; best_x = ex; best_y = ey; }
			}
		}
	if (best_r < 0) return -1;
	int ex = best_x, ey = best_y, x0, y0;
	arena_box(ex, ey, best_d, best_len, n, &x0, &y0);
	bridge_line(ex, ey, best_d, best_len);
	carve_shape(n >= 7 ? SHAPE_OCTAGON : SHAPE_RECT, x0, y0, n, n);
	int a = add_room(x0, y0, n, n, ROOM_FIELD);
	if (a < 0) return -1;
	out->room = a;
	out->ante = best_r;
	out->dir = best_d;
	/* the exit waits on the far side, behind the guardian */
	out->exit_x = layer.rooms[a].ax + dir_dx[best_d] * (n / 2);
	out->exit_y = layer.rooms[a].ay + dir_dy[best_d] * (n / 2);
	return a;
}

bool arena_approach(int x, int y) {
	if (layer.arena < 0 || layer.ante < 0) return false;
	const Room *r = &layer.rooms[layer.ante], *a = &layer.rooms[layer.arena];
	if (x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h) return true;
	/* the bridge meets the middle of the arena's near side (arena_box):
	 * on that line, between the two boxes (where it starts inside the
	 * antechamber's box, that box holds it) */
	int d = layer.arena_dir;
	if (d == DIR_E || d == DIR_W) {
		int from = d == DIR_E ? r->x + r->w : a->x + a->w, to = d == DIR_E ? a->x : r->x;
		return y == a->ay && x >= from && x < to;
	}
	int from = d == DIR_S ? r->y + r->h : a->y + a->h, to = d == DIR_S ? a->y : r->y;
	return x == a->ax && y >= from && y < to;
}
