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

static bool obj_pixel(const GbaVideo *v, unsigned a0, unsigned a2, int w, int tx, int ty, bool flat) {
	unsigned tile = a2 & 0x3FFu, cx = (unsigned)tx >> 3, cy = (unsigned)ty >> 3, ix = (unsigned)tx & 7u, iy = (unsigned)ty & 7u;
	if (a0 & 0x2000u) {
		tile += flat ? (cy * (unsigned)(w >> 3) + cx) * 2u : cy * 32u + cx * 2u;
		return v->vram[0x10000u + (tile & 0x3FFu) * 32u + iy * 8u + ix] != 0;
	}
	tile += flat ? cy * (unsigned)(w >> 3) + cx : cy * 32u + cx;
	unsigned byte = v->vram[0x10000u + (tile & 0x3FFu) * 32u + iy * 4u + (ix >> 1)];
	return ((ix & 1u) ? byte >> 4 : byte & 15u) != 0;
}

static void objs(const GbaVideo *v, GbaLayers *out, uint8_t *prio) {
	bool flat = (rd16(v->io, 0) & 0x40u) != 0;
	for (int n = 0; n < GBA_OBJS; ++n) {
		unsigned a0 = rd16(v->oam, (unsigned)n * 8u), a1 = rd16(v->oam, (unsigned)n * 8u + 2u), a2 = rd16(v->oam, (unsigned)n * 8u + 4u);
		bool turned = (a0 & 0x100u) != 0, twice = turned && (a0 & 0x200u);
		unsigned mode = a0 >> 10 & 3u, shape = a0 >> 14;
		out->w[n] = 0;
		if ((!turned && (a0 & 0x200u)) || mode == 2 || shape == 3) continue;
		int w = sizes[shape][a1 >> 14][0], h = sizes[shape][a1 >> 14][1], bw = twice ? w * 2 : w, bh = twice ? h * 2 : h;
		int x0 = (int)(a1 & 0x1FFu), y0 = (int)(a0 & 0xFFu);
		if (x0 >= W) x0 -= 512;
		if (y0 >= H) y0 -= 256;
		if (x0 + bw <= 0 || y0 + bh <= 0) continue;
		int pa = 256, pb = 0, pc = 0, pd = 256;
		if (turned) {
			unsigned at = (a1 >> 9 & 31u) * 32u;
			pa = (int16_t)rd16(v->oam, at + 6u); pb = (int16_t)rd16(v->oam, at + 14u);
			pc = (int16_t)rd16(v->oam, at + 22u); pd = (int16_t)rd16(v->oam, at + 30u);
		}
		out->x[n] = (int16_t)x0; out->y[n] = (int16_t)y0;
		out->w[n] = (uint8_t)bw; out->h[n] = (uint8_t)bh;
		unsigned p = a2 >> 10 & 3u;
		for (int iy = 0; iy < bh; ++iy)
			for (int ix = 0; ix < bw; ++ix) {
				int sx = x0 + ix, sy = y0 + iy, tx = ix, ty = iy;
				if (sx < 0 || sy < 0 || sx >= W || sy >= H) continue;
				if (turned) {
					int dx = ix - bw / 2, dy = iy - bh / 2;
					tx = ((pa * dx + pb * dy) >> 8) + w / 2;
					ty = ((pc * dx + pd * dy) >> 8) + h / 2;
					if (tx < 0 || ty < 0 || tx >= w || ty >= h) continue;
				} else {
					if (a1 & 0x1000u) tx = w - 1 - tx;
					if (a1 & 0x2000u) ty = h - 1 - ty;
				}
				int i = sy * W + sx;
				/* (of two on a pixel the lower priority's, then the earlier entry's) */
				if (out->obj[i] && prio[i] <= p) continue;
				if (!obj_pixel(v, a0, a2, w, tx, ty, flat)) continue;
				out->obj[i] = (uint8_t)(n + 1);
				out->sheer[i] = mode == 1;
				prio[i] = (uint8_t)p;
			}
	}
}

bool gba_layers(const GbaVideo *v, GbaLayers *out) {
	static uint8_t prio[W * H];
	unsigned dispcnt = rd16(v->io, 0);
	memset(out->obj, 0, sizeof out->obj);
	memset(out->sheer, 0, sizeof out->sheer);
	if (dispcnt & 7u) return false;
	if (dispcnt & 0x1000u) objs(v, out, prio);
	else memset(out->w, 0, sizeof out->w);
	Bg bg[4];
	for (int b = 0; b < 4; ++b) bg[b] = bg_read(v, b);
	for (int y = 0; y < H; ++y)
		for (int x = 0; x < W; ++x) {
			int i = y * W + x, best = GBA_LAYER_NONE;
			unsigned best_p = 4;
			for (int b = 0; b < 4; ++b)
				if (bg[b].on && bg[b].prio < best_p && bg_pixel(v, &bg[b], x, y)) { best = b; best_p = bg[b].prio; }
			out->under[i] = (uint8_t)best;
			/* (a sprite over a background of its priority) */
			out->top[i] = out->obj[i] && prio[i] <= best_p ? GBA_LAYER_OBJ : (uint8_t)best;
		}
	return true;
}
