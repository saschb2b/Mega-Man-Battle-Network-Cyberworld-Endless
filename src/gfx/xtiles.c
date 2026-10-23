/* Another game's 4bpp tiles drawn on the canvas (xtiles.h). */
#include "xtiles.h"

#include <SDL.h>
#include <string.h>

#include "gfx.h"
#include "platform.h"

#define BLOCKS 64          /* textures kept (a board's HUD, window, banner) */
#define BLOCK_TILES 32     /* the most tiles a block holds */

static struct {
	uint32_t key;
	int w, h, flip;
	uint32_t used;         /* the draw it was last drawn at, 0 free */
	SDL_Texture *tex;
} cache[BLOCKS];
static uint32_t draws;

static uint32_t key_of(const uint8_t *tiles, const uint8_t *pal, int n) {
	uint32_t h = 2166136261u;
	for (int i = 0; i < n * 32; ++i) h = (h ^ tiles[i]) * 16777619u;
	for (int i = 0; i < 32; ++i) h = (h ^ pal[i]) * 16777619u;
	return h;
}

static SDL_Texture *block(const uint8_t *tiles, const uint8_t *pal, int w, int h, int flip) {
	if (!tiles || !pal || w <= 0 || h <= 0 || w * h > BLOCK_TILES) return NULL;
	uint32_t key = key_of(tiles, pal, w * h);
	int slot = 0;
	++draws;
	for (int i = 0; i < BLOCKS; ++i) {
		if (cache[i].used && cache[i].key == key && cache[i].w == w && cache[i].h == h && cache[i].flip == flip) {
			cache[i].used = draws;
			return cache[i].tex;
		}
		if (cache[i].used < cache[slot].used) slot = i;
	}
	/* (the one drawn longest ago made anew) */
	if (cache[slot].tex) SDL_DestroyTexture(cache[slot].tex);
	uint32_t px[BLOCK_TILES * 64];
	int W = w * 8, H = h * 8;
	memset(px, 0, sizeof px);
	for (int t = 0; t < w * h; ++t)
		for (int y = 0; y < 8; ++y)
			for (int x = 0; x < 8; ++x) {
				uint8_t v = tiles[t * 32 + y * 4 + x / 2];
				int ci = x & 1 ? v >> 4 : v & 15;
				if (!ci) continue;
				int X = t % w * 8 + x, Y = t / w * 8 + y;
				if (flip & 1) X = W - 1 - X;
				if (flip & 2) Y = H - 1 - Y;
				px[Y * W + X] = bgr555((uint16_t)(pal[ci * 2] | pal[ci * 2 + 1] << 8));
			}
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(px, W, H, 32, W * 4, SDL_PIXELFORMAT_ARGB8888);
	SDL_Texture *tex = s ? SDL_CreateTextureFromSurface(P.renderer, s) : NULL;
	if (tex) SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(s);
	cache[slot].key = key;
	cache[slot].w = w;
	cache[slot].h = h;
	cache[slot].flip = flip;
	cache[slot].used = draws;
	cache[slot].tex = tex;
	return tex;
}

void xtiles_draw(const uint8_t *tiles, const uint8_t *pal, int x, int y, int w, int h, int flip) {
	SDL_Texture *t = block(tiles, pal, w, h, flip);
	if (!t) return;
	SDL_Rect d = { x, y, w * 8, h * 8 };
	SDL_RenderCopy(P.renderer, t, NULL, &d);
}

void xtiles_draw_squashed(const uint8_t *tiles, const uint8_t *pal, int x, int y, int w, int h, int pd) {
	SDL_Texture *t = block(tiles, pal, w, h, 0);
	if (!t || pd <= 0) return;
	/* (its rows seen through the OBJ's own box: shown h * 8 * 256 / pd
	 * high about its middle, what spills past the box cut off) */
	int H = h * 8, shown = H * 256 / pd;
	if (shown <= 0) return;
	if (shown <= H) {
		SDL_Rect d = { x, y + (H - shown) / 2, w * 8, shown };
		SDL_RenderCopy(P.renderer, t, NULL, &d);
		return;
	}
	int rows = H * pd / 256;
	SDL_Rect src = { 0, (H - rows) / 2, w * 8, rows }, d = { x, y, w * 8, H };
	SDL_RenderCopy(P.renderer, t, &src, &d);
}

void xtiles_blend_rect(int x, int y, int w, int h, uint16_t colour, int eva, int evb) {
	if (w <= 0 || h <= 0 || evb < 0 || evb >= 16) return;
	/* (the renderer's blend: what lies beneath kept at evb/16, the colour
	 * scaled so that its share is the colour times eva/16) */
	uint32_t c = bgr555(colour);
	int a = 16 - evb, k[3] = { (int)(c >> 16 & 255), (int)(c >> 8 & 255), (int)(c & 255) };
	for (int i = 0; i < 3; ++i) { k[i] = k[i] * eva / a; if (k[i] > 255) k[i] = 255; }
	fill_rect(x, y, w, h, rgba(k[0], k[1], k[2], a * 255 / 16));
}
