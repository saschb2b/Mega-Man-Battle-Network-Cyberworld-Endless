/* Layouts after BN6's net areas (docs/LEVEL_DESIGN.md). Each builds from
 * the pieces in net_shapes.c; the generator then joins anything left
 * apart, picks where MegaMan arrives and checks the size. */
#include "net_layouts.h"

#include <stdlib.h>

#include "game.h"
#include "net_shapes.h"
#include "net_signature.h"
#include "run.h"

const char *const layout_names[LAYOUT_COUNT] = {
	"route", "field", "ladder", "hub", "slabs", "web", "crosses", "catwalks", "comb", "trail",
};

/* per area: weights of each layout (percent) */
static const uint8_t weights[BIOME_COUNT][LAYOUT_COUNT] = {
	/*                 route field ladder hub slabs web crosses catwalks comb trail */
	[BIOME_CENTRAL]   = { 0, 35, 0, 0, 0, 0, 0, 0, 30, 35 },
	[BIOME_SEASIDE]   = { 40, 60, 0, 0, 0, 0, 0, 0 },
	[BIOME_SKY]       = { 45, 0, 0, 55, 0, 0, 0, 0 },
	[BIOME_GREEN]     = { 45, 0, 55, 0, 0, 0, 0, 0 },
	[BIOME_GRAVEYARD] = { 0, 0, 0, 0, 65, 35, 0, 0 },
	[BIOME_UNDERNET]  = { 0, 0, 0, 0, 0, 45, 25, 30 },
	[BIOME_SECRET]    = { 25, 0, 0, 25, 0, 50, 0, 0 },
	[BIOME_NEST]      = { 0, 0, 0, 0, 40, 0, 60, 0 },
	[BIOME_COMP]      = { 30, 40, 0, 0, 0, 0, 30, 0 },
	[BIOME_HOMEPAGE]  = { 30, 0, 0, 40, 0, 0, 30, 0 },
	[BIOME_COMP_B]    = { 30, 0, 0, 0, 0, 30, 40, 0 },
	[BIOME_ROBOT_COMP]    = { 70, 0, 0, 0, 0, 0, 30, 0 },
	[BIOME_AQUARIUM_COMP] = { 0, 0, 0, 0, 0, 0, 0, 100 },
	[BIOME_JUDGE_COMP]    = { 0, 0, 0, 0, 0, 0, 0, 100 },
	[BIOME_WEATHER_COMP]  = { 0, 100, 0, 0, 0, 0, 0, 0 },
	[BIOME_COPYBOT_COMP]  = { 40, 0, 0, 0, 0, 30, 0, 30 },
	[BIOME_ACDC_HP]       = { 30, 0, 0, 30, 0, 0, 40, 0 },
	[BIOME_GREEN_HP]      = { 30, 0, 0, 30, 0, 0, 40, 0 },
	[BIOME_SKY_HP]        = { 30, 0, 0, 30, 0, 0, 40, 0 },
};

int layout_forced = -1;

int layout_weight(int biome, int layout) {
	if (biome < 0 || biome >= BIOME_COUNT || layout < 0 || layout >= LAYOUT_COUNT) return 0;
	return weights[biome][layout];
}

int layout_pick(int biome) {
	if (layout_forced >= 0 && layout_forced < LAYOUT_COUNT) return layout_forced;
	if (biome < 0 || biome >= BIOME_COUNT) biome = BIOME_CENTRAL;
	int roll = rng_range(0, 99), sum = 0;
	for (int l = 0; l < LAYOUT_COUNT; ++l) {
		sum += weights[biome][l];
		if (roll < sum) return l;
	}
	return LAYOUT_ROUTE;
}

int layout_in_act(int biome, uint32_t act_seed, int index) {
	if (layout_forced >= 0 && layout_forced < LAYOUT_COUNT) return layout_forced;
	if (biome < 0 || biome >= BIOME_COUNT) biome = BIOME_CENTRAL;
	/* a weighted draw without replacement, on its own numbers */
	int order[LAYOUT_COUNT], n = 0, left[LAYOUT_COUNT], total = 0;
	for (int l = 0; l < LAYOUT_COUNT; ++l) { left[l] = weights[biome][l]; total += left[l]; }
	uint32_t r = act_seed | 1;
	while (total > 0) {
		r = r * 1103515245u + 12345u;
		int roll = (int)((r >> 16) % (uint32_t)total), sum = 0;
		for (int l = 0; l < LAYOUT_COUNT; ++l) {
			sum += left[l];
			if (left[l] && roll < sum) { order[n++] = l; total -= left[l]; left[l] = 0; break; }
		}
	}
	return n ? order[index % n] : LAYOUT_ROUTE;
}

/* the platforms of an area's routes */
static int route_shape(int biome) {
	switch (biome) {
	case BIOME_SEASIDE: return rng_range(0, 1) ? SHAPE_RAGGED : SHAPE_RECT;
	case BIOME_SKY: return SHAPE_OCTAGON;
	case BIOME_GRAVEYARD: return SHAPE_RECT;
	case BIOME_UNDERNET: return rng_range(0, 1) ? SHAPE_OCTAGON : SHAPE_RAGGED;
	case BIOME_SECRET: return rng_range(0, 1) ? SHAPE_OCTAGON : SHAPE_PLUS;
	case BIOME_NEST: return rng_range(0, 1) ? SHAPE_PLUS : SHAPE_RECT;
	case BIOME_CENTRAL: return rng_range(0, 2) ? SHAPE_RECT : SHAPE_OCTAGON;
	case BIOME_WEATHER_COMP: return rng_range(0, 1) ? SHAPE_HOLED : SHAPE_RECT;
	case BIOME_COPYBOT_COMP: return SHAPE_RECT;
	case BIOME_JUDGE_COMP: return rng_range(0, 1) ? SHAPE_RAGGED : SHAPE_PLUS;
	default: return SHAPE_RECT;
	}
}

/* ---- helpers ---- */

/* A platform centred on (cx, cy) with a cell of void around it. */
static int platform(int cx, int cy, int w, int h, int shape, int kind) {
	int x = cx - w / 2, y = cy - h / 2;
	if (!box_free(x, y, w, h, 1)) return -1;
	carve_shape(shape, x, y, w, h);
	return add_room(x, y, w, h, kind);
}

/* The layer's signature in its box at `size`, `most` a side at most,
 * centred on (cx, cy) where the box and a panel round it are free; its room
 * or -1 (docs/LEVEL_DESIGN.md, Identity). */
static int sig_fit(int sig, int biome, int cx, int cy, int size, int most) {
	int w, h;
	sig_box(sig, biome, size, &w, &h);
	w = w > most ? most : w;
	h = h > most ? most : h;
	int x = cx - w / 2, y = cy - h / 2;
	return sig && box_free(x, y, w, h, 1) ? sig_carve(sig, biome, x, y, w, h) : -1;
}

/* The layer's signature in the w x h box centred on (cx, cy) where it has
 * room, else a platform of `shape` there. */
static int heart(int sig, int biome, int cx, int cy, int w, int h, int shape) {
	int x = cx - w / 2, y = cy - h / 2;
	if (sig && box_free(x, y, w, h, 1)) {
		int r = sig_carve(sig, biome, x, y, w, h);
		if (r >= 0) return r;
	}
	return platform(cx, cy, w, h, shape, ROOM_PLATFORM);
}

/* The floor cell of room r in line v (a row for d east or west, else a
 * column) furthest in direction d, where a bridge along d leaves it square
 * on: false where that is a corner (no floor beside it along the side) or
 * floor lies beyond it. */
static bool side_at(int r, int d, int v, int *x, int *y) {
	const Room *m = &layer.rooms[r];
	bool along_x = d == DIR_E || d == DIR_W;
	int lo = along_x ? m->x : m->y, n = along_x ? m->w : m->h, s = (d + 1) % 4;
	int step = d == DIR_E || d == DIR_S ? 1 : -1, from = step > 0 ? lo + n - 1 : lo;
	for (int k = 0; k < n; ++k) {
		int u = from - step * k, cx = along_x ? u : v, cy = along_x ? v : u;
		if (!floor_at(cx, cy)) continue;
		if (floor_at(cx + dir_dx[d], cy + dir_dy[d]) ||
			!floor_at(cx + dir_dx[s], cy + dir_dy[s]) || !floor_at(cx - dir_dx[s], cy - dir_dy[s])) return false;
		*x = cx;
		*y = cy;
		return true;
	}
	return false;
}

/* A bridge between rooms a and b that meets each square on in the middle
 * of a side, as the originals' walkways meet their platforms (one entering
 * at a corner, or running into a platform's edge, is a join their tiles
 * never draw): straight across where the two face each other; else out of
 * a towards b and one bend into b's near side; else with a jog halfway. */
static void link(int a, int b) {
	const Room *ra = &layer.rooms[a], *rb = &layer.rooms[b];
	int dx = rb->ax - ra->ax, dy = rb->ay - ra->ay;
	int d = abs(dx) >= abs(dy) ? (dx > 0 ? DIR_E : DIR_W) : (dy > 0 ? DIR_S : DIR_N), back = (d + 2) % 4;
	bool along_x = d == DIR_E || d == DIR_W;
	int ax, ay, bx, by;
	/* straight: the line both sides cross nearest the rooms' middles */
	int lo_a = along_x ? ra->y : ra->x, lo_b = along_x ? rb->y : rb->x;
	int hi_a = lo_a + (along_x ? ra->h : ra->w) - 1, hi_b = lo_b + (along_x ? rb->h : rb->w) - 1;
	int mid = (along_x ? ra->ay + rb->ay : ra->ax + rb->ax) / 2, best = -1;
	for (int v = lo_a > lo_b ? lo_a : lo_b; v <= (hi_a < hi_b ? hi_a : hi_b); ++v)
		if ((best < 0 || abs(v - mid) < abs(best - mid)) && side_at(a, d, v, &ax, &ay) && side_at(b, back, v, &bx, &by)) best = v;
	if (best >= 0) {
		side_at(a, d, best, &ax, &ay);
		side_at(b, back, best, &bx, &by);
		bridge_l(ax, ay, bx, by, along_x);
		return;
	}
	if (!room_edge(a, d, &ax, &ay)) {
		bridge_l(ra->ax, ra->ay, rb->ax, rb->ay, true);
		return;
	}
	/* one bend: along d to b's middle, then across into b's near side, a
	 * row (column) of void between the first leg and b */
	int s = along_x ? (rb->ay > ay ? DIR_S : DIR_N) : (rb->ax > ax ? DIR_E : DIR_W);
	if (side_at(b, (s + 2) % 4, along_x ? rb->ax : rb->ay, &bx, &by)) {
		int gap = along_x ? abs(by - ay) : abs(bx - ax), length = along_x ? abs(bx - ax) : abs(by - ay);
		if (gap >= 2 && length >= 2) {
			bridge_l(ax, ay, bx, by, along_x);
			return;
		}
	}
	/* a jog halfway between the two sides' middles */
	if (room_edge(b, back, &bx, &by)) {
		int u0 = along_x ? ax : ay, u1 = along_x ? bx : by, um = (u0 + u1) / 2;
		if (abs(u1 - u0) >= 4) {
			int jx = along_x ? um : bx, jy = along_x ? by : um;
			bridge_l(ax, ay, jx, jy, along_x);
			bridge_l(jx, jy, bx, by, along_x);
			return;
		}
		bridge_l(ax, ay, bx, by, along_x);
		return;
	}
	bridge_l(ra->ax, ra->ay, rb->ax, rb->ay, true);
}

/* Whether a bridge along d may leave floor cell (x, y) square on: floor on
 * both sides of it across d (the middle of a side, or a walkway it leaves
 * sideways), never a corner. */
static bool square_on(int x, int y, int d) {
	int s = (d + 1) % 4;
	return floor_at(x + dir_dx[s], y + dir_dy[s]) && floor_at(x - dir_dx[s], y - dir_dy[s]);
}

static void random_cell(int *x, int *y) {
	*x = rng_range(WIN_C - 20, WIN_C + 20);
	*y = rng_range(WIN_C - 20, WIN_C + 20);
}

/* Pads on spurs off the floor, where there is room: every floor cell is
 * tried, in random order. */
static void spurs(int want, int minlen, int maxlen) {
	static int16_t cx[MAP_W * MAP_H], cy[MAP_W * MAP_H];
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (floor_at(x, y)) { cx[n] = (int16_t)x; cy[n++] = (int16_t)y; }
	for (int i = n - 1; i > 0; --i) {
		int j = rng_range(0, i);
		int16_t tx = cx[i], ty = cy[i];
		cx[i] = cx[j]; cy[i] = cy[j]; cx[j] = tx; cy[j] = ty;
	}
	for (int i = 0; i < n && want > 0; ++i) {
		int d0 = rng_range(0, 3);
		for (int k = 0; k < 4; ++k) {
			int d = (d0 + k) % 4;
			if (floor_at(cx[i] + dir_dx[d], cy[i] + dir_dy[d]) || !square_on(cx[i], cy[i], d)) continue;
			if (pad_spur(cx[i], cy[i], d, rng_range(minlen, maxlen)) >= 0) { --want; break; }
		}
	}
}

/* Dead-end stubs of 1-3 cells off the floor, clear on both sides. */
static void stubs(int want) {
	for (int tries = 0; tries < want * 60 && want > 0; ++tries) {
		int x, y;
		random_cell(&x, &y);
		int d = rng_range(0, 3), len = rng_range(1, 3);
		if (!floor_at(x, y) || !square_on(x, y, d)) continue;
		int s = (d + 1) % 4;
		bool ok = true;
		for (int k = 1; k <= len + 1 && ok; ++k)
			for (int t = -1; t <= 1 && ok; ++t) {
				int i = x + dir_dx[d] * k + dir_dx[s] * t, j = y + dir_dy[d] * k + dir_dy[s] * t;
				if (floor_at(i, j) || (k <= len && t == 0 && !win_in(i, j))) ok = false;
			}
		if (ok && bridge_line(x, y, d, len)) --want;
	}
}

/* ---- layouts ---- */

/* A route's next platform centred on (nx, ny): a slab, Robot Control's long
 * white slab, or a platform of the area's own shape. */
static int route_platform(int biome, int size, bool slabs, int nx, int ny) {
	int w, h, shape;
	if (slabs) { w = 7 + 2 * rng_range(0, size > 0); h = 7 + 2 * rng_range(0, 1); shape = SHAPE_HOLED; }
	else if (biome == BIOME_ROBOT_COMP) {
		/* Robot Control's long white slabs */
		int along = rng_range(6, 8 + size), across = rng_range(2, 3);
		bool wide = rng_range(0, 1);
		w = wide ? along : across; h = wide ? across : along; shape = SHAPE_RECT;
	}
	else { w = rng_range(3, 5 + size); h = rng_range(3, 5 + size); shape = route_shape(biome); }
	return platform(nx, ny, w, h, shape, ROOM_PLATFORM);
}

/* A route's chain as it grows: its area, size and signature (its box's side
 * `sw`), and where its last platform stands. */
typedef struct { int biome, size, sig, sw, x, y; bool slabs; } Chain;

/* The chain's next platform a step on from its last (the signature where
 * `big`; to it and on from it, `near`, a step past its half, the signature
 * whole in the window across the screen); its room, -1 where none fits. */
static int chain_step(Chain *c, bool big, bool near) {
	int made = -1;
	for (int tries = 0; tries < 24 && made < 0; ++tries) {
		int step = near ? (c->sw + 1) / 2 + (c->slabs ? 6 : 4) + rng_range(0, 3) : c->slabs ? rng_range(10, 12) : rng_range(6, 9);
		int sx = rng_range(0, step), nx = c->x + sx, ny = c->y + step - sx;
		if (abs(nx - ny) > (big ? WIN_U - 1 - (c->sw + 1) / 2 : WIN_U - 3)) continue;
		made = big ? sig_fit(c->sig, c->biome, nx, ny, c->size, 11) : route_platform(c->biome, c->size, c->slabs, nx, ny);
		if (made >= 0) { c->x = nx; c->y = ny; }
	}
	return made;
}

/* One shortcut between platforms two apart, when they are close (never
 * round the signature: the way goes through it). */
static void chain_shortcut(const int *chain, int n) {
	for (int a = 1; a + 2 < n; ++a) {
		const Room *ra = &layer.rooms[chain[a]], *rb = &layer.rooms[chain[a + 2]];
		if (chain[a + 1] == layer.sig_room) continue;
		if (abs(ra->ax - rb->ax) + abs(ra->ay - rb->ay) < 12 && rng_range(0, 2) == 0) { link(chain[a], chain[a + 2]); break; }
	}
}

/* Platforms in a chain winding down the window, its signature two or three
 * platforms in (a slab's chain, its steps longer, one or two; a guardian's
 * layer one sooner, its arena wanting room past it: the chain's heart, the
 * way runs through it, about the window's middle), then pads and stubs. */
static void route(int biome, int size, bool slabs, int sig) {
	Chain c = { biome, size, sig, 0, WIN_C - 10 + rng_range(-2, 2), 0, slabs };
	c.y = WIN_C - 10 + rng_range(-2, 2);
	int chain[10], n = 0, sh, sig_at = sig ? 2 - slabs - layer.boss_layer + rng_range(0, 1) : -1;
	/* (eleven a side at most: a chain wanders, and a bigger one seldom found
	 * room in it) */
	if (sig) {
		sig_box(sig, biome, size, &c.sw, &sh);
		c.sw = c.sw > 11 ? 11 : c.sw;
		sig_at = sig_at < 0 ? 0 : sig_at;
	}
	chain[n++] = platform(c.x, c.y, 3, 3, SHAPE_RECT, ROOM_PAD);
	for (int i = 0; i < 8 && chain[n - 1] >= 0; ++i) {
		bool big = i == sig_at;
		int made = chain_step(&c, big, big || i == sig_at + 1);
		/* (no room for the signature here: a platform first, it further on) */
		if (made < 0 && big && sig_at < 4) { ++sig_at; --i; continue; }
		if (made < 0) break;
		link(chain[n - 1], made);
		chain[n++] = made;
	}
	chain_shortcut(chain, n);
	spurs(2 + size, 1, 3);
	stubs(slabs ? 2 : 3 + 2 * size);
}

/* One big field (ragged, or around a crater) with comb boardwalks; Mr.
 * Weather's is one plain slab, as its comp is, reached by conveyor belts
 * alone (its art has no boardwalks). The field is the layer's signature, in
 * its box a panel or two larger. */
static void field(int biome, int size, int sig) {
	int w = 9 + size + rng_range(0, 2), h = 9 + size + rng_range(0, 2);
	if (sig) {
		sig_box(sig, biome, size, &w, &h);
		w += rng_range(0, 1); h += rng_range(0, 1);
	}
	int fx = WIN_C - w / 2, fy = WIN_C - h / 2;
	bool slab = biome == BIOME_WEATHER_COMP;
	if (!sig || sig_carve(sig, biome, fx, fy, w, h) < 0) {
		carve_shape(biome == BIOME_CENTRAL ? SHAPE_CRATER : slab ? SHAPE_RECT : SHAPE_RAGGED, fx, fy, w, h);
		add_room(fx, fy, w, h, ROOM_FIELD);
	}
	/* boardwalks two cells off some sides, their teeth pointing out */
	int first = rng_range(0, 3), sides = slab ? 0 : 2 + rng_range(0, 1);
	for (int k = 0; k < sides; ++k) {
		int d = (first + k) % 4;   /* the side's outward direction */
		int along = (d == DIR_E || d == DIR_W) ? DIR_S : DIR_E;
		int len = (along == DIR_E ? w : h) + 2;
		int sx = d == DIR_E ? fx + w + 2 : d == DIR_W ? fx - 3 : fx - 2;
		int sy = d == DIR_S ? fy + h + 2 : d == DIR_N ? fy - 3 : fy - 2;
		if (!bridge_line(sx - dir_dx[along], sy - dir_dy[along], along, len)) continue;
		int out = (along + 1) % 4 == d ? 1 : -1;
		teeth(sx - dir_dx[along], sy - dir_dy[along], along, len, out);
		/* rungs back to the field, met square on: where one meets the
		 * ragged edge beside a bite or on a bump, the panels beside it
		 * filled (a rung into a tab was a mouth two panels wide on one side,
		 * the floor going on past it, where the arrow's way held walked
		 * MegaMan over the lane) */
		for (int r = 0; r < 2; ++r) {
			int t = rng_range(2, len - 3), back = (d + 2) % 4, s = 1;
			int rx = sx + dir_dx[along] * t, ry = sy + dir_dy[along] * t;
			for (; s <= 3 && !floor_at(rx + dir_dx[back] * s, ry + dir_dy[back] * s); ++s) put(rx + dir_dx[back] * s, ry + dir_dy[back] * s);
			for (int e = -1; e <= 1 && s <= 3; e += 2) put(rx + dir_dx[back] * s + dir_dx[along] * e, ry + dir_dy[back] * s + dir_dy[along] * e);
		}
	}
	spurs(5 + size, 2, 4);
	stubs(2);
}

/* The ladder's signature: a field across every plank at the line of rungs
 * the way crosses by, t from `line` - 3 to + 3 along the planks and o from
 * o0 to o1 across them (their teeth's lines), the planks running into its
 * two sides as Green Area 2's reach its fields; the teeth beside its corners
 * cleared (one there is a join no map draws). */
static void ladder_heart(int sig, int biome, int along, int across, int o0, int o1, int line) {
	int t0 = line - 3, t1 = line + 3;
	#define GX(t, o) (WIN_C + dir_dx[along] * (t) + dir_dx[across] * (o))
	#define GY(t, o) (WIN_C + dir_dy[along] * (t) + dir_dy[across] * (o))
	int xa = GX(t0, o0), ya = GY(t0, o0), xb = GX(t1, o1), yb = GY(t1, o1);
	if (!win_in(xa, ya) || !win_in(xb, yb) || !win_in(GX(t0, o1), GY(t0, o1)) || !win_in(GX(t1, o0), GY(t1, o0))) return;
	for (int e = 0; e < 2; ++e)
		for (int t = t0 - 1; t <= t1 + 1; t += t1 - t0 + 2) carve_void(GX(t, e ? o1 : o0), GY(t, e ? o1 : o0), 1, 1);
	#undef GX
	#undef GY
	sig_carve(sig, biome, xa < xb ? xa : xb, ya < yb ? ya : yb, abs(xb - xa) + 1, abs(yb - ya) + 1);
}

/* Parallel planks joined by rungs, grass blocks at their ends, and the
 * signature across their middle. */
static void ladder(int biome, int size, int sig) {
	bool along_x = rng_range(0, 1);
	int along = along_x ? DIR_E : DIR_S, across = along_x ? DIR_S : DIR_E;
	int rails = 4 + (size > 0), len = 14 + 2 * size + rng_range(0, 3);
	int start[5], base = -(rails - 1) * 3 / 2;
	for (int r = 0; r < rails; ++r) {
		start[r] = -len / 2 + rng_range(-2, 2);
		int o = base + 3 * r;
		int x = WIN_C + dir_dx[along] * (start[r] - 1) + dir_dx[across] * o;
		int y = WIN_C + dir_dy[along] * (start[r] - 1) + dir_dy[across] * o;
		bridge_line(x, y, along, len);
		/* the outer planks' teeth */
		if (r == 0) teeth(x, y, along, len, (along + 1) % 4 == across ? -1 : 1);
		if (r == rails - 1) teeth(x, y, along, len, (along + 1) % 4 == across ? 1 : -1);
	}
	/* rungs where neighbouring planks overlap: one line of them straight
	 * across every plank near the ladder's middle, where the window is
	 * widest and every plank reaches (the ladder's golden path: the way
	 * crosses it in one leg, where rungs only at random had turned it at
	 * every plank, a lane to line up on a one-panel walkway each, some
	 * twenty calls of stops for a playtester, sessions 60 and 61), then one
	 * or two more a pair at random, to wander by */
	int lo_all = start[0], hi_all = start[0];
	for (int r = 1; r < rails; ++r) {
		if (start[r] > lo_all) lo_all = start[r];
		if (start[r] < hi_all) hi_all = start[r];
	}
	hi_all += len - 1;
	int line = rng_range(-2, 2);
	if (line < lo_all + 1) line = lo_all + 1;
	if (line > hi_all - 2) line = hi_all - 2;
	for (int r = 0; r + 1 < rails; ++r)
		for (int k = 0; k < 2 + rng_range(0, 1); ++k) {
			int lo = start[r] > start[r + 1] ? start[r] : start[r + 1];
			int t = k == 0 ? line : lo + rng_range(1, len - 3);
			int o = base + 3 * r;
			for (int s = 1; s <= 2; ++s)
				put(WIN_C + dir_dx[along] * t + dir_dx[across] * (o + s), WIN_C + dir_dy[along] * t + dir_dy[across] * (o + s));
		}
	if (sig) ladder_heart(sig, biome, along, across, base - 1, base + 3 * (rails - 1) + 1, line);
	/* grass blocks off the first plank's start and the last plank's end */
	for (int e = 0; e < 2; ++e) {
		int r = e ? rails - 1 : 0, o = base + 3 * r;
		int t = e ? start[r] + len + 3 : start[r] - 4;
		int w = rng_range(4, 6), h = rng_range(4, 5);
		int cx = WIN_C + dir_dx[along] * t + dir_dx[across] * o, cy = WIN_C + dir_dy[along] * t + dir_dy[across] * o;
		int b = platform(cx, cy, along_x ? w : h, along_x ? h : w, SHAPE_RECT, ROOM_PLATFORM);
		(void)b;
	}
	spurs(4 + size, 2, 3);
	stubs(2);
}

/* A centre (the signature, where the layer has one) with mirrored spokes to
 * pods, each pair of spokes its own length, so the X is drawn long one way
 * or the other; now and then three of them, the one left out across the
 * arrival's and the exit's axis. */
static void hub(int biome, int size, int sig) {
	int c = size > 1 ? 7 : 5, pod = 5, arm[2] = { rng_range(3, 4), rng_range(3, 4) };
	if (sig) {
		int ch;
		sig_box(sig, biome, size, &c, &ch);
		if (c > 11) c = 11;
	}
	/* (the pods' outer corners in the window, across the screen: their
	 * middles ten panels out at most) */
	for (int k = 0; k < 2; ++k)
		if (c / 2 + arm[k] + pod / 2 + 1 > 10) arm[k] = 10 - c / 2 - pod / 2 - 1;
	int centre = heart(sig, biome, WIN_C, WIN_C, c, c, SHAPE_OCTAGON);
	if (centre < 0) return;
	int axis = rng_range(0, 1), skip = rng_range(0, 2) ? -1 : (axis + 1 + 2 * rng_range(0, 1)) % 4, pods[4];
	for (int d = 0; d < 4; ++d) {
		int off = c / 2 + arm[d & 1] + pod / 2 + 1;
		pods[d] = d == skip ? -1 : platform(WIN_C + dir_dx[d] * off, WIN_C + dir_dy[d] * off, pod, pod, SHAPE_OCTAGON, ROOM_PLATFORM);
		if (pods[d] >= 0) link(centre, pods[d]);
	}
	/* a ring between the pods, whole or in two opposite quarters */
	bool whole = rng_range(0, 1);
	for (int d = 0; d < 4; ++d)
		if ((whole || d % 2 == 0) && pods[d] >= 0 && pods[(d + 1) % 4] >= 0) link(pods[d], pods[(d + 1) % 4]);
	/* mirrored pads further out on two opposite spokes: arrival and exit */
	for (int d = axis; d < 4; d += 2) {
		int ex, ey;
		if (pods[d] >= 0 && room_edge(pods[d], d, &ex, &ey)) pad_spur(ex, ey, d, 2 + (size > 0));
	}
	stubs(size);
}

/* Plateaus scattered over the window, bridges crossing between them, the
 * signature among them about the window's middle, where the others keep
 * their distance from it as from each other. */
static void web(int biome, int size, int sig) {
	int want = 6 + 2 * size, made[16], n = 0;
	/* (its box seven a side at most: a bigger one pushed the plateaus out to
	 * the window's edges, past what the game's tile map holds) */
	if (sig && (made[n] = sig_fit(sig, biome, WIN_C + rng_range(-2, 2), WIN_C + rng_range(-2, 2), size, 7)) >= 0) ++n;
	for (int tries = 0; tries < 400 && n < want; ++tries) {
		int x, y;
		random_cell(&x, &y);
		int w = rng_range(3, 5), h = rng_range(3, 5);
		/* (nine panels across the screen from the middle at most, and
		 * twenty up or down it: plateaus on the window's edges all round,
		 * their spurs past them, were more than the game's tile map holds) */
		if (abs(x - y) > 9 || abs(x + y - 2 * WIN_C) > 20 || !box_free(x - w / 2, y - h / 2, w, h, 4)) continue;   /* room for long bridges between */
		int r = platform(x, y, w, h, route_shape(biome), ROOM_PLATFORM);
		if (r >= 0) made[n++] = r;
	}
	/* a spanning tree by distance, then a couple of crossings (never a
	 * second bridge between two the tree joins: side by side, the way across
	 * took the narrow one as often as the one widened) */
	bool in[16] = { true };
	uint16_t joined[16] = { 0 };
	for (int k = 1; k < n; ++k) {
		int ba = -1, bb = -1, bd = 1 << 30;
		for (int a = 0; a < n; ++a)
			for (int b = 0; b < n && in[a]; ++b) {
				if (in[b]) continue;
				const Room *ra = &layer.rooms[made[a]], *rb = &layer.rooms[made[b]];
				int d = abs(ra->ax - rb->ax) + abs(ra->ay - rb->ay);
				if (d < bd) { bd = d; ba = a; bb = b; }
			}
		if (bb < 0) break;
		in[bb] = true;
		link(made[ba], made[bb]);
		joined[ba] |= (uint16_t)(1u << bb);
		joined[bb] |= (uint16_t)(1u << ba);
	}
	for (int k = 0; k < 2 && n > 3; ++k) {
		int a = rng_range(0, n - 1), b = rng_range(0, n - 1);
		if (a != b && !(joined[a] >> b & 1)) link(made[a], made[b]);
	}
	spurs(1 + size, 2, 3);
	stubs(5 + 2 * size);
}

/* Plus-shaped platforms on a lattice around a big block (the signature,
 * where the layer has one, its neighbours pushed out a panel round it and
 * six from the next ones). The window is narrow across the screen, so the
 * lattice runs along its diagonal: 5-wide pluses 7 apart fit where
 * |i - j| <= 1 (and |i + j| <= 3). */
static void crosses(int biome, int size, int sig) {
	enum { STEP = 7, N = 7, M = N / 2 };
	int node[N][N], s = 7, e = 0;
	for (int j = 0; j < N; ++j)
		for (int i = 0; i < N; ++i) node[j][i] = -1;
	if (sig) {
		int sh;
		sig_box(sig, biome, size, &s, &sh);
		/* (nine at most: the lattice pushed out further left a guardian's
		 * arena no room) */
		if (s > 9) s = 9;
		e = s > 7;
	}
	#define AT(k) (WIN_C + ((k) - M) * STEP + ((k) == M + 1 ? e : (k) == M - 1 ? -e : 0))
	/* grow a tree over the lattice from its middle */
	int want = 6 + size, have = 0, qi[N * N], qj[N * N], nq = 0;
	node[M][M] = heart(sig, biome, WIN_C, WIN_C, s, s, SHAPE_RECT);
	if (node[M][M] < 0) return;
	qi[nq] = M; qj[nq++] = M; ++have;
	for (int tries = 0; tries < 200 && have < want; ++tries) {
		int k = rng_range(0, nq - 1), d = rng_range(0, 3);
		int i = qi[k] + dir_dx[d], j = qj[k] + dir_dy[d];
		if (i < 0 || j < 0 || i >= N || j >= N || node[j][i] >= 0) continue;
		int r = platform(AT(i), AT(j), 5, 5, SHAPE_PLUS, ROOM_PLATFORM);
		if (r < 0) continue;
		node[j][i] = r;
		link(node[qj[k]][qi[k]], r);
		qi[nq] = i; qj[nq++] = j; ++have;
	}
	#undef AT
	/* a loop or two between lattice neighbours */
	for (int k = 0; k < 4; ++k) {
		int i = rng_range(0, N - 2), j = rng_range(0, N - 1);
		bool across = rng_range(0, 1);
		int a = across ? node[i][j] : node[j][i], b = across ? node[i + 1][j] : node[j][i + 1];
		if (a >= 0 && b >= 0 && rng_range(0, 1)) link(a, b);
	}
	spurs(2, 2, 3);
	stubs(3);
}

enum { COMB_LANES = 5, COMB_GAP = 3 };

/* A comb's lanes (t along them from the walkway at `root`, o across from
 * `base`), as long as the window lets them run (|t - o| <= WIN_U,
 * |t + o| <= WIN_V): long and short in turn, the short ones ending in pads
 * and the long ones now and then, never the middle one. */
static void comb_lanes(int base, int root, int size, int len[COMB_LANES], bool pad[COMB_LANES]) {
	for (int r = 0; r < COMB_LANES; ++r) {
		int o = base + COMB_GAP * r, room_to = (o + WIN_U < WIN_V - o ? o + WIN_U : WIN_V - o) - root;
		bool lng = !(r & 1);
		len[r] = lng ? 9 + rng_range(0, 2) + size : 3 + rng_range(0, 1) + size;
		if (len[r] > room_to - 1) len[r] = room_to - 1;
		pad[r] = r != COMB_LANES / 2 && ((r & 1) || rng_range(0, 1));
	}
}

/* After Central Area 2: 1-wide catwalks side by side, long and short in
 * turn, hung off a wide walkway. The short ones end in pads, a rung joins
 * two long ones into a loop and the middle one leads on to a plaza (the
 * signature, in a box of seven); the arrival waits behind the walkway's
 * end. */
static void comb(int biome, int size, int sig) {
	enum { LANES = COMB_LANES, GAP = COMB_GAP, C = LANES / 2 };
	bool along_x = rng_range(0, 1);
	int along = along_x ? DIR_E : DIR_S, across = along_x ? DIR_S : DIR_E;
	int base = -(LANES - 1) * GAP / 2, root = -4, len[LANES];
	bool pad[LANES];
#define AT_X(t, o) (WIN_C + dir_dx[along] * (t) + dir_dx[across] * (o))
#define AT_Y(t, o) (WIN_C + dir_dy[along] * (t) + dir_dy[across] * (o))
	comb_lanes(base, root, size, len, pad);
	/* the middle one leads on to a plaza, which the window must hold */
	int pw = sig ? 7 : 5, pl = sig ? 7 : 4 + (size > 0);
	if (root + len[C] > WIN_U - pw / 2 - pl - 1) len[C] = WIN_U - pw / 2 - pl - 1 - root;
	/* a rung joins it to an outer one at the shorter one's end: the one
	 * loop, the short lane between kept clear of it with its pad */
	int side = rng_range(0, 1) ? LANES - 1 : 0, mid = (side + C) / 2;
	int t_rung = root + (len[side] < len[C] ? len[side] : len[C]);
	if (root + len[mid] + (pad[mid] ? 5 : 1) >= t_rung) {
		len[mid] = t_rung - root - 6;
		if (len[mid] < 2) { len[mid] = t_rung - root - 2; pad[mid] = false; }
	}
	/* the walkway: two panels deep, one past the outer lanes */
	int span = (LANES - 1) * GAP + 3;
	platform(AT_X(root - 1 + 1, 0), AT_Y(root - 1 + 1, 0), along_x ? 2 : span, along_x ? span : 2, SHAPE_RECT, ROOM_PLATFORM);
	for (int r = 0; r < LANES; ++r) {
		int o = base + GAP * r;
		bridge_line(AT_X(root, o), AT_Y(root, o), along, len[r]);
	}
	int o0 = side ? base + GAP * C : base;
	bridge_line(AT_X(t_rung, o0), AT_Y(t_rung, o0), across, 2 * GAP - 1);
	int end = root + len[C];
	if (heart(sig, biome, AT_X(end + 2 + pl / 2, 0), AT_Y(end + 2 + pl / 2, 0), along_x ? pl : pw, along_x ? pw : pl, route_shape(biome)) >= 0)
		put(AT_X(end + 1, 0), AT_Y(end + 1, 0));
	/* the arrival behind the walkway's first end, two panels of catwalk
	 * between, the walkway's back left clear */
	if (platform(AT_X(root - 5, base + 1), AT_Y(root - 5, base + 1), 4, 4, route_shape(biome), ROOM_PLATFORM) >= 0)
		bridge_line(AT_X(root - 1, base + 1), AT_Y(root - 1, base + 1), (along + 2) % 4, 2);
	for (int r = 0; r < LANES; ++r)
		if (pad[r]) pad_spur(AT_X(root + len[r], base + GAP * r), AT_Y(root + len[r], base + GAP * r), along, 1);
#undef AT_X
#undef AT_Y
	spurs(1 + size, 1, 2);
}

/* The trail swelling into its signature: the box from (x, y), where the last
 * leg (`last`: x, y, w, h) ends in its corner, in the window and clear of
 * all but that leg and a panel round it; its side in *s. */
static bool trail_heart(int sig, int biome, int size, int x, int y, const int last[4], int *s) {
	int sh;
	sig_box(sig, biome, size, s, &sh);
	if (!win_in(x, y) || !win_in(x + *s - 1, y) || !win_in(x, y + *s - 1) || !win_in(x + *s - 1, y + *s - 1)) return false;
	for (int j = y - 1; j <= y + *s; ++j)
		for (int i = x - 1; i <= x + *s; ++i)
			if (floor_at(i, j) && !(i >= last[0] - 1 && i <= last[0] + last[2] && j >= last[1] - 1 && j <= last[1] + last[3])) return false;
	return sig_carve(sig, biome, x, y, *s, *s) >= 0;
}

/* A trail's leg from (x, y), along x or y: two or three panels wide, steered
 * back towards the middle (a leg along x carries the path right across the
 * screen, one along y left: or the window's edge cut the legs to stubs),
 * short of the window's edge, a bulge a panel wider on its outer side now
 * and then; its box into leg[4] (x, y, w, h), its length (2 or less: none
 * laid). */
static int trail_leg(int x, int y, bool along_x, int leg[4]) {
	int drift = (x - y) * (along_x ? 1 : -1), len = drift > 3 ? rng_range(4, 5) : drift < -3 ? rng_range(7, 8) : rng_range(5, 8);
	int wide = rng_range(2, 3);
	int bw = along_x ? len : wide, bh = along_x ? wide : len;
	while (len > 2 && !(win_in(x, y) && win_in(x + bw - 1, y) && win_in(x, y + bh - 1) && win_in(x + bw - 1, y + bh - 1))) {
		--len;
		bw = along_x ? len : wide; bh = along_x ? wide : len;
	}
	if (len <= 2) return len;
	for (int j = y; j < y + bh; ++j)
		for (int i = x; i < x + bw; ++i) put(i, j);
	if (len >= 5 && rng_range(0, 2) == 0) {
		int t0 = rng_range(1, len - 3);
		for (int t = t0; t < t0 + 2; ++t) {
			int bx = along_x ? x + t : x - 1, by = along_x ? y - 1 : y + t;
			if (win_in(bx, by)) put(bx, by);
		}
	}
	leg[0] = x; leg[1] = y; leg[2] = bw; leg[3] = bh;
	return len;
}

/* After Central Area 1: a path two or three panels wide winding down the
 * window in legs along x and y by turns (the screen's two diagonals), a
 * bulge a panel wider here and there, and pads hung off its sides on short
 * catwalks. Two or three legs in, it swells into the signature, entered at
 * one corner and left from the one across. */
static void trail(int biome, int size, int sig) {
	int x = WIN_C - 10 + rng_range(-1, 1), y = WIN_C - 10 + rng_range(-1, 1);
	int sig_leg = sig ? 2 + rng_range(0, 1) : -1, last[4] = { x, y, 0, 0 }, s;
	bool along_x = rng_range(0, 1);
	for (int l = 0; l < 7 + size; ++l) {
		if (l == sig_leg && l + 3 < 7 + size && trail_heart(sig, biome, size, x, y, last, &s)) {
			/* (on from the corner across, along the other way) */
			if (along_x) { x += s - 2; y += s - 3; } else { y += s - 2; x += s - 3; }
			along_x = !along_x;
			continue;
		}
		if (l == sig_leg) ++sig_leg;
		int len = trail_leg(x, y, along_x, last);
		if (len <= 2) break;
		/* (a room of every second leg: the legs overlap at their turns) */
		if (!(l & 1)) add_room(last[0], last[1], last[2], last[3], ROOM_LEG);
		/* the next leg turns at this one's end, overlapping its corner */
		if (along_x) x += len - 2; else y += len - 2;
		along_x = !along_x;
	}
	spurs(3 + size, 1, 2);
	stubs(2);
}

/* A maze of catwalks over a lattice of nodes `step` apart, n a side, from
 * (ox, oy); `comp` the comps' whose every layer is one. */
enum { NMAX = 8 };
typedef struct { int step, n, ox, oy; bool comp; } Maze;

#define NODE_X(m, i) ((m)->ox + (m)->step * (i))
#define NODE_Y(m, j) ((m)->oy + (m)->step * (j))

/* A depth-first maze from its middle (else its first node open), a comp's
 * corridors running straight on where they can; `seen` holds the nodes left
 * out (off the window, or round the signature). */
static void maze_grow(const Maze *m, bool seen[NMAX][NMAX]) {
	int si[NMAX * NMAX], sj[NMAX * NMAX], sd[NMAX * NMAX], sp = 0, i0 = m->n / 2, j0 = m->n / 2;
	for (int k = 0; seen[j0][i0] && k < m->n * m->n; ++k) { i0 = k % m->n; j0 = k / m->n; }
	if (seen[j0][i0]) return;
	si[sp] = i0; sj[sp] = j0; sd[sp++] = -1;
	seen[j0][i0] = true;
	put(NODE_X(m, i0), NODE_Y(m, j0));
	while (sp) {
		int i = si[sp - 1], j = sj[sp - 1], last = sd[sp - 1], opts[4], no = 0;
		bool on = false;
		for (int d = 0; d < 4; ++d) {
			int ni = i + dir_dx[d], nj = j + dir_dy[d];
			if (ni >= 0 && nj >= 0 && ni < m->n && nj < m->n && !seen[nj][ni]) { opts[no++] = d; on |= d == last; }
		}
		if (!no) { --sp; continue; }
		int d = m->comp && on && rng_range(0, 99) < 70 ? last : opts[rng_range(0, no - 1)];
		int ni = i + dir_dx[d], nj = j + dir_dy[d];
		seen[nj][ni] = true;
		for (int c = 1; c < m->step; ++c) put(NODE_X(m, i) + c * dir_dx[d], NODE_Y(m, j) + c * dir_dy[d]);
		put(NODE_X(m, ni), NODE_Y(m, nj));
		si[sp] = ni; sj[sp] = nj; sd[sp++] = d;
	}
}

/* The signature's box in the maze's middle, or a node up or down the window
 * from it (a court the catwalks run round: seven a side at most, so they
 * keep their maze, and five in a comp guardian's smaller one, in its
 * middle), and the nodes within a panel of it left out of the maze, so no
 * catwalk runs along its sides; false where it has no room. */
static bool maze_hole(const Maze *m, int sig, int biome, int size, bool seen[NMAX][NMAX], int box[3]) {
	int w, h, most = m->n >= 6 ? 7 : 5, k = m->n >= 6 ? rng_range(-1, 1) : 0;
	sig_box(sig, biome, size, &w, &h);
	int s = w > most ? most : w, x = WIN_C + k * m->step - s / 2, y = WIN_C + k * m->step - s / 2;
	if (!win_in(x, y) || !win_in(x + s - 1, y) || !win_in(x, y + s - 1) || !win_in(x + s - 1, y + s - 1)) return false;
	for (int j = 0; j < m->n; ++j)
		for (int i = 0; i < m->n; ++i)
			if (NODE_X(m, i) >= x - 1 && NODE_X(m, i) <= x + s && NODE_Y(m, j) >= y - 1 && NODE_Y(m, j) <= y + s) seen[j][i] = true;
	box[0] = x; box[1] = y; box[2] = s;
	return true;
}

/* The signature carved in its hole, and on each side a catwalk straight in
 * from the node nearest the side's middle, never at a corner: the maze's
 * crossroads. */
static void maze_heart(const Maze *m, int sig, int biome, const int box[3]) {
	int x = box[0], y = box[1], s = box[2];
	sig_carve(sig, biome, x, y, s, s);
	for (int d = 0; d < 4; ++d) {
		int best = -1, bi = 0, bj = 0, mid = d & 1 ? x + s / 2 : y + s / 2;
		for (int j = 0; j < m->n; ++j)
			for (int i = 0; i < m->n; ++i) {
				int nx = NODE_X(m, i), ny = NODE_Y(m, j), along = d & 1 ? nx : ny;
				/* (beside the side d faces: the node's line meets it inside its corners) */
				int gap = d == DIR_E ? x - nx : d == DIR_W ? nx - (x + s - 1) : d == DIR_S ? y - ny : ny - (y + s - 1);
				bool inside = d & 1 ? nx > x && nx < x + s - 1 : ny > y && ny < y + s - 1;
				if (!floor_at(nx, ny) || !inside || gap < 2 || gap > m->step + 1) continue;
				int score = 100 - abs(along - mid);
				if (score > best) { best = score; bi = nx; bj = ny; }
			}
		if (best < 0) continue;
		for (int k = 1; !floor_at(bi + dir_dx[d] * k, bj + dir_dy[d] * k) && k <= m->step + 1; ++k) put(bi + dir_dx[d] * k, bj + dir_dy[d] * k);
	}
}

/* Loops: walls knocked through (a comp's more, shortcuts that cut its way
 * down). */
static void maze_loops(const Maze *m, int size) {
	for (int k = 0; k < (m->comp ? 8 + 2 * size : 3 + size); ++k) {
		int i = rng_range(0, m->n - 2), j = rng_range(0, m->n - 1);
		if (floor_at(NODE_X(m, i), NODE_Y(m, j)) && floor_at(NODE_X(m, i + 1), NODE_Y(m, j)))
			for (int c = 1; c < m->step; ++c) put(NODE_X(m, i) + c, NODE_Y(m, j));
	}
}

/* A maze of 1-wide catwalks, some dead ends cut back, plazas at the ends,
 * the signature a crossroads in its middle. */
static void catwalks(int biome, int size, int sig) {
	/* (the two comps whose every layer is a maze wind less: theirs turned
	 * twice as often as the other areas' ways, 16 legs from the arrival to
	 * the exit against their 8, and a playtester spent 110 of 249 moves
	 * walking and reading the map there. The Undernet's and CopyBot's share
	 * their layers with other layouts, and their tiles draw it best as it
	 * was) */
	bool comp = biome == BIOME_AQUARIUM_COMP || biome == BIOME_JUDGE_COMP;
	/* lattice nodes per side, two cells apart, a comp's three: two panels
	 * between its corridors (one apart, 44% of the Aquarium's and the
	 * Judge Tree's floor faced floor across one empty panel, against 1 to
	 * 9% elsewhere, and on the Judge Tree's brick walkways a playtester
	 * read the maze as terraces he could not step down to); a comp
	 * guardian's layer a smaller maze, which leaves its arena room (at
	 * eight, eleven Aquarium guardians' layers in twelve found none and
	 * fell back to a plain route) */
	Maze m = { comp ? 3 : 2, comp ? (layer.boss_layer ? 5 : 6) : NMAX, 0, 0, comp };
	m.ox = m.oy = WIN_C - m.step * (m.n - 1) / 2;
	static bool seen[NMAX][NMAX];
	for (int j = 0; j < m.n; ++j)
		for (int i = 0; i < m.n; ++i) seen[j][i] = !win_in(NODE_X(&m, i), NODE_Y(&m, j));
	int box[3] = { 0, 0, 0 };
	bool hole = sig && maze_hole(&m, sig, biome, size, seen, box);
	maze_grow(&m, seen);
	maze_loops(&m, size);
	if (hole) maze_heart(&m, sig, biome, box);
	/* plazas beyond both ends of the maze, up and down the window (the
	 * Aquarium's are its glass pads' size: its water never widens) */
	for (int e = 0; e < 2; ++e) {
		int half = m.step * (m.n - 1) / 2 + 3, off = e ? half : -half;
		if (biome == BIOME_AQUARIUM_COMP) platform(WIN_C + off, WIN_C + off, 4, 4, SHAPE_RECT, ROOM_PLATFORM);
		else platform(WIN_C + off, WIN_C + off, 5, 4 + rng_range(0, 1), route_shape(biome), ROOM_PLATFORM);
	}
	spurs(4 + size, 1, 2);
}

#undef NODE_X
#undef NODE_Y

void layout_build(int layout, int biome, int size, int sig) {
	layer.sig = sig;
	layer.sig_room = -1;
	switch (layout) {
	case LAYOUT_FIELD: field(biome, size, sig); break;
	case LAYOUT_LADDER: ladder(biome, size, sig); break;
	case LAYOUT_HUB: hub(biome, size, sig); break;
	case LAYOUT_SLABS: route(biome, size, true, sig); break;
	case LAYOUT_WEB: web(biome, size, sig); break;
	case LAYOUT_CROSSES: crosses(biome, size, sig); break;
	case LAYOUT_CATWALKS: catwalks(biome, size, sig); break;
	case LAYOUT_COMB: comb(biome, size, sig); break;
	case LAYOUT_TRAIL: trail(biome, size, sig); break;
	default: route(biome, size, false, sig); break;
	}
}
