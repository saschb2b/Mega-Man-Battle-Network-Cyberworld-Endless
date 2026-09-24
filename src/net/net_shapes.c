/* Carving the pieces of a layer into layer.cell (see net_shapes.h). */
#include "net_shapes.h"

#include <stdlib.h>
#include <string.h>

#include "game.h"

const int dir_dx[4] = { 1, 0, -1, 0 }, dir_dy[4] = { 0, 1, 0, -1 };

static bool in_map(int x, int y) { return x >= 1 && y >= 1 && x < MAP_W - 1 && y < MAP_H - 1; }

bool win_in(int x, int y) {
	int dx = x - WIN_C, dy = y - WIN_C;
	return in_map(x, y) && abs(dx - dy) <= WIN_U && abs(dx + dy) <= WIN_V;
}

bool floor_at(int x, int y) { return in_map(x, y) && layer.cell[y][x] == C_PATH; }

void put(int x, int y) {
	if (win_in(x, y)) layer.cell[y][x] = C_PATH;
}

bool box_free(int x, int y, int w, int h, int margin) {
	for (int j = y - margin; j < y + h + margin; ++j)
		for (int i = x - margin; i < x + w + margin; ++i) {
			bool inner = i >= x && i < x + w && j >= y && j < y + h;
			if ((inner && !win_in(i, j)) || floor_at(i, j)) return false;
		}
	return true;
}

static void fill(int x, int y, int w, int h) {
	for (int j = y; j < y + h; ++j)
		for (int i = x; i < x + w; ++i) put(i, j);
}

static void clear(int x, int y) {
	if (in_map(x, y)) layer.cell[y][x] = C_VOID;
}

void carve_shape(int shape, int x, int y, int w, int h) {
	switch (shape) {
	case SHAPE_OCTAGON: {
		/* corners cut: one cell, or a triangle of three on big ones */
		fill(x, y, w, h);
		int cut = w >= 6 && h >= 6 ? 2 : w >= 4 && h >= 4 ? 1 : 0;
		for (int k = 0; k < cut; ++k)
			for (int t = 0; t < cut - k; ++t) {
				clear(x + t, y + k); clear(x + w - 1 - t, y + k);
				clear(x + t, y + h - 1 - k); clear(x + w - 1 - t, y + h - 1 - k);
			}
		break;
	}
	case SHAPE_PLUS: {
		/* a cross: the middle third (at least one cell) of each side */
		int bw = w - 2 * (w / 3), bh = h - 2 * (h / 3);
		fill(x + (w - bw) / 2, y, bw, h);
		fill(x, y + (h - bh) / 2, w, bh);
		break;
	}
	case SHAPE_RAGGED:
		/* bites out of the edges and bumps beyond them, never the corners' neighbours */
		fill(x, y, w, h);
		for (int i = 2; i < w - 2; ++i) {
			if (rng_range(0, 99) < 30) clear(x + i, y);
			else if (rng_range(0, 99) < 20) put(x + i, y - 1);
			if (rng_range(0, 99) < 30) clear(x + i, y + h - 1);
			else if (rng_range(0, 99) < 20) put(x + i, y + h);
		}
		for (int j = 2; j < h - 2; ++j) {
			if (rng_range(0, 99) < 30) clear(x, y + j);
			else if (rng_range(0, 99) < 20) put(x - 1, y + j);
			if (rng_range(0, 99) < 30) clear(x + w - 1, y + j);
			else if (rng_range(0, 99) < 20) put(x + w, y + j);
		}
		break;
	case SHAPE_HOLED:
		/* Graveyard's slabs: a grid of single holes inside a solid rim */
		fill(x, y, w, h);
		for (int j = y + 2; j < y + h - 2; j += 3)
			for (int i = x + 2; i < x + w - 2; i += 3) clear(i, j);
		break;
	case SHAPE_CRATER:
		/* a field around a pit, as Central Area 3's */
		fill(x, y, w, h);
		for (int j = y + 3; j < y + h - 3; ++j)
			for (int i = x + 3; i < x + w - 3; ++i) clear(i, j);
		break;
	default:
		fill(x, y, w, h);
		break;
	}
}

int add_room(int x, int y, int w, int h, int kind) {
	if (layer.nrooms >= MAX_ROOMS) return -1;
	Room *r = &layer.rooms[layer.nrooms];
	*r = (Room){ x, y, w, h, -1, -1, kind };
	/* the floor cell nearest the middle */
	int best = 1 << 30;
	for (int j = y; j < y + h; ++j)
		for (int i = x; i < x + w; ++i) {
			int d = abs(2 * i + 1 - (2 * x + w)) + abs(2 * j + 1 - (2 * y + h));
			if (floor_at(i, j) && d < best) { best = d; r->ax = i; r->ay = j; }
		}
	if (r->ax < 0) return -1;
	return layer.nrooms++;
}

bool bridge_line(int x, int y, int d, int len) {
	for (int k = 1; k <= len; ++k)
		if (!win_in(x + dir_dx[d] * k, y + dir_dy[d] * k)) return false;
	for (int k = 1; k <= len; ++k) put(x + dir_dx[d] * k, y + dir_dy[d] * k);
	return true;
}

static void raw_put(int x, int y) {
	if (in_map(x, y)) layer.cell[y][x] = C_PATH;
}

void bridge_l(int ax, int ay, int bx, int by, bool x_first) {
	int x = ax, y = ay;
	raw_put(x, y);
	while (x != bx || y != by) {
		if (x_first ? x != bx : y == by) x += x < bx ? 1 : -1;
		else y += y < by ? 1 : -1;
		raw_put(x, y);
	}
}

int pad_spur(int x, int y, int d, int len) {
	int sx = dir_dx[(d + 1) % 4], sy = dir_dy[(d + 1) % 4];   /* across */
	int px = x + dir_dx[d] * (len + 2), py = y + dir_dy[d] * (len + 2);   /* the pad's middle */
	/* the bridge (past its first cell) and the pad, a cell clear around */
	for (int k = 2; k <= len + 4; ++k)
		for (int s = -2; s <= 2; ++s) {
			int i = x + dir_dx[d] * k + sx * s, j = y + dir_dy[d] * k + sy * s;
			bool pad = k >= len + 1 && k <= len + 3 && abs(s) <= 1;
			if (floor_at(i, j) || ((pad || s == 0) && k <= len + 3 && !win_in(i, j))) return -1;
		}
	bridge_line(x, y, d, len);
	carve_shape(SHAPE_RECT, px - 1, py - 1, 3, 3);
	return add_room(px - 1, py - 1, 3, 3, ROOM_PAD);
}

void teeth(int x, int y, int d, int len, int side) {
	int s = side > 0 ? (d + 1) % 4 : (d + 3) % 4;
	for (int k = 1; k <= len; k += 2) {
		int i = x + dir_dx[d] * k + dir_dx[s], j = y + dir_dy[d] * k + dir_dy[s];
		/* only where the tooth stands alone */
		if (floor_at(i, j) || floor_at(i + dir_dx[s], j + dir_dy[s]) ||
			floor_at(i + dir_dx[d], j + dir_dy[d]) || floor_at(i - dir_dx[d], j - dir_dy[d])) continue;
		put(i, j);
	}
}

bool room_edge(int r, int d, int *ex, int *ey) {
	const Room *m = &layer.rooms[r];
	int cand[64][2], n = 0;
	for (int j = m->y; j < m->y + m->h; ++j)
		for (int i = m->x; i < m->x + m->w; ++i)
			if (floor_at(i, j) && !floor_at(i + dir_dx[d], j + dir_dy[d]) && n < 64) { cand[n][0] = i; cand[n][1] = j; ++n; }
	if (!n) return false;
	/* the middle of that side, give or take one */
	int best = -1, bd = 1 << 30;
	for (int k = 0; k < n; ++k) {
		int along = d == DIR_E || d == DIR_W ? cand[k][1] - m->ay : cand[k][0] - m->ax;
		int across = d == DIR_E ? -cand[k][0] : d == DIR_W ? cand[k][0] : d == DIR_S ? -cand[k][1] : cand[k][1];
		int score = abs(along) * 4 + across * 8;
		if (score < bd) { bd = score; best = k; }
	}
	*ex = cand[best][0];
	*ey = cand[best][1];
	return true;
}

/* ---- joining ---- */

static int16_t comp[MAP_H][MAP_W];

static int label(void) {
	memset(comp, -1, sizeof comp);
	static int16_t q[MAP_W * MAP_H];
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!floor_at(x, y) || comp[y][x] >= 0) continue;
			int h = 0, t = 0;
			q[t++] = (int16_t)(y * MAP_W + x);
			comp[y][x] = (int16_t)n;
			while (h < t) {
				int c = q[h++], cx = c % MAP_W, cy = c / MAP_W;
				for (int d = 0; d < 4; ++d) {
					int nx = cx + dir_dx[d], ny = cy + dir_dy[d];
					if (!floor_at(nx, ny) || comp[ny][nx] >= 0) continue;
					comp[ny][nx] = (int16_t)n;
					q[t++] = (int16_t)(ny * MAP_W + nx);
				}
			}
			++n;
		}
	return n;
}

void connect_all(void) {
	if (!layer.nrooms) return;
	for (int guard = 0; guard < 32 && label() > 1; ++guard) {
		int main = comp[layer.rooms[0].ay][layer.rooms[0].ax];
		static int16_t mx[MAP_W * MAP_H], my[MAP_W * MAP_H];
		int nm = 0;
		for (int y = 0; y < MAP_H; ++y)
			for (int x = 0; x < MAP_W; ++x)
				if (floor_at(x, y) && comp[y][x] == main) { mx[nm] = (int16_t)x; my[nm++] = (int16_t)y; }
		/* the closest pair of cells between the main piece and any other */
		int best = 1 << 30, ax = 0, ay = 0, bx = 0, by = 0;
		for (int y = 0; y < MAP_H; ++y)
			for (int x = 0; x < MAP_W; ++x) {
				if (!floor_at(x, y) || comp[y][x] == main) continue;
				for (int k = 0; k < nm; ++k) {
					int d = abs(mx[k] - x) + abs(my[k] - y);
					if (d < best) { best = d; ax = mx[k]; ay = my[k]; bx = x; by = y; }
				}
			}
		/* bend where the corner stays in the window */
		bridge_l(ax, ay, bx, by, win_in(bx, ay));
	}
}

int dead_ends(int *xs, int *ys, int max) {
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!floor_at(x, y)) continue;
			int k = 0;
			for (int d = 0; d < 4; ++d) k += floor_at(x + dir_dx[d], y + dir_dy[d]);
			if (k == 1 && n < max) { xs[n] = x; ys[n] = y; ++n; }
		}
	return n;
}
