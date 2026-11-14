#include "stereo.h"

#include <math.h>
#include <string.h>

#include "gba_tiles.h"

/* Where BN6 keeps what is no part of the world: its HP in the corner (as
 * voxel_lift.c's) */
#define HP_RIGHT  48
#define HP_BOTTOM 24

static bool hud_obj(const GbaLayers *l, int n, int hud_rows) {
	if (!l->w[n]) return false;
	if (l->x[n] + l->w[n] <= HP_RIGHT && l->y[n] + l->h[n] <= HP_BOTTOM) return true;
	return hud_rows > 0 ? l->y[n] >= hud_rows : hud_rows < 0 && l->y[n] + l->h[n] <= -hud_rows;
}

/* Whether a background in front of a sprite of priority `p` has a pixel at (x, y) */
static bool covered(const GbaVideo *v, const GbaBg bg[4], int x, int y, unsigned p) {
	for (int b = 0; b < 4; ++b)
		if (bg[b].on && bg[b].prio < p && gba_bg_pixel(v->vram, &bg[b], x, y)) return true;
	return false;
}

/* The HUD's backgrounds: BG0, and BG3 where it stands in front of the map's (a battle's Custom screen) */
static void hud_backgrounds(const GbaVideo *v, const GbaBg bg[4], StereoFrame *out) {
	uint8_t row[STEREO_W];
	bool front3 = gba_bg3_front(v->io);
	for (int b = 0; b < 4; b += 3) {
		if (!bg[b].on || (b == 3 && !front3)) continue;
		for (int y = 0; y < STEREO_H; ++y) {
			gba_screen_row(v->vram, &bg[b], 0, y, STEREO_W, row);
			for (int x = 0; x < STEREO_W; ++x)
				if (row[x]) out->what[y * STEREO_W + x] = STEREO_FLAT;
		}
	}
}

/* One body: the standing pixels joined to (x, y), 8-connected */
static void body_fill(const uint8_t *stand, StereoFrame *out, int x, int y, int id) {
	static int16_t stack[STEREO_N * 2];
	StereoBox *b = &out->body[id];
	int n = 0;
	stack[n++] = (int16_t)x;
	stack[n++] = (int16_t)y;
	out->what[y * STEREO_W + x] = (uint8_t)(STEREO_BODY + id);
	while (n) {
		y = stack[--n];
		x = stack[--n];
		if (x < b->x0) b->x0 = (int16_t)x;
		if (x > b->x1) b->x1 = (int16_t)x;
		if (y < b->y0) b->y0 = (int16_t)y;
		if (y > b->y1) b->y1 = (int16_t)y;
		for (int k = 0; k < 9; ++k) {
			int nx = x + k % 3 - 1, ny = y + k / 3 - 1, j = ny * STEREO_W + nx;
			if (nx < 0 || ny < 0 || nx >= STEREO_W || ny >= STEREO_H || !stand[j] || out->what[j] >= STEREO_BODY) continue;
			out->what[j] = (uint8_t)(STEREO_BODY + id);
			stack[n++] = (int16_t)nx;
			stack[n++] = (int16_t)ny;
		}
	}
}

void stereo_sort(const GbaVideo *v, const GbaLayers *l, int hud_rows, StereoFrame *out) {
	static uint8_t stand[STEREO_N];
	memset(out->what, STEREO_WORLD, sizeof out->what);
	memset(stand, 0, sizeof stand);
	out->nbodies = 0;
	GbaBg bg[4];
	for (unsigned b = 0; b < 4; ++b) bg[b] = gba_bg(v->io, b);
	hud_backgrounds(v, bg, out);
	bool hud[GBA_OBJS];
	for (int n = 0; n < GBA_OBJS; ++n) hud[n] = hud_obj(l, n, hud_rows);
	for (int i = 0; i < STEREO_N; ++i) {
		if (!l->obj[i]) continue;
		/* (the HUD's sprites flat; a blended one, a shadow or a light, the floor's; one behind a background, the background's) */
		if (hud[l->obj[i] - 1]) out->what[i] = STEREO_FLAT;
		else if (!l->sheer[i] && !covered(v, bg, i % STEREO_W, i / STEREO_W, l->prio[i])) stand[i] = 1;
	}
	for (int i = 0; i < STEREO_N; ++i) {
		if (!stand[i] || out->what[i] >= STEREO_BODY) continue;
		/* (past the most, the rest join the last) */
		bool fresh = out->nbodies < STEREO_BODIES;
		int id = fresh ? out->nbodies++ : STEREO_BODIES - 1;
		if (fresh) out->body[id] = (StereoBox){ STEREO_W, STEREO_H, -1, -1 };
		body_fill(stand, out, i % STEREO_W, i / STEREO_W, id);
	}
}

float stereo_depth(int row) {
	float d = (float)(STEREO_H - 1 - row) / (float)(STEREO_H - 1);
	return d < 0 ? 0 : d > 1 ? 1 : d;
}

float stereo_shift(int eye, float sep, int row) { return 0.5f * (float)eye * sep * stereo_depth(row); }

/* A row's holes (where a body or a flat pixel stood) in the world filled
 * from beside them: the left half from the left, the right half from the
 * right, a hole at the row's end from its one side */
static void fill_holes(uint32_t *world, const uint8_t *hole) {
	for (int a = 0; a < STEREO_W;) {
		if (!hole[a]) { ++a; continue; }
		int b = a;
		while (b < STEREO_W && hole[b]) ++b;
		bool has_l = a > 0, has_r = b < STEREO_W;
		uint32_t left = has_l ? world[a - 1] : 0, right = has_r ? world[b] : 0;
		for (int k = a; (has_l || has_r) && k < b; ++k) world[k] = (k - a < (b - a + 1) / 2 ? has_l : !has_r) ? left : right;
		a = b;
	}
}

/* A layer's clear pixels beside its own take their colour, clear still:
 * the GPU's smooth scaling then mixes its edges with its own colour, not
 * with black */
static void edges(uint32_t *layer, int pitch, uint32_t alpha) {
	for (int y = 0; y < STEREO_H; ++y)
		for (int x = 0; x < STEREO_W; ++x) {
			uint32_t *p = &layer[y * pitch + x];
			if (*p & alpha) continue;
			uint32_t near = x > 0 && (p[-1] & alpha) ? p[-1] : x + 1 < STEREO_W && (p[1] & alpha) ? p[1] : 0;
			if (!near && y > 0 && (p[-pitch] & alpha)) near = p[-pitch];
			if (!near && y + 1 < STEREO_H && (p[pitch] & alpha)) near = p[pitch];
			*p = near & ~alpha;
		}
}

void stereo_layers(const StereoFrame *f, const uint32_t *canvas, int canvas_pitch, const uint32_t *picture, StereoLayers *out) {
	uint8_t hole[STEREO_W];
	int x = out->x, y = out->y;
	for (int cy = 0; cy < STEREO_H; ++cy) {
		bool any = false;
		for (int cx = 0; cx < STEREO_W; ++cx) {
			int fx = cx - x, fy = cy - y, o = cy * out->pitch + cx;
			uint32_t c = canvas[cy * canvas_pitch + cx];
			bool in = fx >= 0 && fy >= 0 && fx < STEREO_W && fy < STEREO_H;
			/* (outside the game's picture, or the engine's drawing over it: flat) */
			int what = in && c == picture[fy * STEREO_W + fx] ? f->what[fy * STEREO_W + fx] : STEREO_FLAT;
			out->world[o] = what == STEREO_WORLD ? c : 0;
			out->bodies[o] = what >= STEREO_BODY ? c : 0;
			out->flat[o] = what == STEREO_FLAT ? c : 0;
			hole[cx] = what != STEREO_WORLD;
			any |= !hole[cx];
		}
		out->row_world[cy] = any;
		if (any) fill_holes(&out->world[cy * out->pitch], hole);
		else memcpy(&out->world[cy * out->pitch], &canvas[cy * canvas_pitch], sizeof(uint32_t) * STEREO_W);
	}
	edges(out->bodies, out->pitch, out->alpha);
	edges(out->flat, out->pitch, out->alpha);
}

/* A layer's pixel at the eye's column: drawn where it is not clear */
static void put(uint32_t *out, int w, int ox, int oy, int scale, uint32_t c) {
	for (int sy = 0; sy < scale; ++sy)
		for (int sx = 0; sx < scale; ++sx) {
			int px = ox + sx;
			if (px >= 0 && px < w) out[(oy + sy) * w + px] = c;
		}
}

static void compose_body(const StereoLayers *l, const StereoBox *b, int eye, float sep, int scale, uint32_t *out) {
	int w = STEREO_W * scale, x = l->x, y = l->y;
	float s = stereo_shift(eye, sep, b->y1 + y) * (float)scale;
	for (int cy = b->y0 + y; cy <= b->y1 + y; ++cy)
		for (int cx = b->x0 + x; cx <= b->x1 + x; ++cx) {
			if (cx < 0 || cy < 0 || cx >= STEREO_W || cy >= STEREO_H) continue;
			uint32_t c = l->bodies[cy * l->pitch + cx];
			if (c & l->alpha) put(out, w, (int)floorf((float)(cx * scale) + s + 0.5f), cy * scale, scale, c);
		}
}

void stereo_compose(const StereoLayers *l, const StereoFrame *f, int eye, float sep, int scale, uint32_t *out) {
	int w = STEREO_W * scale;
	memset(out, 0, sizeof *out * (size_t)w * (size_t)(STEREO_H * scale));
	for (int cy = 0; cy < STEREO_H; ++cy) {
		float s = l->row_world[cy] ? stereo_shift(eye, sep, cy) * (float)scale : 0.0f;
		for (int cx = 0; cx < STEREO_W; ++cx) put(out, w, (int)floorf((float)(cx * scale) + s + 0.5f), cy * scale, scale, l->world[cy * l->pitch + cx]);
	}
	/* (the bodies deepest first) */
	bool done[STEREO_BODIES] = { false };
	for (int k = 0; k < f->nbodies; ++k) {
		int next = -1;
		for (int b = 0; b < f->nbodies; ++b)
			if (!done[b] && (next < 0 || f->body[b].y1 < f->body[next].y1)) next = b;
		done[next] = true;
		compose_body(l, &f->body[next], eye, sep, scale, out);
	}
	for (int cy = 0; cy < STEREO_H; ++cy)
		for (int cx = 0; cx < STEREO_W; ++cx) {
			uint32_t c = l->flat[cy * l->pitch + cx];
			if (c & l->alpha) put(out, w, cx * scale, cy * scale, scale, c);
		}
}
