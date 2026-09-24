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

/* Panels near (x, y) not drawn exactly. */
static int unclean_near(const LegalGrid *g, int x, int y) {
	int n = 0;
	for (int dy = -REACH; dy <= REACH; ++dy)
		for (int dx = -REACH; dx <= REACH; ++dx) n += unclean(g, x + dx, y + dy);
	return n;
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
 * both joins, splits and encloses nothing. */
static void topology(const LegalGrid *g, int *floors, int *voids) {
	int W = g->gw + 2, H = g->gh + 2;
	uint8_t *seen = calloc((size_t)W * H, 1);
	int *q = malloc((size_t)W * H * sizeof *q);
	*floors = *voids = 0;
	for (int start = 0; start < W * H; ++start) {
		if (seen[start]) continue;
		bool f = floor_at(g, start % W - 1, start / W - 1);
		++*(f ? floors : voids);
		int h = 0, t = 0;
		q[t++] = start;
		seen[start] = 1;
		while (h < t) {
			int c = q[h++], cx = c % W, cy = c / W;
			for (int dy = -1; dy <= 1; ++dy)
				for (int dx = -1; dx <= 1; ++dx) {
					if (!(dx || dy) || (f && dx && dy)) continue;
					int nx = cx + dx, ny = cy + dy;
					if (nx < 0 || ny < 0 || nx >= W || ny >= H) continue;
					int n = ny * W + nx;
					if (seen[n] || floor_at(g, nx - 1, ny - 1) != f) continue;
					seen[n] = 1;
					q[t++] = n;
				}
		}
	}
	free(seen);
	free(q);
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
static int unclean_near_block(const LegalGrid *g, int x, int y) {
	int n = 0;
	for (int dy = -REACH; dy <= REACH + 1; ++dy)
		for (int dx = -REACH; dx <= REACH + 1; ++dx) n += unclean(g, x + dx, y + dy);
	return n;
}

LegalStats legal_fix(const LegalGrid *g, int budget) {
	LegalStats st = { 0, 0 };
	uint8_t *stuck = calloc((size_t)(g->gw + 2) * (g->gh + 2), 1);
#define STUCK(x, y) stuck[((y) + 1) * (g->gw + 2) + (x) + 1]
	for (bool changed = true; changed && st.edits < budget;) {
		changed = false;
		for (int py = -1; py <= g->gh && st.edits < budget; ++py)
			for (int px = -1; px <= g->gw && st.edits < budget; ++px) {
				if (STUCK(px, py) || !unclean(g, px, py)) continue;
				int best = 0, bx = -1, by = -1;
				for (int dy = -REACH; dy <= REACH; ++dy)
					for (int dx = -REACH; dx <= REACH; ++dx) {
						int x = px + dx, y = py + dy;
						if (x < MARGIN || y < MARGIN || x >= g->gw - MARGIN || y >= g->gh - MARGIN) continue;
						uint8_t *c = &g->cell[y * g->gw + x];
						if (g->locked[y * g->gw + x] || !simple(g, x, y)) continue;
						/* floor is only added into a notch: never a bump on a
						 * path's side or its end drawn out */
						if (!*c && __builtin_popcount(sides(g, x, y)) < 2) continue;
						int before = unclean_near(g, x, y);
						uint8_t was = *c;
						*c = !was;
						int delta = unclean_near(g, x, y) - before;
						*c = was;
						if (delta < best) { best = delta; bx = x; by = y; }
					}
				/* or a 2 x 2 block filled: a walkways' junction made a
				 * platform, as the originals join them */
				int kx = -1, ky = -1;
				for (int dy = -REACH; dy < REACH; ++dy)
					for (int dx = -REACH; dx < REACH; ++dx) {
						int x = px + dx, y = py + dy;
						uint8_t filled[4];
						int before = unclean_near_block(g, x, y);
						int n = block(g, x, y, filled);
						if (n <= 0) { if (n == 0) unblock(g, x, y, filled); continue; }
						int delta = unclean_near_block(g, x, y) - before;
						bool keeps = false;
						if (delta < best) {
							int f1, v1;
							topology(g, &f1, &v1);
							unblock(g, x, y, filled);
							int f0, v0;
							topology(g, &f0, &v0);
							keeps = f1 == f0 && v1 == v0;
						} else unblock(g, x, y, filled);
						if (keeps) { best = delta; kx = x; ky = y; bx = -1; }
					}
				if (kx >= 0) {
					uint8_t filled[4];
					st.edits += block(g, kx, ky, filled);
					bx = kx; by = ky;
				} else if (bx >= 0) {
					uint8_t *c = &g->cell[by * g->gw + bx];
					*c = !*c;
					++st.edits;
				}
				if (bx < 0) { STUCK(px, py) = 1; continue; }
				changed = true;
				/* the panels near it may be worth another try */
				for (int dy = -REACH - 1; dy <= REACH + 1; ++dy)
					for (int dx = -REACH - 1; dx <= REACH + 1; ++dx) {
						int x = bx + dx, y = by + dy;
						if (x >= -1 && y >= -1 && x <= g->gw && y <= g->gh) STUCK(x, y) = 0;
					}
			}
	}
	for (int py = -1; py <= g->gh; ++py)
		for (int px = -1; px <= g->gw; ++px) st.left += unclean(g, px, py);
#undef STUCK
	free(stuck);
	return st;
}
