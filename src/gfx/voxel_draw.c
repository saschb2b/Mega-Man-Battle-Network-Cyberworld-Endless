/* voxel.h's picture: the lit scene through the camera, each pixel a face
 * of a column laid out at the screen's pixels, then the soft layers over
 * it (the sky, the floor's shadow on it, the glow, what is out of focus;
 * docs/engine/voxel.md, The camera and The lens). */
#include "voxel.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define COS30  0.8660254f
#define RAD    0.01745329f
/* How many ground rows on the next row's top may lie to be the same floor going on */
#define RAMP_STEP 4
/* The widest seam that is closed, in voxels (a gap any wider is the void between two floors) */
#define SEAM 3

enum { IS_SKY, IS_MAP, IS_SPRITE, IS_MENDED };

/* The picture being made: each pixel's nearness (the larger the nearer)
 * and what it shows */
static struct { int scale, ow, oh; float *z; uint8_t *what; uint32_t *out; } T;

/* The soft layers, on the frame's 240x160 (made with the picture: no
 * system pays for them that leaves voxel mode off) */
static float *low, *soft, *glow, *tmp, *cover;
/* ... the blurred and the glow as the last pass reads them, 0x00BBGGRR */
static uint32_t *soft8, *glow8;

/* A column (or row) of the picture on the frame's: the frame's two it lies
 * between and how far toward the second, its share of the vignette, and
 * the frame's column (row) the floor's shadow on the sky is read from
 * there. Made with the target: worked out at each pixel, the last pass
 * was the picture's dearest. */
typedef struct { int a, b, shade; float f, vig; unsigned w; } Tap;
static Tap *across, *down;
/* each column's of the window the sky is read from */
static int *sky_x;
/* each column's run of sky so far as the seams are closed (-1: none begun under something yet) */
static int *gap;

static void taps(Tap *t, int n, int len, int shade) {
	for (int o = 0; o < n; ++o) {
		float u = ((float)o + 0.5f) / (float)T.scale, w = u - 0.5f, edge = (u / (float)len - 0.5f) * 2.0f;
		int a = (int)floorf(w);
		t[o] = (Tap){ a < 0 ? 0 : a, a + 1 < len ? a + 1 : len - 1, (int)u - shade, w - (float)a, edge * edge, (unsigned)((w - (float)a) * 256.0f + 0.5f) };
	}
}

static bool target(int scale, uint32_t *out) {
	if (T.scale != scale) {
		free(T.z);
		free(T.what);
		free(across);
		T.scale = scale;
		T.ow = VOX_W * scale;
		T.oh = VOX_H * scale;
		T.z = malloc((size_t)T.ow * (size_t)T.oh * sizeof *T.z);
		T.what = malloc((size_t)T.ow * (size_t)T.oh);
		across = malloc(sizeof *across * (size_t)(T.ow + T.oh));
		down = across ? across + T.ow : NULL;
		free(sky_x);
		sky_x = malloc(sizeof *sky_x * (size_t)T.ow);
		free(gap);
		gap = malloc(sizeof *gap * (size_t)T.ow);
		if (!low) {
			low = malloc(sizeof *low * VOX_N * 19);
			if (low) { soft = low + VOX_N * 3; glow = soft + VOX_N * 3; cover = glow + VOX_N * 3; tmp = cover + VOX_N; soft8 = (uint32_t *)(tmp + VOX_N * 7); glow8 = soft8 + VOX_N; }
		}
		if (!T.z || !T.what || !low || !across || !sky_x || !gap) { T.scale = 0; return false; }
		/* (the floor's shadow on the sky lies 8 columns right of it and 13 rows down) */
		taps(across, T.ow, VOX_W, 8);
		taps(down, T.oh, VOX_H, 13);
	}
	T.out = out;
	for (int i = 0; i < T.ow * T.oh; ++i) T.z[i] = -1e9f;
	memset(T.what, IS_SKY, (size_t)T.ow * (size_t)T.oh);
	return true;
}

/* ---- the camera ---- */

typedef struct { float ky, kh, cn, sn, zoom, depth, x, gy, h, row, near0, bevel; bool whole; int clip0, clip1; } Lens;

static float nearness(const Lens *l, float g, float h) { return 2.0f * g * l->cn + h / COS30 * l->sn; }

static Lens lens(const VoxCamera *c, const VoxLook *k) {
	float a = c->pitch * RAD;
	Lens l = { sinf(a) / 0.5f, cosf(a) / COS30, cosf(a), sinf(a), c->zoom, c->depth, c->x, c->gy, c->h, c->gy - c->h, 0, k->debug ? 0 : k->bevel,
		c->depth == 0.0f, 0, 1 << 30 };
	l.near0 = nearness(&l, c->gy, c->h);
	return l;
}

static int clamp_to(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

/* (BN6's camera, a lower one or a zoom: a point of the lattice at row y
 * and height h shows at the screen's row A + zoom * (ky * y + (ky - kh) * h),
 * its column x at l.x + zoom * (x - l.x); a camera that makes the near
 * larger shows the whole window) */
VoxSpan voxel_span(const VoxCamera *cam, int under, int over) {
	VoxLook plain = { 0 };
	Lens l = lens(cam, &plain);
	if (!l.whole || l.ky < 0.05f || l.zoom < 0.05f) return (VoxSpan){ 0, 0, VOX_WW, VOX_WH };
	float slope = l.zoom * l.ky, a = l.row + l.zoom * (l.kh * l.h - l.ky * l.gy), m = 2.0f;
	float e0 = l.zoom * (l.ky - l.kh) * (float)-under, e1 = l.zoom * (l.ky - l.kh) * (float)over;
	float lo = e0 < e1 ? e0 : e1, hi = e0 < e1 ? e1 : e0;
	/* (and the sky behind, which keeps BN6's place) */
	float ya = (-m - a - hi) / slope, yb = ((float)VOX_H + m - a - lo) / slope, za = l.row - l.row / l.zoom, zb = l.row + ((float)VOX_H - l.row) / l.zoom;
	float xa = l.x - (l.x + m) / l.zoom, xb = l.x + ((float)VOX_W + m - l.x) / l.zoom;
	int y0 = (int)floorf(ya < za ? ya : za) - 2, y1 = (int)ceilf(yb > zb ? yb : zb) + 3;
	int x0 = (int)floorf(xa) - 2, x1 = (int)ceilf(xb) + 3;
	/* (the frame's own pixels always: its sprites and what lies on the floor) */
	VoxSpan sp = { clamp_to(x0 < 0 ? x0 : 0, -VOX_LEFT, 0) + VOX_LEFT, clamp_to(y0 < 0 ? y0 : 0, -VOX_UP, 0) + VOX_UP,
		clamp_to(x1 > VOX_W ? x1 : VOX_W, VOX_W, VOX_W + VOX_LEFT) + VOX_LEFT, clamp_to(y1 > VOX_H ? y1 : VOX_H, VOX_H, VOX_H + VOX_DOWN) + VOX_UP };
	return sp;
}

/* Where the lattice's (x, g, h) shows, in the picture's pixels */
static void project(const Lens *l, float x, float g, float h, float *px, float *py) {
	float s = l->zoom * (1.0f + l->depth * (nearness(l, g, h) - l->near0));
	*px = (float)T.scale * (l->x + s * (x - l->x));
	*py = (float)T.scale * (l->row + s * (l->ky * (g - l->gy) - l->kh * (h - l->h)));
}

static uint32_t scaled(uint32_t c, float f) {
	uint32_t out = 0xFF000000u;
	for (int s = 0; s < 24; s += 8) {
		int v = (int)((float)(c >> s & 0xFF) * f);
		out |= (uint32_t)(v > 255 ? 255 : v) << s;
	}
	return out;
}

/* (floorf is a call on a PC's baseline x86-64, four for each face) */
static inline int down_to(float v) {
	int i = (int)v;
	return i - (v < (float)i);
}

/* A face between two corners of the picture, nearer than what is there:
 * its colour, its edges lit up and left and shaded down and right */
static void face(const Lens *l, const float a[2], const float b[2], float near, uint32_t c, int what) {
	int x0, y0, x1, y1;
	if (l->whole) { x0 = down_to(a[0] + 0.5f); y0 = down_to(a[1] + 0.5f); x1 = down_to(b[0] + 0.5f); y1 = down_to(b[1] + 0.5f); }
	else { x0 = down_to(a[0] < b[0] ? a[0] : b[0]); y0 = down_to(a[1]); x1 = -down_to(-(a[0] < b[0] ? b[0] : a[0])); y1 = -down_to(-b[1]); }
	bool edged = l->bevel > 0 && x1 - x0 >= 3 && y1 - y0 >= 3;
	uint32_t lit = edged ? scaled(c, 1.0f + l->bevel) : c, dim = edged ? scaled(c, 1.0f - l->bevel) : c;
	int fx = x0 < 0 ? 0 : x0, fy = y0 < l->clip0 ? l->clip0 : y0, tx = x1 > T.ow ? T.ow : x1, ty = y1 > l->clip1 ? l->clip1 : y1;
	for (int y = fy; y < ty; ++y)
		for (int x = fx; x < tx; ++x) {
			int i = y * T.ow + x;
			if (near <= T.z[i]) continue;
			T.z[i] = near;
			T.what[i] = (uint8_t)what;
			T.out[i] = y == y0 || x == x0 ? lit : y == y1 - 1 || x == x1 - 1 ? dim : c;
		}
}

/* A floor's top at (x, g) of height h, as far as (gn, hn): the next row's
 * top where the floor goes on (a ramp's rows climb), else a ground row on */
static void top(const Lens *l, int x, const float from[2], const float to[2], uint32_t c) {
	float a[2], b[2], own[2], g = from[0], h = from[1];
	project(l, (float)x, g, h, &a[0], &a[1]);
	project(l, (float)(x + 1), to[0], to[1], &b[0], &b[1]);
	/* (no less than its own ground row: a slope's next row may begin lower or higher on the screen than this one ends) */
	project(l, (float)(x + 1), g + 1.0f, h, &own[0], &own[1]);
	if (own[1] > b[1]) b[1] = own[1];
	face(l, a, b, nearness(l, g + 0.5f, h), c, IS_MAP);
}

/* What stands at ground row g, the frame's pixel (x, y) of it: a row high */
static void upright(const Lens *l, int x, int y, float g, uint32_t c, int what) {
	float a[2], b[2], h = g - (float)y;
	project(l, (float)x, g, h, &a[0], &a[1]);
	project(l, (float)(x + 1), g, h - 1.0f, &b[0], &b[1]);
	face(l, a, b, nearness(l, g, h - 0.5f) + (what == IS_SPRITE ? 0.6f : 0.0f), c, what);
}

/* The faces of a band of the picture's rows, each thread its own band:
 * every one of the window's rows whose faces can reach into it (by the
 * least and most height of its pixels), then the sprites, drawn in the
 * order one thread draws them all, so a pixel two faces tie on is the
 * same one's */
typedef struct { const VoxScene *s; const uint32_t *map, *sprite; const Lens *l; VoxSpan sp; } Faces;
static int16_t row_lo[VOX_WH], row_hi[VOX_WH];

static bool row_reaches(const Lens *l, int j, int from, int to) {
	if (row_lo[j] > row_hi[j]) return false;
	if (!l->whole) return true;
	float y = (float)(j - VOX_UP), pa = (float)T.scale * (l->row + l->zoom * (l->ky * (y + (float)row_lo[j] - l->gy) - l->kh * ((float)row_lo[j] - l->h)));
	float pb = (float)T.scale * (l->row + l->zoom * (l->ky * (y + (float)row_hi[j] - l->gy) - l->kh * ((float)row_hi[j] - l->h)));
	float reach = (float)T.scale * l->zoom * (l->ky * (RAMP_STEP + 1) > l->kh ? l->ky * (RAMP_STEP + 1) : l->kh) + 2.0f;
	return (pa > pb ? pa : pb) + reach >= (float)from && (pa < pb ? pa : pb) - 2.0f < (float)to;
}

static void faces_band(int from, int to, void *ctx) {
	const Faces *c = ctx;
	const VoxScene *s = c->s;
	const uint32_t *map = c->map;
	Lens l = *c->l, smooth = *c->l;
	l.clip0 = smooth.clip0 = from;
	l.clip1 = smooth.clip1 = to;
	smooth.bevel = 0;
	VoxSpan sp = c->sp;
	for (int j = sp.y0; j < sp.y1; ++j) {
		if (!row_reaches(&l, j, from, to)) continue;
		for (int i = sp.x0; i < sp.x1; ++i) {
			int at = j * VOX_WW + i;
			if (!map[at] || s->kind[at] == VOX_SKY) continue;
			if (s->kind[at] == VOX_TOP) {
				int y = j - VOX_UP, under = at + VOX_WW;
				bool on = j + 1 < sp.y1 && map[under] && s->kind[under] == VOX_TOP && s->gy[under] >= s->gy[at] && s->gy[under] - s->gy[at] <= RAMP_STEP;
				float fr[2] = { (float)s->gy[at], (float)(s->gy[at] - y) }, till[2] = { fr[0] + 1.0f, fr[1] };
				if (on) { till[0] = (float)s->gy[under]; till[1] = (float)(s->gy[under] - (y + 1)); }
				/* (a slope's rows are no voxels of their own: no edges on them) */
				bool slope = on || (j > sp.y0 && map[at - VOX_WW] && s->kind[at - VOX_WW] == VOX_TOP && s->gy[at - VOX_WW] - (y - 1) != s->gy[at] - y);
				top(slope ? &smooth : &l, i - VOX_LEFT, fr, till, map[at]);
			} else upright(&l, i - VOX_LEFT, j - VOX_UP, (float)s->gy[at], map[at], IS_MAP);
		}
	}
	for (int i = 0; i < VOX_N; ++i)
		if (c->sprite[i]) upright(&l, i % VOX_W, i / VOX_W, (float)s->bodies[s->body[i]].gy, c->sprite[i], IS_SPRITE);
}

static void faces(const VoxScene *s, const uint32_t *map, const uint32_t *sprite, const Lens *l) {
	VoxSpan sp = vox_span(s);
	for (int j = sp.y0; j < sp.y1; ++j) {
		int lo = 1 << 14, hi = -(1 << 14);
		for (int i = sp.x0; i < sp.x1; ++i) {
			int at = j * VOX_WW + i, h = s->gy[at] - (j - VOX_UP);
			if (!map[at] || s->kind[at] == VOX_SKY) continue;
			if (h < lo) lo = h;
			if (h > hi) hi = h;
		}
		row_lo[j] = (int16_t)lo;
		row_hi[j] = (int16_t)hi;
	}
	voxel_in_bands(faces_band, T.oh, &(Faces){ s, map, sprite, l, sp });
}

/* Seams closed: a top and its side, or a slope and what hangs from it,
 * can land a pixel of the frame apart under a lower camera, and the sky
 * showed through in dots (the owner, 2026-11-13). A run of pixels nothing
 * was drawn on, shorter than SEAM voxels, between something above and
 * below it takes the colour above; then one between something left and
 * right of it the colour on its left. Down each column first, kept a row
 * at a time (a pixel at a time, nine looks each way, it took 1.1 ms). */
static void mended(int i, int from) {
	T.out[i] = T.out[from];
	T.z[i] = T.z[from];
	T.what[i] = IS_MENDED;
}

static void mend_down(int from, int to, void *ctx) {
	int most = *(const int *)ctx;
	for (int ox = from; ox < to; ++ox) gap[ox] = -1;
	for (int oy = 0; oy < T.oh; ++oy)
		for (int ox = from; ox < to; ++ox) {
			int i = oy * T.ow + ox;
			if (T.what[i] == IS_SKY) { if (gap[ox] >= 0) ++gap[ox]; continue; }
			if (gap[ox] > 0 && gap[ox] <= most)
				for (int n = 1; n <= gap[ox]; ++n) mended(i - n * T.ow, i - (gap[ox] + 1) * T.ow);
			gap[ox] = 0;
		}
}

static void mend_across(int from, int to, void *ctx) {
	int most = *(const int *)ctx;
	for (int oy = from; oy < to; ++oy) {
		int run = -1;
		for (int ox = 0; ox < T.ow; ++ox) {
			int i = oy * T.ow + ox;
			if (T.what[i] == IS_SKY) { if (run >= 0) ++run; continue; }
			if (run > 0 && run <= most)
				for (int n = 1; n <= run; ++n) mended(i - n, i - run - 1);
			run = 0;
		}
		for (int i = oy * T.ow; i < (oy + 1) * T.ow; ++i)
			if (T.what[i] == IS_MENDED) T.what[i] = IS_MAP;
	}
}

static void mend(void) {
	int most = SEAM * T.scale - 1;
	voxel_in_bands(mend_down, T.ow, &most);
	voxel_in_bands(mend_across, T.oh, &most);
}

/* The sky where nothing stands: far off, it keeps BN6's place (and what
 * of the map lies beyond the floor with it) */
typedef struct { const VoxScene *s; const uint32_t *map; const Lens *l; VoxSpan sp; } Sky;

static void sky_rows(int from, int to, void *ctx) {
	const Sky *c = ctx;
	float inv = 1.0f / ((float)T.scale * c->l->zoom);
	for (int oy = from; oy < to; ++oy) {
		int wy = (int)floorf(c->l->row + ((float)oy + 0.5f - (float)T.scale * c->l->row) * inv) + VOX_UP;
		wy = wy < c->sp.y0 ? c->sp.y0 : wy >= c->sp.y1 ? c->sp.y1 - 1 : wy;
		for (int ox = 0; ox < T.ow; ++ox) {
			if (T.what[oy * T.ow + ox] != IS_SKY) continue;
			int at = wy * VOX_WW + sky_x[ox];
			T.out[oy * T.ow + ox] = c->s->kind[at] == VOX_SKY && c->map[at] ? c->map[at] : c->s->sky[at];
		}
	}
}

static void sky_fill(const VoxScene *s, const uint32_t *map, const Lens *l) {
	float inv = 1.0f / ((float)T.scale * l->zoom);
	VoxSpan sp = vox_span(s);
	for (int ox = 0; ox < T.ow; ++ox) {
		int wx = (int)floorf(l->x + ((float)ox + 0.5f - (float)T.scale * l->x) * inv) + VOX_LEFT;
		sky_x[ox] = wx < sp.x0 ? sp.x0 : wx >= sp.x1 ? sp.x1 - 1 : wx;
	}
	voxel_in_bands(sky_rows, T.oh, &(Sky){ s, map, l, sp });
}

/* ---- the soft layers, on the frame's 240x160 ---- */


static void (*bands)(VoxBand band, int rows, void *ctx);
void voxel_bands(void (*run)(VoxBand band, int rows, void *ctx)) { bands = run; }

void voxel_in_bands(VoxBand band, int rows, void *ctx) {
	if (bands) bands(band, rows, ctx);
	else band(0, rows, ctx);
}

/* The picture's blocks' mean colour, and how much of each the floor covers */
static void lows(int from, int to, void *ctx) {
	(void)ctx;
	float norm = 1.0f / (float)(T.scale * T.scale * 255);
	for (int y = from; y < to; ++y)
		for (int x = 0; x < VOX_W; ++x) {
			unsigned sum[3] = { 0, 0, 0 }, covered = 0;
			for (int sy = 0; sy < T.scale; ++sy)
				for (int sx = 0; sx < T.scale; ++sx) {
					int i = (y * T.scale + sy) * T.ow + x * T.scale + sx;
					uint32_t c = T.out[i];
					sum[0] += c & 0xFF; sum[1] += c >> 8 & 0xFF; sum[2] += c >> 16 & 0xFF;
					covered += T.what[i] != IS_SKY;
				}
			for (int c = 0; c < 3; ++c) low[(y * VOX_W + x) * 3 + c] = (float)sum[c] * norm;
			cover[y * VOX_W + x] = (float)covered / (float)(T.scale * T.scale);
		}
}

/* Box blurs, each of `ch` channels and radius `r` and its own scratch:
 * across each row, then down each column, the layers blurred together on
 * the same bands (one at a time, the threads were woken sixteen times) */
typedef struct { float *v, *tmp; int ch, r; } Box;
typedef struct { const Box *box; int n; } Boxes;

/* (four rows at a time, each channel's sum its own: one row and channel
 * at a time, each step waited on the last's sum) */
#define BOX_ROWS 4
static void box_rows(const Box *b, int y0, int rows) {
	int ch = b->ch, r = b->r, n = BOX_ROWS * ch;
	float norm = 1.0f / (float)(2 * r + 1), sum[BOX_ROWS * 3] = { 0 };
	const float *v[BOX_ROWS];
	float *t[BOX_ROWS];
	for (int q = 0; q < BOX_ROWS; ++q) {
		int y = y0 + (q < rows ? q : 0);
		v[q] = &b->v[y * VOX_W * ch];
		t[q] = &b->tmp[y * VOX_W * ch];
	}
	for (int x = -r; x <= r; ++x)
		for (int m = 0; m < n; ++m) sum[m] += v[m / ch][(x < 0 ? 0 : x) * ch + m % ch];
	for (int x = 0; x < VOX_W; ++x) {
		int in = (x + r + 1 < VOX_W ? x + r + 1 : VOX_W - 1) * ch, gone = (x - r > 0 ? x - r : 0) * ch;
		for (int m = 0; m < rows * ch; ++m) {
			int q = m / ch, c = m % ch;
			t[q][x * ch + c] = sum[m] * norm;
			sum[m] += v[q][in + c] - v[q][gone + c];
		}
	}
}

static void box_across(int from, int to, void *ctx) {
	const Boxes *all = ctx;
	for (int k = 0; k < all->n; ++k)
		for (int y0 = from; y0 < to; y0 += BOX_ROWS) box_rows(&all->box[k], y0, to - y0 < BOX_ROWS ? to - y0 : BOX_ROWS);
}

/* (down the columns a row at a time, each column's sum kept: a column at
 * a time strode across the rows and missed the cache at every step) */
static void box_down(int from, int to, void *ctx) {
	const Boxes *all = ctx;
	float sum[VOX_W * 3];
	for (int k = 0; k < all->n; ++k) {
		const Box *b = &all->box[k];
		int ch = b->ch, r = b->r, lo = from * ch, hi = to * ch;
		float norm = 1.0f / (float)(2 * r + 1), *v = b->v, *t = b->tmp;
		for (int n = lo; n < hi; ++n) sum[n] = 0;
		for (int y = -r; y <= r; ++y)
			for (int n = lo; n < hi; ++n) sum[n] += t[(y < 0 ? 0 : y) * VOX_W * ch + n];
		for (int y = 0; y < VOX_H; ++y) {
			const float *in = &t[(y + r + 1 < VOX_H ? y + r + 1 : VOX_H - 1) * VOX_W * ch], *gone = &t[(y - r > 0 ? y - r : 0) * VOX_W * ch];
			float *row = &v[y * VOX_W * ch];
			for (int n = lo; n < hi; ++n) {
				row[n] = sum[n] * norm;
				sum[n] += in[n] - gone[n];
			}
		}
	}
}

static void boxes(const Box *box, int n) {
	Boxes all = { box, n };
	voxel_in_bands(box_across, VOX_H, &all);
	voxel_in_bands(box_down, VOX_W, &all);
}

/* The glow: what is brighter than most, spread */
static void glows(int from, int to, void *ctx) {
	float bloom = *(const float *)ctx;
	for (int i = from * VOX_W; i < to * VOX_W; ++i) {
		const float *o = &low[i * 3];
		float most = o[0] > o[1] ? (o[0] > o[2] ? o[0] : o[2]) : (o[1] > o[2] ? o[1] : o[2]);
		float over = cover[i] * (most - (o[0] < o[1] ? (o[0] < o[2] ? o[0] : o[2]) : (o[1] < o[2] ? o[1] : o[2])) * 0.75f - 0.62f) * 4.0f;
		over = over < 0 ? 0 : over > 1 ? 1 : over;
		for (int c = 0; c < 3; ++c) glow[i * 3 + c] = o[c] * over * bloom;
	}
}

static uint32_t packed(const float *c) {
	uint32_t out = 0;
	for (int i = 0; i < 3; ++i) {
		int v = (int)(c[i] * 255.0f + 0.5f);
		out |= (uint32_t)(v > 255 ? 255 : v < 0 ? 0 : v) << (8 * i);
	}
	return out;
}

static void packs(int from, int to, void *ctx) {
	(void)ctx;
	for (int i = from * VOX_W; i < to * VOX_W; ++i) {
		soft8[i] = packed(&soft[i * 3]);
		glow8[i] = packed(&glow[i * 3]);
	}
}

static void layers(const VoxLook *k) {
	voxel_in_bands(lows, VOX_H, NULL);
	voxel_in_bands(glows, VOX_H, &(float){ k->bloom });
	memcpy(soft, low, sizeof *soft * VOX_N * 3);
	Box glow_once = { glow, tmp, 3, 3 }, all[3] = { { glow, tmp, 3, 3 }, { soft, tmp + VOX_N * 3, 3, 2 }, { cover, tmp + VOX_N * 6, 1, 5 } };
	boxes(&glow_once, 1);
	boxes(all, 3);
	voxel_in_bands(packs, VOX_H, NULL);
}

/* Colours packed 0x00BBGGRR worked a pair of channels at a time: `a`
 * toward `b` by w 256ths, each channel by f 256ths, the sum held at 255 */
static inline uint32_t toward(uint32_t a, uint32_t b, unsigned w) {
	uint32_t rb = ((a & 0xFF00FFu) * (256u - w) + (b & 0xFF00FFu) * w) >> 8 & 0xFF00FFu;
	return rb | (((a & 0xFF00u) * (256u - w) + (b & 0xFF00u) * w) >> 8 & 0xFF00u);
}

static inline uint32_t times(uint32_t c, unsigned f) {
	return ((c & 0xFF00FFu) * f >> 8 & 0xFF00FFu) | ((c & 0xFF00u) * f >> 8 & 0xFF00u);
}

static inline uint32_t plus(uint32_t a, uint32_t b) {
	uint32_t rb = (a & 0xFF00FFu) + (b & 0xFF00FFu), g = (a & 0xFF00u) + (b & 0xFF00u), over = rb & 0x1000100u, gover = g & 0x10000u;
	return ((rb | (over - (over >> 8))) & 0xFF00FFu) | ((g | (gover - (gover >> 8))) & 0xFF00u);
}

/* A soft layer at a pixel of the picture, between the frame's four about it */
static inline uint32_t between(const uint32_t *v, const Tap *x, const Tap *y) {
	const uint32_t *r0 = &v[y->a * VOX_W], *r1 = &v[y->b * VOX_W];
	uint32_t a = r0[x->a], b = r0[x->b], c = r1[x->a], d = r1[x->b];
	if (!(a | b | c | d)) return 0;
	return toward(toward(a, b, x->w), toward(c, d, x->w), y->w);
}

typedef struct { const VoxLook *k; const Lens *l; } Finish;

/* The soft layers over the picture: what is out of focus blurred, the
 * floor's shadow on the sky (dimmer under the floor, whose shadow lies far
 * below it, down and to the right), the glow, the vignette. In whole
 * numbers, a pair of channels at a time: in floats, each of its channels
 * apart, it was the picture's dearest pass. */
static void finish(int from, int to, void *ctx) {
	const VoxLook *k = ((const Finish *)ctx)->k;
	const Lens *l = ((const Finish *)ctx)->l;
	for (int oy = from; oy < to; ++oy) {
		const Tap *y = &down[oy];
		const float *shade = y->shade < 0 ? NULL : &cover[y->shade * VOX_W];
		for (int ox = 0; ox < T.ow; ++ox) {
			int i = oy * T.ow + ox;
			const Tap *x = &across[ox];
			uint32_t c = T.out[i];
			float mix, light = 1.0f, vig = 1.0f - 0.09f * (x->vig + y->vig);
			if (T.what[i] == IS_SKY) {
				mix = 0.62f;
				light = k->sky * (1.0f - 0.40f * (shade && x->shade >= 0 ? shade[x->shade] : 0.0f));
			} else {
				mix = fabsf(T.z[i] - l->near0) * k->focus - 0.22f;
				mix = mix < 0 ? 0 : mix > 0.85f ? 0.85f : mix;
			}
			if (mix > 0) c = toward(c, between(soft8, x, y), (unsigned)(mix * 256.0f + 0.5f));
			c = times(c, (unsigned)(light * vig * 256.0f + 0.5f));
			uint32_t gl = between(glow8, x, y);
			T.out[i] = 0xFF000000u | (gl ? plus(c, times(gl, (unsigned)(vig * 256.0f + 0.5f))) : c);
		}
	}
}

bool voxel_draw(const VoxScene *scene, const uint32_t *map, const uint32_t *sprite, const VoxLook *look, const VoxCamera *cam, int scale, uint32_t *out) {
	if (!target(scale, out)) return false;
	Lens l = lens(cam, look);
	faces(scene, map, sprite, &l);
	if (cam->pitch != 30.0f || cam->zoom != 1.0f || cam->depth != 0.0f) mend();
	sky_fill(scene, map, &l);
	if (look->debug) return true;
	layers(look);
	voxel_in_bands(finish, T.oh, &(Finish){ look, &l });
	return true;
}
