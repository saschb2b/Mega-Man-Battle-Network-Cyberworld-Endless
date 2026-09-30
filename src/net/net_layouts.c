/* Layouts after BN6's net areas (docs/LEVEL_DESIGN.md). Each builds from
 * the pieces in net_shapes.c; the generator then joins anything left
 * apart, picks where MegaMan arrives and checks the size. */
#include "net_layouts.h"

#include <stdlib.h>

#include "game.h"
#include "net_shapes.h"
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

/* Platforms in a chain winding up the window, then pads and stubs. */
static void route(int biome, int size, bool slabs) {
	int x = WIN_C - 10 + rng_range(-2, 2), y = WIN_C - 10 + rng_range(-2, 2);
	int prev = platform(x, y, 3, 3, SHAPE_RECT, ROOM_PAD);
	for (int i = 0; i < 8 && prev >= 0; ++i) {
		int made = -1;
		for (int tries = 0; tries < 24 && made < 0; ++tries) {
			int step = slabs ? rng_range(10, 12) : rng_range(6, 9);
			int sx = rng_range(0, step), nx = x + sx, ny = y + step - sx;
			if (abs(nx - ny) > WIN_U - 3) continue;
			int w, h, shape;
			if (slabs) { w = 7 + 2 * rng_range(0, size > 0); h = 7 + 2 * rng_range(0, 1); shape = SHAPE_HOLED; }
			else if (biome == BIOME_ROBOT_COMP) {
				/* Robot Control's long white slabs */
				int along = rng_range(6, 8 + size), across = rng_range(2, 3);
				bool wide = rng_range(0, 1);
				w = wide ? along : across; h = wide ? across : along; shape = SHAPE_RECT;
			}
			else { w = rng_range(3, 5 + size); h = rng_range(3, 5 + size); shape = route_shape(biome); }
			made = platform(nx, ny, w, h, shape, ROOM_PLATFORM);
			if (made >= 0) { x = nx; y = ny; }
		}
		if (made < 0) break;
		link(prev, made);
		prev = made;
	}
	/* one shortcut between platforms two apart, when they are close */
	for (int a = 1; a + 2 < layer.nrooms; ++a) {
		const Room *ra = &layer.rooms[a], *rb = &layer.rooms[a + 2];
		if (abs(ra->ax - rb->ax) + abs(ra->ay - rb->ay) < 12 && rng_range(0, 2) == 0) { link(a, a + 2); break; }
	}
	spurs(2 + size, 1, 3);
	stubs(slabs ? 2 : 3 + 2 * size);
}

/* One big field (ragged, or around a crater) with comb boardwalks; Mr.
 * Weather's is one plain slab, as its comp is, reached by conveyor belts
 * alone (its art has no boardwalks). */
static void field(int biome, int size) {
	int w = 9 + size + rng_range(0, 2), h = 9 + size + rng_range(0, 2);
	int fx = WIN_C - w / 2, fy = WIN_C - h / 2;
	bool slab = biome == BIOME_WEATHER_COMP;
	int shape = biome == BIOME_CENTRAL ? SHAPE_CRATER : slab ? SHAPE_RECT : SHAPE_RAGGED;
	carve_shape(shape, fx, fy, w, h);
	add_room(fx, fy, w, h, ROOM_FIELD);
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
		/* rungs back to the field */
		for (int r = 0; r < 2; ++r) {
			int t = rng_range(2, len - 3);
			int rx = sx + dir_dx[along] * t, ry = sy + dir_dy[along] * t;
			int back = (d + 2) % 4;
			for (int s = 1; s <= 3 && !floor_at(rx + dir_dx[back] * s, ry + dir_dy[back] * s); ++s)
				put(rx + dir_dx[back] * s, ry + dir_dy[back] * s);
		}
	}
	spurs(5 + size, 2, 4);
	stubs(2);
}

/* Parallel planks joined by rungs, grass blocks at their ends. */
static void ladder(int biome, int size) {
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
	/* rungs where neighbouring planks overlap */
	for (int r = 0; r + 1 < rails; ++r)
		for (int k = 0; k < 2 + rng_range(0, 1); ++k) {
			int lo = start[r] > start[r + 1] ? start[r] : start[r + 1];
			int t = lo + rng_range(1, len - 3);
			int o = base + 3 * r;
			for (int s = 1; s <= 2; ++s)
				put(WIN_C + dir_dx[along] * t + dir_dx[across] * (o + s), WIN_C + dir_dy[along] * t + dir_dy[across] * (o + s));
		}
	/* grass blocks off the first plank's start and the last plank's end */
	for (int e = 0; e < 2; ++e) {
		int r = e ? rails - 1 : 0, o = base + 3 * r;
		int t = e ? start[r] + len + 3 : start[r] - 4;
		int w = rng_range(4, 6), h = rng_range(4, 5);
		int cx = WIN_C + dir_dx[along] * t + dir_dx[across] * o, cy = WIN_C + dir_dy[along] * t + dir_dy[across] * o;
		int b = platform(cx, cy, along_x ? w : h, along_x ? h : w, SHAPE_RECT, ROOM_PLATFORM);
		(void)b;
	}
	(void)biome;
	spurs(4 + size, 2, 3);
	stubs(2);
}

/* A centre platform with four mirrored spokes to pods. */
static void hub(int biome, int size) {
	int c = size > 1 ? 7 : 5, arm = rng_range(3, 4), pod = 5;
	int centre = platform(WIN_C, WIN_C, c, c, SHAPE_OCTAGON, ROOM_PLATFORM);
	if (centre < 0) return;
	int pods[4];
	for (int d = 0; d < 4; ++d) {
		int off = c / 2 + arm + pod / 2 + 1;
		pods[d] = platform(WIN_C + dir_dx[d] * off, WIN_C + dir_dy[d] * off, pod, pod,
			pod > 3 ? SHAPE_OCTAGON : SHAPE_RECT, pod > 3 ? ROOM_PLATFORM : ROOM_PAD);
		if (pods[d] >= 0) link(centre, pods[d]);
	}
	/* a ring between the pods, whole or in two opposite quarters */
	bool whole = rng_range(0, 1);
	for (int d = 0; d < 4; ++d)
		if ((whole || d % 2 == 0) && pods[d] >= 0 && pods[(d + 1) % 4] >= 0) link(pods[d], pods[(d + 1) % 4]);
	/* mirrored pads further out on two opposite spokes: arrival and exit */
	int axis = rng_range(0, 1);
	for (int d = axis; d < 4; d += 2) {
		int ex, ey;
		if (pods[d] >= 0 && room_edge(pods[d], d, &ex, &ey)) pad_spur(ex, ey, d, 2 + (size > 0));
	}
	(void)biome;
	stubs(size);
}

/* Plateaus scattered over the window, bridges crossing between them. */
static void web(int biome, int size) {
	int want = 6 + 2 * size, made[16], n = 0;
	for (int tries = 0; tries < 400 && n < want; ++tries) {
		int x, y;
		random_cell(&x, &y);
		int w = rng_range(3, 5), h = rng_range(3, 5);
		if (!box_free(x - w / 2, y - h / 2, w, h, 4)) continue;   /* room for long bridges between */
		int r = platform(x, y, w, h, route_shape(biome), ROOM_PLATFORM);
		if (r >= 0) made[n++] = r;
	}
	/* a spanning tree by distance, then a couple of crossings */
	bool in[16] = { true };
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
	}
	for (int k = 0; k < 2 && n > 3; ++k) {
		int a = rng_range(0, n - 1), b = rng_range(0, n - 1);
		if (a != b) link(made[a], made[b]);
	}
	spurs(1 + size, 2, 3);
	stubs(5 + 2 * size);
}

/* Plus-shaped platforms on a lattice around a big block. The window is
 * narrow across the screen, so the lattice runs along its diagonal: 5-wide
 * pluses 7 apart fit where |i - j| <= 1 (and |i + j| <= 3). */
static void crosses(int biome, int size) {
	enum { STEP = 7, N = 7, M = N / 2 };
	int node[N][N];
	for (int j = 0; j < N; ++j)
		for (int i = 0; i < N; ++i) node[j][i] = -1;
	/* grow a tree over the lattice from its middle */
	int want = 6 + size, have = 0, qi[N * N], qj[N * N], nq = 0;
	node[M][M] = platform(WIN_C, WIN_C, 7, 7, SHAPE_RECT, ROOM_PLATFORM);
	if (node[M][M] < 0) return;
	qi[nq] = M; qj[nq++] = M; ++have;
	for (int tries = 0; tries < 200 && have < want; ++tries) {
		int k = rng_range(0, nq - 1), d = rng_range(0, 3);
		int i = qi[k] + dir_dx[d], j = qj[k] + dir_dy[d];
		if (i < 0 || j < 0 || i >= N || j >= N || node[j][i] >= 0) continue;
		int r = platform(WIN_C + (i - M) * STEP, WIN_C + (j - M) * STEP, 5, 5, SHAPE_PLUS, ROOM_PLATFORM);
		if (r < 0) continue;
		node[j][i] = r;
		link(node[qj[k]][qi[k]], r);
		qi[nq] = i; qj[nq++] = j; ++have;
	}
	/* a loop or two between lattice neighbours */
	for (int k = 0; k < 4; ++k) {
		int i = rng_range(0, N - 2), j = rng_range(0, N - 1);
		bool across = rng_range(0, 1);
		int a = across ? node[i][j] : node[j][i], b = across ? node[i + 1][j] : node[j][i + 1];
		if (a >= 0 && b >= 0 && rng_range(0, 1)) link(a, b);
	}
	(void)biome;
	spurs(2, 2, 3);
	stubs(3);
}

/* After Central Area 2: 1-wide catwalks side by side, long and short in
 * turn, hung off a wide walkway. The short ones end in pads, a rung joins
 * two long ones into a loop and the middle one leads on to a plaza; the
 * arrival waits behind the walkway's end. */
static void comb(int biome, int size) {
	enum { LANES = 5, GAP = 3, C = LANES / 2 };
	bool along_x = rng_range(0, 1);
	int along = along_x ? DIR_E : DIR_S, across = along_x ? DIR_S : DIR_E;
	int base = -(LANES - 1) * GAP / 2, root = -4, len[LANES];
	bool pad[LANES];
#define AT_X(t, o) (WIN_C + dir_dx[along] * (t) + dir_dx[across] * (o))
#define AT_Y(t, o) (WIN_C + dir_dy[along] * (t) + dir_dy[across] * (o))
	/* the lanes (t along them from the walkway, o across), as long as the
	 * window lets them run (|t - o| <= WIN_U, |t + o| <= WIN_V) */
	for (int r = 0; r < LANES; ++r) {
		int o = base + GAP * r, room_to = (o + WIN_U < WIN_V - o ? o + WIN_U : WIN_V - o) - root;
		bool lng = !(r & 1);
		len[r] = lng ? 9 + rng_range(0, 2) + size : 3 + rng_range(0, 1) + size;
		if (len[r] > room_to - 1) len[r] = room_to - 1;
		pad[r] = r != C && ((r & 1) || rng_range(0, 1));
	}
	/* the middle one leads on to a plaza, which the window must hold */
	int pw = 5, pl = 4 + (size > 0);
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
	if (platform(AT_X(end + 2 + pl / 2, 0), AT_Y(end + 2 + pl / 2, 0), along_x ? pl : pw, along_x ? pw : pl,
			route_shape(biome), ROOM_PLATFORM) >= 0)
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

/* After Central Area 1: a path two or three panels wide winding down the
 * window in legs along x and y by turns (the screen's two diagonals), a
 * bulge a panel wider here and there, and pads hung off its sides on short
 * catwalks. */
static void trail(int biome, int size) {
	int x = WIN_C - 10 + rng_range(-1, 1), y = WIN_C - 10 + rng_range(-1, 1);
	bool along_x = rng_range(0, 1);
	for (int l = 0; l < 7 + size; ++l) {
		/* (a leg along x carries the path right across the screen, one
		 * along y left: each is steered back towards the middle, or the
		 * window's edge cut the legs to stubs) */
		int drift = (x - y) * (along_x ? 1 : -1), len = drift > 3 ? rng_range(4, 5) : drift < -3 ? rng_range(7, 8) : rng_range(5, 8);
		int wide = rng_range(2, 3);
		int bw = along_x ? len : wide, bh = along_x ? wide : len;
		/* (short of the window's edge) */
		while (len > 2 && !(win_in(x, y) && win_in(x + bw - 1, y) && win_in(x, y + bh - 1) && win_in(x + bw - 1, y + bh - 1))) {
			--len;
			bw = along_x ? len : wide; bh = along_x ? wide : len;
		}
		if (len <= 2) break;
		for (int j = y; j < y + bh; ++j)
			for (int i = x; i < x + bw; ++i) put(i, j);
		/* a bulge on its outer side, mid-leg */
		if (len >= 5 && rng_range(0, 2) == 0) {
			int t0 = rng_range(1, len - 3);
			for (int t = t0; t < t0 + 2; ++t) {
				int bx = along_x ? x + t : x - 1, by = along_x ? y - 1 : y + t;
				if (win_in(bx, by)) put(bx, by);
			}
		}
		/* (a room of every second leg: the legs overlap at their turns) */
		if (!(l & 1)) add_room(x, y, bw, bh, ROOM_LEG);
		/* the next leg turns at this one's end, overlapping its corner */
		if (along_x) x += len - 2; else y += len - 2;
		along_x = !along_x;
	}
	(void)biome;
	spurs(3 + size, 1, 2);
	stubs(2);
}

/* A maze of 1-wide catwalks, some dead ends cut back, plazas at the ends. */
static void catwalks(int biome, int size) {
	enum { NMAX = 8 };
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
	const int step = comp ? 3 : 2;
	const int N = comp ? (layer.boss_layer ? 5 : 6) : NMAX;
	static bool seen[NMAX][NMAX];
	int ox = WIN_C - step * (N - 1) / 2, oy = ox;
	for (int j = 0; j < N; ++j)
		for (int i = 0; i < N; ++i) seen[j][i] = !win_in(ox + step * i, oy + step * j);
	/* a depth-first maze, a comp's corridors running straight on where
	 * they can */
	int si[NMAX * NMAX], sj[NMAX * NMAX], sd[NMAX * NMAX], sp = 0;
	si[sp] = N / 2; sj[sp] = N / 2; sd[sp++] = -1;
	seen[N / 2][N / 2] = true;
	put(ox + step * (N / 2), oy + step * (N / 2));
	while (sp) {
		int i = si[sp - 1], j = sj[sp - 1], last = sd[sp - 1], opts[4], no = 0;
		bool on = false;
		for (int d = 0; d < 4; ++d) {
			int ni = i + dir_dx[d], nj = j + dir_dy[d];
			if (ni >= 0 && nj >= 0 && ni < N && nj < N && !seen[nj][ni]) { opts[no++] = d; on |= d == last; }
		}
		if (!no) { --sp; continue; }
		int d = comp && on && rng_range(0, 99) < 70 ? last : opts[rng_range(0, no - 1)];
		int ni = i + dir_dx[d], nj = j + dir_dy[d];
		seen[nj][ni] = true;
		for (int c = 1; c < step; ++c) put(ox + step * i + c * dir_dx[d], oy + step * j + c * dir_dy[d]);
		put(ox + step * ni, oy + step * nj);
		si[sp] = ni; sj[sp] = nj; sd[sp++] = d;
	}
	/* loops: walls knocked through (a comp's more, shortcuts that cut its
	 * way down) */
	for (int k = 0; k < (comp ? 8 + 2 * size : 3 + size); ++k) {
		int i = rng_range(0, N - 2), j = rng_range(0, N - 1);
		if (floor_at(ox + step * i, oy + step * j) && floor_at(ox + step * i + step, oy + step * j))
			for (int c = 1; c < step; ++c) put(ox + step * i + c, oy + step * j);
	}
	/* plazas beyond both ends of the maze, up and down the window (the
	 * Aquarium's are its glass pads' size: its water never widens) */
	for (int e = 0; e < 2; ++e) {
		int half = step * (N - 1) / 2 + 3, off = e ? half : -half;
		if (biome == BIOME_AQUARIUM_COMP) platform(WIN_C + off, WIN_C + off, 4, 4, SHAPE_RECT, ROOM_PLATFORM);
		else platform(WIN_C + off, WIN_C + off, 5, 4 + rng_range(0, 1), route_shape(biome), ROOM_PLATFORM);
	}
	spurs(4 + size, 1, 2);
}

void layout_build(int layout, int biome, int size) {
	switch (layout) {
	case LAYOUT_FIELD: field(biome, size); break;
	case LAYOUT_LADDER: ladder(biome, size); break;
	case LAYOUT_HUB: hub(biome, size); break;
	case LAYOUT_SLABS: route(biome, size, true); break;
	case LAYOUT_WEB: web(biome, size); break;
	case LAYOUT_CROSSES: crosses(biome, size); break;
	case LAYOUT_CATWALKS: catwalks(biome, size); break;
	case LAYOUT_COMB: comb(biome, size); break;
	case LAYOUT_TRAIL: trail(biome, size); break;
	default: route(biome, size, false); break;
	}
}
