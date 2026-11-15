/* Raised floor (docs/world/level-design/heights.md). Every ramp of the
 * originals climbs up the screen (towards grid -x or -y), so a raised floor
 * is reached only from its front: floor the rest of the layer meets only on
 * its +x and +y sides is lifted one level, and each corridor into it
 * becomes the area's own stair for its first panels. Two shapes of it:
 *
 *  - a room alone: a dead end with one stair, a room on the way with a
 *    stair at each end;
 *  - a terrace: the floor behind a line across the screen (cells with
 *    x + y, x or y under a bound, as far as they join the room it starts
 *    from), rooms and the walkways between them, whose every way out
 *    crosses that line towards the front.
 *
 * Where the area's originals raise every platform over the walkways
 * (`terraced`: Green's grass, CopyBot Comp's plateaus) every place that can
 * be is, else most. Never the arrival, the exit, the signature, the
 * guardian's arena or the room before it, a trail's legs, a dark layer.
 *
 * The map draws floor raised by a panel's height where flat floor one
 * panel up the screen would be (grid x - 1, y - 1; two panels for two), so
 * that place and its neighbours, and what its side faces cover on the way
 * down, must be free of other floor. Decisions use their own random
 * numbers, so the rest of the layer does not change with them. */
#include <stdlib.h>
#include <string.h>

#include "net.h"

#define RAISE_CHANCE 85   /* % of the places that could be raised */
#define MAX_ENTRIES 6     /* ways into one raised place */
#define TERRACE_DEPTH 4   /* bounds tried past a room's front corner */
#define TERRACE_SHARE 45  /* % of the layer's floor a terrace takes at most */

static uint32_t rnd;
static int roll(int n) { rnd = rnd * 1103515245u + 12345u; return (int)((rnd >> 16) % (uint32_t)n); }

static bool inside_map(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H; }
static bool floor_at(int x, int y) { return inside_map(x, y) && layer.cell[y][x] == C_PATH; }
static bool void_at(int x, int y) { return !inside_map(x, y) || layer.cell[y][x] == C_VOID; }

static bool in_room(const Room *r, int x, int y) { return x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h; }

/* the place being tried, and the cells taken by a raised place, a stair or
 * its landing: none of a later place's (a stair's foot on raised floor, two
 * stairs met) */
static uint8_t sel[MAP_H][MAP_W], taken[MAP_H][MAP_W];
static bool in_sel(int x, int y) { return inside_map(x, y) && sel[y][x]; }

/* A way into the place: its first cell outside, on its side 0 (+x) or 1
 * (+y), how many cells wide (1 or 2) and the stair planned in it. */
typedef struct { int side, x, y, n; Stair st; } Entry;

/* Floor cell (nx, ny) on side `side` added to the ways in: one more cell
 * of a run before it (along y on side 0, x on side 1), else a way of its
 * own; false where a run grows past two or the ways past MAX_ENTRIES. */
static bool add_way(Entry *e, int *ne, int side, int nx, int ny) {
	for (int i = 0; i < *ne; ++i) {
		Entry *r = &e[i];
		if (r->side == side && (side == 0 ? nx == r->x && ny == r->y + r->n : ny == r->y && nx == r->x + r->n)) return ++r->n <= 2;
	}
	if (*ne >= MAX_ENTRIES) return false;
	e[(*ne)++] = (Entry){ side, nx, ny, 1, { 0 } };
	return true;
}

/* The ways in on side `side` beside the place's cell (x, y); false where
 * floor meets it from -x or -y. */
static bool cell_ways(int x, int y, int side, Entry *e, int *ne) {
	static const int d[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	for (int k = 0; k < 4; ++k) {
		int nx = x + d[k][0], ny = y + d[k][1];
		if (!floor_at(nx, ny) || sel[ny][nx]) continue;
		if (k >= 2) return false;
		if (k == side && !add_way(e, ne, side, nx, ny)) return false;
	}
	return true;
}

/* The place's ways in, as runs of floor along its edge; false where one
 * comes from a side no stair climbs from (-x, -y) or is wider than two. */
static bool entries(Entry *e, int *ne) {
	*ne = 0;
	for (int side = 0; side < 2; ++side)
		for (int y = 0; y < MAP_H; ++y)
			for (int x = 0; x < MAP_W; ++x)
				if (sel[y][x] && !cell_ways(x, y, side, e, ne)) return false;
	return *ne > 0;
}

/* Cell (a, b) along the corridor: a cells out from the place, b across
 * from its first row or column. */
static void along(const Entry *e, int a, int b, int *x, int *y) {
	*x = e->side == 0 ? e->x + a : e->x + b;
	*y = e->side == 0 ? e->y + b : e->y + a;
}

/* The stair's first row or column across the corridor (0, or -1 where
 * the run is widened towards its other side: both columns top onto the
 * place's floor, one on an octagon's cut corner climbed into the void);
 * 2 where it tops onto none. */
static int top_row(const Entry *e, int wide) {
	int b0 = 0, tx, ty;
	if (e->n < wide) {
		along(e, -1, 1, &tx, &ty);
		if (!in_sel(tx, ty)) b0 = -1;
	}
	for (int b = b0; b < b0 + wide; ++b) {
		along(e, -1, b, &tx, &ty);
		if (!in_sel(tx, ty)) return 2;
	}
	return b0;
}

/* Cell (a, b) of the stair's flight or beside it: nothing taken or raised
 * there; the corridor itself floor, or void its widening fills; void
 * beside the stair, so no floor meets its ramp mid-climb; no room's floor
 * under it. */
static bool flight_cell(const Entry *e, int a, int b, int b0, int wide, int len) {
	int x, y;
	along(e, a, b, &x, &y);
	if (!inside_map(x, y) || taken[y][x] || sel[y][x] || layer.level[y][x]) return false;
	bool in = b >= b0 && b < b0 + wide, run = b >= 0 && b < e->n;
	if (in && run && !floor_at(x, y)) return false;
	if (a == len) return true;
	if (!void_at(x, y) && (!in || !run)) return false;
	for (int k = 0; in && k < layer.nrooms; ++k)
		if (in_room(&layer.rooms[k], x, y)) return false;
	return true;
}

/* The landing: the corridor runs on past the stair, and its widening
 * leaves no one-panel pit beside it, on along the corridor or across it
 * (a playtester stuck by one, session 76). */
static bool landing(const Entry *e, int b0, int wide, int len) {
	int x, y, px, py, fx, fy;
	for (int b = 0; b < e->n; ++b) {
		along(e, len, b, &x, &y);
		if (!floor_at(x, y)) return false;
	}
	for (int b = b0; b < b0 + wide; ++b) {
		int out = b < 0 ? -1 : 1;
		along(e, len, b, &x, &y);
		if (floor_at(x, y)) continue;
		along(e, len + 1, b, &px, &py);
		along(e, len + 2, b, &fx, &fy);
		if (void_at(px, py) && floor_at(fx, fy)) return false;
		along(e, len, b + out, &px, &py);
		along(e, len, b + 2 * out, &fx, &fy);
		if (void_at(px, py) && floor_at(fx, fy)) return false;
	}
	return true;
}

/* The stair in corridor `e` of the width asked (its run widened from one to
 * two where the place's floor lies beside it), `len` panels from the place,
 * the corridor straight on past it to a landing. */
static bool fits(Entry *e, int width, int len) {
	int wide = width == STAIR_NARROW ? 1 : 2;
	if (e->n > wide) return false;
	int b0 = top_row(e, wide);
	if (b0 > 1) return false;
	for (int a = 0; a <= len; ++a)
		for (int b = b0 - 1; b <= b0 + wide; ++b)
			if (!flight_cell(e, a, b, b0, wide, len)) return false;
	if (!landing(e, b0, wide, len)) return false;
	int tx, ty;
	along(e, 0, b0, &tx, &ty);
	e->st = (Stair){ .x = tx, .y = ty, .dir = e->side == 0 ? STAIR_UP_NX : STAIR_UP_NY, .wide = wide, .len = len };
	return true;
}

/* the stair and its landing */
static bool in_flight(const Stair *s, int x, int y) {
	int w = stair_w(s) + (s->dir == STAIR_UP_NX), h = stair_h(s) + (s->dir == STAIR_UP_NY);
	return x >= s->x && x < s->x + w && y >= s->y && y < s->y + h;
}

/* The stair for each way in: the corridor's own width where the area has
 * one (a ladder up a walkway, two panels up a wide way), else the other;
 * the stairs apart from each other. */
static bool plan_stairs(Entry *e, int ne, const LayerKit *kit) {
	for (int i = 0; i < ne; ++i) {
		int dir = e[i].side == 0 ? STAIR_UP_NX : STAIR_UP_NY;
		bool has[STAIR_WIDTHS] = { kit->stair_dirs >> dir & 1, kit->stair_narrow >> dir & 1 };
		int first = e[i].n == 1 ? STAIR_NARROW : STAIR_WIDE;
		bool ok = false;
		for (int k = 0; k < 2 && !ok; ++k) {
			int w = k ? !first : first;
			int len = kit->stair_len[dir][w] ? kit->stair_len[dir][w] : 2;
			ok = has[w] && fits(&e[i], w, len);
		}
		if (!ok) return false;
		for (int j = 0; j < i; ++j) {
			const Stair *a = &e[i].st, *b = &e[j].st;
			if (a->x <= b->x + stair_w(b) && b->x <= a->x + stair_w(a) && a->y <= b->y + stair_h(b) && b->y <= a->y + stair_h(a)) return false;
		}
	}
	return true;
}

static bool in_stairs(const Entry *e, int ne, int x, int y) {
	for (int i = 0; i < ne; ++i)
		if (in_flight(&e[i].st, x, y)) return true;
	return false;
}

/* Nothing else within one cell of where the place's cell (x, y) is drawn
 * (t panels up the screen), but its stairs. */
static bool clear_round(int x, int y, int t, const Entry *e, int ne) {
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) {
			int ox = x - t + dx, oy = y - t + dy;
			if (floor_at(ox, oy) && !sel[oy][ox] && !in_stairs(e, ne, ox, oy)) return false;
		}
	return true;
}

/* Nothing else within one cell of where the place is drawn, nor where its
 * side faces hang on the way down to its own place. */
static bool clear_on_screen(int shift, const Entry *e, int ne) {
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			for (int t = 1; sel[y][x] && t <= shift; ++t)
				if (!clear_round(x, y, t, e, ne)) return false;
	return true;
}

/* The rooms never raised. */
static bool kept_flat(int i) {
	return i == 0 || i == layer.exit_room || i == layer.sig_room || i == layer.arena || i == layer.ante || layer.rooms[i].kind == ROOM_LEG;
}

/* Every room the place holds a cell of it holds whole, and none kept flat;
 * how many it holds. */
static int whole_rooms(void) {
	int n = 0;
	for (int i = 0; i < layer.nrooms; ++i) {
		const Room *r = &layer.rooms[i];
		int in = 0, all = 0;
		for (int y = r->y; y < r->y + r->h; ++y)
			for (int x = r->x; x < r->x + r->w; ++x) {
				if (!floor_at(x, y)) continue;
				++all;
				in += sel[y][x];
			}
		if (!in) continue;
		if (in != all || kept_flat(i)) return -1;
		++n;
	}
	return n;
}

/* Whether the place in sel can be raised: its stairs planned in e. */
static bool can_raise(const LayerKit *kit, int shift, int floor_cells, Entry *e, int *ne) {
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!sel[y][x]) continue;
			if (taken[y][x] || layer.level[y][x]) return false;
			++n;
		}
	if (n * 100 > floor_cells * TERRACE_SHARE || whole_rooms() < 1) return false;
	return entries(e, ne) && layer.nstairs + *ne <= MAX_STAIRS && plan_stairs(e, *ne, kit) && clear_on_screen(shift, e, *ne);
}

static void raise_sel(const Entry *e, int ne, int rise) {
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (sel[y][x]) layer.level[y][x] = taken[y][x] = 1;
	for (int i = 0; i < ne; ++i) {
		const Stair *s = &e[i].st;
		int w = stair_w(s) + (s->dir == STAIR_UP_NX), h = stair_h(s) + (s->dir == STAIR_UP_NY);
		for (int y = s->y; y < s->y + h; ++y)
			for (int x = s->x; x < s->x + w; ++x) {
				/* (a wide stair's widening, and its landing's) */
				layer.cell[y][x] = C_PATH;
				taken[y][x] = 1;
			}
		layer.stair[layer.nstairs++] = *s;
	}
	layer.rise = rise;
}

static void select_room(const Room *r) {
	memset(sel, 0, sizeof sel);
	for (int y = r->y; y < r->y + r->h; ++y)
		for (int x = r->x; x < r->x + r->w; ++x) sel[y][x] = floor_at(x, y);
}

/* Whether cell (x, y) lies behind line `cut` (0: x + y, 1: x, 2: y)
 * under `bound`. */
static bool behind(int cut, int bound, int x, int y) { return (cut == 0 ? x + y : cut == 1 ? x : y) < bound; }

/* The floor joined to room r behind a line, the room wholly behind it. */
static void select_terrace(const Room *r, int cut, int bound) {
	static int16_t q[MAP_W * MAP_H][2];
	memset(sel, 0, sizeof sel);
	int n = 0;
	for (int y = r->y; y < r->y + r->h; ++y)
		for (int x = r->x; x < r->x + r->w; ++x)
			if (floor_at(x, y) && behind(cut, bound, x, y) && !sel[y][x]) { sel[y][x] = 1; q[n][0] = (int16_t)x; q[n++][1] = (int16_t)y; }
	static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int h = 0; h < n; ++h)
		for (int k = 0; k < 4; ++k) {
			int x = q[h][0] + d[k][0], y = q[h][1] + d[k][1];
			if (!floor_at(x, y) || sel[y][x] || !behind(cut, bound, x, y)) continue;
			sel[y][x] = 1;
			q[n][0] = (int16_t)x;
			q[n++][1] = (int16_t)y;
		}
}

void layer_raise_rooms(uint32_t seed, const LayerKit *kit) {
	rnd = seed ^ 0x5EEDu;
	memset(taken, 0, sizeof taken);
	if (!kit || !(kit->stair_dirs | kit->stair_narrow)) return;
	int rise = kit->rise, shift = rise / 32;
	if (shift < 1) return;
	int floor_cells = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) floor_cells += floor_at(x, y);
	for (int i = 1; i < layer.nrooms && layer.nstairs < MAX_STAIRS; ++i) {
		if (kept_flat(i) || taken[layer.rooms[i].ay][layer.rooms[i].ax]) continue;
		const Room *r = &layer.rooms[i];
		Entry e[MAX_ENTRIES];
		int ne = 0;
		/* the room alone, else a terrace behind a line past its front */
		select_room(r);
		bool ok = can_raise(kit, shift, floor_cells, e, &ne);
		int front[3] = { r->x + r->w + r->y + r->h - 2, r->x + r->w - 1, r->y + r->h - 1 };
		for (int t = 1; !ok && t <= TERRACE_DEPTH; ++t)
			for (int cut = 0; cut < 3 && !ok; ++cut) {
				select_terrace(r, cut, front[cut] + t);
				ok = can_raise(kit, shift, floor_cells, e, &ne);
			}
		if (!ok || roll(100) >= (kit->terraced ? 100 : RAISE_CHANCE)) continue;
		raise_sel(e, ne, rise);
	}
}
