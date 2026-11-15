/* The game's ramps are floor-height cells of type 0x13 (climbing towards
 * world +X) or 0x14 (towards world -Y) in coordinate data section 1: a
 * foot z, value 1 and the height above the foot, rising 4 per 8-unit cell
 * (docs/rom/net-maps.md, Height). A stair is one connected set of them; the
 * originals' are straight runs one panel wide (a ladder: Green's, CopyBot
 * Comp's, Sky's catwalks', the Graveyard's) or two (Sky's and the
 * Undernet's), two to four panels long (docs/world/level-design/heights.md). */
#include "stairs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PANEL 32

static int floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }
static int mod8(int v) { return ((v % 8) + 8) % 8; }

static bool is_ramp(const CoordCell *c) { return c->type == 0x13 || c->type == 0x14; }

/* the connected ramp cells of one type and foot around `seed` */
static int component(const AreaSrc *a, int seed, int *out, char *seen) {
	const CoordCell *s1 = a->sec[1];
	int n = 0;
	out[n++] = seed;
	seen[seed] = 1;
	for (int h = 0; h < n; ++h) {
		const CoordCell *p = &s1[out[h]];
		for (int i = 0; i < a->nsec[1]; ++i) {
			const CoordCell *q = &s1[i];
			if (seen[i] || !is_ramp(q) || q->type != p->type || q->z != p->z) continue;
			if (abs(q->x - p->x) + abs(q->y - p->y) != 8) continue;
			seen[i] = 1;
			out[n++] = i;
		}
	}
	return n;
}

/* Where a cell lies: its middle, a ramp's half a cell nearer its top. The
 * originals set their ramps half a cell off the panels towards their foot
 * (Green Area 1's ladder from Y 0 to 64 between panel edges at -4, 28 and
 * 60; a ramp climbing towards +X the same along X): its foot reaches 4
 * units into the walkway's panel, its top row stops 4 short of the room's,
 * and the step past it (docs/rom/net-maps.md, Height) straddles the edge. */
static void middle(const CoordCell *c, int *cx, int *cy) {
	*cx = c->x + (c->type == 0x13 ? 8 : 4);
	*cy = c->y + (c->type == 0x14 ? 0 : 4);
}

/* A stair's box: its corner with the lowest panel indices, its size in
 * world units and its foot's z. */
typedef struct { int x0, y0, w, h, z0; } Box;

static bool inside(const CoordCell *c, const Box *b) {
	int cx, cy;
	middle(c, &cx, &cy);
	return cx >= b->x0 && cx < b->x0 + b->w && cy >= b->y0 && cy < b->y0 + b->h;
}

static CoordCell *cut(const CoordCell *src, int n, const Box *b, int *count) {
	CoordCell *out = calloc((size_t)n + 1, sizeof *out);
	*count = 0;
	for (int i = 0; out && i < n; ++i)
		if (inside(&src[i], b)) {
			CoordCell c = src[i];
			c.x = (int16_t)(c.x - b->x0);
			c.y = (int16_t)(c.y - b->y0);
			c.z = (int8_t)(c.z - b->z0);
			out[(*count)++] = c;
		}
	return out;
}

/* The tiles that draw the ramp: every tile a ramp cell's box covers, from
 * its foot to its top. (Taken from 8 under the foot to 8 over the top, they
 * brought the original's surroundings along: its bridge pad's gem at an
 * Undernet stair's foot, its court's crosses on the raised room, a sliver
 * of floor in the void beside a Sky stair.) */
static void cut_tiles(const AreaSrc *a, const int *cells, int n, const Box *b, StairTemplate *t) {
	int cap = n * 64;
	t->tiles = calloc((size_t)cap, sizeof *t->tiles);
	char *mark = calloc((size_t)a->tw * a->th, 1);
	if (!t->tiles || !mark) { free(mark); return; }
	int px0 = b->x0 + b->y0 + a->tw * 4, py0 = (b->y0 - b->x0) / 2 - b->z0 + a->th * 4;
	for (int k = 0; k < n; ++k) {
		const CoordCell *c = &a->sec[1][cells[k]];
		for (int dz = 0; dz <= c->height; dz += 2)
			for (int corner = 0; corner < 4; ++corner) {
				int X = c->x + (corner & 1) * 8, Y = c->y + (corner >> 1) * 8;
				int px = X + Y + a->tw * 4, py = (Y - X) / 2 - (c->z + dz) + a->th * 4;
				int tx = floordiv(px, 8), ty = floordiv(py, 8);
				if (tx < 0 || ty < 0 || tx >= a->tw || ty >= a->th || mark[ty * a->tw + tx]) continue;
				mark[ty * a->tw + tx] = 1;
				size_t i = (size_t)ty * a->tw + tx;
				if (t->ntiles >= cap) continue;
				StairTile *st = &t->tiles[t->ntiles++];
				st->px = (int16_t)(tx * 8 - px0);
				st->py = (int16_t)(ty * 8 - py0);
				st->e0 = a->tile[0][i];
				st->e1 = a->layers > 1 ? a->tile[1][i] : 0;
				for (int p = 0; p < 64; ++p) st->opaque += a->idx[(size_t)(ty * 8 + p / 8) * a->tw * 8 + tx * 8 + p % 8] != 0;
			}
	}
	free(mark);
}

/* The rise of the stairs learned (0: none). */
static int stairs_rise(const StairKit t) {
	for (int d = 0; d < STAIR_DIRS; ++d)
		for (int w = 0; w < STAIR_WIDTHS; ++w)
			if (t[d][w].ok) return t[d][w].rise;
	return 0;
}

/* A ramp's shape: its box of panels (A0, B0 its corner, na x nb), its rise
 * and how far it runs along its climb; why it cannot be a layer's stair,
 * NULL where it can. */
typedef struct { int A0, B0, na, nb, rise, along, across, top_cells, steps; } RampShape;

static void shape_of(const AreaSrc *a, const int *cells, int n, RampShape *s) {
	int A1 = -(1 << 20), B1 = -(1 << 20), lo = 1 << 20, hi = -(1 << 20);
	s->A0 = s->B0 = 1 << 20;
	s->rise = 0;
	bool up_x = a->sec[1][cells[0]].type == 0x13;
	for (int k = 0; k < n; ++k) {
		const CoordCell *c = &a->sec[1][cells[k]];
		int mx, my;
		middle(c, &mx, &my);
		int A = floordiv(mx - a->ex, PANEL), B = floordiv(my - a->ey, PANEL);
		if (A < s->A0) s->A0 = A;
		if (A > A1) A1 = A;
		if (B < s->B0) s->B0 = B;
		if (B > B1) B1 = B;
		if (c->height > s->rise) s->rise = c->height;
		int along = up_x ? c->x : c->y;
		if (along < lo) lo = along;
		if (along > hi) hi = along;
	}
	s->na = A1 - s->A0 + 1;
	s->nb = B1 - s->B0 + 1;
	s->along = up_x ? s->na : s->nb;
	s->across = up_x ? s->nb : s->na;
	s->steps = (hi - lo) / 8 + 1;   /* its cells along the climb */
	/* (and its top row at the box's far end: 0x13 at +X, 0x14 at -Y) */
	s->top_cells = 0;
	for (int k = 0; k < n; ++k) {
		const CoordCell *c = &a->sec[1][cells[k]];
		if (c->height == s->rise && (up_x ? c->x == hi : c->y == lo)) ++s->top_cells;
	}
}

static const char *unfit(const AreaSrc *a, const RampShape *s, int n, int ex, int ey, int z0, int have_rise) {
	if (s->across < 1 || s->across > 2) return "not one or two panels wide";
	if (s->along < 2 || s->along > 4) return "not two to four panels long";
	if (s->rise % PANEL || s->rise > 2 * PANEL) return "its rise not one panel's height or two";
	if (s->rise != 4 * s->steps || s->steps != s->along * 4) return "not one even slope over its panels";
	if (s->top_cells < 2) return "no top row";
	if (have_rise && s->rise != have_rise) return "another rise than the area's other stairs";
	/* nothing else's ramp in its box */
	int x0 = a->ex + PANEL * s->A0, y0 = a->ey + PANEL * s->B0, inbox = 0;
	Box box = { x0, y0, PANEL * s->na, PANEL * s->nb, z0 };
	for (int i = 0; i < a->nsec[1]; ++i)
		if (is_ramp(&a->sec[1][i]) && inside(&a->sec[1][i], &box)) ++inbox;
	if (inbox != n) return "another ramp in its box";
	/* its tiles where a generated map's grid has them (an even number of
	 * tiles a side, its panel edges the area's own map's) */
	if (mod8(x0 + y0 + a->tw * 4) != mod8(ex + ey) || mod8((y0 - x0) / 2 - z0 + a->th * 4) != mod8((ey - ex) / 2)) return "its tiles off the layer's grid";
	return NULL;
}

void stairs_learn(const AreaSrc *a, int ex, int ey, StairKit out, bool report) {
	if (!a->sec[1] || !a->nsec[1]) return;
	int *cells = malloc(sizeof(int) * (size_t)(a->nsec[1] + 1));
	char *seen = calloc((size_t)a->nsec[1] + 1, 1);
	if (!cells || !seen) { free(cells); free(seen); return; }
	for (int i = 0; i < a->nsec[1]; ++i) {
		const CoordCell *s = &a->sec[1][i];
		if (!is_ramp(s) || seen[i]) continue;
		int dir = s->type == 0x14 ? STAIR_UP_NX : STAIR_UP_NY;
		int n = component(a, i, cells, seen);
		RampShape sh;
		shape_of(a, cells, n, &sh);
		const char *why = unfit(a, &sh, n, ex, ey, s->z, stairs_rise((const StairTemplate (*)[STAIR_WIDTHS])out));
		int width = sh.across == 1 ? STAIR_NARROW : STAIR_WIDE;
		if (!why && out[dir][width].ok) why = "one of its way and width learned already";
		if (report)
			fprintf(stderr, "stair map %02X:%d type %02X foot z %d rise %d panels %d along %d across, %d cells: %s\n", a->group, a->number,
				s->type, s->z, sh.rise, sh.along, sh.across, n, why ? why : "learned");
		if (why) continue;
		StairTemplate *t = &out[dir][width];
		Box box = { a->ex + PANEL * sh.A0, a->ey + PANEL * sh.B0, PANEL * sh.na, PANEL * sh.nb, s->z };
		t->rise = sh.rise;
		t->len = sh.along;
		t->wide = sh.across;
		t->group = a->group;
		t->number = a->number;
		t->ramp = cut(a->sec[1], a->nsec[1], &box, &t->nramp);
		t->walls = cut(a->sec[0], a->nsec[0], &box, &t->nwalls);
		t->prio = cut(a->sec[2], a->nsec[2], &box, &t->nprio);
		cut_tiles(a, cells, n, &box, t);
		t->ok = t->ramp && t->walls && t->prio && t->tiles;
	}
	free(seen);
	free(cells);
}

void stairs_free(StairKit t) {
	for (int d = 0; d < STAIR_DIRS; ++d)
		for (int w = 0; w < STAIR_WIDTHS; ++w) {
			free(t[d][w].ramp);
			free(t[d][w].walls);
			free(t[d][w].prio);
			free(t[d][w].tiles);
		}
	memset(t, 0, sizeof(StairKit));
}
