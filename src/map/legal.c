/* A layer's floor made drawable. Each panel not drawn exactly tries the
 * cells around it: a floor cell taken away or a void one filled, whichever
 * leaves the fewest such panels near it, as long as the cell is "simple":
 * toggling it neither joins nor splits any floor or void, so routes, rooms,
 * holes and the gaps between walkways stay as the layout made them. */
#include "legal.h"

#include <stdlib.h>
#include <string.h>

#define REACH 2      /* a cell's toggle changes panels this far (floors' materials look a cell further) */
#define MARGIN 2     /* cells kept off the grid's edge */
#define PAD (2 * REACH + 2)   /* the panels asked of lie this far outside the grid at most (a block's from a panel one outside it) */

/* A fix under way. What is asked of the grid as it stands is asked once
 * between two edits, not again for every candidate around a panel: the
 * clean callback reads the cells alone (legal.h), so only an edit changes
 * its answers. Making a run's first layer spent most of its time asking
 * the same panels again (callgrind: docs/devtools/profiling.md). */
typedef struct {
	const LegalGrid *g;
	int8_t *memo;        /* unclean() of each panel as the grid stands, -1 not asked yet */
	int mw, mh;
	int floors, voids;   /* topology() of the grid as it stands, floors -1 not counted yet */
	uint8_t *topo;       /* topology()'s room: the grid in a ring of void (the outside) in a ring of walls */
	int *queue;
	uint8_t *stuck;      /* panels nothing helps, until an edit near them */
} Fix;

static bool floor_at(const LegalGrid *g, int x, int y) {
	return x >= 0 && y >= 0 && x < g->gw && y < g->gh && g->cell[y * g->gw + x];
}

/* A panel with floor in or around it: one the tiles draw. */
static bool drawn(const LegalGrid *g, int x, int y) {
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
			if (floor_at(g, x + dx, y + dy)) return true;
	return false;
}

static bool unclean(const LegalGrid *g, int x, int y) {
	return drawn(g, x, y) && !g->clean(x, y, g->ctx);
}

/* The grid edited: what was known of it is asked again. */
static void edited(Fix *F) {
	memset(F->memo, 0xFF, (size_t)F->mw * F->mh);
	F->floors = -1;
}

/* unclean() of the grid as it stands, asked once between edits. */
static bool unclean_now(Fix *F, int x, int y) {
	int8_t *m = &F->memo[(y + PAD) * F->mw + x + PAD];
	if (*m < 0) *m = unclean(F->g, x, y);
	return *m;
}

/* Panels from (x0, y0) to (x1, y1) not drawn exactly: as the grid stands
 * (`now`), or as a candidate edit has it for the moment. */
static int unclean_in(Fix *F, int x0, int y0, int x1, int y1, bool now) {
	int n = 0;
	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x) n += now ? unclean_now(F, x, y) : unclean(F->g, x, y);
	return n;
}

/* Panels near (x, y) not drawn exactly. */
static int unclean_near(Fix *F, int x, int y, bool now) {
	return unclean_in(F, x - REACH, y - REACH, x + REACH, y + REACH, now);
}

/* The floor cells beside (x, y) (bit: N, E, S, W). */
static unsigned sides(const LegalGrid *g, int x, int y) {
	return floor_at(g, x, y - 1) | floor_at(g, x + 1, y) << 1 | floor_at(g, x, y + 1) << 2 | floor_at(g, x - 1, y) << 3;
}

/* A floor cell where a path branches or turns (not a straight run, not an
 * end). */
static bool bend(const LegalGrid *g, int x, int y) {
	if (!floor_at(g, x, y)) return false;
	unsigned s = sides(g, x, y);
	int n = __builtin_popcount(s);
	return n >= 3 || (n == 2 && s != 0x5 && s != 0xA);
}

/* Pieces of the cells around (x, y) (not through it) that are floor (or
 * void, `want`), joined side by side (or also corner to corner with
 * `corners`), counting only those that touch (x, y) by a side unless
 * `corners`. */
static int pieces(const bool f[3][3], bool want, bool corners) {
	bool seen[3][3] = { { false } };
	int n = 0;
	for (int sy = 0; sy < 3; ++sy)
		for (int sx = 0; sx < 3; ++sx) {
			if ((sx == 1 && sy == 1) || f[sy][sx] != want || seen[sy][sx]) continue;
			int q[8][2], h = 0, t = 0;
			bool touches = false;
			q[t][0] = sx; q[t][1] = sy; ++t;
			seen[sy][sx] = true;
			while (h < t) {
				int cx = q[h][0], cy = q[h][1];
				++h;
				touches |= corners || cx == 1 || cy == 1;
				for (int dy = -1; dy <= 1; ++dy)
					for (int dx = -1; dx <= 1; ++dx) {
						if (!(dx || dy) || (!corners && dx && dy)) continue;
						int nx = cx + dx, ny = cy + dy;
						if (nx < 0 || ny < 0 || nx > 2 || ny > 2 || (nx == 1 && ny == 1)) continue;
						if (f[ny][nx] != want || seen[ny][nx]) continue;
						seen[ny][nx] = true;
						q[t][0] = nx; q[t][1] = ny; ++t;
					}
			}
			n += touches;
		}
	return n;
}

/* Whether toggling (x, y) keeps the floor's and the void's pieces as they
 * are: around it, one piece of floor (joined side by side) touching it, and
 * one of void (joined corner to corner too). */
static bool simple(const LegalGrid *g, int x, int y) {
	bool f[3][3];
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) f[dy + 1][dx + 1] = floor_at(g, x + dx, y + dy);
	return pieces(f, true, false) == 1 && pieces(f, false, true) == 1;
}

/* Pieces of floor (joined side by side) and of void (also corner to
 * corner, the outside counted as void) in the whole grid: an edit that keeps
 * both joins, splits and encloses nothing. The grid is copied into the
 * fix's room inside a ring of void and a ring of walls, which match
 * neither, so the search needs no bounds. */
enum { T_VOID, T_FLOOR, T_WALL, T_SEEN = 4 };

static void topology(Fix *F, int *floors, int *voids) {
	const LegalGrid *g = F->g;
	int W = g->gw + 4, H = g->gh + 4;
	uint8_t *t = F->topo;
	for (int y = 0; y < H; ++y)
		for (int x = 0; x < W; ++x)
			t[y * W + x] = x == 0 || y == 0 || x == W - 1 || y == H - 1 ? T_WALL : floor_at(g, x - 2, y - 2) ? T_FLOOR : T_VOID;
	const int side[4] = { -1, 1, -W, W }, around[8] = { -W - 1, -W, -W + 1, -1, 1, W - 1, W, W + 1 };
	*floors = *voids = 0;
	for (int start = W + 1; start < W * (H - 1) - 1; ++start) {
		uint8_t f = t[start];
		if (f != T_FLOOR && f != T_VOID) continue;
		++*(f == T_FLOOR ? floors : voids);
		const int *step = f == T_FLOOR ? side : around, steps = f == T_FLOOR ? 4 : 8;
		int h = 0, n = 0;
		F->queue[n++] = start;
		t[start] |= T_SEEN;
		while (h < n) {
			int c = F->queue[h++];
			for (int k = 0; k < steps; ++k) {
				int d = c + step[k];
				if (t[d] != f) continue;
				t[d] |= T_SEEN;
				F->queue[n++] = d;
			}
		}
	}
}

/* Fills the void cells of the 2 x 2 block from (x, y) where a path in it
 * bends; how many it filled, -1 if it has no bend or a cell is locked or off
 * limits. */
static int block(const LegalGrid *g, int x, int y, uint8_t filled[4]) {
	int n = 0;
	/* only where a path bends: a straight one keeps its width */
	bool bends = false;
	for (int k = 0; k < 4; ++k) bends |= bend(g, x + k % 2, y + k / 2);
	if (!bends) return -1;
	for (int k = 0; k < 4; ++k) {
		int cx = x + k % 2, cy = y + k / 2;
		if (cx < MARGIN || cy < MARGIN || cx >= g->gw - MARGIN || cy >= g->gh - MARGIN) return -1;
		if (g->locked[cy * g->gw + cx] && !g->cell[cy * g->gw + cx]) return -1;
	}
	for (int k = 0; k < 4; ++k) {
		uint8_t *c = &g->cell[(y + k / 2) * g->gw + x + k % 2];
		filled[k] = !*c;
		if (!*c) *c = 1, ++n;
	}
	return n;
}

static void unblock(const LegalGrid *g, int x, int y, const uint8_t filled[4]) {
	for (int k = 0; k < 4; ++k)
		if (filled[k]) g->cell[(y + k / 2) * g->gw + x + k % 2] = 0;
}

/* Panels near the 2 x 2 block from (x, y) not drawn exactly. */
static int unclean_near_block(Fix *F, int x, int y, bool now) {
	return unclean_in(F, x - REACH, y - REACH, x + REACH + 1, y + REACH + 1, now);
}

/* The cell near panel (px, py) whose toggle leaves the fewest panels near
 * it not drawn exactly: their change (below 0 where it helps) and its place
 * in *bx, *by (-1: none helps). */
static int best_toggle(Fix *F, int px, int py, int *bx, int *by) {
	const LegalGrid *g = F->g;
	int best = 0;
	*bx = *by = -1;
	for (int dy = -REACH; dy <= REACH; ++dy)
		for (int dx = -REACH; dx <= REACH; ++dx) {
			int x = px + dx, y = py + dy;
			if (x < MARGIN || y < MARGIN || x >= g->gw - MARGIN || y >= g->gh - MARGIN) continue;
			uint8_t *c = &g->cell[y * g->gw + x];
			if (g->locked[y * g->gw + x] || !simple(g, x, y)) continue;
			/* floor is only added into a notch: never a bump on a
			 * path's side or its end drawn out */
			if (!*c && __builtin_popcount(sides(g, x, y)) < 2) continue;
			int before = unclean_near(F, x, y, true);
			uint8_t was = *c;
			*c = !was;
			int delta = unclean_near(F, x, y, false) - before;
			*c = was;
			if (delta < best) { best = delta; *bx = x; *by = y; }
		}
	return best;
}

/* Whether the block from (x, y), filled, leaves the grid's topology as it
 * stands; the block is emptied again. */
static bool keeps_topology(Fix *F, int x, int y, const uint8_t filled[4]) {
	int f1, v1;
	topology(F, &f1, &v1);
	unblock(F->g, x, y, filled);
	if (F->floors < 0) topology(F, &F->floors, &F->voids);
	return f1 == F->floors && v1 == F->voids;
}

/* Or a 2 x 2 block filled near panel (px, py), a walkways' junction made a
 * platform as the originals join them: one that does better than `best`
 * and keeps the topology, its corner in *kx, *ky (-1: none). */
static void best_block(Fix *F, int px, int py, int best, int *kx, int *ky) {
	const LegalGrid *g = F->g;
	*kx = *ky = -1;
	for (int dy = -REACH; dy < REACH; ++dy)
		for (int dx = -REACH; dx < REACH; ++dx) {
			int x = px + dx, y = py + dy;
			uint8_t filled[4];
			int before = unclean_near_block(F, x, y, true);
			int n = block(g, x, y, filled);
			if (n <= 0) { if (n == 0) unblock(g, x, y, filled); continue; }
			int delta = unclean_near_block(F, x, y, false) - before;
			if (delta >= best) { unblock(g, x, y, filled); continue; }
			if (keeps_topology(F, x, y, filled)) { best = delta; *kx = x; *ky = y; }
		}
}

#define STUCK(F, x, y) (F)->stuck[((y) + 1) * ((F)->g->gw + 2) + (x) + 1]

/* The edit that helps panel (px, py) most, made: whether there was one. */
static bool fix_panel(Fix *F, int px, int py, LegalStats *st) {
	const LegalGrid *g = F->g;
	int bx, by, kx, ky;
	best_block(F, px, py, best_toggle(F, px, py, &bx, &by), &kx, &ky);
	if (kx >= 0) {
		uint8_t filled[4];
		st->edits += block(g, kx, ky, filled);
		bx = kx; by = ky;
	} else if (bx >= 0) {
		uint8_t *c = &g->cell[by * g->gw + bx];
		*c = !*c;
		++st->edits;
	}
	if (bx < 0) { STUCK(F, px, py) = 1; return false; }
	edited(F);
	/* the panels near it may be worth another try */
	for (int dy = -REACH - 1; dy <= REACH + 1; ++dy)
		for (int dx = -REACH - 1; dx <= REACH + 1; ++dx) {
			int x = bx + dx, y = by + dy;
			if (x >= -1 && y >= -1 && x <= g->gw && y <= g->gh) STUCK(F, x, y) = 0;
		}
	return true;
}

/* One pass over the panels: whether it edited any. */
static bool fix_pass(Fix *F, int budget, LegalStats *st) {
	bool changed = false;
	for (int py = -1; py <= F->g->gh && st->edits < budget; ++py)
		for (int px = -1; px <= F->g->gw && st->edits < budget; ++px)
			if (!STUCK(F, px, py) && unclean_now(F, px, py)) changed |= fix_panel(F, px, py, st);
	return changed;
}

LegalStats legal_fix(const LegalGrid *g, int budget) {
	LegalStats st = { 0, 0 };
	int mw = g->gw + 2 * PAD, mh = g->gh + 2 * PAD;
	size_t room = (size_t)(g->gw + 4) * (size_t)(g->gh + 4);
	Fix F = { g, malloc((size_t)mw * mh), mw, mh, -1, 0, malloc(room), malloc(room * sizeof(int)),
		calloc((size_t)(g->gw + 2) * (size_t)(g->gh + 2), 1) };
	if (F.memo && F.topo && F.queue && F.stuck) {
		edited(&F);
		while (st.edits < budget && fix_pass(&F, budget, &st)) {}
		for (int py = -1; py <= g->gh; ++py)
			for (int px = -1; px <= g->gw; ++px) st.left += unclean_now(&F, px, py);
	}
	free(F.memo);
	free(F.topo);
	free(F.queue);
	free(F.stuck);
	return st;
}
