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

static int fdiv(int a, int b) { return a / b - (a % b < 0); }

/* Whether panel (A, B) is floor on the ground. */
static bool ground(const AreaSrc *a, int A, int B) {
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	return area_src_walled_floor(a, X, Y) == 1 && area_src_height(a, X, Y) == 0;
}

/* The panel whose ground top holds map pixel (x, y). */
static void panel_at(const AreaSrc *a, int x, int y, int *A, int *B) {
	int u = x - a->tw * 4, v = 2 * (y - FLOOR_DV - a->th * 4);
	*A = fdiv(fdiv(u - v, 2) - a->ex, 32);
	*B = fdiv(fdiv(u + v, 2) - a->ey, 32);
}

/* A piece while it is measured: its tiles, and the pixels it draws by the
 * ground panels they lie on. */
typedef struct {
	int n, x0, y0, x1, y1;
	int on, off, other;
	int A[PANELS], B[PANELS], count[PANELS], npanels;
	bool gnd[PANELS], edge;
} Group;

static void count_tile(const AreaSrc *a, int tx, int ty, Group *g) {
	int W = a->tw * 8;
	for (int y = ty * 8; y < ty * 8 + 8; ++y)
		for (int x = tx * 8; x < tx * 8 + 8; ++x) {
			if (!a->front[(size_t)y * W + x]) continue;
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

/* The group of second-layer tiles joined to (sx, sy), its tiles' indexes in
 * `queue`; `seen` marks them. */
static void group_from(const AreaSrc *a, int sx, int sy, uint8_t *seen, int *queue, Group *g) {
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
				if (nx < 0 || ny < 0 || nx >= tw || ny >= th || seen[ny * tw + nx] || !(a->tile[1][ny * tw + nx] & 0x3FF)) continue;
				seen[ny * tw + nx] = 1;
				queue[g->n++] = ny * tw + nx;
			}
	}
}

/* Whether the group is a side piece; its panel in (*A, *B). */
static bool grips(const AreaSrc *a, const int *queue, Group *g, int *A, int *B) {
	if (g->edge || g->n < MIN_TILES || g->n > MAX_TILES || g->x1 - g->x0 >= MAX_W || g->y1 - g->y0 >= MAX_H) return false;
	for (int k = 0; k < g->n; ++k) count_tile(a, queue[k] % a->tw, queue[k] / a->tw, g);
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
		if (p->tiles[k].px != q->tiles[k].px || p->tiles[k].py != q->tiles[k].py || p->tiles[k].e1 != q->tiles[k].e1) return false;
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

void sides_learn(const AreaSrc *a, SideBook *out) {
	if (a->layers < 2 || !a->rings || !a->front) return;
	int tw = a->tw, th = a->th;
	uint8_t *seen = calloc((size_t)tw * th, 1);
	int *queue = malloc((size_t)tw * th * sizeof *queue);
	for (int i = 0; seen && queue && i < tw * th && out->n < SIDES_MAX; ++i) {
		if (seen[i] || !(a->tile[1][i] & 0x3FF)) continue;
		Group g;
		int A, B;
		group_from(a, i % tw, i / tw, seen, queue, &g);
		if (!grips(a, queue, &g, &A, &B)) continue;
		SidePiece *p = &out->piece[out->n];
		p->tiles = calloc((size_t)g.n, sizeof *p->tiles);
		if (!p->tiles) break;
		p->ntiles = g.n;
		p->floor = 0;
		for (int k = 0; k < 9; ++k)
			if (ground(a, A + k % 3 - 1, B + k / 3 - 1)) p->floor |= (uint16_t)(1u << k);
		int ax = area_px(tw, a->ex + 32 * A, a->ey + 32 * B), ay = area_py(th, a->ex + 32 * A, a->ey + 32 * B);
		for (int k = 0; k < g.n; ++k)
			p->tiles[k] = (StairTile){ .px = (int16_t)(queue[k] % tw * 8 - ax), .py = (int16_t)(queue[k] / tw * 8 - ay), .e1 = a->tile[1][queue[k]] };
		bool dup = false;
		for (int k = 0; k < out->n && !dup; ++k) dup = same(&out->piece[k], p);
		if (dup) { free(p->tiles); continue; }
		priorities(a, A, B, p);
		under(&g, A, B, p);
		++out->n;
	}
	free(seen);
	free(queue);
}
