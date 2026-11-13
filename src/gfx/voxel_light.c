/* voxel.h's light: each pixel of the scene under the sun
 * (docs/engine/voxel.md, The light). */
#include "voxel.h"

#include <math.h>
#include <stdlib.h>

#define SLAB_THICK   8      /* a slab's body under its top, in rows: what it shades */
#define SUN_STEPS    48
#define BODY_REACH   90.0f  /* how far a sprite's shadow is looked for, in ground rows */
#define BODY_DEEP    3      /* a body's depth: its outline again this many rows behind */
#define FADE_WAY     120.0f /* the way along which a shadow fades to nothing, in steps toward the sun */
#define SOFT         2      /* a shadow's edge: the rows and columns it is spread over, each way */

VoxLook voxel_look_default(void) {
	return (VoxLook){ .sun_x = 0.40f, .sun_g = 0.46f, .sun_h = 0.95f, .ambient = 0.62f, .wall = 0.86f, .bloom = 0.20f,
		.focus = 0.0045f, .sky = 0.80f, .bevel = 0.045f, .debug = 0 };
}

/* What is left of a shadow after `t` steps of its way */
static float left_after(float t) {
	float left = 1.0f - t / FADE_WAY;
	return left < 0.25f ? 0.25f : left;
}

/* How much of the sun the point (x, gy, h) loses to a sprite on its way
 * there: all of it right behind the body, less the farther its shadow
 * has travelled (the sky's light fills a long shadow in) */
static float body_shade(const VoxScene *s, const VoxLook *k, float x, float gy, float h, int self) {
	float most = 0;
	for (int id = 1; id <= s->nbodies; ++id) {
		const VoxBody *b = &s->bodies[id];
		if (id == self) continue;
		for (int deep = 0; deep < BODY_DEEP; ++deep) {
			float d = (float)(b->gy - deep) - gy;
			if (d <= 0.25f || d > BODY_REACH) continue;
			float t = d / k->sun_g;
			int px = (int)floorf(x - k->sun_x * t + 0.5f), py = (int)floorf((float)b->gy - (h + k->sun_h * t));
			if (px < b->x0 || px > b->x1 || py < b->y0 || py > b->y1 || s->body[py * VOX_W + px] != id) continue;
			if (left_after(t) > most) most = left_after(t);
		}
	}
	return most;
}

/* ... and to a raised slab's body or a stair's */
static float floor_shade(const VoxGround *g, const VoxLook *k, float x, float gy, float h) {
	/* (a stair casts on what lies beside it, not on its own steps) */
	bool on_stair = vox_ramp(g, (int)floorf(x + 0.5f), (int)floorf(gy)) != 0;
	for (int l = 1; l < VOX_LEVELS; ++l) {
		if (g->z[l] <= 0 || h >= (float)g->z[l]) continue;
		for (int n = 1; n <= SUN_STEPS; ++n) {
			float t = (float)n, hh = h + k->sun_h * t;
			if (hh >= (float)g->z[l]) break;
			int cx = (int)floorf(x - k->sun_x * t + 0.5f), cg = (int)floorf(gy + k->sun_g * t), ramp = vox_ramp(g, cx, cg) - 1;
			bool slab = hh > (float)(g->z[l] - SLAB_THICK) && (vox_floor(g, cx, cg) >> l & 1u);
			if (slab || (!on_stair && ramp > 2 && hh < (float)(ramp - 2) && hh > (float)(ramp - SLAB_THICK))) return left_after(t);
		}
	}
	return 0;
}

static float shade_at(const VoxScene *s, const VoxGround *g, const VoxLook *k, float x, float gy, float h, int self) {
	float a = body_shade(s, k, x, gy, h, self), b = floor_shade(g, k, x, gy, h);
	return a > b ? a : b;
}

/* The floor a little darker where a body stands on it */
static float contact(const VoxScene *s, int x, float gy) {
	float dark = 0;
	for (int id = 1; id <= s->nbodies; ++id) {
		const VoxBody *b = &s->bodies[id];
		float r = (float)(b->x1 - b->x0 + 1) * 0.55f, dx = (float)x - (float)(b->x0 + b->x1) * 0.5f, dg = 2.0f * (gy - (float)b->gy + 1.0f);
		float d = (dx * dx + dg * dg) / (r * r);
		if (d < 1.0f && 1.0f - d > dark) dark = 1.0f - d;
	}
	return 1.0f - 0.22f * dark * dark;
}

static uint32_t shaded(uint32_t c, float light, float sun) {
	/* (a shadow is the sky's light: a little blue) */
	float cool = 1.0f - sun;
	float r = (float)(c & 0xFF) * light * (1.0f - 0.05f * cool), gr = (float)(c >> 8 & 0xFF) * light * (1.0f - 0.02f * cool), b = (float)(c >> 16 & 0xFF) * light * (1.0f + 0.05f * cool);
	int ir = (int)(r + 0.5f), ig = (int)(gr + 0.5f), ib = (int)(b + 0.5f);
	return 0xFF000000u | (uint32_t)(ib > 255 ? 255 : ib) << 16 | (uint32_t)(ig > 255 ? 255 : ig) << 8 | (uint32_t)(ir > 255 ? 255 : ir);
}

static const uint32_t kinds[3] = { 0xFF4D1F1Au, 0xFF59D940u, 0xFF2673E6u };

typedef struct { const VoxScene *s; const VoxGround *g; const VoxLook *k; uint32_t *map; } Light;

/* The window's shadow, each pixel's share of the sun lost, and its spread */
static float *lost, *spread;

static void lost_rows(int from, int to, void *ctx) {
	const Light *c = ctx;
	const VoxScene *s = c->s;
	for (int j = from; j < to; ++j)
		for (int i = 0; i < VOX_WW; ++i) {
			int at = j * VOX_WW + i, kind = s->kind[at];
			lost[at] = 0;
			if (!s->map[at] || kind == VOX_SKY || c->k->debug) continue;
			float h = (float)(s->gy[at] - (j - VOX_UP));
			lost[at] = shade_at(s, c->g, c->k, (float)(i - VOX_LEFT), (float)s->gy[at] + (kind == VOX_TOP ? 0.5f : 0.0f), h + 0.5f, 0);
		}
}

/* A shadow's soft edge: each pixel the mean of those about it, across
 * then down */
static void soften(void) {
	float norm = 1.0f / (float)(2 * SOFT + 1);
	for (int j = 0; j < VOX_WH; ++j)
		for (int i = 0; i < VOX_WW; ++i) {
			float sum = 0;
			for (int d = -SOFT; d <= SOFT; ++d) sum += lost[j * VOX_WW + (i + d < 0 ? 0 : i + d >= VOX_WW ? VOX_WW - 1 : i + d)];
			spread[j * VOX_WW + i] = sum * norm;
		}
	for (int j = 0; j < VOX_WH; ++j)
		for (int i = 0; i < VOX_WW; ++i) {
			float sum = 0;
			for (int d = -SOFT; d <= SOFT; ++d) sum += spread[(j + d < 0 ? 0 : j + d >= VOX_WH ? VOX_WH - 1 : j + d) * VOX_WW + i];
			lost[j * VOX_WW + i] = sum * norm;
		}
}

static void light_rows(int from, int to, void *ctx) {
	const Light *c = ctx;
	const VoxScene *s = c->s;
	const VoxLook *k = c->k;
	for (int j = from; j < to; ++j)
		for (int i = 0; i < VOX_WW; ++i) {
			int at = j * VOX_WW + i, kind = s->kind[at], x = i - VOX_LEFT, y = j - VOX_UP;
			uint32_t px = s->map[at];
			if (!px || kind == VOX_SKY) { c->map[at] = k->debug && px ? kinds[0] : px; continue; }
			if (k->debug) { c->map[at] = shaded(kinds[kind], (float)(0.4 + 0.6 * ((px & 0xFF) + (px >> 8 & 0xFF)) / 510.0), 1.0f); continue; }
			float h = (float)(s->gy[at] - y), face = 1.0f, sun = 1.0f - lost[at];
			if (kind == VOX_TOP) {
				face = contact(s, x, (float)s->gy[at]);
				/* (the edge that catches the light) */
				if (j + 1 < VOX_WH && s->kind[at + VOX_WW] == VOX_WALL) face *= 1.10f;
			} else {
				/* (a side: less of the sun, and less of the sky's light the deeper under its edge) */
				float under = h < 0 ? -h : 0;
				sun *= 0.9f;
				face = k->wall * (1.0f - 0.011f * (under > 30.0f ? 30.0f : under));
			}
			c->map[at] = shaded(px, face * (k->ambient + (1.0f - k->ambient) * sun), sun);
		}
}

void voxel_light(const VoxScene *s, const VoxGround *g, const VoxLook *k, uint32_t *map, uint32_t *sprite) {
	if (!lost) {
		lost = malloc(sizeof *lost * VOX_WN * 2);
		if (!lost) return;
		spread = lost + VOX_WN;
	}
	Light light = { s, g, k, map };
	voxel_in_bands(lost_rows, VOX_WH, &light);
	soften();
	voxel_in_bands(light_rows, VOX_WH, &light);
	for (int i = 0; i < VOX_N; ++i) {
		uint32_t c = s->sprite[i];
		if (!c) { sprite[i] = 0; continue; }
		if (k->debug) { sprite[i] = 0xFF40E6F2u; continue; }
		const VoxBody *b = &s->bodies[s->body[i]];
		float sun = 1.0f - shade_at(s, g, k, (float)(i % VOX_W), (float)b->gy, (float)(b->gy - i / VOX_W) + 0.5f, s->body[i]);
		/* (one who stands in a shadow keeps more of its light than the floor: it is the eye's mark) */
		sprite[i] = shaded(c, 0.80f + 0.20f * sun, sun);
	}
}
