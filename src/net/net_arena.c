/* The guardian's arena (docs/LEVEL_DESIGN.md, Guardians): a square platform
 * at the end of a single bridge, as far from MegaMan's arrival as the layer
 * allows. The room the bridge leaves from is the antechamber, where the
 * last services before the fight stand. */
#include "net_arena.h"

#include <string.h>

#include "net_shapes.h"

/* Walking distance of every floor cell from room 0's standing cell. */
static void distances(int16_t dist[MAP_H][MAP_W]) {
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(dist, -1, sizeof(int16_t) * MAP_W * MAP_H);
	int h = 0, t = 0;
	qx[t] = (int16_t)layer.rooms[0].ax; qy[t++] = (int16_t)layer.rooms[0].ay;
	dist[qy[0]][qx[0]] = 0;
	while (h < t) {
		int x = qx[h], y = qy[h++];
		for (int d = 0; d < 4; ++d) {
			int nx = x + dir_dx[d], ny = y + dir_dy[d];
			if (!floor_at(nx, ny) || dist[ny][nx] >= 0) continue;
			dist[ny][nx] = (int16_t)(dist[y][x] + 1);
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
}

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

int arena_attach(int n, ArenaInfo *out) {
	static int16_t dist[MAP_H][MAP_W];
	distances(dist);
	int best = -1, best_r = -1, best_d = 0, best_len = 0;
	for (int r = 1; r < layer.nrooms; ++r)
		for (int d = 0; d < 4; ++d) {
			int ex, ey;
			if (!room_edge(r, d, &ex, &ey) || dist[ey][ex] < 0) continue;
			for (int len = 3; len <= 5; ++len) {
				int x0, y0;
				arena_box(ex, ey, d, len, n, &x0, &y0);
				if (!bridge_clear(ex, ey, d, len) || !box_free(x0, y0, n, n, 1)) continue;
				int score = dist[ey][ex] + len;
				if (score > best) { best = score; best_r = r; best_d = d; best_len = len; }
			}
		}
	if (best_r < 0) return -1;
	int ex, ey, x0, y0;
	room_edge(best_r, best_d, &ex, &ey);
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
