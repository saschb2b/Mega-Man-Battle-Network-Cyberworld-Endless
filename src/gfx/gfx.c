#include "gfx.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rom.h"

/* ------------------------------------------------------------------ */
/* Sprites
 *
 * Layout (after a 4-byte header): u32 offsets to animations. An animation
 * is a run of 20-byte frames: tileset, palette, sub-animation and object
 * list offsets, then delay and flags (0x80 last frame, 0x40 loop). Each
 * object is 5 bytes: tile, x, y, size|hflip<<6|vflip<<7, shape|bank<<4.
 * Compressed sprites (pointer bit 31) are LZ77 with a 4-byte prefix. */

typedef struct CachedFrame {
	uint32_t frame_off;
	int pal;
	int part;         /* 0 whole frame, n: only its object n-1 */
	SDL_Texture *tex;
	int ox, oy, w, h; /* bounds relative to origin */
	struct CachedFrame *next;
} CachedFrame;

struct Sprite {
	const uint8_t *base; /* points at the animation table */
	size_t size;
	uint8_t *owned;
	int anims;
	CachedFrame *cache;
};

static Sprite *sprite_table[10][256];

static inline uint32_t le32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

Sprite *sprite_get(int cat, int idx) {
	if (cat < 0 || cat >= 10 || idx < 0 || idx >= 256) return NULL;
	if (sprite_table[cat][idx]) return sprite_table[cat][idx];
	uint32_t list = rom_off(rom_u32(R.layout->sprite_lists + cat * 4));
	uint32_t ptr = rom_u32(list + idx * 4);
	Sprite *s = calloc(1, sizeof *s);
	if (ptr & 0x80000000u) {
		uint32_t off = rom_off(ptr & 0x7FFFFFFFu);
		size_t n;
		uint8_t *d = lz77_decompress(R.data + off, ROM_SIZE - off, &n);
		if (!d || n < 12) { free(d); free(s); return NULL; }
		s->owned = d;
		s->base = d + 8; /* 4-byte size prefix + 4-byte header */
		s->size = n - 8;
	} else if (rom_is_ptr(ptr)) {
		s->base = R.data + rom_off(ptr) + 4;
		s->size = ROM_SIZE - rom_off(ptr) - 4;
	} else {
		free(s);
		return NULL;
	}
	s->anims = (int)(le32(s->base) / 4);
	if (s->anims <= 0 || s->anims > 256) s->anims = 0;
	sprite_table[cat][idx] = s;
	return s;
}

int sprite_anim_count(const Sprite *s) { return s ? s->anims : 0; }

static const uint8_t *frame_ptr(const Sprite *s, int anim, int frame) {
	if (!s || anim < 0 || anim >= s->anims) return NULL;
	const uint8_t *f = s->base + le32(s->base + anim * 4);
	for (int i = 0; i < frame; ++i) {
		if (f[18] & 0x80) return NULL;
		f += 20;
	}
	return f;
}

int sprite_frame_count(const Sprite *s, int anim) {
	const uint8_t *f = frame_ptr(s, anim, 0);
	if (!f) return 0;
	int n = 1;
	while (!(f[18] & 0x80) && n < 256) { f += 20; ++n; }
	return n;
}

static const uint8_t obj_dims[3][4][2] = {
	{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } },
	{ { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
	{ { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } },
};

static CachedFrame *build_frame(Sprite *s, const uint8_t *f, int pal, int part) {
	const uint8_t *b = s->base;
	const uint8_t *tiles = b + le32(f);
	uint32_t tiles_len = le32(tiles);
	tiles += 4;
	const uint8_t *pals = b + le32(f + 4);
	uint32_t pal_len = le32(pals);
	pals += 4;
	const uint8_t *mini = b + le32(f + 8);
	int list = (mini + le32(mini))[0]; /* object list of the first sub-animation entry */
	const uint8_t *objtab = b + le32(f + 12);
	const uint8_t *obj = objtab + le32(objtab + list * 4);

	int minx = 1 << 20, miny = 1 << 20, maxx = -(1 << 20), maxy = -(1 << 20);
	int count = 0;
	int seen = 0;
	for (const uint8_t *o = obj; !(o[0] == 0xFF && o[1] == 0xFF) && seen < 128; o += 5, ++seen) {
		int shape = o[4] & 3, size = o[3] & 3;
		if (shape > 2 || (part && seen != part - 1)) continue;
		++count;
		int x = (int8_t)o[1], y = (int8_t)o[2];
		int w = obj_dims[shape][size][0], h = obj_dims[shape][size][1];
		if (x < minx) minx = x;
		if (y < miny) miny = y;
		if (x + w > maxx) maxx = x + w;
		if (y + h > maxy) maxy = y + h;
	}
	CachedFrame *cf = calloc(1, sizeof *cf);
	cf->frame_off = (uint32_t)(f - b);
	cf->pal = pal;
	cf->part = part;
	if (!count || maxx <= minx) { cf->w = cf->h = 0; return cf; }
	int W = maxx - minx, H = maxy - miny;
	uint32_t *px = calloc((size_t)W * H, 4);
	int banks = (int)(pal_len / 32);
	if (banks < 1) banks = 1;
	int k = 0;
	for (const uint8_t *o = obj; !(o[0] == 0xFF && o[1] == 0xFF); o += 5, ++k) {
		int shape = o[4] & 3, size = o[3] & 3;
		if (shape > 2 || (part && k != part - 1)) continue;
		int x = (int8_t)o[1] - minx, y = (int8_t)o[2] - miny;
		int w = obj_dims[shape][size][0], h = obj_dims[shape][size][1];
		bool hf = o[3] & 0x40, vf = o[3] & 0x80;
		int bank = ((o[4] >> 4) + pal) % banks;
		const uint8_t *pl = pal >= SPRITE_ROM_PAL && pal != SPRITE_WHITE ? R.data + (pal - SPRITE_ROM_PAL) : pals + bank * 32;
		int tw = w / 8;
		for (int ty = 0; ty < h / 8; ++ty) {
			for (int tx = 0; tx < tw; ++tx) {
				uint32_t t = o[0] + ty * tw + tx;
				if ((t + 1) * 32 > tiles_len) continue;
				const uint8_t *td = tiles + t * 32;
				for (int py = 0; py < 8; ++py) {
					for (int pxl = 0; pxl < 8; ++pxl) {
						uint8_t v = td[py * 4 + pxl / 2];
						int ci = (pxl & 1) ? v >> 4 : v & 15;
						if (!ci) continue;
						int X = tx * 8 + pxl, Y = ty * 8 + py;
						if (hf) X = w - 1 - X;
						if (vf) Y = h - 1 - Y;
						px[(y + Y) * W + x + X] = pal == SPRITE_WHITE ? 0xFFFFFFFFu : bgr555((uint16_t)(pl[ci * 2] | pl[ci * 2 + 1] << 8));
					}
				}
			}
		}
	}
	SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(px, W, H, 32, W * 4, SDL_PIXELFORMAT_ARGB8888);
	cf->tex = SDL_CreateTextureFromSurface(P.renderer, surf);
	SDL_SetTextureBlendMode(cf->tex, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(surf);
	free(px);
	cf->ox = minx; cf->oy = miny; cf->w = W; cf->h = H;
	return cf;
}

static CachedFrame *get_frame_part(Sprite *s, int anim, int frame, int pal, int part) {
	const uint8_t *f = frame_ptr(s, anim, frame);
	if (!f) return NULL;
	uint32_t off = (uint32_t)(f - s->base);
	for (CachedFrame *c = s->cache; c; c = c->next)
		if (c->frame_off == off && c->pal == pal && c->part == part) return c;
	CachedFrame *c = build_frame(s, f, pal, part);
	c->next = s->cache;
	s->cache = c;
	return c;
}

static CachedFrame *get_frame(Sprite *s, int anim, int frame, int pal) { return get_frame_part(s, anim, frame, pal, 0); }

void anim_play(Anim *a, Sprite *s, int anim) {
	a->spr = s;
	a->anim = anim;
	a->frame = 0;
	a->done = false;
	a->fresh = false;
	const uint8_t *f = frame_ptr(s, anim, 0);
	a->timer = f ? f[16] : 1;
}

void anim_update(Anim *a) {
	if (a->fresh) { a->fresh = false; return; }
	const uint8_t *f = frame_ptr(a->spr, a->anim, a->frame);
	if (!f) return;
	if (--a->timer > 0) return;
	if (f[18] & 0x80) {
		if (f[18] & 0x40) { anim_play(a, a->spr, a->anim); return; }
		a->done = true;
		a->timer = 1;
		return;
	}
	a->frame++;
	a->timer = f[20 + 16];
	if (a->timer <= 0) a->timer = 1;
}

int gfx_obj_mosaic;
int gfx_obj_alpha = 255;

/* OBJ mosaic: n x n blocks aligned to the 240x160 view, each showing the
 * sprite pixel at its top-left corner (transparent when that is outside). */
static void draw_mosaic(CachedFrame *c, SDL_Rect dst, bool flip, int n) {
	int x0 = dst.x - ((dst.x - P.core_x) % n + n) % n;
	int y0 = dst.y - ((dst.y - P.core_y) % n + n) % n;
	for (int by = y0; by < dst.y + dst.h; by += n) {
		int sy = by - dst.y;
		if (sy < 0) continue;
		for (int bx = x0; bx < dst.x + dst.w; bx += n) {
			int sx = bx - dst.x;
			if (sx < 0) continue;
			SDL_Rect src = { flip ? c->w - 1 - sx : sx, sy, 1, 1 }, d = { bx, by, n, n };
			SDL_RenderCopy(P.renderer, c->tex, &src, &d);
		}
	}
}

static void draw_cached(CachedFrame *c, int x, int y, bool flip, int fx) {
	if (!c || !c->tex) return;
	SDL_Rect dst = { flip ? x - c->ox - c->w : x + c->ox, y + c->oy, c->w, c->h };
	Uint8 alpha = (Uint8)((fx & FX_GHOST) ? gfx_obj_alpha / 2 : gfx_obj_alpha);
	SDL_SetTextureAlphaMod(c->tex, alpha);
	if (fx & FX_DARK) SDL_SetTextureColorMod(c->tex, 90, 90, 120);
	else SDL_SetTextureColorMod(c->tex, 255, 255, 255);
	SDL_SetTextureBlendMode(c->tex, SDL_BLENDMODE_BLEND);
	if (gfx_obj_mosaic > 1) { draw_mosaic(c, dst, flip, gfx_obj_mosaic); return; }
	SDL_RenderCopyEx(P.renderer, c->tex, NULL, &dst, 0, NULL, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
	if (fx & FX_FLASH) {
		SDL_SetTextureBlendMode(c->tex, SDL_BLENDMODE_ADD);
		SDL_RenderCopyEx(P.renderer, c->tex, NULL, &dst, 0, NULL, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
		SDL_RenderCopyEx(P.renderer, c->tex, NULL, &dst, 0, NULL, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
		SDL_SetTextureBlendMode(c->tex, SDL_BLENDMODE_BLEND);
	}
}

void sprite_draw_frame(Sprite *s, int anim, int frame, int x, int y, bool flip, int pal, int fx) {
	draw_cached(get_frame(s, anim, frame, pal), x, y, flip, fx);
}

void anim_draw(const Anim *a, int x, int y, bool flip, int pal, int fx) {
	if (!a->spr) return;
	sprite_draw_frame(a->spr, a->anim, a->frame, x, y, flip, pal, fx);
}

/* ------------------------------------------------------------------ */
/* Tiles to textures */

static void blit_tile(uint32_t *dst, int stride, int dx, int dy, const uint8_t *tile, const uint8_t *pal, bool hf, bool vf) {
	for (int py = 0; py < 8; ++py) {
		for (int px = 0; px < 8; ++px) {
			uint8_t v = tile[py * 4 + px / 2];
			int ci = (px & 1) ? v >> 4 : v & 15;
			if (!ci) continue;
			int X = hf ? 7 - px : px, Y = vf ? 7 - py : py;
			dst[(dy + Y) * stride + dx + X] = bgr555((uint16_t)(pal[ci * 2] | pal[ci * 2 + 1] << 8));
		}
	}
}

static SDL_Texture *texture_from_pixels(uint32_t *px, int w, int h) {
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(px, w, h, 32, w * 4, SDL_PIXELFORMAT_ARGB8888);
	SDL_Texture *t = SDL_CreateTextureFromSurface(P.renderer, s);
	SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(s);
	return t;
}

/* ------------------------------------------------------------------ */
/* Single ROM tiles, cached by (tile, palette, flip) */

/* A tile is a ROM offset, or 0x80000000 | block << 24 | offset into one of
 * the LZ77 blocks registered with gfx_lz_ref. */
static uint32_t extra_lz[8]; /* blocks registered with gfx_lz_ref */
static int extra_n;

uint32_t gfx_lz_ref(uint32_t lz) {
	int i = 0;
	while (i < extra_n && extra_lz[i] != lz) ++i;
	if (i == extra_n) {
		if (extra_n >= 8) return 0;
		extra_lz[extra_n++] = lz;
	}
	return 0x80000000u | (uint32_t)i << 24;
}

static const uint8_t *tile_data(uint32_t tile) {
	static uint8_t *blocks[8];
	static size_t block_len[8];
	if (!(tile & 0x80000000u)) return tile + 32 <= ROM_SIZE ? R.data + tile : NULL;
	int b = (int)((tile >> 24) & 0x7F);
	uint32_t off = tile & 0xFFFFFF;
	if (b >= extra_n) return NULL;
	uint32_t src = extra_lz[b];
	if (!blocks[b]) blocks[b] = lz77_decompress(R.data + src, ROM_SIZE - src, &block_len[b]);
	return blocks[b] && off + 32 <= block_len[b] ? blocks[b] + off : NULL;
}

typedef struct { uint32_t key_tile, key_pal; uint8_t flip, used; SDL_Texture *tex; } TileEntry;

static const uint8_t *pal_bytes(uint32_t pal) {
	return pal + 32 <= ROM_SIZE ? R.data + pal : NULL;
}
static TileEntry tile_cache[4096];

static SDL_Texture *tile_texture(uint32_t tile, uint32_t pal, int flip) {
	uint32_t h = (tile * 2654435761u ^ pal * 40503u ^ (uint32_t)flip) & 4095;
	for (int probe = 0; probe < 4096; ++probe, h = (h + 1) & 4095) {
		TileEntry *e = &tile_cache[h];
		if (e->used && e->key_tile == tile && e->key_pal == pal && e->flip == flip) return e->tex;
		if (!e->used) {
			uint32_t px[64] = { 0 };
			const uint8_t *pb = pal_bytes(pal), *td = tile_data(tile);
			if (td && pb) blit_tile(px, 8, 0, 0, td, pb, flip & 1, flip & 2);
			e->used = 1;
			e->key_tile = tile;
			e->key_pal = pal;
			e->flip = (uint8_t)flip;
			e->tex = texture_from_pixels(px, 8, 8);
			return e->tex;
		}
	}
	return NULL;
}

void rom_tile(uint32_t tile, uint32_t pal, int x, int y, int flip) {
	SDL_Texture *t = tile_texture(tile, pal, flip);
	if (!t) return;
	SDL_Rect d = { x, y, 8, 8 };
	SDL_RenderCopy(P.renderer, t, NULL, &d);
}

void rom_tiles(uint32_t first, uint32_t pal, int x, int y, int w, int h, int flip) {
	for (int ty = 0; ty < h; ++ty)
		for (int tx = 0; tx < w; ++tx) {
			int dx = (flip & 1) ? w - 1 - tx : tx, dy = (flip & 2) ? h - 1 - ty : ty;
			rom_tile(first + (uint32_t)(ty * w + tx) * 32, pal, x + dx * 8, y + dy * 8, flip);
		}
}

/* ------------------------------------------------------------------ */
/* Font: 8x16 cells at FONT_BASE + code * 64, two tones (fill, shade). */

#define FONT_BASE 0x6B5A2C
#define FONT_TOP 2
static SDL_Texture *font_tex;
static uint8_t font_w[256];

static int ascii_code(unsigned char ch) {
	if (ch >= 1 && ch <= 5) return 0x40 + ch - 1; /* version marks */
	if (ch == ' ') return 0;
	if (ch >= '0' && ch <= '9') return 1 + ch - '0';
	if (ch >= 'A' && ch <= 'Z') return 0x0B + ch - 'A';
	if (ch >= 'a' && ch <= 'z') return 0x26 + ch - 'a';
	switch (ch) {
	case '*': return 0x25;
	case '-': return 0x98;
	case '=': return 0x9A;
	case ':': return 0x9B;
	case '%': return 0x9C;
	case '?': return 0x9D;
	case '+': return 0x9E;
	case '!': return 0xA2;
	case '&': return 0xA3;
	case ',': return 0xA4;
	case '.': return 0xA6;
	case ';': return 0xA8;
	case '\'': return 0xA9;
	case '"': return 0xAA;
	case '~': return 0xAB;
	case '/': return 0xAC;
	case '(': return 0xAD;
	case ')': return 0xAE;
	case '>': return 0xB1;
	case '_': return 0xB2;
	default: return -1;
	}
}

static void build_font(void) {
	/* Atlas of 16x16 cells indexed by byte 0-127 (1-5 are version marks,
	 * 32-127 ASCII). Fill is white so it can be tinted; shade stays dark. */
	static uint32_t px[16 * 16 * 128];
	memset(px, 0, sizeof px);
	for (int ch = 1; ch < 128; ++ch) {
		if (ch > 5 && ch < 32) continue;
		int code = ascii_code((unsigned char)ch);
		int cell = ch;
		if (code < 0) { font_w[ch] = 4; continue; }
		const uint8_t *g = R.data + FONT_BASE + code * 64;
		int maxx = 0;
		for (int half = 0; half < 2; ++half) {
			for (int py = 0; py < 8; ++py) {
				for (int x = 0; x < 8; ++x) {
					uint8_t v = g[half * 32 + py * 4 + x / 2];
					int ci = (x & 1) ? v >> 4 : v & 15;
					if (!ci) continue;
					int y = half * 8 + py - FONT_TOP;
					if (y < 0 || y >= 16) continue;
					px[y * 16 * 128 + cell * 16 + x] = ci == 1 ? 0xFFFFFFFFu : 0xFF182040u;
					if (x + 1 > maxx) maxx = x + 1;
				}
			}
		}
		font_w[ch] = (uint8_t)(code == 0 ? 4 : maxx);
	}
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(px, 16 * 128, 16, 32, 16 * 128 * 4, SDL_PIXELFORMAT_ARGB8888);
	font_tex = SDL_CreateTextureFromSurface(P.renderer, s);
	SDL_SetTextureBlendMode(font_tex, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(s);
}

int text_width(const char *s) {
	int w = 0;
	for (; *s; ++s) {
		unsigned char ch = (unsigned char)*s;
		if ((ch < 32 && ch > 5) || ch == 0 || ch >= 128) continue;
		w += font_w[ch];
	}
	return w;
}

void text_draw_scaled(int x, int y, const char *s, SDL_Color c, int align, int scale) {
	if (align == TEXT_CENTER) x -= text_width(s) * scale / 2;
	else if (align == TEXT_RIGHT) x -= text_width(s) * scale;
	SDL_SetTextureColorMod(font_tex, c.r, c.g, c.b);
	SDL_SetTextureAlphaMod(font_tex, c.a);
	for (; *s; ++s) {
		unsigned char ch = (unsigned char)*s;
		if ((ch < 32 && ch > 5) || ch >= 128) continue;
		SDL_Rect src = { ch * 16, 0, 16, 16 };
		SDL_Rect dst = { x, y, 16 * scale, 16 * scale };
		SDL_RenderCopy(P.renderer, font_tex, &src, &dst);
		x += font_w[ch] * scale;
	}
}

void text_draw(int x, int y, const char *s, SDL_Color c, int align) { text_draw_scaled(x, y, s, c, align, 1); }

void text_drawf(int x, int y, SDL_Color c, int align, const char *fmt, ...) {
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);
	text_draw(x, y, buf, c, align);
}

/* ------------------------------------------------------------------ */

void fill_rect(int x, int y, int w, int h, SDL_Color c) {
	SDL_SetRenderDrawBlendMode(P.renderer, c.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(P.renderer, c.r, c.g, c.b, c.a);
	SDL_Rect r = { x, y, w, h };
	SDL_RenderFillRect(P.renderer, &r);
}

bool gfx_init(void) {
	build_font();
	return true;
}
