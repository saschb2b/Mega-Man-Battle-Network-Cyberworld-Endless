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

enum { IS_SKY, IS_MAP, IS_SPRITE };

/* The picture being made: each pixel's nearness (the larger the nearer)
 * and what it shows */
static struct { int scale, ow, oh; float *z; uint8_t *what; uint32_t *out; } T;

static bool target(int scale, uint32_t *out) {
	if (T.scale != scale) {
		free(T.z);
		free(T.what);
		T.scale = scale;
		T.ow = VOX_W * scale;
		T.oh = VOX_H * scale;
		T.z = malloc((size_t)T.ow * (size_t)T.oh * sizeof *T.z);
		T.what = malloc((size_t)T.ow * (size_t)T.oh);
		if (!T.z || !T.what) { T.scale = 0; return false; }
	}
	T.out = out;
	for (int i = 0; i < T.ow * T.oh; ++i) T.z[i] = -1e9f;
	memset(T.what, IS_SKY, (size_t)T.ow * (size_t)T.oh);
	return true;
}

/* ---- the camera ---- */

typedef struct { float ky, kh, cn, sn, zoom, depth, x, gy, h, row, near0, bevel; bool whole; } Lens;

static float nearness(const Lens *l, float g, float h) { return 2.0f * g * l->cn + h / COS30 * l->sn; }

static Lens lens(const VoxCamera *c, const VoxLook *k) {
	float a = c->pitch * RAD;
	Lens l = { sinf(a) / 0.5f, cosf(a) / COS30, cosf(a), sinf(a), c->zoom, c->depth, c->x, c->gy, c->h, c->gy - c->h, 0, k->debug ? 0 : k->bevel,
		c->depth == 0.0f };
	l.near0 = nearness(&l, c->gy, c->h);
	return l;
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

/* A face between two corners of the picture, nearer than what is there:
 * its colour, its edges lit up and left and shaded down and right */
static void face(const Lens *l, const float a[2], const float b[2], float near, uint32_t c, int what) {
	int x0, y0, x1, y1;
	if (l->whole) { x0 = (int)floorf(a[0] + 0.5f); y0 = (int)floorf(a[1] + 0.5f); x1 = (int)floorf(b[0] + 0.5f); y1 = (int)floorf(b[1] + 0.5f); }
	else { x0 = (int)floorf(a[0] < b[0] ? a[0] : b[0]); y0 = (int)floorf(a[1]); x1 = (int)ceilf(a[0] < b[0] ? b[0] : a[0]); y1 = (int)ceilf(b[1]); }
	bool edged = l->bevel > 0 && x1 - x0 >= 3 && y1 - y0 >= 3;
	uint32_t lit = edged ? scaled(c, 1.0f + l->bevel) : c, dim = edged ? scaled(c, 1.0f - l->bevel) : c;
	int fx = x0 < 0 ? 0 : x0, fy = y0 < 0 ? 0 : y0, tx = x1 > T.ow ? T.ow : x1, ty = y1 > T.oh ? T.oh : y1;
	for (int y = fy; y < ty; ++y)
		for (int x = fx; x < tx; ++x) {
			int i = y * T.ow + x;
			if (near <= T.z[i]) continue;
			T.z[i] = near;
			T.what[i] = (uint8_t)what;
			T.out[i] = y == y0 || x == x0 ? lit : y == y1 - 1 || x == x1 - 1 ? dim : c;
		}
}

/* A floor's top at (x, g) of height h: a ground row deep */
static void top(const Lens *l, int x, float g, float h, uint32_t c) {
	float a[2], b[2];
	project(l, (float)x, g, h, &a[0], &a[1]);
	project(l, (float)(x + 1), g + 1.0f, h, &b[0], &b[1]);
	face(l, a, b, nearness(l, g + 0.5f, h), c, IS_MAP);
}

/* What stands at ground row g, the frame's pixel (x, y) of it: a row high */
static void upright(const Lens *l, int x, int y, float g, uint32_t c, int what) {
	float a[2], b[2], h = g - (float)y;
	project(l, (float)x, g, h, &a[0], &a[1]);
	project(l, (float)(x + 1), g, h - 1.0f, &b[0], &b[1]);
	face(l, a, b, nearness(l, g, h - 0.5f) + (what == IS_SPRITE ? 0.6f : 0.0f), c, what);
}

static void faces(const VoxScene *s, const uint32_t *map, const uint32_t *sprite, const Lens *l) {
	for (int j = 0; j < VOX_WH; ++j)
		for (int i = 0; i < VOX_WW; ++i) {
			int at = j * VOX_WW + i;
			if (!map[at] || s->kind[at] == VOX_SKY) continue;
			if (s->kind[at] == VOX_TOP) top(l, i - VOX_LEFT, (float)s->gy[at], (float)(s->gy[at] - (j - VOX_UP)), map[at]);
			else upright(l, i - VOX_LEFT, j - VOX_UP, (float)s->gy[at], map[at], IS_MAP);
		}
	for (int i = 0; i < VOX_N; ++i)
		if (sprite[i]) upright(l, i % VOX_W, i / VOX_W, (float)s->bodies[s->body[i]].gy, sprite[i], IS_SPRITE);
}

/* The sky where nothing stands: far off, it keeps BN6's place (and what
 * of the map lies beyond the floor with it) */
static void sky_fill(const VoxScene *s, const uint32_t *map, const Lens *l) {
	float inv = 1.0f / ((float)T.scale * l->zoom);
	for (int oy = 0; oy < T.oh; ++oy) {
		int wy = (int)floorf(l->row + ((float)oy + 0.5f - (float)T.scale * l->row) * inv) + VOX_UP;
		wy = wy < 0 ? 0 : wy >= VOX_WH ? VOX_WH - 1 : wy;
		for (int ox = 0; ox < T.ow; ++ox) {
			if (T.what[oy * T.ow + ox] != IS_SKY) continue;
			int wx = (int)floorf(l->x + ((float)ox + 0.5f - (float)T.scale * l->x) * inv) + VOX_LEFT;
			wx = wx < 0 ? 0 : wx >= VOX_WW ? VOX_WW - 1 : wx;
			int at = wy * VOX_WW + wx;
			T.out[oy * T.ow + ox] = s->kind[at] == VOX_SKY && map[at] ? map[at] : s->sky[at];
		}
	}
}

/* ---- the soft layers, on the frame's 240x160 ---- */

static float low[VOX_N * 3], soft[VOX_N * 3], glow[VOX_N * 3], tmp[VOX_N * 3], cover[VOX_N];

static void box(float *v, int ch, int r) {
	float norm = 1.0f / (float)(2 * r + 1);
	for (int y = 0; y < VOX_H; ++y)
		for (int c = 0; c < ch; ++c) {
			float sum = 0;
			for (int x = -r; x <= r; ++x) sum += v[(y * VOX_W + (x < 0 ? 0 : x)) * ch + c];
			for (int x = 0; x < VOX_W; ++x) {
				tmp[(y * VOX_W + x) * ch + c] = sum * norm;
				int in = x + r + 1 < VOX_W ? x + r + 1 : VOX_W - 1, gone = x - r > 0 ? x - r : 0;
				sum += v[(y * VOX_W + in) * ch + c] - v[(y * VOX_W + gone) * ch + c];
			}
		}
	for (int x = 0; x < VOX_W; ++x)
		for (int c = 0; c < ch; ++c) {
			float sum = 0;
			for (int y = -r; y <= r; ++y) sum += tmp[((y < 0 ? 0 : y) * VOX_W + x) * ch + c];
			for (int y = 0; y < VOX_H; ++y) {
				v[(y * VOX_W + x) * ch + c] = sum * norm;
				int in = y + r + 1 < VOX_H ? y + r + 1 : VOX_H - 1, gone = y - r > 0 ? y - r : 0;
				sum += tmp[(in * VOX_W + x) * ch + c] - tmp[(gone * VOX_W + x) * ch + c];
			}
		}
}

/* The picture's blocks' mean colour, and how much of each the floor covers */
static void lows(void) {
	float norm = 1.0f / (float)(T.scale * T.scale * 255);
	for (int y = 0; y < VOX_H; ++y)
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

static void layers(const VoxLook *k) {
	lows();
	/* (the glow: what is brighter than most, spread) */
	for (int i = 0; i < VOX_N; ++i) {
		const float *o = &low[i * 3];
		float most = o[0] > o[1] ? (o[0] > o[2] ? o[0] : o[2]) : (o[1] > o[2] ? o[1] : o[2]);
		float over = cover[i] * (most + (o[0] + o[1] + o[2]) / 3.0f - 1.62f) * 4.0f;
		over = over < 0 ? 0 : over > 1 ? 1 : over;
		for (int c = 0; c < 3; ++c) glow[i * 3 + c] = o[c] * over * k->bloom;
	}
	box(glow, 3, 3);
	box(glow, 3, 3);
	memcpy(soft, low, sizeof soft);
	box(soft, 3, 2);
	box(cover, 1, 5);
}

/* A soft layer at the frame's place (u, v), between its four pixels */
static inline void between(const float *v, float u, float w, float out[3]) {
	u -= 0.5f;
	w -= 0.5f;
	int x0 = (int)floorf(u), y0 = (int)floorf(w);
	float fx = u - (float)x0, fy = w - (float)y0;
	int x1 = x0 + 1 < VOX_W ? x0 + 1 : VOX_W - 1, y1 = y0 + 1 < VOX_H ? y0 + 1 : VOX_H - 1;
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	const float *a = &v[(y0 * VOX_W + x0) * 3], *b = &v[(y0 * VOX_W + x1) * 3], *c = &v[(y1 * VOX_W + x0) * 3], *d = &v[(y1 * VOX_W + x1) * 3];
	for (int i = 0; i < 3; ++i) out[i] = (a[i] * (1 - fx) + b[i] * fx) * (1 - fy) + (c[i] * (1 - fx) + d[i] * fx) * fy;
}

static inline uint32_t pack(const float c[3], float f) {
	uint32_t out = 0xFF000000u;
	for (int i = 0; i < 3; ++i) {
		int v = (int)(c[i] * f * 255.0f + 0.5f);
		out |= (uint32_t)(v > 255 ? 255 : v < 0 ? 0 : v) << (8 * i);
	}
	return out;
}

/* The sky's share of light at the frame's (u, v): dimmer under the floor,
 * whose shadow lies far below it, down and to the right */
static float sky_light(const VoxLook *k, float u, float v) {
	int sx = (int)u - 8, sy = (int)v - 13;
	float under = sx < 0 || sy < 0 ? 0.0f : cover[sy * VOX_W + sx];
	return k->sky * (1.0f - 0.40f * under);
}

static void finish(const VoxLook *k, const Lens *l) {
	float inv = 1.0f / (float)T.scale;
	for (int oy = 0; oy < T.oh; ++oy) {
		float v = ((float)oy + 0.5f) * inv, vy = (v / (float)VOX_H - 0.5f) * 2.0f;
		for (int ox = 0; ox < T.ow; ++ox) {
			int i = oy * T.ow + ox;
			uint32_t c = T.out[i];
			float u = ((float)ox + 0.5f) * inv, rgb[3] = { (float)(c & 0xFF) / 255.0f, (float)(c >> 8 & 0xFF) / 255.0f, (float)(c >> 16 & 0xFF) / 255.0f };
			float s[3], gl[3], mix, light = 1.0f;
			if (T.what[i] == IS_SKY) { mix = 0.62f; light = sky_light(k, u, v); }
			else {
				mix = fabsf(T.z[i] - l->near0) * k->focus - 0.22f;
				mix = mix < 0 ? 0 : mix > 0.85f ? 0.85f : mix;
			}
			if (mix > 0) {
				between(soft, u, v, s);
				for (int n = 0; n < 3; ++n) rgb[n] += (s[n] - rgb[n]) * mix;
			}
			between(glow, u, v, gl);
			for (int n = 0; n < 3; ++n) rgb[n] = rgb[n] * light + gl[n];
			float vx = (u / (float)VOX_W - 0.5f) * 2.0f;
			T.out[i] = pack(rgb, 1.0f - 0.09f * (vx * vx + vy * vy));
		}
	}
}

bool voxel_draw(const VoxScene *scene, const uint32_t *map, const uint32_t *sprite, const VoxLook *look, const VoxCamera *cam, int scale, uint32_t *out) {
	if (!target(scale, out)) return false;
	Lens l = lens(cam, look);
	faces(scene, map, sprite, &l);
	sky_fill(scene, map, &l);
	if (look->debug) return true;
	layers(look);
	finish(look, &l);
	return true;
}
