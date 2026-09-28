/* A counter in the original maps (docs/LEVEL_DESIGN.md, Props): its art is
 * a clean block of second-layer tiles over an untouched floor, and a ring
 * of wall cells (section 0) keeps MegaMan off it. The ring's back row lies
 * on a panel edge, the counter reaching half a panel or more towards its
 * front; its navi stands one cell behind that edge, its talk centre
 * shifted 8 units towards the front, so MegaMan speaks to it across the
 * counter (the Net Dealers' capsules of Sky Area 3 and Central Area 2, the
 * NetCafe desks of Sky Area 1 and Green Area 2). */
#include "props.h"

#include <stdlib.h>
#include <string.h>

#include "gfx.h"

#define PANEL 32
#define REACH 48   /* how far from the given point the ring may lie */

static int floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

/* The wall cells joined (8 ways) to the one nearest (x, y), within REACH. */
static int ring(const AreaSrc *a, int x, int y, int *cells) {
	const CoordCell *w = a->sec[0];
	int best = -1, bd = 1 << 30;
	for (int i = 0; i < a->nsec[0]; ++i) {
		int dx = w[i].x + 4 - x, dy = w[i].y + 4 - y;
		if (abs(dx) <= REACH && abs(dy) <= REACH && dx * dx + dy * dy < bd) { bd = dx * dx + dy * dy; best = i; }
	}
	if (best < 0) return 0;
	char *seen = calloc((size_t)a->nsec[0], 1);
	int n = 0;
	cells[n++] = best;
	seen[best] = 1;
	for (int h = 0; h < n; ++h)
		for (int i = 0; i < a->nsec[0]; ++i) {
			if (seen[i] || abs(w[i].x - w[cells[h]].x) > 8 || abs(w[i].y - w[cells[h]].y) > 8) continue;
			if (abs(w[i].x + 4 - x) > REACH || abs(w[i].y + 4 - y) > REACH) continue;
			seen[i] = 1;
			cells[n++] = i;
		}
	free(seen);
	return n;
}

/* The second layer's tiles over the ring's box (z 0 to a panel up): the
 * counter's art, joined 8 ways from those, kept within the box and a tile
 * round it. */
static void cut_tiles(const AreaSrc *a, int X0, int Y0, int X1, int Y1, int Xa, int Ya, int z0, PropStamp *p) {
	if (a->layers < 2) return;
	int px0 = 1 << 20, px1 = -(1 << 20), py0 = 1 << 20, py1 = -(1 << 20);
	for (int k = 0; k < 8; ++k) {
		int X = k & 1 ? X1 : X0, Y = k & 2 ? Y1 : Y0, z = z0 + (k & 4 ? PANEL : 0);
		int px = area_px(a->tw, X, Y), py = area_py(a->th, X, Y) - z;
		if (px < px0) px0 = px;
		if (px > px1) px1 = px;
		if (py < py0) py0 = py;
		if (py > py1) py1 = py;
	}
	int tx0 = floordiv(px0, 8) - 1, tx1 = floordiv(px1, 8) + 1, ty0 = floordiv(py0, 8) - 1, ty1 = floordiv(py1, 8) + 1;
	if (tx0 < 0) tx0 = 0;
	if (ty0 < 0) ty0 = 0;
	if (tx1 >= a->tw) tx1 = a->tw - 1;
	if (ty1 >= a->th) ty1 = a->th - 1;
	int bw = tx1 - tx0 + 1, bh = ty1 - ty0 + 1;
	if (bw <= 0 || bh <= 0) return;
	const uint16_t *l1 = a->tile[1];
	char *in = calloc((size_t)bw * bh, 1);
	int *q = malloc(sizeof(int) * (size_t)bw * bh), n = 0;
	/* seeds: the art over the ring itself */
	for (int ty = ty0 + 1; ty < ty1; ++ty)
		for (int tx = tx0 + 1; tx < tx1; ++tx)
			if (l1[(size_t)ty * a->tw + tx] & 0x3FF) { in[(ty - ty0) * bw + tx - tx0] = 1; q[n++] = (ty - ty0) * bw + tx - tx0; }
	for (int h = 0; h < n; ++h) {
		int cx = q[h] % bw, cy = q[h] / bw;
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx) {
				int x = cx + dx, y = cy + dy;
				if (x < 0 || y < 0 || x >= bw || y >= bh || in[y * bw + x]) continue;
				if (!(l1[(size_t)(y + ty0) * a->tw + x + tx0] & 0x3FF)) continue;
				in[y * bw + x] = 1;
				q[n++] = y * bw + x;
			}
	}
	p->tiles = calloc((size_t)n + 1, sizeof *p->tiles);
	/* (from the anchor's point on the counter's own floor) */
	int ax = area_px(a->tw, Xa, Ya), ay = area_py(a->th, Xa, Ya) - z0;
	for (int k = 0; k < n; ++k) {
		int tx = q[k] % bw + tx0, ty = q[k] / bw + ty0;
		StairTile *t = &p->tiles[p->ntiles++];
		t->px = (int16_t)(tx * 8 - ax);
		t->py = (int16_t)(ty * 8 - ay);
		t->e0 = 0;
		t->e1 = l1[(size_t)ty * a->tw + tx];
	}
	free(q);
	free(in);
}

bool props_learn_counter(const AreaSrc *a, int x, int y, int faces, PropStamp *out) {
	memset(out, 0, sizeof *out);
	int *cells = malloc(sizeof(int) * (size_t)(a->nsec[0] + 1));
	int n = ring(a, x, y, cells);
	if (n < 4) { free(cells); return false; }
	/* its box, and the height of the floor it stands on (Green Area 2's
	 * NetCafe is raised 32, its desk drawn that much higher) */
	int X0 = 1 << 20, X1 = -(1 << 20), Y0 = 1 << 20, Y1 = -(1 << 20), z0 = 1 << 20;
	for (int k = 0; k < n; ++k) {
		const CoordCell *c = &a->sec[0][cells[k]];
		if (c->z < z0) z0 = c->z;
		if (c->x < X0) X0 = c->x;
		if (c->x > X1) X1 = c->x;
		if (c->y < Y0) Y0 = c->y;
		if (c->y > Y1) Y1 = c->y;
	}
	/* its back row on a panel edge: the panels it stands on from there,
	 * its anchor their corner with the lowest X and Y */
	int Xa, Ya;
	if (faces == FACES_X) {
		if ((Y0 + 4 - a->ey) & 31) { free(cells); return false; }
		int Aa = floordiv(X0 + 4 - a->ex, PANEL), Ab = floordiv(X1 + 3 - a->ex, PANEL);
		out->len = Ab - Aa + 1;
		Xa = a->ex + PANEL * Aa;
		Ya = Y0 + 4;
		out->navi_x = (X0 + X1) / 2 + 4 - Xa;
		out->navi_y = Y0 + 4 - 8 - Ya;
		out->talk_x = 0;
		out->talk_y = 8;
	} else {
		if ((X1 + 4 - a->ex) & 31) { free(cells); return false; }
		int Ba = floordiv(Y0 + 4 - a->ey, PANEL), Bb = floordiv(Y1 + 3 - a->ey, PANEL);
		out->len = Bb - Ba + 1;
		Xa = X1 + 4 - PANEL;
		Ya = a->ey + PANEL * Ba;
		out->navi_x = X1 + 4 + 8 - Xa;
		out->navi_y = (Y0 + Y1) / 2 + 4 - Ya;
		out->talk_x = -8;
		out->talk_y = 0;
	}
	out->walls = calloc((size_t)n, sizeof *out->walls);
	for (int k = 0; k < n; ++k) {
		CoordCell c = a->sec[0][cells[k]];
		c.x = (int16_t)(c.x - Xa);
		c.y = (int16_t)(c.y - Ya);
		c.z = (int8_t)(c.z - z0);
		out->walls[out->nwalls++] = c;
	}
	free(cells);
	cut_tiles(a, X0, Y0, X1 + 8, Y1 + 8, Xa, Ya, z0, out);
	int u = 4 - a->tw * 4, v = 4 - a->th * 4;
	out->fx = ((u - 2 * v) / 2 - a->ex) & 31;
	out->fy = ((u + 2 * v) / 2 - a->ey) & 31;
	out->ok = out->len > 0 && out->ntiles > 0;
	return out->ok;
}

static bool learn_art(const AreaSrc *a, int seed, bool floor, PropStamp *out);

bool props_learn_ornament(const AreaSrc *a, int seed, PropStamp *out) { return learn_art(a, seed, true, out); }
bool props_learn_void_art(const AreaSrc *a, int seed, PropStamp *out) { return learn_art(a, seed, false, out); }

static bool learn_art(const AreaSrc *a, int seed, bool floor, PropStamp *out) {
	memset(out, 0, sizeof *out);
	if (a->layers < 2) return false;
	for (int i = 0; i < a->tw * a->th; ++i) {
		if ((a->tile[1][i] & 0x3FF) != seed) continue;
		/* the art round it (the second layer's tiles joined 8 ways, within
		 * two panels' width), and the panel under its middle at z 0 */
		int sx = i % a->tw, sy = i / a->tw, n = 0;
		int bx0 = sx - 8, by0 = sy - 6, bw = 17, bh = 13;
		char in[17 * 13] = { 0 };
		int q[17 * 13];
		q[n++] = (sy - by0) * bw + sx - bx0;
		in[q[0]] = 1;
		for (int h = 0; h < n; ++h) {
			int cx = q[h] % bw, cy = q[h] / bw;
			for (int dy = -1; dy <= 1; ++dy)
				for (int dx = -1; dx <= 1; ++dx) {
					int x = cx + dx, y = cy + dy, tx = x + bx0, ty = y + by0;
					if (x < 0 || y < 0 || x >= bw || y >= bh || in[y * bw + x] || tx < 0 || ty < 0 || tx >= a->tw || ty >= a->th) continue;
					if (!(a->tile[1][(size_t)ty * a->tw + tx] & 0x3FF)) continue;
					in[y * bw + x] = 1;
					q[n++] = y * bw + x;
				}
		}
		if (n < 8 || n > 60) continue;
		int px = 0, py = 0;
		for (int k = 0; k < n; ++k) { px += (q[k] % bw + bx0) * 8 + 4; py += (q[k] / bw + by0) * 8 + 4; }
		px = px / n - a->tw * 4;
		py = py / n - a->th * 4;
		int X = (px - 2 * py) / 2, Y = (px + 2 * py) / 2;
		int Xa = a->ex + PANEL * floordiv(X - a->ex, PANEL), Ya = a->ey + PANEL * floordiv(Y - a->ey, PANEL);
		/* (on floor: an ornament of a pad, not art hung in the void; or
		 * the void's own) */
		if (area_src_walled_floor(a, Xa + 16, Ya + 16) != (floor ? 1 : 0)) continue;
		out->tiles = calloc((size_t)n, sizeof *out->tiles);
		int ax = area_px(a->tw, Xa, Ya), ay = area_py(a->th, Xa, Ya);
		for (int k = 0; k < n; ++k) {
			int tx = q[k] % bw + bx0, ty = q[k] / bw + by0;
			StairTile *t = &out->tiles[out->ntiles++];
			t->px = (int16_t)(tx * 8 - ax);
			t->py = (int16_t)(ty * 8 - ay);
			t->e1 = a->tile[1][(size_t)ty * a->tw + tx];
		}
		out->len = 1;
		out->ok = true;
		return true;
	}
	return false;
}

/* The tiles of layer `l` that draw colour `want` (RGB), a group of 2 to 24
 * joined 8 ways on one panel with floor all round. */
static bool floor_emblem_on(const AreaSrc *a, uint32_t want, int l, PropStamp *out) {
	int W = a->tw * 8, cells = a->tw * a->th;
	uint8_t *mark = calloc((size_t)cells, 1);
	int *q = malloc(sizeof *q * (size_t)cells);
	for (int t = 0; t < cells; ++t)
		for (int y = 0; y < 8 && !mark[t]; ++y)
			for (int x = 0; x < 8; ++x) {
				size_t i = (size_t)(t / a->tw * 8 + y) * W + t % a->tw * 8 + x;
				uint32_t c = l ? (a->front[i] ? a->px[i] : 0) : a->px0[i];
				if (c >> 24 && (c & 0xFFFFFF) == want) { mark[t] = 1; break; }
			}
	for (int t0 = 0; t0 < cells && !out->ok; ++t0) {
		if (mark[t0] != 1) continue;
		int n = 0, sx = 0, sy = 0;
		q[n++] = t0;
		mark[t0] = 2;
		for (int h = 0; h < n; ++h)
			for (int dy = -1; dy <= 1; ++dy)
				for (int dx = -1; dx <= 1; ++dx) {
					int x = q[h] % a->tw + dx, y = q[h] / a->tw + dy;
					if (x < 0 || y < 0 || x >= a->tw || y >= a->th || mark[y * a->tw + x] != 1) continue;
					mark[y * a->tw + x] = 2;
					q[n++] = y * a->tw + x;
				}
		if (n < 2 || n > 24) continue;
		for (int k = 0; k < n; ++k) { sx += q[k] % a->tw * 8 + 4; sy += q[k] / a->tw * 8 + 4; }
		int px = sx / n - a->tw * 4, py = sy / n - a->th * 4;
		int X = (px - 2 * py) / 2, Y = (px + 2 * py) / 2;
		int Xa = a->ex + PANEL * floordiv(X - a->ex, PANEL), Ya = a->ey + PANEL * floordiv(Y - a->ey, PANEL);
		bool inside = true;
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx) inside &= area_src_walled_floor(a, Xa + 16 + PANEL * dx, Ya + 16 + PANEL * dy) == 1;
		if (!inside) continue;
		int ax = area_px(a->tw, Xa, Ya), ay = area_py(a->th, Xa, Ya);
		out->tiles = calloc((size_t)n, sizeof *out->tiles);
		for (int k = 0; k < n; ++k)
			out->tiles[k] = (StairTile){ .px = (int16_t)(q[k] % a->tw * 8 - ax), .py = (int16_t)(q[k] / a->tw * 8 - ay),
				.e0 = l ? 0 : a->tile[0][q[k]], .e1 = l ? a->tile[1][q[k]] : 0 };
		out->ntiles = n;
		out->len = 1;
		out->ok = true;
	}
	free(q);
	free(mark);
	return out->ok;
}

bool props_learn_floor_emblem(const AreaSrc *a, uint16_t bgr, PropStamp *out) {
	memset(out, 0, sizeof *out);
	uint32_t want = bgr555(bgr) & 0xFFFFFF;
	return floor_emblem_on(a, want, 0, out) || (a->layers > 1 && floor_emblem_on(a, want, 1, out));
}

void props_mirror_walls(const AreaSrc *a, AreaSrc *m) {
	/* a wall's type says where its floor lies: (dx, dy) goes to (-dy, -dx),
	 * so NE and NW edges trade (1, 4), SE and SW (2, 3), the E and W
	 * corners (5, 8), and the originals' 9 and 0x0B beside 5 and 7 go as
	 * those do */
	static const uint8_t type[16] = { 0, 4, 3, 2, 1, 8, 6, 7, 5, 8, 10, 7, 12, 13, 14, 15 };
	free(m->sec[0]);
	m->sec[0] = malloc(sizeof(CoordCell) * (size_t)(a->nsec[0] + 1));
	m->nsec[0] = a->nsec[0];
	for (int i = 0; i < a->nsec[0]; ++i) {
		CoordCell c = a->sec[0][i];
		int x = c.x, y = c.y;
		c.x = (int16_t)(-y - 8);
		c.y = (int16_t)(-x - 8);
		if (c.type < 16) c.type = type[c.type];
		m->sec[0][i] = c;
	}
}

void props_free(PropStamp *p) {
	free(p->tiles);
	free(p->walls);
	memset(p, 0, sizeof *p);
}
