#include "touch_art.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "minifont.h"
#include "platform.h"

/* ---- a small rasterizer: each shape a signed distance (negative inside),
 * covering a pixel's width at its edge; a layer's coverage is gathered in
 * a mask, then laid over the picture in one colour ---- */

/* premultiplied RGBA; the layer's coverage, and the box it has touched */
typedef struct { int w, h; float *px, *mask; int x0, y0, x1, y1; } Img;
typedef float (*Sdf)(float x, float y, const void *c);

typedef struct { float cx, cy, hw, hh, r; } RBox;
static float sd_rbox(float x, float y, const void *c) {
	const RBox *b = c;
	float r = fminf(b->r, fminf(b->hw, b->hh));
	float qx = fabsf(x - b->cx) - b->hw + r, qy = fabsf(y - b->cy) - b->hh + r;
	float ox = fmaxf(qx, 0), oy = fmaxf(qy, 0);
	return sqrtf(ox * ox + oy * oy) + fminf(fmaxf(qx, qy), 0) - r;
}

typedef struct { float cx, cy, r; } Circle;
static float sd_circle(float x, float y, const void *c) {
	const Circle *k = c;
	return hypotf(x - k->cx, y - k->cy) - k->r;
}

typedef struct { RBox a, b; } Cross;
static float sd_cross(float x, float y, const void *c) {
	const Cross *k = c;
	return fminf(sd_rbox(x, y, &k->a), sd_rbox(x, y, &k->b));
}

typedef struct { float x[3], y[3]; } Tri;
static float sd_tri(float px, float py, const void *c) {
	const Tri *t = c;
	float d = 1e9f, s = 1;
	int inside = 0;
	for (int i = 0, j = 2; i < 3; j = i++) {
		float ex = t->x[i] - t->x[j], ey = t->y[i] - t->y[j], vx = px - t->x[j], vy = py - t->y[j];
		float k = fmaxf(0, fminf(1, (vx * ex + vy * ey) / (ex * ex + ey * ey)));
		float qx = vx - ex * k, qy = vy - ey * k;
		d = fminf(d, qx * qx + qy * qy);
		/* (the crossings of a ray to the right: an odd count is inside) */
		if ((t->y[i] > py) != (t->y[j] > py) && px < ex * (py - t->y[j]) / ey + t->x[j]) inside ^= 1;
	}
	if (inside) s = -1;
	return s * sqrtf(d);
}

/* A line of width 2 * half along another shape's edge. */
typedef struct { Sdf f; const void *c; float half; } Stroke;
static float sd_stroke(float x, float y, const void *c) {
	const Stroke *s = c;
	return fabsf(s->f(x, y, s->c)) - s->half;
}

static Img img_new(int w, int h) {
	Img m = { w, h, calloc((size_t)w * h * 4, sizeof(float)), calloc((size_t)w * h, sizeof(float)), w, h, 0, 0 };
	return m;
}

/* (the mask's layer reaches into box x0-x1, y0-y1) */
static void touched(Img *m, int x0, int y0, int x1, int y1) {
	if (x0 < m->x0) m->x0 = x0 < 0 ? 0 : x0;
	if (y0 < m->y0) m->y0 = y0 < 0 ? 0 : y0;
	if (x1 > m->x1) m->x1 = x1 > m->w ? m->w : x1;
	if (y1 > m->y1) m->y1 = y1 > m->h ? m->h : y1;
}

static void img_free(Img *m) { free(m->px); free(m->mask); }

/* A blurred edge's coverage `d` pixels out from it, blurred `soft`: a
 * Gaussian's, from a table over three of its widths each way. */
static float blurred(float d, float soft) {
	static float table[257];
	if (!table[0]) for (int i = 0; i <= 256; ++i) table[i] = 0.5f * erfcf((i / 256.f * 6 - 3) / 1.41421f);
	float t = (d / soft + 3) / 6 * 256;
	if (t <= 0) return 1;
	if (t >= 256) return 0;
	int i = (int)t;
	return table[i] + (table[i + 1] - table[i]) * (t - i);
}

static float sharp(float d) { return fmaxf(0, fminf(1, 0.5f - d)); }

/* Adds a shape to the mask within the box x0-x1, y0-y1 (it lies inside):
 * sharp (soft 0), or blurred `soft` pixels (a glow). */
static void mask_shape_in(Img *m, Sdf f, const void *c, float soft, int x0, int y0, int x1, int y1) {
	if (!m->px || !m->mask) return;
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 > m->w) x1 = m->w;
	if (y1 > m->h) y1 = m->h;
	touched(m, x0, y0, x1, y1);
	for (int y = y0; y < y1; ++y)
		for (int x = x0; x < x1; ++x) {
			float d = f(x + 0.5f, y + 0.5f, c), cov = soft > 0 ? blurred(d, soft) : sharp(d);
			float *k = &m->mask[y * m->w + x];
			if (cov > *k) *k = cov;
		}
}

static void mask_shape(Img *m, Sdf f, const void *c, float soft) { mask_shape_in(m, f, c, soft, 0, 0, m->w, m->h); }


/* ... a dashed one: `dashes` dashes round (cx, cy), each on for 3/5 of its turn. */
static void mask_dashed(Img *m, Sdf f, const void *c, float cx, float cy, int dashes) {
	if (!m->px || !m->mask) return;
	touched(m, 0, 0, m->w, m->h);
	for (int y = 0; y < m->h; ++y)
		for (int x = 0; x < m->w; ++x) {
			float d = f(x + 0.5f, y + 0.5f, c), cov = fmaxf(0, fminf(1, 0.5f - d));
			if (cov <= 0) continue;
			float r = hypotf(x + 0.5f - cx, y + 0.5f - cy), t = (atan2f(y + 0.5f - cy, x + 0.5f - cx) / 6.2831853f + 0.5f) * dashes;
			t -= floorf(t);
			/* (the dash's ends a pixel soft, measured along the turn) */
			float len = 6.2831853f * r / dashes, pos = t * len, on = len * 0.6f;
			cov *= pos < on ? fmaxf(0, fminf(1, fminf(pos, on - pos) + 0.5f)) : 0;
			float *k = &m->mask[y * m->w + x];
			if (cov > *k) *k = cov;
		}
}

static void mask_rect(Img *m, int x0, int y0, int w, int h) {
	if (!m->mask) return;
	touched(m, x0, y0, x0 + w, y0 + h);
	for (int y = y0 < 0 ? 0 : y0; y < y0 + h && y < m->h; ++y)
		for (int x = x0 < 0 ? 0 : x0; x < x0 + w && x < m->w; ++x) m->mask[y * m->w + x] = 1;
}

/* Lays colour c over pixel p at coverage k (0-1). */
static void over(float *p, SDL_Color c, float k) {
	k *= c.a / 255.f;
	if (k <= 0) return;
	p[0] = c.r / 255.f * k + p[0] * (1 - k);
	p[1] = c.g / 255.f * k + p[1] * (1 - k);
	p[2] = c.b / 255.f * k + p[2] * (1 - k);
	p[3] = k + p[3] * (1 - k);
}

/* Lays the mask's layer over the picture in colour c, and clears it. */
static void paint(Img *m, SDL_Color c) {
	if (!m->px || !m->mask) return;
	for (int y = m->y0; y < m->y1; ++y)
		for (int x = m->x0; x < m->x1; ++x) {
			int i = y * m->w + x;
			if (m->mask[i] <= 0) continue;
			over(&m->px[i * 4], c, m->mask[i]);
			m->mask[i] = 0;
		}
	m->x0 = m->w;
	m->y0 = m->h;
	m->x1 = m->y1 = 0;
}

/* Letters in the 3x5 font, `scale` pixels to one, their top-left at
 * (x0, y0), with an outline round them. */
static void letters(Img *m, int x0, int y0, const char *s, int scale, SDL_Color ink, SDL_Color outline) {
	int o = (scale + 1) / 3 > 1 ? (scale + 1) / 3 : 1;
	for (int pass = 0; pass < 2; ++pass) {
		for (int i = 0; s[i]; ++i) {
			unsigned short g = minifont_glyph(s[i]);
			for (int row = 0; row < 5; ++row)
				for (int col = 0; col < 3; ++col) {
					if (!(g >> ((4 - row) * 3 + (2 - col)) & 1)) continue;
					int x = x0 + (i * 4 + col) * scale, y = y0 + row * scale;
					if (pass == 0) mask_rect(m, x - o, y - o, scale + 2 * o, scale + 2 * o);
					else mask_rect(m, x, y, scale, scale);
				}
		}
		paint(m, pass == 0 ? outline : ink);
	}
}

int art_text_width(const char *s, int scale) { return minifont_width(s, scale); }

/* ---- the pictures ---- */

static const SDL_Color RIM = { 8, 20, 30, 150 };

/* The arms' ways out of the middle: up, right, down, left. */
static const int arm_dx[4] = { 0, 1, 0, -1 }, arm_dy[4] = { -1, 0, 1, 0 };

static void arrow(Img *m, float cx, float cy, float R, int arm, SDL_Color c) {
	float t = R * 0.2f, dx = (float)arm_dx[arm], dy = (float)arm_dy[arm];
	float tx = cx + dx * R * 0.72f, ty = cy + dy * R * 0.72f;
	/* (the tip out, the base across the arm) */
	Tri tri = { { tx + dx * t * 0.6f, tx - dx * t * 0.5f - dy * t * 0.75f, tx - dx * t * 0.5f + dy * t * 0.75f },
		{ ty + dy * t * 0.6f, ty - dy * t * 0.5f - dx * t * 0.75f, ty - dy * t * 0.5f + dx * t * 0.75f } };
	mask_shape_in(m, sd_tri, &tri, 0, (int)(tx - t) - 2, (int)(ty - t) - 2, (int)(tx + t) + 3, (int)(ty + t) + 3);
	paint(m, c);
}

/* A shape's dark rim round it, its light edge, its see-through fill: each
 * on its own band of the shape, not laid over the others (one pass, the
 * shape's distance once a pixel). */
static void glass(Img *m, Sdf f, const void *c, const ArtSpec *s) {
	if (!m->px) return;
	float rim = s->rim_w, edge = s->edge_w;
	for (int y = 0; y < m->h; ++y)
		for (int x = 0; x < m->w; ++x) {
			float d = f(x + 0.5f, y + 0.5f, c);
			if (d > rim + 1) continue;
			float out = sharp(d - rim), at = sharp(d), in = sharp(d + edge);
			float *p = &m->px[(y * m->w + x) * 4];
			if (rim > 0) over(p, RIM, out - at);
			if (edge > 0) over(p, s->edge, at - in);
			over(p, s->fill, in);
		}
}

static void draw_dpad(Img *m, const ArtSpec *s, float cx, float cy) {
	float R = s->w / 2, arm = R * 0.36f, r = arm * 0.55f;
	Cross k = { { cx, cy, arm, R, r }, { cx, cy, R, arm, r } };
	glass(m, sd_cross, &k, s);
	/* a soft ring in the middle, where the thumb rests */
	Circle mid = { cx, cy, arm * 0.42f };
	Stroke ring = { sd_circle, &mid, s->edge_w * 0.33f };
	int reach = (int)(arm * 0.42f + s->edge_w) + 3;
	mask_shape_in(m, sd_stroke, &ring, 0, (int)cx - reach, (int)cy - reach, (int)cx + reach + 1, (int)cy + reach + 1);
	SDL_Color e = s->edge;
	e.a = (Uint8)(e.a * 3 / 4);
	paint(m, e);
	for (int i = 0; i < 4; ++i) arrow(m, cx, cy, R, i, s->ink);
}

static void draw_arm(Img *m, const ArtSpec *s, float cx, float cy) {
	float R = s->w / 2, arm = R * 0.36f, dx = (float)arm_dx[s->arm], dy = (float)arm_dy[s->arm];
	/* the glow round the thumb, the arm lit, its arrow dark */
	Circle glow = { cx + dx * R * 0.62f, cy + dy * R * 0.62f, R * 0.55f };
	int gr = (int)(R * 0.55f + R * 0.12f * 3) + 2;
	mask_shape_in(m, sd_circle, &glow, R * 0.12f, (int)glow.cx - gr, (int)glow.cy - gr, (int)glow.cx + gr + 1, (int)glow.cy + gr + 1);
	paint(m, s->glow_color);
	float a = arm - s->edge_w, from = arm, to = R - s->edge_w, mid = (from + to) / 2, half = (to - from) / 2;
	RBox lit = { cx + dx * mid, cy + dy * mid, dx ? half : a, dx ? a : half, arm * 0.45f };
	mask_shape_in(m, sd_rbox, &lit, 0, (int)(lit.cx - lit.hw) - 2, (int)(lit.cy - lit.hh) - 2, (int)(lit.cx + lit.hw) + 3, (int)(lit.cy + lit.hh) + 3);
	paint(m, s->fill);
	arrow(m, cx, cy, R, s->arm, s->ink);
}

/* A plate or a disc: its glow when pressed, the dark rim, the light edge,
 * the fill, the letters. */
static void draw_button(Img *m, const ArtSpec *s, float cx, float cy) {
	bool disc = s->kind == ART_DISC;
	float k = s->shrink > 0 ? s->shrink : 1, hw = s->w / 2, hh = disc ? hw : s->h / 2, small = fminf(hw, hh);
	float r = disc ? hw : fminf(s->radius, small);
	if (s->glow > 0) {
		float g = small * (s->glow - 1);
		RBox b = { cx, cy, hw + g, hh + g, disc ? hw + g : r + g };
		mask_shape(m, sd_rbox, &b, small * 0.18f);
		paint(m, s->glow_color);
	}
	hw *= k; hh *= k; r *= k;
	RBox b = { cx, cy, hw, hh, r };
	glass(m, sd_rbox, &b, s);
}

static void draw_dashed(Img *m, const ArtSpec *s, float cx, float cy) {
	float hw = s->w / 2, hh = s->round ? hw : s->h / 2;
	RBox b = { cx, cy, hw, hh, s->round ? hw : s->radius };
	Stroke line = { sd_rbox, &b, s->edge_w / 2 };
	float turn = s->round ? 6.2831853f * hw : 4 * (hw + hh);
	int dashes = (int)(turn / (s->edge_w * 6));
	mask_dashed(m, sd_stroke, &line, cx, cy, dashes < 8 ? 8 : dashes);
	paint(m, s->edge);
}

/* The picture as a texture, cut to what it covers. */
static Art rasterize(const ArtSpec *s) {
	Art a = { 0 };
	float hw = s->w / 2, hh = s->kind == ART_DISC || s->kind == ART_DPAD || s->kind == ART_ARM || (s->kind == ART_DASHED && s->round) ? hw : s->h / 2;
	float pad = s->rim_w + 3 + (s->glow > 0 ? fminf(hw, hh) * (s->glow - 1 + 0.6f) : 0) + (s->kind == ART_ARM ? hw * 0.6f : 0);
	int w = (int)ceilf(2 * (hw + pad)), h = (int)ceilf(2 * (hh + pad));
	if (w < 1 || h < 1 || w > 4096 || h > 4096) return a;
	Img m = img_new(w, h);
	if (!m.px || !m.mask) { img_free(&m); return a; }
	float cx = w / 2.f, cy = h / 2.f;
	switch (s->kind) {
	case ART_DPAD: draw_dpad(&m, s, cx, cy); break;
	case ART_ARM: draw_arm(&m, s, cx, cy); break;
	case ART_DASHED: draw_dashed(&m, s, cx, cy); break;
	default: draw_button(&m, s, cx, cy); break;
	}
	if (s->label[0] && s->scale > 0) {
		int tw = minifont_width(s->label, s->scale), th = 5 * s->scale;
		letters(&m, (int)floorf(cx - tw / 2.f + 0.5f), (int)floorf(cy - th / 2.f + 0.5f), s->label, s->scale, s->ink, s->ink_edge);
	}
	/* (cut to the pixels it covers) */
	int x0 = w, y0 = h, x1 = -1, y1 = -1;
	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x)
			if (m.px[(y * w + x) * 4 + 3] > 1.f / 512) {
				if (x < x0) x0 = x;
				if (x > x1) x1 = x;
				if (y < y0) y0 = y;
				if (y > y1) y1 = y;
			}
	if (x1 >= x0 && y1 >= y0) {
		int cw = x1 - x0 + 1, ch = y1 - y0 + 1;
		SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, cw, ch, 32, SDL_PIXELFORMAT_ARGB8888);
		if (sf) {
			for (int y = 0; y < ch; ++y) {
				Uint32 *row = (Uint32 *)((Uint8 *)sf->pixels + y * sf->pitch);
				for (int x = 0; x < cw; ++x) {
					const float *p = &m.px[((y + y0) * w + x + x0) * 4];
					float al = p[3];
					Uint32 A = (Uint32)(al * 255 + 0.5f), R = 0, G = 0, B = 0;
					if (al > 0) {
						R = (Uint32)fminf(255, p[0] / al * 255 + 0.5f);
						G = (Uint32)fminf(255, p[1] / al * 255 + 0.5f);
						B = (Uint32)fminf(255, p[2] / al * 255 + 0.5f);
					}
					row[x] = A << 24 | R << 16 | G << 8 | B;
				}
			}
			a.tex = SDL_CreateTextureFromSurface(P.renderer, sf);
			SDL_FreeSurface(sf);
			if (a.tex) {
				SDL_SetTextureBlendMode(a.tex, SDL_BLENDMODE_BLEND);
				SDL_SetTextureScaleMode(a.tex, SDL_ScaleModeLinear);
				a.w = cw;
				a.h = ch;
				a.mx = cx - x0;
				a.my = cy - y0;
			}
		}
	}
	img_free(&m);
	return a;
}

/* ---- the pictures kept ---- */

#define KEPT 96
#define UNUSED_FOR 600   /* frames a picture is kept unused (a size the editor left) */
static struct { ArtSpec spec; Art art; uint32_t used; bool on; } kept[KEPT];
static uint32_t frame_no = 1;

Art art_get(const ArtSpec *s) {
	int free_slot = -1, oldest = 0;
	for (int i = 0; i < KEPT; ++i) {
		if (!kept[i].on) { if (free_slot < 0) free_slot = i; continue; }
		if (!memcmp(&kept[i].spec, s, sizeof *s)) { kept[i].used = frame_no; return kept[i].art; }
		if (kept[i].used < kept[oldest].used || !kept[oldest].on) oldest = i;
	}
	int i = free_slot >= 0 ? free_slot : oldest;
	if (kept[i].on && kept[i].art.tex) SDL_DestroyTexture(kept[i].art.tex);
	memcpy(&kept[i].spec, s, sizeof *s);
	kept[i].art = rasterize(s);
	kept[i].used = frame_no;
	kept[i].on = true;
	return kept[i].art;
}

void art_draw(const ArtSpec *s, float cx, float cy, float alpha) {
	if (alpha <= 0) return;
	Art a = art_get(s);
	if (!a.tex) return;
	SDL_SetTextureAlphaMod(a.tex, (Uint8)(fminf(alpha, 1) * 255 + 0.5f));
	SDL_Rect dst = { (int)floorf(cx - a.mx + 0.5f), (int)floorf(cy - a.my + 0.5f), a.w, a.h };
	SDL_RenderCopy(P.renderer, a.tex, NULL, &dst);
}

void art_tick(void) {
	++frame_no;
	for (int i = 0; i < KEPT; ++i)
		if (kept[i].on && frame_no - kept[i].used > UNUSED_FOR) {
			if (kept[i].art.tex) SDL_DestroyTexture(kept[i].art.tex);
			kept[i].on = false;
		}
}

void art_flush(void) {
	for (int i = 0; i < KEPT; ++i) {
		if (kept[i].on && kept[i].art.tex) SDL_DestroyTexture(kept[i].art.tex);
		kept[i].on = false;
	}
}

int art_text(float x, float y, const char *s, int scale, SDL_Color ink, SDL_Color outline, int align) {
	int w = minifont_width(s, scale), o = (scale + 1) / 3 > 1 ? (scale + 1) / 3 : 1;
	int x0 = (int)floorf(x - (align == 0 ? w / 2.f : align > 0 ? (float)w : 0) + 0.5f), y0 = (int)floorf(y - 5 * scale / 2.f + 0.5f);
	SDL_SetRenderDrawBlendMode(P.renderer, SDL_BLENDMODE_BLEND);
	for (int pass = 0; pass < 2; ++pass) {
		SDL_Color c = pass ? ink : outline;
		if (!c.a) continue;
		SDL_SetRenderDrawColor(P.renderer, c.r, c.g, c.b, c.a);
		for (int i = 0; s[i]; ++i) {
			unsigned short g = minifont_glyph(s[i]);
			for (int row = 0; row < 5; ++row)
				for (int col = 0; col < 3; ++col) {
					if (!(g >> ((4 - row) * 3 + (2 - col)) & 1)) continue;
					int px = x0 + (i * 4 + col) * scale, py = y0 + row * scale;
					SDL_Rect r = pass ? (SDL_Rect){ px, py, scale, scale } : (SDL_Rect){ px - o, py - o, scale + 2 * o, scale + 2 * o };
					SDL_RenderFillRect(P.renderer, &r);
				}
		}
	}
	return w;
}
