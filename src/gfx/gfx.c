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

/* CachedFrame's part for a whole frame kept as pixels, for memory */
#define RAW_PART (-1)

typedef struct CachedFrame {
	uint32_t frame_off;
	int pal;
	int part;         /* 0 whole frame, n: only its object n-1 */
	SDL_Texture *tex;
	uint32_t *raw;    /* ... or, drawn into memory (sprite_draw_into), its RGBA8888 pixels */
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

/* Sprite `idx` of list `cat` in ROM `rom`, whose list table is at `lists` */
static Sprite *sprite_load(const uint8_t *rom, uint32_t lists, int cat, int idx) {
	uint32_t at = lists + (uint32_t)cat * 4;
	if (!rom || at + 4 > ROM_SIZE || !rom_is_ptr(le32(rom + at))) return NULL;
	uint32_t list = rom_off(le32(rom + at)) + (uint32_t)idx * 4;
	if (list + 4 > ROM_SIZE) return NULL;
	uint32_t ptr = le32(rom + list);
	Sprite *s = calloc(1, sizeof *s);
	if (!s) return NULL;
	if (ptr & 0x80000000u && rom_is_ptr(ptr & 0x7FFFFFFFu)) {
		uint32_t off = rom_off(ptr & 0x7FFFFFFFu);
		size_t n;
		uint8_t *d = lz77_decompress(rom + off, ROM_SIZE - off, &n);
		if (!d || n < 12) { free(d); free(s); return NULL; }
		s->owned = d;
		s->base = d + 8; /* 4-byte size prefix + 4-byte header */
		s->size = n - 8;
	} else if (rom_is_ptr(ptr)) {
		s->base = rom + rom_off(ptr) + 4;
		s->size = ROM_SIZE - rom_off(ptr) - 4;
	} else {
		free(s);
		return NULL;
	}
	s->anims = (int)(le32(s->base) / 4);
	if (s->anims <= 0 || s->anims > 256) s->anims = 0;
	return s;
}

Sprite *sprite_get(int cat, int idx) {
	if (cat < 0 || cat >= 10 || idx < 0 || idx >= 256 || !R.layout) return NULL;
	if (!sprite_table[cat][idx]) sprite_table[cat][idx] = sprite_load(R.data, R.layout->sprite_lists, cat, idx);
	return sprite_table[cat][idx];
}

Sprite *sprite_get_rom(const uint8_t *rom, uint32_t lists, int cat, int idx) {
	/* (a few, kept: the launcher's cartridges' faces) */
	static struct { const uint8_t *rom; int cat, idx; Sprite *s; } kept[8];
	if (!rom || cat < 0 || cat >= 10 || idx < 0 || idx >= 256) return NULL;
	if (rom == R.data) return sprite_get(cat, idx);
	for (int i = 0; i < 8; ++i)
		if (kept[i].s && kept[i].rom == rom && kept[i].cat == cat && kept[i].idx == idx) return kept[i].s;
	for (int i = 0; i < 8; ++i)
		if (!kept[i].s) {
			kept[i].s = sprite_load(rom, lists, cat, idx);
			kept[i].rom = rom;
			kept[i].cat = cat;
			kept[i].idx = idx;
			return kept[i].s;
		}
	return NULL;
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

/* One OBJ's tiles into a frame's pixels (W wide) at (x, y), turned as it
 * says, in palette pl (NULL: white). */
static void blit_obj(uint32_t *px, int W, const uint8_t *o, int x, int y, const uint8_t *tiles, uint32_t tiles_len, const uint8_t *pl) {
	int shape = o[4] & 3, size = o[3] & 3;
	int w = obj_dims[shape][size][0], h = obj_dims[shape][size][1];
	bool hf = o[3] & 0x40, vf = o[3] & 0x80;
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
					px[(y + Y) * W + x + X] = !pl ? 0xFFFFFFFFu : bgr555((uint16_t)(pl[ci * 2] | pl[ci * 2 + 1] << 8));
				}
			}
		}
	}
}

/* A frame's objects' bounds (object part - 1 alone where part > 0): how
 * many objects it has */
static int frame_bounds(const uint8_t *obj, int part, int *minx, int *miny, int *maxx, int *maxy) {
	int count = 0, seen = 0;
	*minx = *miny = 1 << 20;
	*maxx = *maxy = -(1 << 20);
	for (const uint8_t *o = obj; !(o[0] == 0xFF && o[1] == 0xFF) && seen < 128; o += 5, ++seen) {
		int shape = o[4] & 3, size = o[3] & 3;
		if (shape > 2 || (part > 0 && seen != part - 1)) continue;
		++count;
		int x = (int8_t)o[1], y = (int8_t)o[2];
		int w = obj_dims[shape][size][0], h = obj_dims[shape][size][1];
		if (x < *minx) *minx = x;
		if (y < *miny) *miny = y;
		if (x + w > *maxx) *maxx = x + w;
		if (y + h > *maxy) *maxy = y + h;
	}
	return count;
}

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

	int minx, miny, maxx, maxy, count = frame_bounds(obj, part, &minx, &miny, &maxx, &maxy);
	CachedFrame *cf = calloc(1, sizeof *cf);
	if (!cf) return NULL;
	cf->frame_off = (uint32_t)(f - b);
	cf->pal = pal;
	cf->part = part;
	if (!count || maxx <= minx) { cf->w = cf->h = 0; return cf; }
	int W = maxx - minx, H = maxy - miny;
	uint32_t *px = calloc((size_t)W * H, 4);
	if (!px) { cf->w = cf->h = 0; return cf; }   /* (drawn as nothing, as an empty frame) */
	int banks = (int)(pal_len / 32);
	if (banks < 1) banks = 1;
	int k = 0;
	for (const uint8_t *o = obj; !(o[0] == 0xFF && o[1] == 0xFF); o += 5, ++k) {
		if ((o[4] & 3) > 2 || (part > 0 && k != part - 1)) continue;
		int bank = ((o[4] >> 4) + pal) % banks;
		const uint8_t *pl = pal >= SPRITE_ROM_PAL && pal != SPRITE_WHITE ? R.data + (pal - SPRITE_ROM_PAL) : pals + bank * 32;
		blit_obj(px, W, o, (int8_t)o[1] - minx, (int8_t)o[2] - miny, tiles, tiles_len, pal == SPRITE_WHITE ? NULL : pl);
	}
	cf->ox = minx; cf->oy = miny; cf->w = W; cf->h = H;
	/* (kept as pixels for memory: ARGB to RGBA, colour 0 left 0) */
	if (part == RAW_PART) {
		for (int i = 0; i < W * H; ++i)
			if (px[i]) px[i] = px[i] << 8 | 0xFF;
		cf->raw = px;
		return cf;
	}
	SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(px, W, H, 32, W * 4, SDL_PIXELFORMAT_ARGB8888);
	cf->tex = SDL_CreateTextureFromSurface(P.renderer, surf);
	SDL_SetTextureBlendMode(cf->tex, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(surf);
	free(px);
	return cf;
}

static CachedFrame *get_frame_part(Sprite *s, int anim, int frame, int pal, int part) {
	const uint8_t *f = frame_ptr(s, anim, frame);
	if (!f) return NULL;
	uint32_t off = (uint32_t)(f - s->base);
	for (CachedFrame *c = s->cache; c; c = c->next)
		if (c->frame_off == off && c->pal == pal && c->part == part) return c;
	CachedFrame *c = build_frame(s, f, pal, part);
	if (!c) return NULL;
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

static void raw_tile(uint32_t tile, uint32_t pal, int x, int y, int flip);
static bool raw_tile_on(void);

/* One 8x8 ROM tile with a ROM palette. flip: bit 0 horizontal, bit 1 vertical. */
static void rom_tile(uint32_t tile, uint32_t pal, int x, int y, int flip) {
	if (raw_tile_on()) {
		raw_tile(tile, pal, x, y, flip);
		return;
	}
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

static void raw_tile_scaled(uint32_t tile, uint32_t pal, int x, int y, int scale);

void rom_tiles_scaled(uint32_t first, uint32_t pal, int x, int y, int w, int h, int scale) {
	for (int ty = 0; ty < h; ++ty)
		for (int tx = 0; tx < w; ++tx) {
			uint32_t tile = first + (uint32_t)(ty * w + tx) * 32;
			int dx = x + tx * 8 * scale, dy = y + ty * 8 * scale;
			if (raw_tile_on()) {
				raw_tile_scaled(tile, pal, dx, dy, scale);
				continue;
			}
			SDL_Texture *t = tile_texture(tile, pal, 0);
			SDL_Rect d = { dx, dy, 8 * scale, 8 * scale };
			if (t) SDL_RenderCopy(P.renderer, t, NULL, &d);
		}
}

/* ------------------------------------------------------------------ */
/* Drawing into memory (gfx_draw_into): RGBA8888 pixels, blended as SDL's
 * software renderer blends them, so the picture is its own to the pixel. */

static struct { uint32_t *px; int w, h, stride; } raw;

/* ROM tiles as drawn into memory, decoded once: RGBA8888, 0 for colour 0
 * (a second screen's thirty chip icons decoded at each draw were 2 ms of a
 * New 3DS's) */
#define RAW_TILES 1024
static struct { uint32_t tile, pal; int flip; bool used; uint32_t px[64]; } raw_tiles[RAW_TILES];
static int raw_tiles_used;

static const uint32_t *raw_tile_px(uint32_t tile, uint32_t pal, int flip) {
	uint32_t h = (tile * 2654435761u ^ pal * 40503u ^ (uint32_t)flip) & (RAW_TILES - 1);
	for (;; h = (h + 1) & (RAW_TILES - 1)) {
		if (raw_tiles[h].used && raw_tiles[h].tile == tile && raw_tiles[h].pal == pal && raw_tiles[h].flip == flip) return raw_tiles[h].px;
		if (!raw_tiles[h].used) break;
	}
	/* (full past three quarters: begun again) */
	if (raw_tiles_used >= RAW_TILES * 3 / 4) {
		memset(raw_tiles, 0, sizeof raw_tiles);
		raw_tiles_used = 0;
		h = (tile * 2654435761u ^ pal * 40503u ^ (uint32_t)flip) & (RAW_TILES - 1);
	}
	const uint8_t *pb = pal_bytes(pal), *td = tile_data(tile);
	if (!td || !pb) return NULL;
	uint32_t *px = raw_tiles[h].px;
	memset(px, 0, sizeof raw_tiles[h].px);
	blit_tile(px, 8, 0, 0, td, pb, flip & 1, flip & 2);
	for (int i = 0; i < 64; ++i)
		if (px[i]) px[i] = px[i] << 8 | 0xFF;
	raw_tiles[h].tile = tile;
	raw_tiles[h].pal = pal;
	raw_tiles[h].flip = flip;
	raw_tiles[h].used = true;
	++raw_tiles_used;
	return px;
}

/* ... `scale` times as large, a block of pixels each */
static void raw_tile_scaled(uint32_t tile, uint32_t pal, int x, int y, int scale) {
	const uint32_t *px = raw_tile_px(tile, pal, 0);
	if (!px) return;
	for (int j = 0; j < 8 * scale; ++j) {
		int qy = y + j;
		if (qy < 0 || qy >= raw.h) continue;
		for (int i = 0; i < 8 * scale; ++i) {
			int qx = x + i;
			uint32_t c = px[(j / scale) * 8 + i / scale];
			if (c && qx >= 0 && qx < raw.w) raw.px[qy * raw.stride + qx] = c;
		}
	}
}

/* A ROM tile into memory: its colours in RGBA8888, colour 0 left as it was */
static void raw_tile(uint32_t tile, uint32_t pal, int x, int y, int flip) {
	const uint32_t *px = raw_tile_px(tile, pal, flip);
	if (!px) return;
	for (int j = 0; j < 8; ++j)
		for (int i = 0; i < 8; ++i) {
			int qx = x + i, qy = y + j;
			uint32_t c = px[j * 8 + i];
			if (c && qx >= 0 && qy >= 0 && qx < raw.w && qy < raw.h) raw.px[qy * raw.stride + qx] = c;
		}
}

static bool raw_tile_on(void) { return raw.px != NULL; }

void sprite_draw_into(Sprite *s, int anim, int frame, int x, int y, int pal, int scale) {
	CachedFrame *c = raw.px ? get_frame_part(s, anim, frame, pal, RAW_PART) : NULL;
	if (!c || !c->raw) return;
	x += c->ox * scale;
	y += c->oy * scale;
	for (int j = 0; j < c->h * scale; ++j) {
		int qy = y + j;
		if (qy < 0 || qy >= raw.h) continue;
		for (int i = 0; i < c->w * scale; ++i) {
			int qx = x + i;
			uint32_t v = c->raw[(j / scale) * c->w + i / scale];
			if (v && qx >= 0 && qx < raw.w) raw.px[qy * raw.stride + qx] = v;
		}
	}
}

void gfx_draw_into(uint32_t *px, int w, int h, int pitch) {
	raw.px = px;
	raw.w = w;
	raw.h = h;
	raw.stride = pitch / 4;
}

/* a fill's blend over one pixel: (r, g, b) already times a / 255 */
static uint32_t raw_blend(uint32_t d, unsigned r, unsigned g, unsigned b, unsigned a) {
	unsigned inva = 255 - a;
	return (inva * (d >> 24) / 255 + r) << 24 | (inva * (d >> 16 & 0xFF) / 255 + g) << 16
		| (inva * (d >> 8 & 0xFF) / 255 + b) << 8 | (inva * (d & 0xFF) / 255 + a);
}

static void raw_fill(int x, int y, int w, int h, SDL_Color c) {
	int x1 = x + w, y1 = y + h;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x1 > raw.w) x1 = raw.w;
	if (y1 > raw.h) y1 = raw.h;
	if (x >= x1 || y >= y1) return;
	unsigned a = c.a, r = c.r * a / 255, g = c.g * a / 255, b = c.b * a / 255;
	uint32_t solid = (uint32_t)c.r << 24 | (uint32_t)c.g << 16 | (uint32_t)c.b << 8 | a;
	if (a == 255) {
		for (int j = y; j < y1; ++j)
			for (uint32_t *p = raw.px + j * raw.stride + x, *e = p + (x1 - x); p < e; ++p) *p = solid;
		return;
	}
	/* (the last pixel's blend kept: a fill mostly lies on one colour) */
	uint32_t in = raw.px[y * raw.stride + x], out = raw_blend(in, r, g, b, a);
	for (int j = y; j < y1; ++j)
		for (uint32_t *p = raw.px + j * raw.stride + x, *e = p + (x1 - x); p < e; ++p) {
			if (*p != in) { in = *p; out = raw_blend(in, r, g, b, a); }
			*p = out;
		}
}

/* ------------------------------------------------------------------ */
/* Font: 8x16 cells at FONT_BASE + code * 64, two tones (fill, shade). */

#define FONT_BASE 0x6B5A2C
#define FONT_TOP 2
static SDL_Texture *font_tex;
static uint8_t font_w[256];
/* the atlas's pixels (ARGB8888), kept for drawing into memory */
static uint32_t font_px[16 * 16 * 128];
/* ... and each glyph's drawn texels in their order (ty << 4 | tx, 0x100
 * for the shade), glyph ch's from glyph_first[ch]: drawing into memory
 * walks these alone, not each glyph's 256 (the 3DS's bottom screen) */
static uint16_t glyph_texels[16 * 16 * 128];
static uint16_t glyph_first[129];

int text_code(unsigned char ch) {
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
	uint32_t *px = font_px;
	memset(font_px, 0, sizeof font_px);
	for (int ch = 1; ch < 128; ++ch) {
		if (ch > 5 && ch < 32) continue;
		int code = text_code((unsigned char)ch);
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
	int n = 0;
	for (int ch = 0; ch < 128; ++ch) {
		glyph_first[ch] = (uint16_t)n;
		for (int ty = 0; ty < 16; ++ty)
			for (int tx = 0; tx < 16; ++tx) {
				uint32_t t = font_px[ty * 16 * 128 + ch * 16 + tx];
				if (t) glyph_texels[n++] = (uint16_t)((t == 0xFFFFFFFFu ? 0 : 0x100) | ty << 4 | tx);
			}
	}
	glyph_first[128] = (uint16_t)n;
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(font_px, 16 * 128, 16, 32, 16 * 128 * 4, SDL_PIXELFORMAT_ARGB8888);
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

static void raw_text(int x, int y, const char *s, SDL_Color c, int scale);

void text_draw_scaled(int x, int y, const char *s, SDL_Color c, int align, int scale) {
	if (align == TEXT_CENTER) x -= text_width(s) * scale / 2;
	else if (align == TEXT_RIGHT) x -= text_width(s) * scale;
	if (raw.px) { raw_text(x, y, s, c, scale); return; }
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

/* the atlas's texels times the text's colour, as SDL's blit modulates
 * and blends them */
static void raw_text(int x, int y, const char *s, SDL_Color c, int scale) {
	/* (the atlas's two tones, white and the shade 0x182040, times the
	 * colour; every texel drawn is opaque) */
	unsigned sa = c.a;
	if (!sa) return;
	unsigned tone[2][3] = { { c.r, c.g, c.b }, { 0x18 * c.r / 255u, 0x20 * c.g / 255u, 0x40 * c.b / 255u } };
	if (sa < 255)
		for (int k = 0; k < 2; ++k)
			for (int i = 0; i < 3; ++i) tone[k][i] = tone[k][i] * sa / 255;
	for (; *s; ++s) {
		unsigned char ch = (unsigned char)*s;
		if ((ch < 32 && ch > 5) || ch >= 128) continue;
		for (int k = glyph_first[ch]; k < glyph_first[ch + 1]; ++k) {
			int tx = glyph_texels[k] & 15, ty = glyph_texels[k] >> 4 & 15;
			const unsigned *t = tone[glyph_texels[k] >> 8];
			for (int j = 0; j < scale; ++j)
				for (int i = 0; i < scale; ++i) {
					int px = x + tx * scale + i, py = y + ty * scale + j;
					if (px < 0 || py < 0 || px >= raw.w || py >= raw.h) continue;
					uint32_t *d = raw.px + py * raw.stride + px;
					*d = sa == 255 ? t[0] << 24 | t[1] << 16 | t[2] << 8 | 255 : raw_blend(*d, t[0], t[1], t[2], sa);
				}
		}
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
	if (raw.px) { raw_fill(x, y, w, h, c); return; }
	SDL_SetRenderDrawBlendMode(P.renderer, c.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(P.renderer, c.r, c.g, c.b, c.a);
	SDL_Rect r = { x, y, w, h };
	SDL_RenderFillRect(P.renderer, &r);
}

void fill_rects(const SDL_Rect *r, int n, SDL_Color c) {
	if (n <= 0) return;
	if (raw.px) {
		for (int i = 0; i < n; ++i) raw_fill(r[i].x, r[i].y, r[i].w, r[i].h, c);
		return;
	}
	SDL_SetRenderDrawBlendMode(P.renderer, c.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(P.renderer, c.r, c.g, c.b, c.a);
	SDL_RenderFillRects(P.renderer, r, n);
}

bool gfx_init(void) {
	/* (once: the launcher reads it as BN6 goes in, the game's start again) */
	if (!font_tex) build_font();
	return true;
}
