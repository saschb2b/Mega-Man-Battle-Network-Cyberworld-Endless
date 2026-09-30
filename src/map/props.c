/* A counter in the original maps (docs/LEVEL_DESIGN.md, Props): its art is
 * a clean block of second-layer tiles over an untouched floor, and a ring
 * of wall cells (section 0) keeps MegaMan off it. The ring's back row lies
 * on a panel edge, the counter reaching half a panel or more towards its
 * front; its navi stands one cell behind that edge, its talk centre
 * shifted 8 units towards the front, so MegaMan speaks to it across the
 * counter (the Net Dealers' capsules of Sky Area 3 and Central Area 2, the
 * NetCafe desks of Sky Area 1 and Green Area 2). */
#include "props.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "gfx.h"

#define PANEL 32
#define REACH 48   /* how far from the given point the ring may lie */
#define PRIO_REACH 24   /* how far round its ring a counter's layer priorities are taken */
#define SPRITE_HALF_W 10   /* MegaMan's sprite on the map: half its width, */
#define SPRITE_H 38        /* and its height above his feet, in pixels */

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
	/* its layer priorities: the original's round its ring, behind it and
	 * beside its ends, where MegaMan goes behind its art (Green Area 2's
	 * NetCafe desk: its aisle and its far end; without them MegaMan at
	 * that end was drawn over the desk). Only those where MegaMan standing
	 * there overlaps the counter's art: behind the layer's other art (a
	 * raised pad beside it) he is drawn in front as before, where one of
	 * the original's cut him in half. */
	int bx0 = 1 << 20, bx1 = -(1 << 20), by0 = 1 << 20, by1 = -(1 << 20);
	for (int k = 0; k < out->ntiles; ++k) {
		const StairTile *t = &out->tiles[k];
		if (t->px < bx0) bx0 = t->px;
		if (t->px + 8 > bx1) bx1 = t->px + 8;
		if (t->py < by0) by0 = t->py;
		if (t->py + 8 > by1) by1 = t->py + 8;
	}
	out->prio = calloc((size_t)a->nsec[2] + 1, sizeof *out->prio);
	for (int k = 0; k < a->nsec[2]; ++k) {
		CoordCell c = a->sec[2][k];
		if (c.x < X0 - PRIO_REACH || c.x >= X1 + 8 + PRIO_REACH || c.y < Y0 - PRIO_REACH || c.y >= Y1 + 8 + PRIO_REACH) continue;
		c.x = (int16_t)(c.x - Xa);
		c.y = (int16_t)(c.y - Ya);
		c.z = (int8_t)(c.z - z0);
		/* (his feet at the cell's middle on the screen, from the anchor's
		 * point as the tiles are; his sprite about 20 x 38 above them) */
		int fx = c.x + c.y + 8, fy = (c.y - c.x) / 2 - c.z;
		if (fx + SPRITE_HALF_W <= bx0 || fx - SPRITE_HALF_W >= bx1 || fy <= by0 || fy - SPRITE_H >= by1) continue;
		out->prio[out->nprio++] = c;
	}
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
		/* (not art that draws mostly black: Sky's tile 0x379 is the pit in
		 * one of its platforms, not a gem) */
		long dark = 0, lit = 0;
		for (int k = 0; k < n; ++k)
			for (int y = 0; y < 8; ++y)
				for (int x = 0; x < 8; ++x) {
					uint32_t c = a->px[(size_t)((q[k] / bw + by0) * 8 + y) * (size_t)(a->tw * 8) + (size_t)((q[k] % bw + bx0) * 8 + x)];
					if (!(c >> 24)) continue;
					if (((c >> 16) & 255) + ((c >> 8) & 255) + (c & 255) < 90) ++dark; else ++lit;
				}
		if (dark > lit) continue;
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

/* The hue bucket of the floor around map pixel (cx, cy), as tiles.c's
 * style_at: 0-11, 12 grey. */
static int hue_at(const AreaSrc *a, int cx, int cy) {
	long r = 0, g = 0, b = 0, n = 0;
	int W = a->tw * 8, H = a->th * 8;
	for (int y = cy - 4; y < cy + 4; ++y)
		for (int x = cx - 8; x < cx + 8; ++x) {
			if (x < 0 || y < 0 || x >= W || y >= H) continue;
			uint32_t c = area_src_floor_px(a, (size_t)y * W + x);
			r += (c >> 16) & 255; g += (c >> 8) & 255; b += c & 255; ++n;
		}
	if (!n) return 12;
	float fr = (float)r / n, fg = (float)g / n, fb = (float)b / n;
	float mx = fr > fg ? (fr > fb ? fr : fb) : (fg > fb ? fg : fb), mn = fr < fg ? (fr < fb ? fr : fb) : (fg < fb ? fg : fb);
	if (mx <= 0 || (mx - mn) / mx <= 0.25f) return 12;
	float d = mx - mn, hue = mx == fr ? (fg - fb) / d : mx == fg ? 2 + (fb - fr) / d : 4 + (fr - fg) / d;
	hue /= 6;
	if (hue < 0) hue += 1;
	int k = (int)(hue * 12);
	return k > 11 ? 11 : k;
}

/* Whether tile (tx, ty) of `a` draws only the island of 3 x 3 panels from
 * panel (A, B), its faces and the void: none of its first layer's pixels
 * lies on another floor (at z 0, the floor drawn 4 below its edges). */
static bool island_only(const AreaSrc *a, int A, int B, int tx, int ty) {
	int W = a->tw * 8;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			int px = tx * 8 + x, py = ty * 8 + y;
			if (!(a->px0[(size_t)py * W + px] >> 24)) continue;
			int u = px - a->tw * 4, w = 2 * (py - 4 - a->th * 4);
			int pA = floordiv((u - w) / 2 - a->ex, PANEL), pB = floordiv((u + w) / 2 - a->ey, PANEL);
			if (pA >= A && pA <= A + 2 && pB >= B && pB <= B + 2) continue;
			if (area_src_walled_floor(a, a->ex + 16 + PANEL * pA, a->ey + 16 + PANEL * pB) == 1) return false;
		}
	return true;
}

bool props_learn_pad(const AreaSrc *a, uint16_t hues, PropStamp *out) {
	if (!out->ok) memset(out, 0, sizeof *out);
	/* the map's islands, around its middle, fewest bridges beside their
	 * lower sides first */
	struct { int A, B, low; } isl[32];
	int ni = 0;
	int A0 = -(a->tw * 8 + a->th * 8) / PANEL, A1 = -A0;
	for (int B = A0; B <= A1; ++B)
		for (int A = A0; A <= A1; ++A) {
			int Xa = a->ex + PANEL * A, Ya = a->ey + PANEL * B;
			bool island = true;
			int beside = 0;
			for (int j = -1; j <= 3 && island; ++j)
				for (int i = -1; i <= 3 && island; ++i) {
					int f = area_src_walled_floor(a, Xa + 16 + PANEL * i, Ya + 16 + PANEL * j);
					bool in = i >= 0 && i <= 2 && j >= 0 && j <= 2, corner = (i < 0 || i > 2) && (j < 0 || j > 2);
					if (in) island = f == 1;
					else if (corner) island = f != 1;
					else beside += f == 1;
				}
			if (!island || beside > 4) continue;
			int cx = area_px(a->tw, Xa + 48, Ya + 48), cy = area_py(a->th, Xa + 48, Ya + 48);
			if (!(hues >> hue_at(a, cx, cy + 4) & 1)) continue;
			/* its lower sides on screen, world -X and +Y */
			int low = 0;
			for (int k = 0; k <= 2; ++k)
				low += (area_src_walled_floor(a, Xa + 16 - PANEL, Ya + 16 + PANEL * k) == 1) + (area_src_walled_floor(a, Xa + 16 + PANEL * k, Ya + 16 + PANEL * 3) == 1);
			if (ni == 32) continue;
			int k = ni++;
			while (k > 0 && isl[k - 1].low > low) { isl[k] = isl[k - 1]; --k; }
			isl[k].A = A; isl[k].B = B; isl[k].low = low;
		}
	if (!ni) return out->ok;
	/* its tiles: over the diamond of the first island's panels (the floor
	 * drawn 4 below their edges), the faces under its lower edges and the
	 * tiles its rim reaches from outside; each in place, where one of the
	 * islands has it, as a tile drawing its island alone (e1 1 while
	 * learning): a bridge beside one comes along in its tiles there (the
	 * steps of the classes' pieces had shown along a pad's edges where its
	 * stamp left them out) */
	static StairTile got[600];
	int n = 0;
	bool had = out->ok;
	if (had) {
		n = out->ntiles < 600 ? out->ntiles : 600;
		memcpy(got, out->tiles, sizeof *got * (size_t)n);
	}
	for (int k = 0; k < ni; ++k) {
		int A = isl[k].A, B = isl[k].B, Xa = a->ex + PANEL * A, Ya = a->ey + PANEL * B;
		int cx = area_px(a->tw, Xa + 48, Ya + 48), cy = area_py(a->th, Xa + 48, Ya + 48);
		int ax = area_px(a->tw, Xa, Ya), ay = area_py(a->th, Xa, Ya);
		for (int ty = (cy - 56) / 8; ty <= (cy + 72) / 8; ++ty)
			for (int tx = (cx - 104) / 8; tx <= (cx + 104) / 8; ++tx) {
				if (tx < 0 || ty < 0 || tx >= a->tw || ty >= a->th || !(a->tile[0][(size_t)ty * a->tw + tx] & 0x3FF)) continue;
				float dx = (float)(tx * 8 + 4 - cx), dy = (float)(ty * 8 + 4 - cy - 4);
				float in = fabsf(dx) / 96.0f + fabsf(dy) / 48.0f, under = dy > 0 ? fabsf(dx) / 96.0f + (dy - 12.0f) / 48.0f : 9.0f;
				/* (the rim past the diamond along its upper edges and
				 * corners: below, what hangs from a bridge beside it would
				 * come along, posts floating under a pad) */
				bool core = in <= 1.0f || under <= 1.0f, alone = island_only(a, A, B, tx, ty);
				if (!core && !(in <= 1.25f && dy <= 4.0f && alone)) continue;
				int16_t px = (int16_t)(tx * 8 - ax), py = (int16_t)(ty * 8 - ay);
				uint16_t e0 = a->tile[0][(size_t)ty * a->tw + tx];
				int j = 0;
				while (j < n && (got[j].px != px || got[j].py != py)) ++j;
				if (j < n) {
					if (!got[j].e1 && alone) got[j] = (StairTile){ .px = px, .py = py, .e0 = e0, .e1 = 1 };
					continue;
				}
				/* (a new place: the first island's core, or a tile drawing
				 * its island alone) */
				if ((!alone && (k > 0 || had)) || n == 600) continue;
				got[n++] = (StairTile){ .px = px, .py = py, .e0 = e0, .e1 = (uint16_t)alone };
			}
	}
	if (n < 40) return out->ok;
	free(out->tiles);
	out->tiles = calloc((size_t)n, sizeof *out->tiles);
	memcpy(out->tiles, got, sizeof *got * (size_t)n);
	out->ntiles = n;
	out->len = 3;
	if (!had || isl[0].low < out->low) out->low = isl[0].low;
	out->ok = true;
	return true;
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
	free(m->sec[2]);
	m->sec[2] = malloc(sizeof(CoordCell) * (size_t)(a->nsec[2] + 1));
	m->nsec[2] = a->nsec[2];
	for (int i = 0; i < a->nsec[2]; ++i) {
		CoordCell c = a->sec[2][i];
		int x = c.x, y = c.y;
		c.x = (int16_t)(-y - 8);
		c.y = (int16_t)(-x - 8);
		m->sec[2][i] = c;
	}
}

void props_free(PropStamp *p) {
	free(p->tiles);
	free(p->walls);
	free(p->prio);
	memset(p, 0, sizeof *p);
}
