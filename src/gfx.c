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

void anim_start(Anim *a, Sprite *s, int anim) {
	anim_play(a, s, anim);
	a->fresh = true;
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

void sprite_draw_part(Sprite *s, int anim, int frame, int part, int x, int y, bool flip, int pal) {
	draw_cached(get_frame_part(s, anim, frame, pal, part + 1), x, y, flip, 0);
}

void anim_draw(const Anim *a, int x, int y, bool flip, int pal, int fx) {
	if (!a->spr) return;
	sprite_draw_frame(a->spr, a->anim, a->frame, x, y, flip, pal, fx);
}

SDL_Rect sprite_frame_bounds(Sprite *s, int anim, int frame, int pal) {
	CachedFrame *c = get_frame(s, anim, frame, pal);
	SDL_Rect r = { 0, 0, 0, 0 };
	if (c) { r.x = c->ox; r.y = c->oy; r.w = c->w; r.h = c->h; }
	return r;
}

/* ------------------------------------------------------------------ */
/* Panels */

#include "panel_layout.inc"

static uint8_t *panel_tileset;
static size_t panel_tileset_len;
static SDL_Texture *panel_tex[13][4][2];

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

static SDL_Texture *build_panel(int type, int row, int side) {
	int rows = row == 3 ? 1 : 3;
	uint32_t px[40 * 24] = { 0 };
	for (int ty = 0; ty < rows; ++ty) {
		for (int tx = 0; tx < 5; ++tx) {
			uint16_t e = panel_layout[type][row][side][ty][tx];
			int tile = e & 0x1FF;
			if ((size_t)(tile + 1) * 32 > panel_tileset_len) continue;
			const uint8_t *pal = R.data + R.layout->panel_pals[(e >> 11) & 7];
			blit_tile(px, 40, tx * 8, ty * 8, panel_tileset + tile * 32, pal, e & 0x200, e & 0x400);
		}
	}
	return texture_from_pixels(px, 40, rows * 8);
}


/* The game swaps every tile of a threatened panel for one solid tile in
 * colour 14 of its warning palette: a flat yellow panel. */
void panel_draw_warn(int type, int row, int side, int x, int y) {
	(void)type; (void)row; (void)side;
	uint32_t c = bgr555(rom_u16(R.layout->ui.panel_warn_pal + 14 * 2));
	fill_rect(x, y, 40, 24, rgba((c >> 16) & 255, (c >> 8) & 255, c & 255, 255));
}

void panel_draw(int type, int row, int side, int x, int y) {
	if (type < 1 || type > 12) return;
	if (!panel_tex[type][row][side]) panel_tex[type][row][side] = build_panel(type, row, side);
	SDL_Rect dst = { x, y, 40, 24 };
	SDL_RenderCopy(P.renderer, panel_tex[type][row][side], NULL, &dst);
}

void panel_edge_draw(int side, int x, int y) {
	if (!panel_tex[2][3][side]) panel_tex[2][3][side] = build_panel(2, 3, side);
	SDL_Rect dst = { x, y, 40, 8 };
	SDL_RenderCopy(P.renderer, panel_tex[2][3][side], NULL, &dst);
}

/* ------------------------------------------------------------------ */
/* Single ROM tiles, cached by (tile, palette, flip) */

#include "hud_layout.inc"

/* A tile is a ROM offset, or 0x80000000 | block << 24 | offset into one of
 * the LZ77 blocks the game decompresses for its UI. */
static uint32_t extra_lz[8]; /* blocks registered with gfx_lz_ref */
static int extra_n;
#define HUD_LZ_N ((int)(sizeof hud_lz_blocks / sizeof *hud_lz_blocks))

uint32_t gfx_lz_ref(uint32_t lz) {
	int i = 0;
	while (i < extra_n && extra_lz[i] != lz) ++i;
	if (i == extra_n) {
		if (extra_n >= 8) return 0;
		extra_lz[extra_n++] = lz;
	}
	return 0x80000000u | (uint32_t)(HUD_LZ_N + i) << 24;
}

static const uint8_t *tile_data(uint32_t tile) {
	static uint8_t *blocks[16];
	static size_t block_len[16];
	if (!(tile & 0x80000000u)) return tile + 32 <= ROM_SIZE ? R.data + tile : NULL;
	int b = (int)((tile >> 24) & 0x7F);
	uint32_t off = tile & 0xFFFFFF;
	if (b >= HUD_LZ_N + extra_n || b >= 16) return NULL;
	uint32_t src = b < HUD_LZ_N ? hud_lz_blocks[b] : extra_lz[b - HUD_LZ_N];
	if (!blocks[b]) blocks[b] = lz77_decompress(R.data + src, ROM_SIZE - src, &block_len[b]);
	return blocks[b] && off + 32 <= block_len[b] ? blocks[b] + off : NULL;
}

typedef struct { uint32_t key_tile, key_pal; uint8_t flip, used; SDL_Texture *tex; } TileEntry;

#define PAL_HANDLE 0x80000000u
static uint8_t custom_pals[16][32];
static int custom_pal_n;

uint32_t gfx_palette(const uint16_t colors[16]) {
	for (int i = 0; i < custom_pal_n; ++i)
		if (!memcmp(custom_pals[i], colors, 32)) return PAL_HANDLE | (uint32_t)i;
	if (custom_pal_n >= 16) return PAL_HANDLE;
	memcpy(custom_pals[custom_pal_n], colors, 32);
	return PAL_HANDLE | (uint32_t)custom_pal_n++;
}

static const uint8_t *pal_bytes(uint32_t pal) {
	if (pal & PAL_HANDLE) return custom_pals[pal & 15];
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

/* A tile block as an affine OBJ: rotated (degrees clockwise) and scaled
 * about its centre. A few blocks are cached as textures. */
void rom_tiles_affine(uint32_t first, uint32_t pal, int x, int y, int w, int h, double angle, double scale) {
	static struct { uint32_t first, pal; int w, h; SDL_Texture *tex; } cache[8];
	SDL_Texture *tex = NULL;
	for (int i = 0; i < 8 && !tex; ++i) {
		if (cache[i].tex && cache[i].first == first && cache[i].pal == pal && cache[i].w == w && cache[i].h == h) tex = cache[i].tex;
		else if (!cache[i].tex) {
			tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w * 8, h * 8);
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
			SDL_Texture *prev = SDL_GetRenderTarget(P.renderer);
			SDL_SetRenderTarget(P.renderer, tex);
			SDL_SetRenderDrawColor(P.renderer, 0, 0, 0, 0);
			SDL_RenderClear(P.renderer);
			rom_tiles(first, pal, 0, 0, w, h, 0);
			SDL_SetRenderTarget(P.renderer, prev);
			cache[i].first = first; cache[i].pal = pal; cache[i].w = w; cache[i].h = h; cache[i].tex = tex;
		}
	}
	if (!tex) { rom_tiles(first, pal, x, y, w, h, 0); return; }
	int dw = (int)(w * 8 * scale + 0.5), dh = (int)(h * 8 * scale + 0.5);
	SDL_Rect dst = { x + w * 4 - dw / 2, y + h * 4 - dh / 2, dw, dh };
	SDL_RenderCopyEx(P.renderer, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
}

static SDL_Texture *hud_tex[6];

static SDL_Texture *build_hud(const HudCell *cells, int rows, int cols) {
	uint32_t *px = calloc((size_t)rows * cols * 64, 4);
	for (int ty = 0; ty < rows; ++ty)
		for (int tx = 0; tx < cols; ++tx) {
			const HudCell *c = &cells[ty * cols + tx];
			const uint8_t *td = c->tile ? tile_data(c->tile) : NULL;
			if (td) blit_tile(px, cols * 8, tx * 8, ty * 8, td, R.data + hud_pals[c->pal], c->flip & 1, c->flip & 2);
		}
	SDL_Texture *t = texture_from_pixels(px, cols * 8, rows * 8);
	free(px);
	return t;
}

void hud_window(int which, int x, int y) {
	if (which < 0 || which > HUD_GAMEOVER_TEXT) return;
	if (!hud_tex[which])
		hud_tex[which] = which == HUD_CUSTOM ? build_hud(&hud_custom[0][0], 20, 15)
			: which == HUD_RESULT ? build_hud(&hud_result[0][0], 18, 24)
			: which == HUD_TEXT ? build_hud(&hud_text[0][0], 8, 30)
			: which == HUD_FOLDER ? build_hud(&hud_folder[0][0], 20, 30)
			: which == HUD_GAMEOVER_BG ? build_hud(&hud_gameover_bg[0][0], 20, 32) : build_hud(&hud_gameover_text[0][0], 20, 30);
	int w, h;
	SDL_QueryTexture(hud_tex[which], NULL, NULL, &w, &h);
	SDL_Rect d = { x, y, w, h };
	SDL_RenderCopy(P.renderer, hud_tex[which], NULL, &d);
}

/* PET menu: the buttons share one palette whose label colours (5-12) the
 * game sets per row, white for the selected button, blended toward the
 * button green for the rest. */
void hud_pet_menu(int x, int y, int selected) {
	static SDL_Texture *variant[8];
	if (selected < 0 || selected > 7) selected = 0;
	if (!variant[selected]) {
		uint16_t pal[16];
		uint32_t base = 0x6C7CF8;
		for (int i = 0; i < 16; ++i) pal[i] = rom_u16(base + (uint32_t)i * 2);
		for (int i = 0; i < 8; ++i) {
			if (i == selected) continue;
			uint16_t a = pal[5 + i], b = pal[2];
			int r = ((a & 31) + (b & 31)) / 2, g = (((a >> 5) & 31) + ((b >> 5) & 31)) / 2, bl = (((a >> 10) & 31) + ((b >> 10) & 31)) / 2;
			pal[5 + i] = (uint16_t)(r | g << 5 | bl << 10);
		}
		uint32_t handle = gfx_palette(pal);
		uint32_t *px = calloc(30 * 20 * 64, 4);
		for (int ty = 0; ty < 20; ++ty)
			for (int tx = 0; tx < 30; ++tx) {
				const HudCell *c = &hud_pet[ty][tx];
				const uint8_t *td = c->tile ? tile_data(c->tile) : NULL;
				if (!td) continue;
				const uint8_t *pb = hud_pals[c->pal] == base ? pal_bytes(handle) : R.data + hud_pals[c->pal];
				blit_tile(px, 240, tx * 8, ty * 8, td, pb, c->flip & 1, c->flip & 2);
			}
		variant[selected] = texture_from_pixels(px, 240, 160);
		free(px);
	}
	SDL_Rect d = { x, y, 240, 160 };
	SDL_RenderCopy(P.renderer, variant[selected], NULL, &d);
	for (size_t i = 0; i < sizeof hud_pet_objs / sizeof *hud_pet_objs; ++i) {
		int dx = hud_pet_objs[i].group && hud_pet_objs[i].group - 1 == selected ? 8 : 0;
		rom_tile(hud_pet_objs[i].tile, hud_pals[hud_pet_objs[i].pal], x + hud_pet_objs[i].x + dx, y + hud_pet_objs[i].y, hud_pet_objs[i].flip);
	}
}

/* The banners are affine OBJs scaled vertically about their centre line, with
 * 16-pixel boxes that clip a stretch above 1 (pd = 256 / scale, as OAM). */
void hud_banner_scaled(int which, int x, int y, int pd) {
	static SDL_Texture *tex[3];
	if (pd <= 0 || which < 0 || which > 2) return;
	if (!tex[which]) {
		tex[which] = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 240, 16);
		SDL_SetTextureBlendMode(tex[which], SDL_BLENDMODE_BLEND);
		SDL_Texture *prev = SDL_GetRenderTarget(P.renderer);
		SDL_SetRenderTarget(P.renderer, tex[which]);
		SDL_SetRenderDrawColor(P.renderer, 0, 0, 0, 0);
		SDL_RenderClear(P.renderer);
		hud_banner(which, 0, 0);
		SDL_SetRenderTarget(P.renderer, prev);
	}
	SDL_Rect src = { 0, 0, 240, 16 }, dst = { x, y, 240, 16 };
	if (pd >= 256) {
		dst.h = 16 * 256 / pd;
		dst.y = y + (16 - dst.h) / 2;
	} else {
		src.h = 16 * pd / 256;
		src.y = (16 - src.h) / 2;
	}
	SDL_RenderCopy(P.renderer, tex[which], &src, &dst);
}

void hud_banner(int which, int x, int y) {
	for (int i = 0; i < 5; ++i) {
		const uint32_t *tiles = which == 2 ? hud_banner_lost[i].tiles : which ? hud_banner_deleted[i].tiles : hud_banner_start[i].tiles;
		int bx = which == 2 ? hud_banner_lost[i].x : which ? hud_banner_deleted[i].x : hud_banner_start[i].x;
		uint32_t pal = hud_pals[which == 2 ? hud_banner_lost[i].pal : which ? hud_banner_deleted[i].pal : hud_banner_start[i].pal];
		for (int t = 0; t < 8; ++t)
			if (tiles[t]) rom_tile(tiles[t], pal, x + bx + (t % 4) * 8, y + (t / 4) * 8, 0);
	}
}

/* ------------------------------------------------------------------ */
/* Chip art */

static SDL_Texture *chip_images[512];

/* A 56x48 picture of 42 consecutive tiles; colour 0 is its background. */
static SDL_Texture *art_image(uint32_t img, uint32_t pal) {
	uint32_t px[56 * 48] = { 0 };
	for (int t = 0; t < 42; ++t)
		blit_tile(px, 56, (t % 7) * 8, (t / 7) * 8, R.data + img + t * 32, R.data + pal, false, false);
	uint32_t bg = bgr555(rom_u16(pal));
	for (int i = 0; i < 56 * 48; ++i) if (!px[i]) px[i] = bg;
	return texture_from_pixels(px, 56, 48);
}

SDL_Texture *chip_image(int chip) {
	if (chip < 0 || chip >= 512) return NULL;
	if (chip_images[chip]) return chip_images[chip];
	uint32_t rec = R.layout->chip_data + chip * 0x2C;
	uint32_t img = rom_u32(rec + 0x24), pal = rom_u32(rec + 0x28);
	if (!rom_is_ptr(img) || !rom_is_ptr(pal)) return NULL;
	chip_images[chip] = art_image(rom_off(img), rom_off(pal));
	return chip_images[chip];
}

SDL_Texture *zenny_image(void) {
	static SDL_Texture *t;
	if (!t) t = art_image(R.layout->ui.zenny_art, R.layout->ui.zenny_pal);
	return t;
}

static SDL_Texture *chip_icons[512];

SDL_Texture *chip_icon(int chip) {
	if (chip < 0 || chip >= 512) return NULL;
	if (chip_icons[chip]) return chip_icons[chip];
	uint32_t icon = rom_u32(R.layout->chip_data + chip * 0x2C + 0x20);
	if (!rom_is_ptr(icon)) return NULL;
	uint32_t px[16 * 16] = { 0 };
	for (int t = 0; t < 4; ++t)
		blit_tile(px, 16, (t % 2) * 8, (t / 2) * 8, R.data + rom_off(icon) + t * 32, R.data + R.layout->chip_icon_pal, false, false);
	chip_icons[chip] = texture_from_pixels(px, 16, 16);
	return chip_icons[chip];
}

/* ------------------------------------------------------------------ */
/* Battle background: 32x32 blocks in a checkerboard over the backdrop. */

/* ------------------------------------------------------------------ */
/* Battle backgrounds, as the game builds them: a BGAnimData record per id
 * (LZ77 tiles, LZ77 32x32 tilemap, palette) and a list of GFX animations
 * (tile copies and palette copies stepped on timers). The picture is a
 * function of the frame count, so drawing changes no state but a cache. */

#define BG_IDS 0x16
#define BG_ANIMS 12
#define BG_STEPS 48

typedef struct {
	uint8_t cmd;          /* 4 tile copy, 0 palette copy */
	uint32_t src, dest;   /* ROM offset / VRAM or palette byte offset */
	int count;            /* tiles, or palette bytes */
	int nsteps;
	bool loop;
	uint32_t step[BG_STEPS];
	int delay[BG_STEPS];
	int total;
} BgAnim;

typedef struct {
	bool loaded, ok;
	uint8_t vram[0x8000];
	uint8_t pal[512];
	uint16_t map[32 * 32];
	BgAnim anim[BG_ANIMS];
	int nanim;
	SDL_Texture *tex;
	int key[BG_ANIMS];
} BattleBg;

static BattleBg bgs[BG_IDS];

static bool rom_ptr_ok(uint32_t v) { return rom_is_ptr(v); }

/* rec: BGAnimData pointer; list: GFX animation list pointer; vbase: the
 * address of the background's tile block (tile 0) in VRAM. */
static void bg_load_rec(BattleBg *b, uint32_t rec, uint32_t list, uint32_t vbase) {
	b->loaded = true;
	if (!rom_ptr_ok(rec)) return;
	uint32_t r = rom_off(rec);
	uint32_t gfx = rom_u32(r), gdest = rom_u32(r + 4), map = rom_u32(r + 8), pal = rom_u32(r + 16), pdest = rom_u32(r + 20), psize = rom_u32(r + 24);
	if (!rom_ptr_ok(gfx) || !rom_ptr_ok(map)) return;
	size_t n = 0;
	uint32_t g = rom_off(gfx);
	uint8_t *t = lz77_decompress(R.data + g + rom_u32(g + 4), ROM_SIZE - (g + rom_u32(g + 4)), &n);
	uint32_t at = gdest - vbase;
	if (t && at < sizeof b->vram) memcpy(b->vram + at, t, n < sizeof b->vram - at ? n : sizeof b->vram - at);
	free(t);
	uint32_t m = rom_off(map);
	int mw = R.data[m], mh = R.data[m + 1];
	t = lz77_decompress(R.data + m + 12, ROM_SIZE - (m + 12), &n);
	if (t) {
		for (int y = 0; y < mh && y < 32; ++y)
			for (int x = 0; x < mw && x < 32; ++x)
				if ((size_t)(y * mw + x) * 2 + 1 < n) b->map[y * 32 + x] = (uint16_t)(t[(y * mw + x) * 2] | t[(y * mw + x) * 2 + 1] << 8);
		free(t);
	}
	if (rom_ptr_ok(pal) && pdest >= 0x03001960u && pdest - 0x03001960u + psize <= sizeof b->pal)
		memcpy(b->pal + (pdest - 0x03001960u), R.data + rom_off(pal) + 4, psize);
	if (rom_ptr_ok(list)) {
		for (uint32_t a = rom_off(list); b->nanim < BG_ANIMS && rom_u32(a) != 0xFFFFFFFFu; a += 4) {
			uint32_t d = rom_u32(a);
			if (!rom_ptr_ok(d)) break;
			d = rom_off(d);
			BgAnim *an = &b->anim[b->nanim];
			an->cmd = R.data[d + 8];
			if (an->cmd == 4) {
				an->src = rom_off(rom_u32(d));
				an->dest = rom_u32(d + 4) - vbase;
				an->count = R.data[d + 10];
				/* the net's lists also animate the floor, elsewhere in VRAM */
				if (an->dest >= sizeof b->vram) continue;
			} else if (an->cmd == 0) {
				an->dest = rom_u32(d) - 0x03001960u;
				an->count = (int)rom_u32(d + 4);
			} else continue;
			an->nsteps = an->total = 0;
			for (uint32_t q = d + 12; an->nsteps < BG_STEPS; q += 8) {
				uint32_t nx = rom_u32(q);
				if (nx <= 1) { an->loop = nx == 1; break; }
				an->step[an->nsteps] = rom_off(nx);
				an->delay[an->nsteps] = (int)rom_u32(q + 4);
				an->total += an->delay[an->nsteps];
				an->nsteps++;
			}
			if (an->nsteps > 0 && an->total > 0) b->nanim++;
		}
	}
	b->ok = true;
}

static void bg_load(int id, BattleBg *b) {
	bg_load_rec(b, rom_u32(R.layout->battle_bg_table + (uint32_t)id * 4), rom_u32(R.layout->battle_bg_anims + (uint32_t)id * 4), 0x06000000u);
}

static int bg_anim_step(const BgAnim *an, int frame) {
	int t = an->loop ? frame % an->total : (frame < an->total ? frame : an->total - 1);
	for (int i = 0; i < an->nsteps; ++i) {
		if (t < an->delay[i]) return i;
		t -= an->delay[i];
	}
	return an->nsteps - 1;
}

static void bg_apply(BattleBg *b, const BgAnim *an, int step) {
	uint32_t s = an->step[step];
	if (an->cmd == 4) {
		for (int i = 0; i < an->count; ++i) {
			uint16_t idx = rom_u16(s + (uint32_t)i * 2);
			uint32_t to = an->dest + (uint32_t)i * 32;
			if (to + 32 <= sizeof b->vram) memcpy(b->vram + to, R.data + an->src + (uint32_t)idx * 32, 32);
		}
	} else if (an->dest + (uint32_t)an->count <= sizeof b->pal) {
		memcpy(b->pal + an->dest, R.data + s, (size_t)an->count);
	}
}

static void bg_render(BattleBg *b) {
	static uint32_t px[256 * 256];
	uint32_t backdrop = bgr555((uint16_t)(b->pal[0] | b->pal[1] << 8));
	for (int ty = 0; ty < 32; ++ty)
		for (int tx = 0; tx < 32; ++tx) {
			uint16_t e = b->map[ty * 32 + tx];
			const uint8_t *tile = b->vram + (e & 0x3FF) * 32;
			const uint8_t *pal = b->pal + ((e >> 12) & 15) * 32;
			bool hf = e & 0x400, vf = e & 0x800;
			for (int py = 0; py < 8; ++py)
				for (int x = 0; x < 8; ++x) {
					uint8_t v = tile[py * 4 + x / 2];
					int ci = (x & 1) ? v >> 4 : v & 15;
					int X = hf ? 7 - x : x, Y = vf ? 7 - py : py;
					px[(ty * 8 + Y) * 256 + tx * 8 + X] = ci ? bgr555((uint16_t)(pal[ci * 2] | pal[ci * 2 + 1] << 8)) : backdrop;
				}
		}
	if (!b->tex) {
		b->tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
		SDL_SetTextureScaleMode(b->tex, SDL_ScaleModeNearest);
	}
	SDL_UpdateTexture(b->tex, NULL, px, 256 * 4);
}

/* Scroll per id, measured from BG1's offset registers (pixels the picture
 * moves per frame): most drift right 1/2 and down 1/4. */
static void bg_scroll(int id, int frame, int *sx, int *sy) {
	int dx = frame / 2, dy = frame / 4;
	switch (id) {
	case 0x06: dx = 0; break;
	case 0x0A: case 0x0C: dy = 0; break;
	case 0x0F: case 0x11: dx = dy = 0; break;
	case 0x12: dx = 0; dy = -(frame / 4); break;
	case 0x14: dx = -(frame / 16); dy = 0; break;
	default: break;
	}
	*sx = dx; *sy = dy;
}

static void bg_show(BattleBg *b, int frame, int sx, int sy) {
	bool dirty = !b->tex;
	for (int i = 0; i < b->nanim; ++i) {
		int st = bg_anim_step(&b->anim[i], frame);
		if (st != b->key[i] || !b->tex) { b->key[i] = st; dirty = true; }
	}
	if (dirty) {
		for (int i = 0; i < b->nanim; ++i) bg_apply(b, &b->anim[i], b->key[i]);
		bg_render(b);
	}
	int ox = ((P.core_x + sx) % 256 + 256) % 256 - 256, oy = ((P.core_y + sy) % 256 + 256) % 256 - 256;
	for (int y = oy; y < P.h; y += 256)
		for (int x = ox; x < P.w; x += 256) {
			SDL_Rect dst = { x, y, 256, 256 };
			SDL_RenderCopy(P.renderer, b->tex, NULL, &dst);
		}
}

void battle_bg_draw(int id, int frame) {
	if (id < 0 || id >= BG_IDS) id = 7;
	BattleBg *b = &bgs[id];
	if (!b->loaded) bg_load(id, b);
	if (!b->ok) { b = &bgs[7]; id = 7; if (!b->loaded) bg_load(7, b); if (!b->ok) return; }
	int sx, sy;
	bg_scroll(id, frame, &sx, &sy);
	bg_show(b, frame, sx, sy);
}

/* The net areas load their background into BG3 with tiles at 0x06008000;
 * the scroll callbacks move it by counters (BGScrollCB_BG3Diagonal3to2Scroll:
 * right 8/16 and down 4/16 of a pixel a frame; BG3SlowRightScroll: offset
 * +1/16, the picture drifting left). */
void area_bg_draw(int area, int frame) {
	static BattleBg areas[8];
	if (area < 0 || area >= 8) return;
	BattleBg *b = &areas[area];
	const __typeof__(R.layout->net_area[0]) *a = &R.layout->net_area[area];
	if (!b->loaded) bg_load_rec(b, a->bg + 0x08000000u, a->bg_anims ? a->bg_anims + 0x08000000u : 0, 0x06008000u);
	if (!b->ok) { battle_bg_draw(7, frame); return; }
	int sx = 0, sy = 0;
	if (a->scroll == 1) { sx = frame / 2; sy = frame / 4; }
	else if (a->scroll == 2) sx = -(frame / 16);
	bg_show(b, frame, sx, sy);
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

/* ---- Chat font ---- */
static SDL_Texture *chat_tex;
static uint8_t chat_left[128], chat_adv[128];
static struct { uint32_t pal; SDL_Texture *tex; } chat_variants[4];

static SDL_Texture *build_chat_atlas(uint32_t pal_off);

static void build_chat_font(void) {
	chat_tex = build_chat_atlas(R.layout->ui.chat_pal);
}

/* The chat font in a given ROM palette (colour 1 fill, 2 shadow). */
static SDL_Texture *chat_atlas(uint32_t pal_off) {
	if (pal_off == R.layout->ui.chat_pal) return chat_tex;
	for (int i = 0; i < 4; ++i) {
		if (chat_variants[i].pal == pal_off) return chat_variants[i].tex;
		if (!chat_variants[i].pal) {
			chat_variants[i].pal = pal_off;
			chat_variants[i].tex = build_chat_atlas(pal_off);
			return chat_variants[i].tex;
		}
	}
	return chat_tex;
}

static SDL_Texture *build_chat_atlas(uint32_t pal_off) {
	static uint32_t px[16 * 128 * 16];
	memset(px, 0, sizeof px);
	const uint8_t *pal = R.data + pal_off;
	uint32_t c1 = bgr555((uint16_t)(pal[2] | pal[3] << 8)), c2 = bgr555((uint16_t)(pal[4] | pal[5] << 8));
	for (int ch = 1; ch < 128; ++ch) {
		if (ch > 5 && ch < 32) continue;
		int code = ascii_code((unsigned char)ch);
		if (code < 0) { chat_adv[ch] = 4; continue; }
		const uint8_t *g = R.data + R.layout->ui.chat_font + code * 64;
		int minx = 8, maxx = -1;
		for (int y = 0; y < 16; ++y)
			for (int x = 0; x < 8; ++x) {
				uint8_t v = g[y * 4 + x / 2];
				int ci = (x & 1) ? v >> 4 : v & 15;
				if (!ci) continue;
				px[y * 16 * 128 + ch * 16 + x] = ci == 1 ? c1 : c2;
				if (x < minx) minx = x;
				if (x > maxx) maxx = x;
			}
		/* glyphs sit anywhere in their 8-pixel cell: trim and pack them */
		if (maxx < 0) { chat_left[ch] = 0; chat_adv[ch] = 5; continue; }
		chat_left[ch] = (uint8_t)minx;
		chat_adv[ch] = (uint8_t)(maxx - minx + 2); /* one pixel of air after the shadow */
	}
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(px, 16 * 128, 16, 32, 16 * 128 * 4, SDL_PIXELFORMAT_ARGB8888);
	SDL_Texture *t = SDL_CreateTextureFromSurface(P.renderer, s);
	SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(s);
	return t;
}

void chat_draw_pal(int x, int y, const char *s, uint32_t pal) {
	SDL_Texture *t = chat_atlas(pal);
	for (; *s; ++s) {
		unsigned char ch = (unsigned char)*s;
		if (ch >= 128 || (ch > 5 && ch < 32)) continue;
		SDL_Rect src = { ch * 16 + chat_left[ch], 0, chat_adv[ch] - 1, 16 };
		SDL_Rect dst = { x, y - 3, chat_adv[ch] - 1, 16 };
		SDL_RenderCopy(P.renderer, t, &src, &dst);
		x += chat_adv[ch];
	}
}

int chat_width(const char *s) {
	int w = 0;
	for (; *s; ++s) {
		unsigned char ch = (unsigned char)*s;
		if (ch >= 128 || (ch > 5 && ch < 32)) continue;
		w += chat_adv[ch];
	}
	return w;
}

void chat_draw(int x, int y, const char *s, int max_chars) {
	/* glyph shapes start at cell row 3; y is the top of the capitals */
	for (int n = 0; *s && (max_chars < 0 || n < max_chars); ++s, ++n) {
		unsigned char ch = (unsigned char)*s;
		if (ch >= 128 || (ch > 5 && ch < 32)) continue;
		SDL_Rect src = { ch * 16 + chat_left[ch], 0, chat_adv[ch] - 1, 16 };
		SDL_Rect dst = { x, y - 3, chat_adv[ch] - 1, 16 };
		SDL_RenderCopy(P.renderer, chat_tex, &src, &dst);
		x += chat_adv[ch];
	}
}

void chat_draw_cells(int x, int y, const char *s) {
	for (; *s; ++s, x += 8) {
		unsigned char ch = (unsigned char)*s;
		if (ch >= 128 || (ch > 5 && ch < 32)) continue;
		SDL_Rect src = { ch * 16, 0, 8, 16 };
		SDL_Rect dst = { x, y, 8, 16 };
		SDL_RenderCopy(P.renderer, chat_tex, &src, &dst);
	}
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

void text_draw(int x, int y, const char *s, SDL_Color c, int align) {
	if (align == TEXT_CENTER) x -= text_width(s) / 2;
	else if (align == TEXT_RIGHT) x -= text_width(s);
	SDL_SetTextureColorMod(font_tex, c.r, c.g, c.b);
	SDL_SetTextureAlphaMod(font_tex, c.a);
	for (; *s; ++s) {
		unsigned char ch = (unsigned char)*s;
		if ((ch < 32 && ch > 5) || ch >= 128) continue;
		SDL_Rect src = { ch * 16, 0, 16, 16 };
		SDL_Rect dst = { x, y, 16, 16 };
		SDL_RenderCopy(P.renderer, font_tex, &src, &dst);
		x += font_w[ch];
	}
}

/* The battle font as the game lays it on a BG: 8x16 cells in a ROM palette. */
void text_draw_cells(int x, int y, const char *s, uint32_t pal) {
	for (; *s; ++s, x += 8) {
		int code = ascii_code((unsigned char)*s);
		if (code <= 0) continue;
		rom_tile(FONT_BASE + (uint32_t)code * 64, pal, x, y, 0);
		rom_tile(FONT_BASE + (uint32_t)code * 64 + 32, pal, x, y + 8, 0);
	}
}

void text_drawf(int x, int y, SDL_Color c, int align, const char *fmt, ...) {
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);
	text_draw(x, y, buf, c, align);
}

int text_lines(int x, int y, int line_h, const char *s, SDL_Color c, int align) {
	char line[128];
	int n = 0;
	while (*s) {
		const char *e = strchr(s, '\n');
		size_t len = e ? (size_t)(e - s) : strlen(s);
		if (len >= sizeof line) len = sizeof line - 1;
		memcpy(line, s, len);
		line[len] = 0;
		text_draw(x, y + n * line_h, line, c, align);
		++n;
		s += len;
		if (*s == '\n') ++s;
	}
	return n;
}

/* ------------------------------------------------------------------ */

void fill_rect(int x, int y, int w, int h, SDL_Color c) {
	SDL_SetRenderDrawBlendMode(P.renderer, c.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(P.renderer, c.r, c.g, c.b, c.a);
	SDL_Rect r = { x, y, w, h };
	SDL_RenderFillRect(P.renderer, &r);
}

void draw_rect(int x, int y, int w, int h, SDL_Color c) {
	SDL_SetRenderDrawBlendMode(P.renderer, c.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(P.renderer, c.r, c.g, c.b, c.a);
	SDL_Rect r = { x, y, w, h };
	SDL_RenderDrawRect(P.renderer, &r);
}

void draw_frame(int x, int y, int w, int h, SDL_Color fill, SDL_Color edge) {
	fill_rect(x + 1, y + 1, w - 2, h - 2, fill);
	fill_rect(x + 1, y, w - 2, 1, edge);
	fill_rect(x + 1, y + h - 1, w - 2, 1, edge);
	fill_rect(x, y + 1, 1, h - 2, edge);
	fill_rect(x + w - 1, y + 1, 1, h - 2, edge);
}

bool gfx_init(void) {
	panel_tileset = lz77_decompress(R.data + R.layout->panel_tiles, ROM_SIZE - R.layout->panel_tiles, &panel_tileset_len);
	if (!panel_tileset) return false;
	build_font();
	build_chat_font();
	return true;
}
