/* The game's ramps are floor-height cells of type 0x13 (climbing towards
 * world +X) or 0x14 (towards world -Y) in coordinate data section 1: a
 * foot z, value 1 and the height above the foot, rising 4 per 8-unit cell
 * (docs/ROM_DATA.md). A stair is one connected set of them. */
#include "stairs.h"

#include <stdlib.h>
#include <string.h>

#define PANEL 32

static int floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

static bool is_ramp(const CoordCell *c) { return c->type == 0x13 || c->type == 0x14; }

/* the connected ramp cells of one type and foot around `seed` */
static int component(const AreaSrc *a, int seed, int *out) {
	const CoordCell *s1 = a->sec[1];
	char *seen = calloc((size_t)a->nsec[1], 1);
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
	free(seen);
	return n;
}

static bool inside(const CoordCell *c, int x0, int y0) {
	int cx = c->x + 4, cy = c->y + 4;
	return cx >= x0 && cx < x0 + 2 * PANEL && cy >= y0 && cy < y0 + 2 * PANEL;
}

static CoordCell *cut(const CoordCell *src, int n, int x0, int y0, int z0, int *count) {
	CoordCell *out = calloc((size_t)n + 1, sizeof *out);
	*count = 0;
	for (int i = 0; i < n; ++i)
		if (inside(&src[i], x0, y0)) {
			CoordCell c = src[i];
			c.x = (int16_t)(c.x - x0);
			c.y = (int16_t)(c.y - y0);
			c.z = (int8_t)(c.z - z0);
			out[(*count)++] = c;
		}
	return out;
}

/* The tiles that draw the ramp: every tile a ramp cell's box covers, from a
 * little under its foot to a little over its top. */
static void cut_tiles(const AreaSrc *a, const int *cells, int n, int x0, int y0, int z0, StairTemplate *t) {
	int cap = n * 64;
	t->tiles = calloc((size_t)cap, sizeof *t->tiles);
	char *mark = calloc((size_t)a->tw * a->th, 1);
	int px0 = x0 + y0 + a->tw * 4, py0 = (y0 - x0) / 2 - z0 + a->th * 4;
	for (int k = 0; k < n; ++k) {
		const CoordCell *c = &a->sec[1][cells[k]];
		for (int dz = -8; dz <= c->height + 8; dz += 2)
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
			}
	}
	free(mark);
}

void stairs_learn(const AreaSrc *a, StairTemplate out[STAIR_DIRS]) {
	memset(out, 0, sizeof(StairTemplate) * STAIR_DIRS);
	int *cells = malloc(sizeof(int) * (size_t)(a->nsec[1] + 1));
	for (int i = 0; i < a->nsec[1]; ++i) {
		const CoordCell *s = &a->sec[1][i];
		if (!is_ramp(s)) continue;
		int dir = s->type == 0x14 ? STAIR_UP_NX : STAIR_UP_NY;
		if (out[dir].ok) continue;
		int n = component(a, i, cells);
		/* its panels: exactly 2 x 2 */
		int A0 = 1 << 20, A1 = -(1 << 20), B0 = 1 << 20, B1 = -(1 << 20), rise = 0;
		for (int k = 0; k < n; ++k) {
			const CoordCell *c = &a->sec[1][cells[k]];
			int A = floordiv(c->x + 4 - a->ex, PANEL), B = floordiv(c->y + 4 - a->ey, PANEL);
			if (A < A0) A0 = A;
			if (A > A1) A1 = A;
			if (B < B0) B0 = B;
			if (B > B1) B1 = B;
			if (c->height > rise) rise = c->height;
		}
		if (A1 - A0 != 1 || B1 - B0 != 1 || rise % PANEL) continue;
		StairTemplate *t = &out[dir];
		int x0 = a->ex + PANEL * A0, y0 = a->ey + PANEL * B0, z0 = s->z;
		t->rise = rise;
		t->ramp = cut(a->sec[1], a->nsec[1], x0, y0, z0, &t->nramp);
		t->walls = cut(a->sec[0], a->nsec[0], x0, y0, z0, &t->nwalls);
		t->prio = cut(a->sec[2], a->nsec[2], x0, y0, z0, &t->nprio);
		cut_tiles(a, cells, n, x0, y0, z0, t);
		t->ok = true;
	}
	free(cells);
}

void stairs_free(StairTemplate t[STAIR_DIRS]) {
	for (int d = 0; d < STAIR_DIRS; ++d) {
		free(t[d].ramp);
		free(t[d].walls);
		free(t[d].prio);
		free(t[d].tiles);
	}
	memset(t, 0, sizeof(StairTemplate) * STAIR_DIRS);
}
