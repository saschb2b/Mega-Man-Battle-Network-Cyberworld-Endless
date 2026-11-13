#include "gba_layers.h"

#include <string.h>

#define W 240
#define H 160

static unsigned rd16(const uint8_t *p, unsigned at) { return (unsigned)p[at] | (unsigned)p[at + 1] << 8; }

/* ---- the backgrounds: whether background `b` has a pixel at (x, y) ---- */

typedef struct { bool on; unsigned prio, chars, screen, size, hofs, vofs; bool wide; } Bg;

static Bg bg_read(const GbaVideo *v, int b) {
	unsigned cnt = rd16(v->io, 0x08u + 2u * (unsigned)b);
	return (Bg){ (rd16(v->io, 0) >> (8 + b) & 1u) != 0, cnt & 3u, (cnt >> 2 & 3u) * 0x4000u, (cnt >> 8 & 31u) * 0x800u, cnt >> 14,
		rd16(v->io, 0x10u + 4u * (unsigned)b) & 0x1FFu, rd16(v->io, 0x12u + 4u * (unsigned)b) & 0x1FFu, (cnt & 0x80u) != 0 };
}

static bool bg_pixel(const GbaVideo *v, const Bg *b, int x, int y) {
	unsigned px = ((unsigned)x + b->hofs) & ((b->size & 1u) ? 511u : 255u), py = ((unsigned)y + b->vofs) & ((b->size & 2u) ? 511u : 255u);
	/* (the screen blocks: side by side, one over the other, or two rows of two) */
	unsigned block = (px >> 8) + (py >> 8) * ((b->size & 1u) ? 2u : 1u);
	unsigned e = rd16(v->vram, (b->screen + block * 0x800u + ((py >> 3 & 31u) * 32u + (px >> 3 & 31u)) * 2u) & 0xFFFEu);
	unsigned tx = px & 7u, ty = py & 7u;
	if (e & 0x400u) tx = 7u - tx;
	if (e & 0x800u) ty = 7u - ty;
	if (b->wide) return v->vram[(b->chars + (e & 0x3FFu) * 64u + ty * 8u + tx) & 0xFFFFu] != 0;
	unsigned byte = v->vram[(b->chars + (e & 0x3FFu) * 32u + ty * 4u + (tx >> 1)) & 0xFFFFu];
	return ((tx & 1u) ? byte >> 4 : byte & 15u) != 0;
}

/* ---- the sprites ---- */

static const uint8_t sizes[3][4][2] = { { { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } }, { { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
	{ { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } } };

/* The sprites' palette entry (of their 256) a sprite's tiles show at
 * (tx, ty) in it, 0 clear */
static unsigned obj_pixel(const GbaVideo *v, unsigned a0, unsigned a2, int w, int tx, int ty, bool flat) {
	unsigned tile = a2 & 0x3FFu, cx = (unsigned)tx >> 3, cy = (unsigned)ty >> 3, ix = (unsigned)tx & 7u, iy = (unsigned)ty & 7u;
	if (a0 & 0x2000u) {
		tile += flat ? (cy * (unsigned)(w >> 3) + cx) * 2u : cy * 32u + cx * 2u;
		return v->vram[0x10000u + (tile & 0x3FFu) * 32u + iy * 8u + ix];
	}
	tile += flat ? cy * (unsigned)(w >> 3) + cx : cy * 32u + cx;
	unsigned byte = v->vram[0x10000u + (tile & 0x3FFu) * 32u + iy * 4u + (ix >> 1)];
	unsigned n = (ix & 1u) ? byte >> 4 : byte & 15u;
	return n ? (a2 >> 12) * 16u + n : 0u;
}

/* A sprite's palette entry as mGBA shows it (5 bits a channel, its top
 * bits again below) */
static uint32_t obj_colour(const GbaVideo *v, unsigned n) {
	unsigned c = rd16(v->pal, 0x200u + n * 2u);
	uint32_t out = 0xFF000000u;
	for (unsigned s = 0; s < 3; ++s) {
		unsigned ch = c >> (5u * s) & 31u;
		out |= (ch << 3 | ch >> 2) << (8u * s);
	}
	return out;
}

typedef struct { unsigned a0, a1, a2; int x0, y0, w, h, bw, bh, pa, pb, pc, pd; bool turned; } Obj;

/* OAM's entry `n`: false where it is not shown (off, a window's, or off the screen) */
static bool obj_read(const GbaVideo *v, int n, Obj *o) {
	unsigned at = (unsigned)n * 8u;
	*o = (Obj){ rd16(v->oam, at), rd16(v->oam, at + 2u), rd16(v->oam, at + 4u), 0, 0, 0, 0, 0, 0, 256, 0, 0, 256, false };
	o->turned = (o->a0 & 0x100u) != 0;
	unsigned shape = o->a0 >> 14;
	if ((!o->turned && (o->a0 & 0x200u)) || (o->a0 >> 10 & 3u) == 2 || shape == 3) return false;
	bool twice = o->turned && (o->a0 & 0x200u);
	o->w = sizes[shape][o->a1 >> 14][0];
	o->h = sizes[shape][o->a1 >> 14][1];
	o->bw = twice ? o->w * 2 : o->w;
	o->bh = twice ? o->h * 2 : o->h;
	o->x0 = (int)(o->a1 & 0x1FFu);
	o->y0 = (int)(o->a0 & 0xFFu);
	if (o->x0 >= W) o->x0 -= 512;
	if (o->y0 >= H) o->y0 -= 256;
	if (o->turned) {
		unsigned group = (o->a1 >> 9 & 31u) * 32u;
		o->pa = (int16_t)rd16(v->oam, group + 6u); o->pb = (int16_t)rd16(v->oam, group + 14u);
		o->pc = (int16_t)rd16(v->oam, group + 22u); o->pd = (int16_t)rd16(v->oam, group + 30u);
	}
	return o->x0 + o->bw > 0 && o->y0 + o->bh > 0;
}

/* The pixel of its tiles its box's (ix, iy) shows: false where none */
static bool obj_texel(const Obj *o, int ix, int iy, int *tx, int *ty) {
	if (!o->turned) {
		*tx = (o->a1 & 0x1000u) ? o->w - 1 - ix : ix;
		*ty = (o->a1 & 0x2000u) ? o->h - 1 - iy : iy;
		return true;
	}
	int dx = ix - o->bw / 2, dy = iy - o->bh / 2;
	*tx = ((o->pa * dx + o->pb * dy) >> 8) + o->w / 2;
	*ty = ((o->pc * dx + o->pd * dy) >> 8) + o->h / 2;
	return *tx >= 0 && *ty >= 0 && *tx < o->w && *ty < o->h;
}

/* A sprite of one colour, wider than high: the blob BN6 lays under one who
 * stands (measured: MegaMan's a 32x8 of one dark colour) */
static bool obj_blob(const GbaVideo *v, const Obj *o, bool flat) {
	unsigned first = 0;
	if (o->turned || o->w < 2 * o->h) return false;
	for (int ty = 0; ty < o->h; ++ty)
		for (int tx = 0; tx < o->w; ++tx) {
			unsigned c = obj_pixel(v, o->a0, o->a2, o->w, tx, ty, flat);
			if (c && first && c != first) return false;
			if (c) first = c;
		}
	return first != 0;
}

static void objs(const GbaVideo *v, GbaLayers *out, uint8_t *prio) {
	bool flat = (rd16(v->io, 0) & 0x40u) != 0;
	for (int n = 0; n < GBA_OBJS; ++n) {
		Obj o;
		out->w[n] = 0;
		if (!obj_read(v, n, &o)) continue;
		out->x[n] = (int16_t)o.x0; out->y[n] = (int16_t)o.y0;
		out->w[n] = (uint8_t)o.bw; out->h[n] = (uint8_t)o.bh;
		unsigned p = o.a2 >> 10 & 3u;
		bool lies = (o.a0 >> 10 & 3u) == 0 && (p == 3 || obj_blob(v, &o, flat));
		for (int iy = 0; iy < o.bh; ++iy)
			for (int ix = 0; ix < o.bw; ++ix) {
				int sx = o.x0 + ix, sy = o.y0 + iy, tx, ty, i = sy * W + sx;
				if (sx < 0 || sy < 0 || sx >= W || sy >= H) continue;
				unsigned c = obj_texel(&o, ix, iy, &tx, &ty) ? obj_pixel(v, o.a0, o.a2, o.w, tx, ty, flat) : 0;
				if (!c) continue;
				/* (one that lies on the floor: its colour kept, under whoever stands on it) */
				if (lies) {
					if (!out->lying[i] && v->pal) out->lying[i] = obj_colour(v, c);
					continue;
				}
				/* (of two on a pixel the lower priority's, then the earlier entry's) */
				if (out->obj[i] && prio[i] <= p) continue;
				out->obj[i] = (uint8_t)(n + 1);
				out->sheer[i] = (o.a0 >> 10 & 3u) == 1;
				prio[i] = (uint8_t)p;
			}
	}
}

bool gba_bg3_front(const uint8_t *io) {
	unsigned on = rd16(io, 0) >> 8, p3 = rd16(io, 0x0E) & 3u;
	if (!(on & 8u)) return false;
	for (unsigned b = 1; b <= 2; ++b)
		if ((on >> b & 1u) && (rd16(io, 0x08u + 2u * b) & 3u) <= p3) return false;
	return true;
}

bool gba_layers(const GbaVideo *v, GbaLayers *out) {
	static uint8_t prio[W * H];
	unsigned dispcnt = rd16(v->io, 0);
	memset(out->obj, 0, sizeof out->obj);
	memset(out->sheer, 0, sizeof out->sheer);
	memset(out->lying, 0, sizeof out->lying);
	if (dispcnt & 7u) return false;
	if (dispcnt & 0x1000u) objs(v, out, prio);
	else memset(out->w, 0, sizeof out->w);
	Bg bg[4];
	for (int b = 0; b < 4; ++b) bg[b] = bg_read(v, b);
	bool front3 = gba_bg3_front(v->io);
	for (int y = 0; y < H; ++y)
		for (int x = 0; x < W; ++x) {
			int i = y * W + x, best = GBA_LAYER_NONE;
			unsigned best_p = 4;
			for (int b = 0; b < 4; ++b)
				if (bg[b].on && bg[b].prio < best_p && bg_pixel(v, &bg[b], x, y)) { best = b; best_p = bg[b].prio; }
			out->under[i] = (uint8_t)best;
			out->hud[i] = best == 0 || (best == 3 && front3);
			/* (a sprite over a background of its priority) */
			out->top[i] = out->obj[i] && prio[i] <= best_p ? GBA_LAYER_OBJ : (uint8_t)best;
		}
	return true;
}
