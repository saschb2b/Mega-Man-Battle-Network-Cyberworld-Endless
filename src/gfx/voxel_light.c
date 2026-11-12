/* voxel.h's light: each pixel of the scene under the sun
 * (docs/engine/voxel.md, The light). */
#include "voxel.h"

#include <math.h>

#define SLAB_THICK   8      /* a slab's body under its top, in rows: what it shades */
#define SUN_STEPS    48
#define BODY_REACH   90.0f  /* how far a sprite's shadow is looked for, in ground rows */
#define BODY_DEEP    3      /* a body's depth: its outline again this many rows behind */

VoxLook voxel_look_default(void) {
	return (VoxLook){ .sun_x = 0.40f, .sun_g = 0.46f, .sun_h = 0.95f, .ambient = 0.50f, .wall = 0.86f, .bloom = 0.20f,
		.focus = 0.0045f, .sky = 0.80f, .bevel = 0.045f, .debug = 0 };
}

/* Whether the point (x, gy, h) sees the sun: no sprite and no slab's body
 * on its way there */
static bool sunlit(const VoxScene *s, const VoxGround *g, const VoxLook *k, float x, float gy, float h, int self) {
	for (int id = 1; id <= s->nbodies; ++id) {
		const VoxBody *b = &s->bodies[id];
		if (id == self) continue;
		for (int deep = 0; deep < BODY_DEEP; ++deep) {
			float d = (float)(b->gy - deep) - gy;
			if (d <= 0.25f || d > BODY_REACH) continue;
			float t = d / k->sun_g;
			int px = (int)floorf(x - k->sun_x * t + 0.5f), py = (int)floorf((float)b->gy - (h + k->sun_h * t));
			if (px < b->x0 || px > b->x1 || py < b->y0 || py > b->y1) continue;
			if (s->body[py * VOX_W + px] == id) return false;
		}
	}
	for (int l = 1; l < VOX_LEVELS; ++l) {
		if (g->z[l] <= 0 || h >= (float)g->z[l]) continue;
		for (int n = 1; n <= SUN_STEPS; ++n) {
			float t = (float)n, hh = h + k->sun_h * t;
			if (hh >= (float)g->z[l]) break;
			if (hh > (float)(g->z[l] - SLAB_THICK) && (vox_floor(g, (int)floorf(x - k->sun_x * t + 0.5f), (int)floorf(gy + k->sun_g * t)) >> l & 1u)) return false;
		}
	}
	return true;
}

/* How much of the sun reaches (x, gy, h): three rays a little apart, a
 * shadow's edge in thirds */
static float sun_at(const VoxScene *s, const VoxGround *g, const VoxLook *k, float x, float gy, float h, int self) {
	float sun = 0;
	for (int n = -1; n <= 1; ++n) sun += sunlit(s, g, k, x + 0.45f * (float)n, gy, h, self) ? 1.0f / 3.0f : 0.0f;
	return sun;
}

/* The floor darker where a body stands on it */
static float contact(const VoxScene *s, int x, float gy) {
	float dark = 0;
	for (int id = 1; id <= s->nbodies; ++id) {
		const VoxBody *b = &s->bodies[id];
		float r = (float)(b->x1 - b->x0 + 1) * 0.55f, dx = (float)x - (float)(b->x0 + b->x1) * 0.5f, dg = 2.0f * (gy - (float)b->gy + 1.0f);
		float d = (dx * dx + dg * dg) / (r * r);
		if (d < 1.0f && 1.0f - d > dark) dark = 1.0f - d;
	}
	return 1.0f - 0.34f * dark;
}

static uint32_t shaded(uint32_t c, float light, float sun) {
	/* (a shadow is the sky's light: a little blue) */
	float cool = 1.0f - sun;
	float r = (float)(c & 0xFF) * light * (1.0f - 0.10f * cool), gr = (float)(c >> 8 & 0xFF) * light * (1.0f - 0.03f * cool), b = (float)(c >> 16 & 0xFF) * light * (1.0f + 0.10f * cool);
	int ir = (int)(r + 0.5f), ig = (int)(gr + 0.5f), ib = (int)(b + 0.5f);
	return 0xFF000000u | (uint32_t)(ib > 255 ? 255 : ib) << 16 | (uint32_t)(ig > 255 ? 255 : ig) << 8 | (uint32_t)(ir > 255 ? 255 : ir);
}

static const uint32_t kinds[3] = { 0xFF4D1F1Au, 0xFF59D940u, 0xFF2673E6u };

void voxel_light(const VoxScene *s, const VoxGround *g, const VoxLook *k, uint32_t *map, uint32_t *sprite) {
	for (int j = 0; j < VOX_WH; ++j)
		for (int i = 0; i < VOX_WW; ++i) {
			int at = j * VOX_WW + i, kind = s->kind[at], x = i - VOX_LEFT, y = j - VOX_UP;
			uint32_t c = s->map[at];
			if (!c || kind == VOX_SKY) { map[at] = k->debug && c ? kinds[0] : c; continue; }
			if (k->debug) { map[at] = shaded(kinds[kind], (float)(0.4 + 0.6 * ((c & 0xFF) + (c >> 8 & 0xFF)) / 510.0), 1.0f); continue; }
			float h = (float)(s->gy[at] - y), face = 1.0f, sun;
			if (kind == VOX_TOP) {
				sun = sun_at(s, g, k, (float)x, (float)s->gy[at] + 0.5f, h + 0.5f, 0);
				face = contact(s, x, (float)s->gy[at]);
				/* (the edge that catches the light) */
				if (j + 1 < VOX_WH && s->kind[at + VOX_WW] == VOX_WALL) face *= 1.10f;
			} else {
				/* (a side: less of the sun, and less of the sky's light the deeper under its edge) */
				float under = h < 0 ? -h : 0;
				sun = 0.9f * sun_at(s, g, k, (float)x, (float)s->gy[at], h + 0.5f, 0);
				face = k->wall * (1.0f - 0.011f * (under > 30.0f ? 30.0f : under));
			}
			map[at] = shaded(c, face * (k->ambient + (1.0f - k->ambient) * sun), sun);
		}
	for (int i = 0; i < VOX_N; ++i) {
		uint32_t c = s->sprite[i];
		if (!c) { sprite[i] = 0; continue; }
		if (k->debug) { sprite[i] = 0xFF40E6F2u; continue; }
		const VoxBody *b = &s->bodies[s->body[i]];
		float sun = sun_at(s, g, k, (float)(i % VOX_W), (float)b->gy, (float)(b->gy - i / VOX_W) + 0.5f, s->body[i]);
		/* (one who stands in a shadow keeps more of its light than the floor: it is the eye's mark) */
		sprite[i] = shaded(c, 0.72f + 0.28f * sun, sun);
	}
}
