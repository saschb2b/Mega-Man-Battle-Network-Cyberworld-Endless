/* intro_logo.h. Each letter is a few strokes of a round pen (lines and
 * arcs of ellipses, in pixels of the letters' own size, the baseline at
 * 0), whose distance field is made once; every frame each letter is drawn
 * through its field turned, scaled and moved, its edge smoothed by the
 * distance and its glow taken from the same distance. In frames:
 *
 *   1-8      in from black, onto a dark blue screen
 *   6-62     the letters fly in, each from its own side, turning and
 *            spiralling in as they shrink to size, four frames apart; a
 *            pluck as each settles, rising
 *   66-101   a hop runs through the word and a band of light sweeps it
 *            with the hop; a chord as it starts, a glow left round the
 *            letters, a sparkle on four tops as the light leaves them
 *   118-128  out to black */
#include "intro_logo.h"

#include <math.h>
#include <stdbool.h>

#include "audio.h"
#include "platform.h"

#define NAME "Saschb2b"
#define LETTERS 8
#define PEN 1.7f            /* half the pen's width */
#define PAD 8.0f            /* the field round a letter's ink, for its glow */
#define FIELD_K 2           /* the field's samples a pixel */
#define FIELD_MAX 80
#define SEGS 40
#define GAP 2.8f            /* between two letters' ink */
#define WORD_W 164.0f       /* the word's width on the screen */
#define BASELINE 92.0f      /* ... and its baseline */
#define FAR 1e9f
#define PI_F 3.14159265f

#define FLY_AT 6
#define FLY_STEP 4
#define FLY_LEN 28
#define WAVE_AT 66
#define WAVE_STEP 3
#define WAVE_LEN 14
#define HOP 5.0f
#define SWEEP_SPEED 6.3f    /* the light's pixels a frame: a letter on per step of the hop */
#define SPARKLE_LEN 18

typedef struct {
	char ch;
	float seg[SEGS][4];     /* the pen's middle: x0, y0, x1, y1 */
	int nseg;
	float ink[4];           /* the ink's box: left, top, right, bottom */
	float fx, fy;           /* the field's first sample */
	int fw, fh;
	float field[FIELD_MAX * FIELD_MAX];   /* distance to the pen's middle */
} Glyph;

static Glyph glyphs[7];
static int nglyphs;

static struct {
	const Glyph *g;
	float cx, cy;           /* its centre on the screen, landed */
	float gx, gy;           /* ... and in its own space */
} L[LETTERS];
static float size_k;        /* the letters' scale on the screen */
static float word_left;
static float glow_x[CORE_W], glow_y[CORE_H];   /* the light behind the word, by column and row */
static bool built;

typedef struct { float x, y, rot, scale, alpha; } Pose;

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
static float rad(float deg) { return deg * PI_F / 180.0f; }
/* e^-x for x >= 0, near enough for light, and none at all from 6 on: a
 * glow ends inside the box a letter is drawn in, which a longer tail
 * showed */
static float fall(float x) {
	if (x >= 6) return 0;
	float a = 1 - x / 6;
	a *= a * a;
	return a * a;
}

static uint32_t rgb(float r, float g, float b) {
	return 0xFF000000u | (uint32_t)clampf(r, 0, 255) << 16 | (uint32_t)clampf(g, 0, 255) << 8 | (uint32_t)clampf(b, 0, 255);
}

/* ---- the letters ---- */

static Glyph *glyph_new(char ch) {
	Glyph *g = &glyphs[nglyphs++];
	g->ch = ch;
	g->nseg = 0;
	return g;
}

static void line(Glyph *g, float x0, float y0, float x1, float y1) {
	if (g->nseg >= SEGS) return;
	float *s = g->seg[g->nseg++];
	s[0] = x0;
	s[1] = y0;
	s[2] = x1;
	s[3] = y1;
}

/* an arc of the ellipse round (cx, cy) from a0 to a1 degrees, clockwise
 * as they grow (y runs down) */
static void arc(Glyph *g, float cx, float cy, float rx, float ry, float a0, float a1) {
	int n = (int)ceilf(fabsf(a1 - a0) / 15.0f);
	float x = cx + rx * cosf(rad(a0)), y = cy + ry * sinf(rad(a0));
	for (int i = 1; i <= n; ++i) {
		float a = rad(a0 + (a1 - a0) * (float)i / (float)n);
		float nx = cx + rx * cosf(a), ny = cy + ry * sinf(a);
		line(g, x, y, nx, ny);
		x = nx;
		y = ny;
	}
}

/* Saschb2b's letters, ours: geometric, one pen, the x-height 14 and the
 * capital's and the ascenders' height 21 */
static void letters_shape(void) {
	Glyph *g = glyph_new('S');
	arc(g, 6.3f, -15.0f, 4.7f, 4.5f, -25, -270);
	arc(g, 6.3f, -6.0f, 5.3f, 4.5f, -90, 155);
	g = glyph_new('a');
	arc(g, 6.5f, -7.0f, 5.0f, 5.5f, 0, 360);
	line(g, 11.5f, -12.5f, 11.5f, -1.5f);
	g = glyph_new('s');
	arc(g, 5.8f, -9.75f, 3.9f, 2.75f, -20, -270);
	arc(g, 5.8f, -4.25f, 4.3f, 2.75f, -90, 160);
	g = glyph_new('c');
	arc(g, 6.5f, -7.0f, 5.0f, 5.5f, 45, 315);
	g = glyph_new('h');
	line(g, 1.5f, -19.5f, 1.5f, -1.5f);
	arc(g, 6.5f, -7.0f, 5.0f, 5.5f, 180, 360);
	line(g, 11.5f, -7.0f, 11.5f, -1.5f);
	g = glyph_new('b');
	line(g, 1.5f, -19.5f, 1.5f, -1.5f);
	arc(g, 6.5f, -7.0f, 5.0f, 5.5f, 0, 360);
	g = glyph_new('2');
	arc(g, 6.5f, -14.5f, 5.0f, 5.0f, 200, 400);
	line(g, 6.5f + 5.0f * cosf(rad(40)), -14.5f + 5.0f * sinf(rad(40)), 1.5f, -1.5f);
	line(g, 1.5f, -1.5f, 12.0f, -1.5f);
}

static float seg_dist2(const float *s, float x, float y) {
	float dx = s[2] - s[0], dy = s[3] - s[1], l2 = dx * dx + dy * dy;
	float t = l2 > 0 ? clampf(((x - s[0]) * dx + (y - s[1]) * dy) / l2, 0, 1) : 0;
	float ex = s[0] + t * dx - x, ey = s[1] + t * dy - y;
	return ex * ex + ey * ey;
}

/* the ink's box, and the field over it and its margin */
static void glyph_field(Glyph *g) {
	float *k = g->ink;
	k[0] = k[1] = FAR;
	k[2] = k[3] = -FAR;
	for (int i = 0; i < g->nseg; ++i)
		for (int e = 0; e < 4; e += 2) {
			float x = g->seg[i][e], y = g->seg[i][e + 1];
			k[0] = fminf(k[0], x - PEN);
			k[1] = fminf(k[1], y - PEN);
			k[2] = fmaxf(k[2], x + PEN);
			k[3] = fmaxf(k[3], y + PEN);
		}
	g->fx = k[0] - PAD;
	g->fy = k[1] - PAD;
	g->fw = (int)((k[2] - k[0] + 2 * PAD) * FIELD_K) + 2;
	g->fh = (int)((k[3] - k[1] + 2 * PAD) * FIELD_K) + 2;
	if (g->fw > FIELD_MAX) g->fw = FIELD_MAX;
	if (g->fh > FIELD_MAX) g->fh = FIELD_MAX;
	for (int j = 0; j < g->fh; ++j)
		for (int i = 0; i < g->fw; ++i) {
			float x = g->fx + (float)i / FIELD_K, y = g->fy + (float)j / FIELD_K, d = FAR;
			for (int s = 0; s < g->nseg; ++s) d = fminf(d, seg_dist2(g->seg[s], x, y));
			g->field[j * g->fw + i] = sqrtf(d);
		}
}

static float field_at(const Glyph *g, float x, float y) {
	float u = (x - g->fx) * FIELD_K, v = (y - g->fy) * FIELD_K;
	if (u < 0 || v < 0 || u >= (float)(g->fw - 1) || v >= (float)(g->fh - 1)) return FAR;
	int i = (int)u, j = (int)v;
	float a = u - (float)i, b = v - (float)j;
	const float *f = g->field + j * g->fw + i;
	return (f[0] * (1 - a) + f[1] * a) * (1 - b) + (f[g->fw] * (1 - a) + f[g->fw + 1] * a) * b;
}

static const Glyph *glyph_of(char ch) {
	for (int i = 0; i < nglyphs; ++i)
		if (glyphs[i].ch == ch) return &glyphs[i];
	return &glyphs[0];
}

/* the letters' fields, their places in the word, the light behind it */
static void build(void) {
	letters_shape();
	for (int i = 0; i < nglyphs; ++i) glyph_field(&glyphs[i]);
	float total = -GAP;
	for (int i = 0; i < LETTERS; ++i) {
		L[i].g = glyph_of(NAME[i]);
		total += L[i].g->ink[2] - L[i].g->ink[0] + GAP;
	}
	size_k = WORD_W / total;
	float x = word_left = (CORE_W - WORD_W) / 2;
	for (int i = 0; i < LETTERS; ++i) {
		const float *k = L[i].g->ink;
		L[i].gx = (k[0] + k[2]) / 2;
		L[i].gy = (k[1] + k[3]) / 2;
		L[i].cx = x + (L[i].gx - k[0]) * size_k;
		L[i].cy = BASELINE + L[i].gy * size_k;
		x += (k[2] - k[0] + GAP) * size_k;
	}
	for (int c = 0; c < CORE_W; ++c) {
		float d = (float)(c - CORE_W / 2) / 95.0f;
		glow_x[c] = expf(-d * d);
	}
	for (int r = 0; r < CORE_H; ++r) {
		float d = ((float)r - (BASELINE - 12)) / 42.0f;
		glow_y[r] = expf(-d * d);
	}
	built = true;
}

/* ---- the motion ---- */

/* letter i at frame t: flying in (eased out, each from its own side, the
 * golden angle apart, so they come from all round), then its hop */
static Pose pose(int i, int t) {
	Pose p = { L[i].cx, L[i].cy, 0, 1, 1 };
	float u = (float)(t - FLY_AT - i * FLY_STEP) / FLY_LEN;
	if (u <= 0) {
		p.alpha = 0;
		return p;
	}
	if (u < 1) {
		float k = (1 - u) * (1 - u) * (1 - u), dir = 0.6f + (float)i * 2.39996f, swirl = 1.3f * k;
		float ox = cosf(dir) * 170 * k, oy = sinf(dir) * 170 * k;
		p.x += ox * cosf(swirl) - oy * sinf(swirl);
		p.y += ox * sinf(swirl) + oy * cosf(swirl);
		p.rot = (i & 1 ? 2.6f : -2.6f) * k;
		p.scale = 1 + 1.4f * k;
		p.alpha = clampf(u / 0.3f, 0, 1);
	}
	float v = (float)(t - WAVE_AT - i * WAVE_STEP) / WAVE_LEN;
	if (v > 0 && v < 1) {
		float h = sinf(PI_F * v);
		p.y -= HOP * h;
		p.scale *= 1 + 0.06f * h;
	}
	return p;
}

/* the band of light's middle at frame t, slanting: a pixel's distance
 * from it, in its widths */
static float band_off(int x, int y, float xb) { return ((float)x + 0.5f * ((float)y - BASELINE) - xb) / 10.0f; }

static float band_x(int t) { return word_left - 30 + (float)(t - WAVE_AT) * SWEEP_SPEED; }

/* ---- the picture ---- */

static void background(uint32_t *px, float light) {
	for (int y = 0; y < CORE_H; ++y) {
		float v = (float)y / CORE_H, r = 5 + 9 * v, g = 8 + 12 * v, b = 22 + 34 * v;
		for (int x = 0; x < CORE_W; ++x) {
			float k = light * glow_x[x] * glow_y[y];
			px[y * CORE_W + x] = rgb(r + 30 * k, g + 60 * k, b + 130 * k);
		}
	}
}

/* one pixel of a letter: its glow added, then its ink over it, cyan at
 * the tops to violet at the foot, white where the light is */
static uint32_t shade(uint32_t o, float cover, float glow, float light, float qy) {
	float r = (float)(o >> 16 & 255) + glow * 70, g = (float)(o >> 8 & 255) + glow * 200, b = (float)(o & 255) + glow * 255;
	float v = clampf((qy + 21) / 21, 0, 1), w = 0.85f * light;
	float fr = 110 + 50 * v, fg = 236 - 140 * v, fb = 255;
	fr += (255 - fr) * w;
	fg += (255 - fg) * w;
	fb += (255 - fb) * w;
	return rgb(r + (fr - r) * cover, g + (fg - g) * cover, b + (fb - b) * cover);
}

/* letter i in its pose: the box it can reach (its ink's, its field's once
 * it glows), each pixel's place in its own space, its distance there */
static void letter_draw(uint32_t *px, int i, Pose p, int t, float halo) {
	if (p.alpha <= 0) return;
	const Glyph *g = L[i].g;
	bool glowing = t >= WAVE_AT - 8;
	float k = size_k * p.scale, c = cosf(p.rot), s = sinf(p.rot), edge = PEN * k, xb = band_x(t);
	float m = glowing ? PAD : 0, hw = (g->ink[2] - g->ink[0]) / 2 + m, hh = (g->ink[3] - g->ink[1]) / 2 + m;
	float reach = sqrtf(hw * hw + hh * hh) * k;
	int x0 = (int)fmaxf(0, p.x - reach), x1 = (int)fminf(CORE_W - 1, p.x + reach);
	int y0 = (int)fmaxf(0, p.y - reach), y1 = (int)fminf(CORE_H - 1, p.y + reach);
	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x) {
			float dx = (float)x + 0.5f - p.x, dy = (float)y + 0.5f - p.y;
			float qx = L[i].gx + (c * dx + s * dy) / k, qy = L[i].gy + (c * dy - s * dx) / k;
			float d = field_at(g, qx, qy) * k, cover = clampf(edge - d + 0.5f, 0, 1) * p.alpha;
			float light = 0, glow = 0;
			if (glowing) {
				float u = band_off(x, y, xb);
				light = fall(u * u);
				glow = (halo + light) * p.alpha * fall(fmaxf(0, d - edge) / (1.8f * size_k));
			}
			if (cover <= 0 && glow < 0.004f) continue;
			px[y * CORE_W + x] = shade(px[y * CORE_W + x], cover, glow, light, qy);
		}
}

static void add_white(uint32_t *px, int x, int y, float a) {
	if (x < 0 || y < 0 || x >= CORE_W || y >= CORE_H || a <= 0) return;
	uint32_t o = px[y * CORE_W + x];
	px[y * CORE_W + x] = rgb((float)(o >> 16 & 255) + 255 * a, (float)(o >> 8 & 255) + 255 * a, (float)(o & 255) + 255 * a);
}

/* the sparkles the light leaves on four tops: a letter and a point of its own */
static const struct { int letter; float x, y; } sparks[4] = {
	{ 0, 6.3f, -19.5f }, { 4, 1.5f, -19.5f }, { 6, 6.5f, -19.5f }, { 7, 1.5f, -19.5f },
};

static void sparkles(uint32_t *px, int t) {
	for (int n = 0; n < 4; ++n) {
		int i = sparks[n].letter;
		Pose p = pose(i, t);
		float k = size_k * p.scale, sx = p.x + (sparks[n].x - L[i].gx) * k, sy = p.y + (sparks[n].y - L[i].gy) * k;
		/* (from the frame the light's middle reaches it) */
		float since = (float)t - (float)WAVE_AT - (sx + 0.5f * (sy - BASELINE) - band_x(WAVE_AT)) / SWEEP_SPEED;
		if (since < 0 || since >= SPARKLE_LEN) continue;
		float len = 6 * sinf(PI_F * since / SPARKLE_LEN);
		int x = (int)sx, y = (int)sy;
		add_white(px, x, y, fminf(1, len / 3));
		for (int d = 1; d <= (int)len; ++d) {
			float a = (1 - (float)d / len) * (1 - (float)d / len);
			add_white(px, x + d, y, a);
			add_white(px, x - d, y, a);
			add_white(px, x, y + d, a);
			add_white(px, x, y - d, a);
			if (d <= (int)(len / 2)) {
				add_white(px, x + d, y + d, a * 0.5f);
				add_white(px, x - d, y - d, a * 0.5f);
				add_white(px, x + d, y - d, a * 0.5f);
				add_white(px, x - d, y + d, a * 0.5f);
			}
		}
	}
}

void intro_logo_render(uint32_t *px, int t) {
	if (!built) build();
	float after = clampf((float)(t - WAVE_AT) / 24.0f, 0, 1);
	background(px, 0.22f + 0.30f * after);
	for (int i = 0; i < LETTERS; ++i) letter_draw(px, i, pose(i, t), t, 0.14f * after);
	if (t >= WAVE_AT) sparkles(px, t);
}

void intro_logo_sound(int t) {
	/* the plucks rise through D major's pentatonic scale as the letters
	 * settle; the chord with the light is D major with its ninth */
	static const float pluck[LETTERS] = { 587.33f, 659.26f, 739.99f, 880.00f, 987.77f, 1174.66f, 1318.51f, 1479.98f };
	static const float chord[5] = { 293.66f, 587.33f, 739.99f, 880.00f, 1318.51f };
	for (int i = 0; i < LETTERS; ++i)
		if (t == FLY_AT + i * FLY_STEP + FLY_LEN * 3 / 4) audio_tone(pluck[i], 0.30, 0.16f, 0);
	if (t == WAVE_AT)
		for (int k = 0; k < 5; ++k) audio_tone(chord[k], 1.9, 0.10f, 0.035 * k);
}
