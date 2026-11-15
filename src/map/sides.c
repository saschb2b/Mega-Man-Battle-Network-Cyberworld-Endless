/* Side pieces (sides.h). A map's second tile layer holds what it sets in
 * front of its floor. A group of its tiles joined 8 ways is a side piece
 * where it grips ground floor and hangs off it: some of what it draws lies
 * on a ground panel's top, a third or more off every ground panel's. What
 * lies on the floor alone is a pad's centrepiece or an emblem, set by rules
 * of their own; what touches no floor is scenery (decor.c). */
#include "sides.h"

#include <stdlib.h>
#include <string.h>

#define MIN_TILES    4
#define MAX_TILES  160
#define MAX_W       20   /* tiles */
#define MAX_H       14
#define ON_FLOOR     8   /* pixels on its panel's top at least: it grips it */
#define OFF_SHARE    3   /* tenths of its pixels off the ground floor at least */
#define FLOOR_DV     4   /* a floor's top is drawn this far below its panel's edges */
#define PANELS      32   /* panels a piece may lie over, counted apart */
#define UNDER_PX     6   /* pixels over a panel at least for the panel to count as under it */

static int div_floor(int a, int b) { return a / b - (a % b < 0); }

/* Whether panel (A, B) is floor on the ground (a raised view's: at its level). */
static bool ground(const AreaSrc *a, int A, int B) {
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	return area_src_walled_floor(a, X, Y) == 1 && area_src_height(a, X, Y) == a->level;
}

/* The panel whose ground top holds map pixel (x, y). */
static void panel_at(const AreaSrc *a, int x, int y, int *A, int *B) {
	int u = x - a->tw * 4, v = 2 * (y - FLOOR_DV - a->th * 4);
	*A = div_floor(div_floor(u - v, 2) - a->ex, 32);
	*B = div_floor(div_floor(u + v, 2) - a->ey, 32);
}

/* A piece while it is measured: its tiles, and the pixels it draws by the
 * ground panels they lie on. */
typedef struct {
	int n, x0, y0, x1, y1;
	int on, off, other;
	int A[PANELS], B[PANELS], count[PANELS], npanels;
	bool gnd[PANELS], edge;
} Group;

/* (`first`: of the first layer's pixels, else the second's) */
static void count_tile(const AreaSrc *a, bool first, int tx, int ty, Group *g) {
	int W = a->tw * 8;
	for (int y = ty * 8; y < ty * 8 + 8; ++y)
		for (int x = tx * 8; x < tx * 8 + 8; ++x) {
			if (!(first ? a->idx0[(size_t)y * W + x] : a->front[(size_t)y * W + x])) continue;
			int A, B;
			panel_at(a, x, y, &A, &B);
			bool gnd = ground(a, A, B);
			++*(gnd ? &g->on : &g->off);
			/* (floor of another height under it, a bridge or a raised floor's
			 * own: no panel a layer's must match) */
			if (!gnd && area_src_walled_floor(a, a->ex + 16 + 32 * A, a->ey + 16 + 32 * B) == 1) { ++g->other; continue; }
			int k = 0;
			while (k < g->npanels && (g->A[k] != A || g->B[k] != B)) ++k;
			if (k == g->npanels && k < PANELS) { g->A[k] = A; g->B[k] = B; g->gnd[k] = gnd; g->count[k] = 0; ++g->npanels; }
			if (k < PANELS) ++g->count[k];
		}
}

#define HANG_FACE  8   /* a floor's side face reaches this far under its edge at most */
#define HANG_UP   48   /* ... and what stands under it begins within this of the floor */

static bool off_floor(const AreaSrc *a, int x, int y) {
	int A, B;
	panel_at(a, x, y, &A, &B);
	return !ground(a, A, B);
}

/* Whether first-layer tile (tx, ty) lies under a floor's face or free of
 * any: drawn, and over no floor nor within a face's height under one. */
static bool hang_tile(const AreaSrc *a, int tx, int ty) {
	if (!(a->tile[0][ty * a->tw + tx] & 0x3FF)) return false;
	for (int k = 0; k < 4; ++k) {
		int x = tx * 8 + 7 * (k & 1), y = ty * 8 + 7 * (k >> 1);
		if (!off_floor(a, x, y) || !off_floor(a, x, y - HANG_FACE)) return false;
	}
	return true;
}

/* Whether tile (x, y) is one a group is made of: the second layer's, or
 * (`hang`) the first's under a floor. */
static bool member(const AreaSrc *a, bool hang, int x, int y) {
	return hang ? hang_tile(a, x, y) : (a->tile[1][y * a->tw + x] & 0x3FF) != 0;
}

/* The group of such tiles joined to (sx, sy), its tiles' indexes in
 * `queue`; `seen` marks them. */
static void group_from(const AreaSrc *a, bool hang, int sx, int sy, uint8_t *seen, int *queue, Group *g) {
	int tw = a->tw, th = a->th;
	memset(g, 0, sizeof *g);
	g->x0 = g->x1 = sx; g->y0 = g->y1 = sy;
	queue[g->n++] = sy * tw + sx;
	seen[sy * tw + sx] = 1;
	for (int h = 0; h < g->n; ++h) {
		int x = queue[h] % tw, y = queue[h] / tw;
		if (x < g->x0) g->x0 = x;
		if (x > g->x1) g->x1 = x;
		if (y < g->y0) g->y0 = y;
		if (y > g->y1) g->y1 = y;
		g->edge |= x == 0 || y == 0 || x == tw - 1 || y == th - 1;
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx) {
				int nx = x + dx, ny = y + dy;
				if (nx < 0 || ny < 0 || nx >= tw || ny >= th || seen[ny * tw + nx] || !member(a, hang, nx, ny)) continue;
				seen[ny * tw + nx] = 1;
				queue[g->n++] = ny * tw + nx;
			}
	}
}

/* Whether the group is a side piece; its panel in (*A, *B). */
static bool grips(const AreaSrc *a, const int *queue, Group *g, int *A, int *B) {
	if (g->edge || g->n < MIN_TILES || g->n > MAX_TILES || g->x1 - g->x0 >= MAX_W || g->y1 - g->y0 >= MAX_H) return false;
	for (int k = 0; k < g->n; ++k) count_tile(a, false, queue[k] % a->tw, queue[k] / a->tw, g);
	if (g->on < ON_FLOOR || g->off * 10 < OFF_SHARE * (g->on + g->off) || g->other * 10 > g->on + g->off) return false;
	int best = -1;
	for (int k = 0; k < g->npanels; ++k)
		if (g->gnd[k] && (best < 0 || g->count[k] > g->count[best])) best = k;
	if (best < 0) return false;
	*A = g->A[best]; *B = g->B[best];
	/* (an arrow lane's frame is its lane's, set with it) */
	return !area_src_arrow_at(a, a->ex + 16 + 32 * *A, a->ey + 16 + 32 * *B);
}

/* The panels group g draws over, from panel (A, B), into p. */
static void under(const Group *g, int A, int B, SidePiece *p) {
	p->nunder = 0;
	for (int k = 0; k < g->npanels && p->nunder < SIDE_UNDER; ++k) {
		if (g->count[k] < UNDER_PX || abs(g->A[k] - A) > 100 || abs(g->B[k] - B) > 100) continue;
		p->under[p->nunder][0] = (int8_t)(g->A[k] - A);
		p->under[p->nunder][1] = (int8_t)(g->B[k] - B);
		p->under[p->nunder++][2] = g->gnd[k];
	}
}

static bool same(const SidePiece *p, const SidePiece *q) {
	if (p->ntiles != q->ntiles || p->floor != q->floor) return false;
	for (int k = 0; k < p->ntiles; ++k)
		if (p->tiles[k].px != q->tiles[k].px || p->tiles[k].py != q->tiles[k].py || p->tiles[k].e1 != q->tiles[k].e1 || p->tiles[k].e0 != q->tiles[k].e0) return false;
	return true;
}

/* The map's layer priorities on panel (A, B) and the eight round it, from
 * the panel's corner: BN6 draws MegaMan behind a claw on the panel it grips
 * (CopyBot Comp: 27 to 48 cells within a panel of each of its claws, of the
 * map's 434). None without memory. */
static void priorities(const AreaSrc *a, int A, int B, SidePiece *p) {
	int X0 = a->ex + 32 * A, Y0 = a->ey + 32 * B;
	p->prio = calloc((size_t)a->nsec[2] + 1, sizeof *p->prio);
	p->nprio = 0;
	for (int k = 0; p->prio && k < a->nsec[2]; ++k) {
		CoordCell c = a->sec[2][k];
		if (c.x < X0 - 32 || c.x >= X0 + 64 || c.y < Y0 - 32 || c.y >= Y0 + 64) continue;
		c.x = (int16_t)(c.x - X0);
		c.y = (int16_t)(c.y - Y0);
		p->prio[p->nprio++] = c;
	}
}

/* A new piece of up to n tiles for panel (A, B) of map a, its
 * neighbourhood set; NULL where memory runs out. */
static SidePiece *begin(const AreaSrc *a, int n, int A, int B, SideBook *out) {
	SidePiece *p = &out->piece[out->n];
	memset(p, 0, sizeof *p);
	p->tiles = calloc((size_t)n, sizeof *p->tiles);
	if (!p->tiles) return NULL;
	p->ntiles = n;
	for (int k = 0; k < 9; ++k)
		if (ground(a, A + k % 3 - 1, B + k / 3 - 1)) p->floor |= (uint16_t)(1u << k);
	return p;
}

/* Map tile i's place from panel (A, B)'s corner. */
static StairTile placed(const AreaSrc *a, int i, int A, int B) {
	int ax = area_px(a->tw, a->ex + 32 * A, a->ey + 32 * B), ay = area_py(a->th, a->ex + 32 * A, a->ey + 32 * B);
	return (StairTile){ .px = (int16_t)(i % a->tw * 8 - ax), .py = (int16_t)(i / a->tw * 8 - ay) };
}

/* Piece p, group g's on panel (A, B), into the book unless it holds its
 * look. */
static void finish(const AreaSrc *a, const Group *g, int A, int B, SidePiece *p, SideBook *out) {
	for (int k = 0; k < out->n; ++k)
		if (same(&out->piece[k], p)) { free(p->tiles); p->tiles = NULL; return; }
	if (!p->hang) priorities(a, A, B, p);
	under(g, A, B, p);
	++out->n;
}

/* Group g, on panel (A, B), into the book unless it holds its look. */
static void keep(const AreaSrc *a, const int *queue, const Group *g, int A, int B, int kind, SideBook *out) {
	bool hang = kind != 0;
	SidePiece *p = begin(a, g->n, A, B, out);
	if (!p) return;
	p->hang = hang;
	p->behind = kind == 2;
	for (int k = 0; k < g->n; ++k) {
		p->tiles[k] = placed(a, queue[k], A, B);
		*(hang ? &p->tiles[k].e0 : &p->tiles[k].e1) = a->tile[hang ? 0 : 1][queue[k]];
	}
	finish(a, g, A, B, p, out);
}

void sides_learn(const AreaSrc *a, SideBook *out) {
	if (a->layers < 2 || !a->rings || !a->front) return;
	int tw = a->tw, th = a->th;
	uint8_t *seen = calloc((size_t)tw * th, 1);
	int *queue = malloc((size_t)tw * th * sizeof *queue);
	for (int i = 0; seen && queue && i < tw * th && out->n < SIDES_MAX; ++i) {
		if (seen[i] || !(a->tile[1][i] & 0x3FF)) continue;
		Group g;
		int A, B;
		group_from(a, false, i % tw, i / tw, seen, queue, &g);
		if (!grips(a, queue, &g, &A, &B)) continue;
		keep(a, queue, &g, A, B, 0, out);
	}
	free(seen);
	free(queue);
}

#define BEHIND_DOWN  16   /* what stands behind a floor ends within this above it */
#define BEHIND_TILES 24   /* ... and is no larger: an exit's arrow is */

/* Whether a floor's panel lies straight above group g's top row within
 * HANG_UP (`below`: straight below its bottom row within BEHIND_DOWN), in
 * (*A, *B) the one most of that row's columns meet. */
static bool meets(const AreaSrc *a, const int *queue, const Group *g, bool below, int *A, int *B) {
	int row = below ? g->y1 : g->y0, from = below ? row * 8 + 8 : row * 8 - 1, step = below ? 1 : -1, reach = below ? BEHIND_DOWN : HANG_UP;
	int PA[2 * MAX_W], PB[2 * MAX_W], votes[2 * MAX_W], n = 0, best = 0;
	for (int k = 0; k < g->n; ++k) {
		if (queue[k] / a->tw != row) continue;
		for (int x = queue[k] % a->tw * 8 + 2; x < queue[k] % a->tw * 8 + 8 && n < 2 * MAX_W; x += 4)
			for (int y = from; y >= 0 && y < a->th * 8 && abs(y - from) < reach; y += step) {
				if (off_floor(a, x, y)) continue;
				int j = 0;
				panel_at(a, x, y, &PA[n], &PB[n]);
				while (PA[j] != PA[n] || PB[j] != PB[n]) ++j;
				if (j == n) votes[n++] = 0;
				if (++votes[j] > votes[best]) best = j;
				break;
			}
	}
	if (!n) return false;
	*A = PA[best]; *B = PB[best];
	return true;
}

/* Whether group g stands under a floor, over none itself; `behind`: whether
 * it stands behind one and under none (a pad's knob over a walkway behind
 * it is its pad's). */
static bool hangs(const AreaSrc *a, const int *queue, Group *g, bool behind, int *A, int *B) {
	if (g->edge || g->n < MIN_TILES || g->n > (behind ? BEHIND_TILES : MAX_TILES) || g->x1 - g->x0 >= MAX_W || g->y1 - g->y0 >= MAX_H) return false;
	for (int k = 0; k < g->n; ++k) count_tile(a, true, queue[k] % a->tw, queue[k] / a->tw, g);
	if (g->on || g->other * 10 > g->off) return false;
	return behind ? !meets(a, queue, g, false, A, B) && meets(a, queue, g, true, A, B) : meets(a, queue, g, false, A, B);
}

/* The first layer's groups off the floor of map or view r that stand under
 * a floor (kind 1) or behind one (kind 2), into the book. */
static void first_learn(const AreaSrc *r, int kind, SideBook *out) {
	if (!r->rings || !r->idx0) return;
	int tw = r->tw, th = r->th;
	uint8_t *seen = calloc((size_t)tw * th, 1);
	int *queue = malloc((size_t)tw * th * sizeof *queue);
	for (int i = 0; seen && queue && i < tw * th && out->n < SIDES_MAX; ++i) {
		if (seen[i] || !hang_tile(r, i % tw, i / tw)) continue;
		Group g;
		int A, B;
		group_from(r, true, i % tw, i / tw, seen, queue, &g);
		if (hangs(r, queue, &g, kind == 2, &A, &B)) keep(r, queue, &g, A, B, kind, out);
	}
	free(seen);
	free(queue);
}

void hangs_learn(const AreaSrc *r, SideBook *out) { first_learn(r, 1, out); }

void behind_learn(const AreaSrc *a, SideBook *out) { first_learn(a, 2, out); }

#define STAND_TILES 400   /* what stands behind a floor on both layers is no larger (a tank: 215) */
#define STAND_W      24   /* tiles */
#define STAND_H      24
#define STAND_OFF     7   /* tenths of its pixels off the floor at least: it meets it at its foot */

/* A group of the second layer cut as what stands behind a floor: where it
 * is a row of copies, the step from one to the next (lower on the screen,
 * in front of it), its front copy and how far back its back copy stands. */
typedef struct {
	const AreaSrc *a;
	const int *queue, *owner;   /* the group's tiles; each map tile's group (its first tile), this one `id` */
	const Group *g;
	int id, dx, dy, back;
	uint8_t *mark;              /* all clear, and left so */
	int *unit, n;               /* the front copy's tiles */
	Group u;                    /* ... measured */
	int lo[STAND_H], hi[STAND_H], rows;   /* its rows' ends */
} Row;

/* Whether tile (x, y) is on the map and a copy of tile i: the row's, with
 * its second-layer entry. */
static bool copy_at(const Row *r, int i, int x, int y) {
	const AreaSrc *a = r->a;
	return x >= 0 && y >= 0 && x < a->tw && y < a->th && r->owner[y * a->tw + x] == r->id && a->tile[1][i] == a->tile[1][y * a->tw + x];
}

/* Whether the group is a row of copies, a step (dx, dy) tiles apart: the
 * step a third of its tiles repeat at, the most. */
static bool repeat_step(Row *r) {
	int most = 0, tw = r->a->tw;
	for (int y = 0; y <= STAND_H; ++y)
		for (int x = y ? -STAND_W : 1; x <= STAND_W; ++x) {
			int n = 0;
			for (int k = 0; k < r->g->n; ++k) n += copy_at(r, r->queue[k], r->queue[k] % tw + x, r->queue[k] / tw + y);
			if (n > most) { most = n; r->dx = x; r->dy = y; }
		}
	return most * 3 >= r->g->n;
}

/* The front copy into r->unit: the tiles with no copy a step on, the
 * largest lot of them joined 8 ways (with no step, the whole group). */
static void front_copy(Row *r) {
	const AreaSrc *a = r->a;
	int tw = a->tw, best = 0, nbest = 0, n = 0;
	const int *q = r->queue;
	if (!r->dx && !r->dy) { memcpy(r->unit, q, (size_t)r->g->n * sizeof *q); r->n = r->g->n; return; }
	for (int k = 0; k < r->g->n; ++k) r->mark[q[k]] = !copy_at(r, q[k], q[k] % tw + r->dx, q[k] / tw + r->dy);
	for (int k = 0; k < r->g->n; ++k) {
		if (r->mark[q[k]] != 1) continue;
		int from = n;
		r->unit[n++] = q[k];
		r->mark[q[k]] = 2;
		for (int h = from; h < n; ++h)
			for (int d = 0; d < 9; ++d) {
				int x = r->unit[h] % tw + d % 3 - 1, y = r->unit[h] / tw + d / 3 - 1;
				if (x < 0 || y < 0 || x >= tw || y >= a->th || r->mark[y * tw + x] != 1) continue;
				r->mark[y * tw + x] = 2;
				r->unit[n++] = y * tw + x;
			}
		if (n - from > nbest) { best = from; nbest = n - from; }
	}
	for (int k = 0; k < r->g->n; ++k) r->mark[q[k]] = 0;
	memmove(r->unit, r->unit + best, (size_t)nbest * sizeof *r->unit);
	r->n = nbest;
}

/* How many steps back the back copy stands from the front one: the
 * farthest that half the front copy's tiles repeat at. */
static int back_steps(const Row *r) {
	int j = 0, tw = r->a->tw;
	while (r->dx || r->dy) {
		int same = 0;
		for (int k = 0; k < r->n; ++k) same += copy_at(r, r->unit[k], r->unit[k] % tw - (j + 1) * r->dx, r->unit[k] / tw - (j + 1) * r->dy);
		if (same * 2 < r->n) break;
		++j;
	}
	return j;
}

/* The front copy's box and pixels (r->u) and its rows' ends. */
static void measure(Row *r) {
	int tw = r->a->tw;
	Group *u = &r->u;
	memset(u, 0, sizeof *u);
	u->n = r->n;
	u->x0 = u->x1 = r->unit[0] % tw;
	u->y0 = u->y1 = r->unit[0] / tw;
	for (int k = 0; k < r->n; ++k) {
		int x = r->unit[k] % tw, y = r->unit[k] / tw;
		if (x < u->x0) u->x0 = x;
		if (x > u->x1) u->x1 = x;
		if (y < u->y0) u->y0 = y;
		if (y > u->y1) u->y1 = y;
		count_tile(r->a, false, x, y, u);
	}
	r->rows = u->y1 - u->y0 + 1;
	for (int y = 0; y < STAND_H; ++y) { r->lo[y] = tw; r->hi[y] = -1; }
	for (int k = 0; k < r->n && r->rows <= STAND_H; ++k) {
		int x = r->unit[k] % tw, y = r->unit[k] / tw - u->y0;
		if (x < r->lo[y]) r->lo[y] = x;
		if (x > r->hi[y]) r->hi[y] = x;
	}
}

/* Whether tile (x, y) lies in the front copy's rows, between their ends. */
static bool in_rows(const Row *r, int x, int y) {
	int k = y - r->u.y0;
	return k >= 0 && k < r->rows && x >= r->lo[k] && x <= r->hi[k];
}

/* Where to read the front copy's tile (x, y): the copies stand over the one
 * behind them, so where the copy behind the front one reaches into it (as
 * its map draws it, the two in one tile), the back copy, which nothing
 * stands over; elsewhere the front copy. */
static int stand_source(const Row *r, int x, int y) {
	int bx = x - r->back * r->dx, by = y - r->back * r->dy, tw = r->a->tw;
	if (!r->back || !in_rows(r, x + r->dx, y + r->dy) || in_rows(r, x - r->dx, y - r->dy)) return y * tw + x;
	return bx >= 0 && by >= 0 && bx < tw && by < r->a->th ? by * tw + bx : y * tw + x;
}

/* Piece p's tiles, the front copy's on panel (A, B), each as the copy that
 * shows it whole draws it: its second layer's, and the first layer's off
 * the floor between its ends in each row, what shows through it. */
static void stand_tiles(const Row *r, int A, int B, SidePiece *p) {
	const AreaSrc *a = r->a;
	int tw = a->tw;
	p->ntiles = 0;
	for (int y = r->u.y0; y <= r->u.y1; ++y)
		for (int x = r->lo[y - r->u.y0]; x <= r->hi[y - r->u.y0]; ++x) {
			int src = stand_source(r, x, y);
			bool off = hang_tile(a, src % tw, src / tw);
			uint16_t e0 = off ? a->tile[0][src] : 0, e1 = a->tile[1][src] & 0x3FF ? a->tile[1][src] : 0;
			if (!e0 && !e1) continue;
			StairTile *t = &p->tiles[p->ntiles++];
			*t = placed(a, y * tw + x, A, B);
			t->e0 = e0;
			t->e1 = e1;
			t->over = e1 && (a->tile[0][src] & 0x3FF) && !off;
		}
}

/* The row r's group as what stands behind its view's floor, its front copy
 * where it is a row of them, into the book. */
static void stand(Row *r, SideBook *out) {
	const Group *g = r->g, *u = &r->u;
	int A, B;
	r->dx = r->dy = 0;
	bool whole = g->n <= STAND_TILES && g->x1 - g->x0 < STAND_W && g->y1 - g->y0 < STAND_H;
	if (!whole && !repeat_step(r)) return;
	front_copy(r);
	measure(r);
	if (r->n < MIN_TILES || r->n > STAND_TILES || u->x1 - u->x0 >= STAND_W || u->y1 - u->y0 >= STAND_H) return;
	/* (its foot on the floor, the rest off it, over no other floor; a
	 * floor straight below it and none above) */
	if (u->on < ON_FLOOR || u->off * 10 < STAND_OFF * (u->on + u->off) || u->other * 10 > u->on + u->off) return;
	if (meets(r->a, r->unit, u, false, &A, &B) || !meets(r->a, r->unit, u, true, &A, &B)) return;
	SidePiece *p = begin(r->a, (u->x1 - u->x0 + 1) * (u->y1 - u->y0 + 1), A, B, out);
	if (!p) return;
	p->hang = p->behind = p->both = true;
	r->back = back_steps(r);
	stand_tiles(r, A, B, p);
	finish(r->a, u, A, B, p, out);
}

void stands_learn(const AreaSrc *a, SideBook *out) {
	if (a->layers < 2 || !a->rings || !a->idx0) return;
	size_t cells = (size_t)a->tw * a->th;
	uint8_t *seen = calloc(cells, 1), *mark = calloc(cells, 1);
	int *queue = malloc(cells * sizeof *queue), *owner = malloc(cells * sizeof *owner), *unit = malloc(cells * sizeof *unit);
	for (size_t i = 0; owner && i < cells; ++i) owner[i] = -1;
	for (size_t i = 0; seen && mark && queue && owner && unit && i < cells && out->n < SIDES_MAX; ++i) {
		if (seen[i] || !(a->tile[1][i] & 0x3FF)) continue;
		Group g;
		group_from(a, false, (int)(i % (size_t)a->tw), (int)(i / (size_t)a->tw), seen, queue, &g);
		for (int k = 0; k < g.n; ++k) owner[queue[k]] = (int)i;
		Row r = { .a = a, .queue = queue, .owner = owner, .g = &g, .id = (int)i, .mark = mark, .unit = unit };
		if (!g.edge && g.n >= MIN_TILES) stand(&r, out);
	}
	free(seen);
	free(mark);
	free(queue);
	free(owner);
	free(unit);
}
