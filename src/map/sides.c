/* Side pieces (sides.h). A map's second tile layer holds what it sets in
 * front of its floor. A group of its tiles joined 8 ways is a side piece
 * where it grips ground floor and hangs off it: some of what it draws lies
 * on a ground panel's top, a third or more off every ground panel's. What
 * lies on the floor alone is a pad's centrepiece or an emblem, set by rules
 * of their own; what touches no floor is scenery (decor.c). */
#include "sides.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "tiles.h"

#define MIN_TILES    4
#define MAX_TILES  160
#define MAX_W       20   /* tiles */
#define MAX_H       14
#define ON_FLOOR     8   /* pixels on its panel's top at least: it grips it */
#define OFF_SHARE    3   /* tenths of its pixels off the ground floor at least */
#define FLOOR_DV     4   /* a floor's top is drawn this far below its panel's edges */
#define PANELS      32   /* panels a piece may lie over, counted apart */
#define UNDER_PX     6   /* pixels over a panel at least for the panel to count as under it */
#define LOOKS       24   /* looks of a floor's sides, under or behind it, a book takes at most */

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
 * (`hang`) the first's under a floor, or (`grow`) one it marks. */
static bool member(const AreaSrc *a, bool hang, const uint8_t *grow, int x, int y) {
	if (grow) return grow[y * a->tw + x] & 1;
	return hang ? hang_tile(a, x, y) : (a->tile[1][y * a->tw + x] & 0x3FF) != 0;
}

/* The group of such tiles joined to tile `start` (y * tw + x), its tiles'
 * indexes in `queue`; `seen` marks them. */
static void group_from(const AreaSrc *a, bool hang, const uint8_t *grow, int start, uint8_t *seen, int *queue, Group *g) {
	int tw = a->tw, th = a->th, sx = start % tw, sy = start / tw;
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
				if (nx < 0 || ny < 0 || nx >= tw || ny >= th || seen[ny * tw + nx] || !member(a, hang, grow, nx, ny)) continue;
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

/* A new piece of up to n tiles for panel (A, B) of map a, at the book's end
 * (grown as looks come), its neighbourhood set; NULL where the book is full
 * or memory runs out. */
static SidePiece *begin(const AreaSrc *a, int n, int A, int B, SideBook *out) {
	if (out->n >= SIDES_MAX) return NULL;
	if (out->n == out->cap) {
		int cap = out->cap ? 2 * out->cap : 8;
		SidePiece *more = realloc(out->piece, (size_t)cap * sizeof *more);
		if (!more) return NULL;
		out->piece = more;
		out->cap = cap;
	}
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
 * look; whether it went in. */
static bool finish(const AreaSrc *a, const Group *g, int A, int B, SidePiece *p, SideBook *out) {
	for (int k = 0; k < out->n; ++k)
		if (same(&out->piece[k], p)) { free(p->tiles); p->tiles = NULL; return false; }
	/* (MegaMan passes in front of what stands behind a floor: none) */
	if (!p->hang && !p->behind) priorities(a, A, B, p);
	under(g, A, B, p);
	++out->n;
	return true;
}

/* Group g, on panel (A, B), into the book unless it holds its look; kind 0
 * a side piece, 1 under a floor, 2 behind one, 3 a side piece with the
 * first layer's tiles under it (SidePiece.over), 4 behind a floor on the
 * second layer, with them too. */
static void keep(const AreaSrc *a, const int *queue, const Group *g, int A, int B, int kind, SideBook *out) {
	bool hang = kind == 1 || kind == 2 || kind == 5;
	int tw = a->tw;
	SidePiece *p = begin(a, g->n, A, B, out);
	if (!p) return;
	p->hang = hang;
	p->behind = kind == 2 || kind == 4;
	p->over = kind == 3 || kind == 4;
	p->grows = kind == 5;
	for (int k = 0; k < g->n; ++k) {
		p->tiles[k] = placed(a, queue[k], A, B);
		*(hang ? &p->tiles[k].e0 : &p->tiles[k].e1) = a->tile[hang ? 0 : 1][queue[k]];
		if (p->over) p->tiles[k].e0 = drawn0(a, queue[k] % tw, queue[k] / tw) ? a->tile[0][queue[k]] : 0;
	}
	if (finish(a, g, A, B, p, out) && kind == 4) stands_on(a, queue, g, A, B, p);
}

/* The second layer's side pieces (kind 0), with the first layer's tiles
 * under them (3: sides_learn_under's) or behind a floor (4). */
static void second_learn(const AreaSrc *a, int kind, SideBook *out) {
	if (a->layers < 2 || !a->rings || !a->front) return;
	int tw = a->tw, th = a->th;
	uint8_t *seen = calloc((size_t)tw * th, 1);
	int *queue = malloc((size_t)tw * th * sizeof *queue);
	for (int i = 0; seen && queue && i < tw * th && out->n < LOOKS; ++i) {
		if (seen[i] || !(a->tile[1][i] & 0x3FF)) continue;
		Group g;
		int A, B;
		group_from(a, false, NULL, i, seen, queue, &g);
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
	for (int i = 0; seen && queue && i < tw * th && out->n < LOOKS; ++i) {
		if (seen[i] || !hang_tile(r, i % tw, i / tw)) continue;
		Group g;
		int A, B;
		group_from(r, true, NULL, i, seen, queue, &g);
		if (hangs(r, queue, &g, kind, &A, &B)) keep(r, queue, &g, A, B, kind, out);
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
			t->foot = e1 && (a->tile[0][src] & 0x3FF) && !off;
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

/* ---- what grows on a floor's edges ----
 *
 * The Judge Tree's walkways carry tufts of grass on their back rims and
 * planters stacked at their walls' feet, on the first layer in the very
 * tiles that draw the rim and the wall: the floor's own art with
 * something more. A tile grows on the floor where it draws some pixels
 * past the floor's body (its top, its rim, its walls), and tiles that do
 * joined 8 ways are one piece; the ones among them that draw the floor's
 * art too are set over a layer's own, the others over nothing. */

#define RIM_UP      3   /* a floor's back rim's outline stands this far over its top at most (2 on the Judge Tree's) */
#define PAST_PX     4   /* pixels a tile draws past the floor's body at least to grow on it */
#define BODY_PX     8   /* ... and of the floor's art to draw that too (a planter's corner in a wall's room is not) */
#define EDGE_TILES 24   /* ... and what grows is no larger (the Judge Tree's stumps, its tree's court are), */
#define EDGE_WHOLE 48   /* ... with the ring of the floor's tiles round it no larger than this */

/* Which of the map's pixels lie on a ground panel's top, a bit each. */
static uint8_t *tops_of(const AreaSrc *a) {
	int W = a->tw * 8, H = a->th * 8;
	uint8_t *t = calloc(((size_t)W * H + 7) / 8, 1);
	for (int y = 0; t && y < H; ++y) {
		int lastA = 0, lastB = 0;
		bool on = false, known = false;
		for (int x = 0; x < W; ++x) {
			int A, B;
			panel_at(a, x, y, &A, &B);
			if (!known || A != lastA || B != lastB) { on = ground(a, A, B); lastA = A; lastB = B; known = true; }
			if (on) t[((size_t)y * W + x) >> 3] |= (uint8_t)(1u << (x & 7));
		}
	}
	return t;
}

static bool top_at(const AreaSrc *a, const uint8_t *t, int x, int y) {
	int W = a->tw * 8;
	return y >= 0 && y < a->th * 8 && t[((size_t)y * W + x) >> 3] >> (x & 7) & 1;
}

/* What first-layer pixel (x, y) is to the floor: 0 past its body, 1 its
 * top, 2 its rim (its top just under it), 3 its wall (its top within the
 * wall's height over it). */
static int body_at(const AreaSrc *a, const uint8_t *t, int face, int x, int y) {
	if (top_at(a, t, x, y)) return 1;
	for (int d = 1; d <= RIM_UP; ++d)
		if (top_at(a, t, x, y + d)) return 2;
	for (int d = 1; d <= face; ++d)
		if (top_at(a, t, x, y - d)) return 3;
	return 0;
}

/* Per tile: bit 0 it grows on the floor, bit 1 it draws the floor's art
 * too (BODY_PX of its pixels); and over the map, how many of those pixels are a rim's and a
 * wall's, in rim[] and wall[] by tile. */
static uint8_t *grow_marks(const AreaSrc *a, int face, uint16_t *rim, uint16_t *wall) {
	int tw = a->tw, th = a->th, W = tw * 8;
	uint8_t *t = tops_of(a), *m = calloc((size_t)tw * th, 1);
	for (int i = 0; t && m && i < tw * th; ++i) {
		if (!(a->tile[0][i] & 0x3FF)) continue;
		int past = 0, body = 0;
		for (int y = i / tw * 8; y < i / tw * 8 + 8; ++y)
			for (int x = i % tw * 8; x < i % tw * 8 + 8; ++x) {
				if (!a->idx0[(size_t)y * W + x]) continue;
				int b = body_at(a, t, face, x, y);
				++*(b ? &body : &past);
				if (b == 2) ++rim[i];
				if (b == 3) ++wall[i];
			}
		m[i] = (uint8_t)((past >= PAST_PX) | (body >= BODY_PX) << 1);
	}
	free(t);
	return m;
}

/* Group g, its tiles in `queue`, with the tiles round it that draw the
 * floor's art (`mark` bit 1) added; whether it is still no larger than
 * EDGE_WHOLE, off the map's edge. */
static bool ring(const AreaSrc *a, const uint8_t *mark, int *queue, Group *g) {
	int tw = a->tw, th = a->th, n = g->n;
	for (int k = 0; k < n; ++k)
		for (int d = 0; d < 9; ++d) {
			int x = queue[k] % tw + d % 3 - 1, y = queue[k] / tw + d / 3 - 1;
			if (x < 0 || y < 0 || x >= tw || y >= th || !(mark[y * tw + x] & 2) || mark[y * tw + x] & 1) continue;
			int j = n;
			while (j < g->n && queue[j] != y * tw + x) ++j;
			if (j < g->n) continue;
			if (g->n == EDGE_WHOLE) return false;
			queue[g->n++] = y * tw + x;
			if (x < g->x0) g->x0 = x;
			if (x > g->x1) g->x1 = x;
			if (y < g->y0) g->y0 = y;
			if (y > g->y1) g->y1 = y;
			g->edge |= x == 0 || y == 0 || x == tw - 1 || y == th - 1;
		}
	return !g->edge && g->x1 - g->x0 < MAX_W && g->y1 - g->y0 < MAX_H;
}

/* Into p->under, every panel of map a whose top, rim or wall (`face`
 * pixels tall) could draw in the tiles of group g, from its panel (A, B),
 * as ground floor or none: a layer sets it where each is the same, so its
 * tiles go over no other floor's art (a wall of a walkway further back
 * reaching down to a tuft) and none is missing under them. Whether they
 * fit in SIDE_REACH. */
static bool reach(const AreaSrc *a, int face, const int *queue, const Group *g, int A, int B, SidePiece *p) {
	p->nunder = 0;
	for (int k = 0; k < g->n; ++k)
		for (int y = queue[k] / a->tw * 8 - face; y < queue[k] / a->tw * 8 + 8 + RIM_UP; ++y)   /* (the walls of panels over it, the rims of those under it) */
			for (int x = queue[k] % a->tw * 8; x < queue[k] % a->tw * 8 + 8; x += 2) {
				int PA, PB, j = 0;
				panel_at(a, x, y, &PA, &PB);
				while (j < p->nunder && (p->under[j][0] != PA - A || p->under[j][1] != PB - B)) ++j;
				if (j < p->nunder) continue;
				if (j == SIDE_REACH || abs(PA - A) > 100 || abs(PB - B) > 100) return false;
				p->under[j][0] = (int8_t)(PA - A);
				p->under[j][1] = (int8_t)(PB - B);
				p->under[j][2] = ground(a, PA, PB);
				++p->nunder;
			}
	return true;
}

/* Whether panel (A, B) is no floor, or the area's own (`panels`). */
static bool own_or_none(const AreaSrc *a, const uint8_t *panels, int A, int B) {
	if (!ground(a, A, B)) return true;
	if (A < -TILES_SRC_SPAN / 2 || B < -TILES_SRC_SPAN / 2 || A >= TILES_SRC_SPAN / 2 || B >= TILES_SRC_SPAN / 2) return false;
	int v = panels[(B + TILES_SRC_SPAN / 2) * TILES_SRC_SPAN + A + TILES_SRC_SPAN / 2];
	return !(v & TILE_PAD) && ((v & 3) == TILE_A || (v & 3) == TILE_B);
}

/* Whether piece p, on panel (A, B), stands among the area's own floor alone. */
static bool among_own(const AreaSrc *a, const uint8_t *panels, const SidePiece *p, int A, int B) {
	for (int k = 0; k < 9; ++k)
		if (!own_or_none(a, panels, A + k % 3 - 1, B + k / 3 - 1)) return false;
	for (int k = 0; k < p->nunder; ++k)
		if (!own_or_none(a, panels, A + p->under[k][0], B + p->under[k][1])) return false;
	return true;
}

/* What edges_learn works with over one map. */
typedef struct {
	const AreaSrc *a;
	int face;
	const uint8_t *panels;   /* tiles_src_panels's */
	uint8_t *mark;           /* grow_marks's, and its rim's and wall's pixels by tile */
	uint16_t *rim, *wall;
	uint8_t *seen;
	int *queue;
} Edges;

/* The group of tiles that grow on the floor from tile `start`, into the
 * book where it is a piece. */
static void edge_group(Edges *e, int start, SideBook *out) {
	const AreaSrc *a = e->a;
	int *queue = e->queue, A, B, nrim = 0, nwall = 0;
	Group g;
	group_from(a, true, e->mark, start, e->seen, queue, &g);
	if (g.edge || g.n > EDGE_TILES || g.x1 - g.x0 >= MAX_W || g.y1 - g.y0 >= MAX_H) return;
	for (int k = 0; k < g.n; ++k) {
		nrim += e->rim[queue[k]];
		nwall += e->wall[queue[k]];
	}
	/* (cut whole: with the tiles round it that draw the floor's art,
	 * where the rest of it lies, the mound under a tuft's blades, a
	 * planter's top over the wall's foot; a tile the same as the
	 * layer's own there is set over it as it is) */
	if (!ring(a, e->mark, queue, &g)) return;
	for (int k = 0; k < g.n; ++k) count_tile(a, true, queue[k] % a->tw, queue[k] / a->tw, &g);
	/* (a tuft stands on a rim, the floor straight under it; a planter
	 * at a wall's foot, the floor straight over it) */
	if (g.other * 10 > g.on + g.off || !(nrim || nwall) || !meets(a, queue, &g, nrim >= nwall, &A, &B)) return;
	if (area_src_arrow_at(a, a->ex + 16 + 32 * A, a->ey + 16 + 32 * B) || !ground(a, A, B)) return;
	int n = out->n;
	keep(a, queue, &g, A, B, 5, out);
	if (out->n == n) return;
	SidePiece *p = &out->piece[n];
	if (!reach(a, e->face, queue, &g, A, B, p) || !among_own(a, e->panels, p, A, B)) {
		free(p->tiles);
		free(p->prio);
		--out->n;
		return;
	}
	for (int k = 0; k < g.n; ++k)
		if (e->mark[queue[k]] & 2) p->onto |= (uint64_t)1 << k;
	if (emu_debug_on()) fprintf(stderr, "edges map %x:%d: %s at panel %d,%d, %d tiles, %d panels reached\n", a->group, a->number,
		nrim >= nwall ? "on a rim" : "at a wall's foot", A, B, g.n, p->nunder);
}

void edges_learn(const AreaSrc *a, int face, const uint8_t *panels, SideBook *out) {
	if (!a->rings || !a->idx0 || !panels) return;
	size_t n = (size_t)a->tw * a->th;
	Edges e = { a, face, panels, NULL, calloc(n, sizeof *e.rim), calloc(n, sizeof *e.wall), calloc(n, 1), malloc(n * sizeof *e.queue) };
	if (e.rim && e.wall) e.mark = grow_marks(a, face, e.rim, e.wall);
	for (size_t i = 0; e.mark && e.seen && e.queue && i < n && out->n < SIDES_MAX; ++i)
		if (!e.seen[i] && e.mark[i] & 1) edge_group(&e, (int)i, out);
	free(e.mark);
	free(e.seen);
	free(e.queue);
	free(e.rim);
	free(e.wall);
}

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

/* The piece book `one` took, into out, the book's array freed (the piece's
 * tiles and priorities are out's now); whether it took one. */
static bool book_one(SideBook *one, SidePiece *out) {
	bool got = one->n > 0;
	if (got) *out = one->piece[0];
	free(one->piece);
	return got;
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
		group_from(a, false, NULL, i, seen, queue, &g);
		if (g.edge || g.n < MIN_TILES || g.n > HOLE_TILES || g.x1 - g.x0 >= HOLE_SIDE || g.y1 - g.y0 >= HOLE_SIDE) continue;
		for (int k = 0; k < g.n; ++k) count_tile(a, false, queue[k] % tw, queue[k] / tw, &g);
		if (g.npanels >= PANELS || !in_hole(a, &g, &A, &B, hole)) continue;
		SideBook one = { .n = 0 };
		keep(a, queue, &g, A, B, 0, &one);
		if (!book_one(&one, out)) continue;
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
