/* Layouts after BN6's net areas (docs/LEVEL_DESIGN.md). Each builds from
 * the pieces in net_shapes.c; the generator then joins anything left
 * apart, picks where MegaMan arrives and checks the size. */
#include "net_layouts.h"

#include <stdlib.h>

#include "game.h"
#include "net_shapes.h"
#include "run.h"

const char *const layout_names[LAYOUT_COUNT] = {
	"route", "field", "ladder", "hub", "slabs", "web", "crosses", "catwalks",
};

/* per area: weights of each layout (percent) */
static const uint8_t weights[BIOME_COUNT][LAYOUT_COUNT] = {
	/*                 route field ladder hub slabs web crosses catwalks */
	[BIOME_CENTRAL]   = { 35, 35, 0, 0, 0, 0, 0, 30 },
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
	[BIOME_ROBOT_COMP]    = { 40, 0, 0, 35, 0, 0, 25, 0 },
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

/* A bridge between rooms a and b: straight out of a towards b, one bend. */
static void link(int a, int b) {
	const Room *ra = &layer.rooms[a], *rb = &layer.rooms[b];
	int dx = rb->ax - ra->ax, dy = rb->ay - ra->ay;
	int d = abs(dx) >= abs(dy) ? (dx > 0 ? DIR_E : DIR_W) : (dy > 0 ? DIR_S : DIR_N);
	int ax, ay, bx, by;
	if (!room_edge(a, d, &ax, &ay) || !room_edge(b, (d + 2) % 4, &bx, &by)) {
		bridge_l(ra->ax, ra->ay, rb->ax, rb->ay, true);
		return;
	}
	bridge_l(ax, ay, bx, by, d == DIR_E || d == DIR_W);
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
			if (floor_at(cx[i] + dir_dx[d], cy[i] + dir_dy[d])) continue;
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
		if (!floor_at(x, y)) continue;
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
		int run = (d == DIR_E || d == DIR_W) ? DIR_S : DIR_E;
		int len = (run == DIR_E ? w : h) + 2;
		int sx = d == DIR_E ? fx + w + 2 : d == DIR_W ? fx - 3 : fx - 2;
		int sy = d == DIR_S ? fy + h + 2 : d == DIR_N ? fy - 3 : fy - 2;
		if (!bridge_line(sx - dir_dx[run], sy - dir_dy[run], run, len)) continue;
		int out = (run + 1) % 4 == d ? 1 : -1;
		teeth(sx - dir_dx[run], sy - dir_dy[run], run, len, out);
		/* rungs back to the field */
		for (int r = 0; r < 2; ++r) {
			int t = rng_range(2, len - 3);
			int rx = sx + dir_dx[run] * t, ry = sy + dir_dy[run] * t;
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
	int run = along_x ? DIR_E : DIR_S, across = along_x ? DIR_S : DIR_E;
	int rails = 4 + (size > 0), len = 14 + 2 * size + rng_range(0, 3);
	int start[5], base = -(rails - 1) * 3 / 2;
	for (int r = 0; r < rails; ++r) {
		start[r] = -len / 2 + rng_range(-2, 2);
		int o = base + 3 * r;
		int x = WIN_C + dir_dx[run] * (start[r] - 1) + dir_dx[across] * o;
		int y = WIN_C + dir_dy[run] * (start[r] - 1) + dir_dy[across] * o;
		bridge_line(x, y, run, len);
		/* the outer planks' teeth */
		if (r == 0) teeth(x, y, run, len, (run + 1) % 4 == across ? -1 : 1);
		if (r == rails - 1) teeth(x, y, run, len, (run + 1) % 4 == across ? 1 : -1);
	}
	/* rungs where neighbouring planks overlap */
	for (int r = 0; r + 1 < rails; ++r)
		for (int k = 0; k < 2 + rng_range(0, 1); ++k) {
			int lo = start[r] > start[r + 1] ? start[r] : start[r + 1];
			int t = lo + rng_range(1, len - 3);
			int o = base + 3 * r;
			for (int s = 1; s <= 2; ++s)
				put(WIN_C + dir_dx[run] * t + dir_dx[across] * (o + s), WIN_C + dir_dy[run] * t + dir_dy[across] * (o + s));
		}
	/* grass blocks off the first plank's start and the last plank's end */
	for (int e = 0; e < 2; ++e) {
		int r = e ? rails - 1 : 0, o = base + 3 * r;
		int t = e ? start[r] + len + 3 : start[r] - 4;
		int w = rng_range(4, 6), h = rng_range(4, 5);
		int cx = WIN_C + dir_dx[run] * t + dir_dx[across] * o, cy = WIN_C + dir_dy[run] * t + dir_dy[across] * o;
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

/* Plus-shaped platforms on a lattice around a big block. */
static void crosses(int biome, int size) {
	enum { STEP = 9 };
	int node[5][5];
	for (int j = 0; j < 5; ++j)
		for (int i = 0; i < 5; ++i) node[j][i] = -1;
	/* grow a tree over the lattice from its middle */
	int want = 6 + size, have = 0, qi[25], qj[25], nq = 0;
	node[2][2] = platform(WIN_C, WIN_C, 7, 7, SHAPE_RECT, ROOM_PLATFORM);
	if (node[2][2] < 0) return;
	qi[nq] = 2; qj[nq++] = 2; ++have;
	for (int tries = 0; tries < 200 && have < want; ++tries) {
		int k = rng_range(0, nq - 1), d = rng_range(0, 3);
		int i = qi[k] + dir_dx[d], j = qj[k] + dir_dy[d];
		if (i < 0 || j < 0 || i > 4 || j > 4 || node[j][i] >= 0) continue;
		int r = platform(WIN_C + (i - 2) * STEP, WIN_C + (j - 2) * STEP, 7, 7, SHAPE_PLUS, ROOM_PLATFORM);
		if (r < 0) continue;
		node[j][i] = r;
		link(node[qj[k]][qi[k]], r);
		qi[nq] = i; qj[nq++] = j; ++have;
	}
	/* a loop or two between lattice neighbours */
	for (int k = 0; k < 2; ++k) {
		int i = rng_range(0, 3), j = rng_range(0, 4);
		if (node[j][i] >= 0 && node[j][i + 1] >= 0 && rng_range(0, 1)) link(node[j][i], node[j][i + 1]);
	}
	(void)biome;
	spurs(2, 2, 3);
	stubs(3);
}

/* A maze of 1-wide catwalks, some dead ends cut back, plazas at the ends. */
static void catwalks(int biome, int size) {
	enum { N = 8 };   /* lattice nodes per side, two cells apart */
	static bool seen[N][N];
	int ox = WIN_C - N + 1, oy = WIN_C - N + 1;
	for (int j = 0; j < N; ++j)
		for (int i = 0; i < N; ++i) seen[j][i] = !win_in(ox + 2 * i, oy + 2 * j);
	/* a depth-first maze */
	int si[N * N], sj[N * N], sp = 0;
	si[sp] = N / 2; sj[sp++] = N / 2;
	seen[N / 2][N / 2] = true;
	put(ox + N, oy + N);
	while (sp) {
		int i = si[sp - 1], j = sj[sp - 1], opts[4], no = 0;
		for (int d = 0; d < 4; ++d) {
			int ni = i + dir_dx[d], nj = j + dir_dy[d];
			if (ni >= 0 && nj >= 0 && ni < N && nj < N && !seen[nj][ni]) opts[no++] = d;
		}
		if (!no) { --sp; continue; }
		int d = opts[rng_range(0, no - 1)];
		int ni = i + dir_dx[d], nj = j + dir_dy[d];
		seen[nj][ni] = true;
		put(ox + 2 * i + dir_dx[d], oy + 2 * j + dir_dy[d]);
		put(ox + 2 * ni, oy + 2 * nj);
		si[sp] = ni; sj[sp++] = nj;
	}
	/* loops: a few walls knocked through */
	for (int k = 0; k < 3 + size; ++k) {
		int i = rng_range(0, N - 2), j = rng_range(0, N - 1);
		if (floor_at(ox + 2 * i, oy + 2 * j) && floor_at(ox + 2 * i + 2, oy + 2 * j)) put(ox + 2 * i + 1, oy + 2 * j);
	}
	/* plazas beyond both ends of the maze, up and down the window (the
	 * Aquarium's are its glass pads' size: its water never widens) */
	for (int e = 0; e < 2; ++e) {
		int off = e ? N + 2 : -N - 2;
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
	default: route(biome, size, false); break;
	}
}
