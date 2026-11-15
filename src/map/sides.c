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

static bool meets(const AreaSrc *a, const int *queue, const Group *g, bool below, int *A, int *B);
static void behind_each(const AreaSrc *a, const int *queue, const Group *g, int A, int B, SideBook *out);
static void stands_on(const AreaSrc *a, const int *queue, const Group *g, int A, int B, SidePiece *p);
static bool hangs(const AreaSrc *a, const int *queue, Group *g, int kind, int *A, int *B);

/* Whether the group is a side piece; its panel in (*A, *B). `under`: or
 * one that stands under a floor's lower edge touching none of its top,
 * kept with the panel straight above it (the Nest's crystals). */
static bool grips(const AreaSrc *a, const int *queue, Group *g, bool under, int *A, int *B) {
	if (g->edge || g->n < MIN_TILES || g->n > MAX_TILES || g->x1 - g->x0 >= MAX_W || g->y1 - g->y0 >= MAX_H) return false;
	for (int k = 0; k < g->n; ++k) count_tile(a, false, queue[k] % a->tw, queue[k] / a->tw, g);
	if (g->other * 10 > g->on + g->off) return false;
	if (under && !g->on && meets(a, queue, g, false, A, B)) return !area_src_arrow_at(a, a->ex + 16 + 32 * *A, a->ey + 16 + 32 * *B);
	if (g->on < ON_FLOOR || g->off * 10 < OFF_SHARE * (g->on + g->off)) return false;
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

/* The map's layer priorities on the panels from (A + box[0], B + box[2])
 * to (A + box[1], B + box[3]), from panel (A, B)'s corner. None without
 * memory. */
static void priorities_over(const AreaSrc *a, int A, int B, const int box[4], SidePiece *p) {
	int a0 = box[0], a1 = box[1], b0 = box[2], b1 = box[3];
	int X0 = a->ex + 32 * A, Y0 = a->ey + 32 * B;
	p->prio = calloc((size_t)a->nsec[2] + 1, sizeof *p->prio);
	p->nprio = 0;
	for (int k = 0; p->prio && k < a->nsec[2]; ++k) {
		CoordCell c = a->sec[2][k];
		if (c.x < X0 + 32 * a0 || c.x >= X0 + 32 * (a1 + 1) || c.y < Y0 + 32 * b0 || c.y >= Y0 + 32 * (b1 + 1)) continue;
		c.x = (int16_t)(c.x - X0);
		c.y = (int16_t)(c.y - Y0);
		p->prio[p->nprio++] = c;
	}
}

/* The map's layer priorities on panel (A, B) and the eight round it: BN6
 * draws MegaMan behind a claw on the panel it grips (CopyBot Comp: 27 to 48
 * cells within a panel of each of its claws, of the map's 434). */
static void priorities(const AreaSrc *a, int A, int B, SidePiece *p) {
	static const int round[4] = { -1, 1, -1, 1 };
	priorities_over(a, A, B, round, p);
}

/* Whether the first layer draws any pixel of tile (tx, ty): an entry that
 * draws none is the void a layer leaves empty. */
static bool drawn0(const AreaSrc *a, int tx, int ty) {
	for (int y = ty * 8; y < ty * 8 + 8; ++y)
		for (int x = tx * 8; x < tx * 8 + 8; ++x)
			if (a->idx0[(size_t)y * a->tw * 8 + x]) return true;
	return false;
}

/* Group g, on panel (A, B), into the book unless it holds its look; kind 0
 * a side piece, 1 under a floor, 2 behind one, 3 a side piece with the
 * first layer's tiles under it (SidePiece.over), 4 behind a floor on the
 * second layer, with them too. */
static void keep(const AreaSrc *a, const int *queue, const Group *g, int A, int B, int kind, SideBook *out) {
	bool hang = kind == 1 || kind == 2;
	int tw = a->tw, th = a->th;
	SidePiece *p = &out->piece[out->n];
	memset(p, 0, sizeof *p);
	p->tiles = calloc((size_t)g->n, sizeof *p->tiles);
	if (!p->tiles) return;
	p->ntiles = g->n;
	p->hang = hang;
	p->behind = kind == 2 || kind == 4;
	p->over = kind == 3 || kind == 4;
	for (int k = 0; k < 9; ++k)
		if (ground(a, A + k % 3 - 1, B + k / 3 - 1)) p->floor |= (uint16_t)(1u << k);
	int ax = area_px(tw, a->ex + 32 * A, a->ey + 32 * B), ay = area_py(th, a->ex + 32 * A, a->ey + 32 * B);
	for (int k = 0; k < g->n; ++k) {
		p->tiles[k] = (StairTile){ .px = (int16_t)(queue[k] % tw * 8 - ax), .py = (int16_t)(queue[k] / tw * 8 - ay) };
		*(hang ? &p->tiles[k].e0 : &p->tiles[k].e1) = a->tile[hang ? 0 : 1][queue[k]];
		if (p->over) p->tiles[k].e0 = drawn0(a, queue[k] % tw, queue[k] / tw) ? a->tile[0][queue[k]] : 0;
	}
	for (int k = 0; k < out->n; ++k)
		if (same(&out->piece[k], p)) { free(p->tiles); p->tiles = NULL; return; }
	/* (MegaMan passes in front of what stands behind a floor: none) */
	if (!hang && !p->behind) priorities(a, A, B, p);
	under(g, A, B, p);
	if (kind == 4) stands_on(a, queue, g, A, B, p);
	++out->n;
}

/* The second layer's side pieces (kind 0), with the first layer's tiles
 * under them (3: sides_learn_under's) or behind a floor (4). */
static void second_learn(const AreaSrc *a, int kind, SideBook *out) {
	if (a->layers < 2 || !a->rings || !a->front) return;
	int tw = a->tw, th = a->th;
	uint8_t *seen = calloc((size_t)tw * th, 1);
	int *queue = malloc((size_t)tw * th * sizeof *queue);
	for (int i = 0; seen && queue && i < tw * th && out->n < SIDES_MAX; ++i) {
		if (seen[i] || !(a->tile[1][i] & 0x3FF)) continue;
		Group g;
		int A, B;
		group_from(a, false, i % tw, i / tw, seen, queue, &g);
		if (kind == 4) {
			if (hangs(a, queue, &g, 4, &A, &B)) behind_each(a, queue, &g, A, B, out);
		} else if (grips(a, queue, &g, kind == 3, &A, &B)) keep(a, queue, &g, A, B, kind, out);
	}
	free(seen);
	free(queue);
}

void sides_learn(const AreaSrc *a, SideBook *out) { second_learn(a, 0, out); }

void sides_learn_under(const AreaSrc *a, SideBook *out) { second_learn(a, 3, out); }

void sides_learn_behind(const AreaSrc *a, SideBook *out) { second_learn(a, 4, out); }

#define BEHIND_DOWN  16   /* what stands behind a floor ends within this above it */
#define BEHIND_TILES 24   /* ... and is no larger: an exit's arrow is */

#define MET_MAX (2 * MAX_W)

/* The floor panels straight above group g's top row within HANG_UP
 * (`below`: straight below its bottom row within BEHIND_DOWN), one for
 * each column that meets one, into P; how many, and in *best the one most
 * columns meet. */
static int met(const AreaSrc *a, const int *queue, const Group *g, bool below, int P[MET_MAX][2], int *best) {
	int row = below ? g->y1 : g->y0, from = below ? row * 8 + 8 : row * 8 - 1, step = below ? 1 : -1, reach = below ? BEHIND_DOWN : HANG_UP;
	int votes[MET_MAX], n = 0;
	*best = 0;
	for (int k = 0; k < g->n; ++k) {
		if (queue[k] / a->tw != row) continue;
		for (int x = queue[k] % a->tw * 8 + 2; x < queue[k] % a->tw * 8 + 8 && n < MET_MAX; x += 4)
			for (int y = from; y >= 0 && y < a->th * 8 && abs(y - from) < reach; y += step) {
				if (off_floor(a, x, y)) continue;
				int j = 0;
				panel_at(a, x, y, &P[n][0], &P[n][1]);
				while (P[j][0] != P[n][0] || P[j][1] != P[n][1]) ++j;
				if (j == n) votes[n++] = 0;
				if (++votes[j] > votes[*best]) *best = j;
				break;
			}
	}
	return n;
}

/* Whether a floor's panel lies straight above group g's top row within
 * HANG_UP (`below`: straight below its bottom row within BEHIND_DOWN), in
 * (*A, *B) the one most of that row's columns meet. */
static bool meets(const AreaSrc *a, const int *queue, const Group *g, bool below, int *A, int *B) {
	int P[MET_MAX][2], best, n = met(a, queue, g, below, P, &best);
	if (!n) return false;
	*A = P[best][0]; *B = P[best][1];
	return true;
}

/* The floor panels piece p (group g) stands behind, straight below its
 * bottom row, from panel (A, B), into its list of panels as ground floor:
 * a bowl at the inner corner of a stepped edge is set where both steps
 * are, not floating off one. */
static void stands_on(const AreaSrc *a, const int *queue, const Group *g, int A, int B, SidePiece *p) {
	int P[MET_MAX][2], best, n = met(a, queue, g, true, P, &best);
	for (int j = 0; j < n && p->nunder < SIDE_UNDER; ++j) {
		int k = 0, dA = P[j][0] - A, dB = P[j][1] - B;
		while (k < p->nunder && (p->under[k][0] != dA || p->under[k][1] != dB)) ++k;
		if (k < p->nunder || abs(dA) > 100 || abs(dB) > 100) continue;
		p->under[p->nunder][0] = (int8_t)dA;
		p->under[p->nunder][1] = (int8_t)dB;
		p->under[p->nunder++][2] = 1;
	}
}

/* Whether group g (kind 1) stands under a floor, over none itself; 2:
 * whether it stands behind one and under none (a pad's knob over a walkway
 * behind it is its pad's); 4: the same of the second layer's, a row of them
 * one group (the Undernet's fire bowls along a walkway). */
static bool hangs(const AreaSrc *a, const int *queue, Group *g, int kind, int *A, int *B) {
	if (g->edge || g->n < MIN_TILES || g->n > (kind == 2 ? BEHIND_TILES : MAX_TILES) || g->x1 - g->x0 >= MAX_W || g->y1 - g->y0 >= MAX_H) return false;
	for (int k = 0; k < g->n; ++k) count_tile(a, kind != 4, queue[k] % a->tw, queue[k] / a->tw, g);
	if (g->on || g->other * 10 > g->off) return false;
	return kind != 1 ? !meets(a, queue, g, false, A, B) && meets(a, queue, g, true, A, B) : meets(a, queue, g, false, A, B);
}

/* Whether the tile (dx, dy) tiles from tile i (an index into the map) is
 * one of group g's, marked in `in` over its box, with the same entry. */
static bool repeats(const AreaSrc *a, const Group *g, const uint8_t *in, int i, int dx, int dy) {
	int x = i % a->tw + dx, y = i / a->tw + dy;
	return x >= g->x0 && x <= g->x1 && y >= g->y0 && y <= g->y1 && in[(y - g->y0) * MAX_W + x - g->x0]
		&& a->tile[1][y * a->tw + x] == a->tile[1][i];
}

/* Group g, standing behind a floor, into the book: whole, or where most of
 * it repeats at one step (a row of the Undernet's fire bowls along a
 * walkway), each whole copy alone with the panel it stands behind, which
 * a layer sets in rows as it sets behind_learn's. */
static void behind_each(const AreaSrc *a, const int *queue, const Group *g, int A, int B, SideBook *out) {
	uint8_t in[MAX_H * MAX_W] = { 0 };
	for (int k = 0; k < g->n; ++k) in[(queue[k] / a->tw - g->y0) * MAX_W + queue[k] % a->tw - g->x0] = 1;
	int best = 0, sx = 0, sy = 0;
	for (int dy = -(g->y1 - g->y0); dy <= g->y1 - g->y0; ++dy)
		for (int dx = dy > 0 ? 0 : 1; dx <= g->x1 - g->x0; ++dx) {
			int same = 0;
			for (int k = 0; k < g->n; ++k) same += repeats(a, g, in, queue[k], dx, dy);
			if (same > best) { best = same; sx = dx; sy = dy; }
		}
	if (best * 2 < g->n) { keep(a, queue, g, A, B, 4, out); return; }
	int *part = malloc((size_t)g->n * sizeof *part), n0 = 0;
	/* (the first copy: its tiles that repeat no tile a step back) */
	for (int k = 0; part && k < g->n; ++k)
		if (!repeats(a, g, in, queue[k], -sx, -sy)) part[n0++] = queue[k];
	for (int c = 0; part && n0 && out->n < SIDES_MAX; ++c) {
		Group s;
		memset(&s, 0, sizeof s);
		s.x0 = s.y0 = 1 << 20;
		s.x1 = s.y1 = -1;
		int copy[MAX_TILES];
		for (int k = 0; k < n0; ++k) {
			int x = part[k] % a->tw + c * sx, y = part[k] / a->tw + c * sy, i = y * a->tw + x;
			if (!repeats(a, g, in, part[k], c * sx, c * sy)) { s.n = 0; break; }
			copy[s.n++] = i;
			if (x < s.x0) s.x0 = x;
			if (x > s.x1) s.x1 = x;
			if (y < s.y0) s.y0 = y;
			if (y > s.y1) s.y1 = y;
		}
		if (s.n < n0) break;
		int PA, PB;
		for (int k = 0; k < s.n; ++k) count_tile(a, false, copy[k] % a->tw, copy[k] / a->tw, &s);
		if (meets(a, copy, &s, true, &PA, &PB)) keep(a, copy, &s, PA, PB, 4, out);
	}
	free(part);
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
		if (hangs(r, queue, &g, kind, &A, &B)) keep(r, queue, &g, A, B, kind, out);
	}
	free(seen);
	free(queue);
}

void hangs_learn(const AreaSrc *r, SideBook *out) { first_learn(r, 1, out); }

void behind_learn(const AreaSrc *a, SideBook *out) { first_learn(a, 2, out); }

#define HOLE_TILES 400   /* a piece in a hole is no larger (End Area's pagoda: 163) */
#define HOLE_SIDE   32   /* tiles a side */
#define HOLE_PX    512   /* pixels over a panel at least for it to be the hole's: half its top */
#define HOLE_MAX     3   /* panels a side */

/* Whether panel (A, B) is floor of the walls' at all. */
static bool walled(const AreaSrc *a, int A, int B) { return area_src_walled_floor(a, a->ex + 16 + 32 * A, a->ey + 16 + 32 * B) == 1; }

/* Whether panel (A, B) lies inside the floor: floor within HOLE_MAX panels
 * of it each way along world X and Y (a roof over the void past a rim does
 * not). */
static bool enclosed(const AreaSrc *a, int A, int B) {
	static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int k = 0; k < 4; ++k) {
		int s = 1;
		while (s <= HOLE_MAX && !walled(a, A + d[k][0] * s, B + d[k][1] * s)) ++s;
		if (s > HOLE_MAX) return false;
	}
	return true;
}

/* Whether group g stands in a walled hole: the panels inside the floor it
 * covers half of or more that are no floor make a block of HOLE_MAX a side
 * at most, ground floor all round it; the block's lowest panel in (*A, *B),
 * its size in hole[]. */
static bool in_hole(const AreaSrc *a, const Group *g, int *A, int *B, uint8_t hole[2]) {
	int a0 = 1 << 20, a1 = -(1 << 20), b0 = a0, b1 = a1, n = 0;
	for (int k = 0; k < g->npanels; ++k) {
		if (g->count[k] < HOLE_PX || walled(a, g->A[k], g->B[k]) || !enclosed(a, g->A[k], g->B[k])) continue;
		if (g->A[k] < a0) a0 = g->A[k];
		if (g->A[k] > a1) a1 = g->A[k];
		if (g->B[k] < b0) b0 = g->B[k];
		if (g->B[k] > b1) b1 = g->B[k];
		++n;
	}
	if (!n || a1 - a0 >= HOLE_MAX || b1 - b0 >= HOLE_MAX || n != (a1 - a0 + 1) * (b1 - b0 + 1)) return false;
	for (int i = a0 - 1; i <= a1 + 1; ++i)
		for (int j = b0 - 1; j <= b1 + 1; ++j) {
			bool inside = i >= a0 && i <= a1 && j >= b0 && j <= b1;
			if (inside ? walled(a, i, j) : !ground(a, i, j)) return false;
		}
	*A = a0; *B = b0;
	hole[0] = (uint8_t)(a1 - a0 + 1);
	hole[1] = (uint8_t)(b1 - b0 + 1);
	return true;
}

void hole_learn(const AreaSrc *a, SidePiece *out) {
	if (out->ntiles || a->layers < 2 || !a->rings || !a->front) return;
	int tw = a->tw, th = a->th;
	uint8_t *seen = calloc((size_t)tw * th, 1);
	int *queue = malloc((size_t)tw * th * sizeof *queue);
	for (int i = 0; seen && queue && i < tw * th && !out->ntiles; ++i) {
		if (seen[i] || !(a->tile[1][i] & 0x3FF)) continue;
		Group g;
		int A, B;
		uint8_t hole[2];
		group_from(a, false, i % tw, i / tw, seen, queue, &g);
		if (g.edge || g.n < MIN_TILES || g.n > HOLE_TILES || g.x1 - g.x0 >= HOLE_SIDE || g.y1 - g.y0 >= HOLE_SIDE) continue;
		for (int k = 0; k < g.n; ++k) count_tile(a, false, queue[k] % tw, queue[k] / tw, &g);
		if (g.npanels >= PANELS || !in_hole(a, &g, &A, &B, hole)) continue;
		SideBook one = { .n = 0 };
		keep(a, queue, &g, A, B, 0, &one);
		if (!one.n) continue;
		*out = one.piece[0];
		out->hole[0] = hole[0];
		out->hole[1] = hole[1];
		/* (the priorities over every panel it draws over and one round
		 * them: MegaMan passes behind it there) */
		int a0 = 0, a1 = hole[0] - 1, b0 = 0, b1 = hole[1] - 1;
		for (int k = 0; k < out->nunder; ++k) {
			if (out->under[k][0] < a0) a0 = out->under[k][0];
			if (out->under[k][0] > a1) a1 = out->under[k][0];
			if (out->under[k][1] < b0) b0 = out->under[k][1];
			if (out->under[k][1] > b1) b1 = out->under[k][1];
		}
		free(out->prio);
		int box[4] = { a0 - 1, a1 + 1, b0 - 1, b1 + 1 };
		priorities_over(a, A, B, box, out);
	}
	free(seen);
	free(queue);
}
