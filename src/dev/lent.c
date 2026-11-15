/* What the layers take from a map: every tile picture an area's generated
 * layers draw is kept (and its mirror image, which a layer may set), then
 * each of the area's original maps is counted against them, tile by tile
 * and layer by layer, and drawn with the tiles no layer used tinted red. A
 * tile's picture is its 64 pixels as the game shows them, so a tile set
 * twice in a tile set counts once. */
#include "lent.h"

#include <stdlib.h>
#include <string.h>

#include "tiles.h"

#define SLOTS (1u << 18)   /* (a map has a few thousand tile pictures) */
#define TINT 0xFFFF2050u
#define HIGHEST 128   /* the highest floor looked for, world units */

static uint64_t *seen;

void lent_begin(void) {
	if (!seen) seen = malloc(SLOTS * sizeof *seen);
	if (seen) memset(seen, 0, SLOTS * sizeof *seen);
}

/* Whether `set` holds key (never 0); with `add`, it does afterwards. */
static bool held(uint64_t *set, uint64_t key, bool add) {
	for (uint32_t h = (uint32_t)(key >> 20);; ++h) {
		uint64_t *s = &set[h & (SLOTS - 1)];
		if (*s == key) return true;
		if (*s) continue;
		if (add) *s = key;
		return false;
	}
}

/* Tile (tx, ty)'s picture in a map's pixels as a number, mirrored or not; 0
 * where it draws nothing. */
static uint64_t picture(const uint32_t *px, int tw, int tx, int ty, bool mirror) {
	uint64_t h = 0xCBF29CE484222325ull;
	bool any = false;
	for (int i = 0; i < 64; ++i) {
		int x = mirror ? 7 - i % 8 : i % 8;
		uint32_t c = px[(size_t)(ty * 8 + i / 8) * tw * 8 + tx * 8 + x];
		if (!(c >> 24)) c = 0;
		any |= c != 0;
		h = (h ^ c) * 0x100000001B3ull;
	}
	return any ? h | 1 : 0;
}

/* One layer of a tile map alone, drawn: `tiles` both layers, tw x th each. */
static uint32_t *layer_alone(int rom, int group, int number, const uint16_t *tiles, int tw, int th, int l) {
	size_t cells = (size_t)tw * th;
	uint16_t *t = calloc(cells * 2, sizeof *t);
	if (!t) return NULL;
	memcpy(t + l * cells, tiles + l * cells, cells * sizeof *t);
	uint32_t *px = area_src_render(rom, group, number, t, tw, th);
	free(t);
	return px;
}

bool lent_layer(int rom, int group, int number, const uint16_t *tiles, int tw, int th) {
	for (int l = 0; l < 2; ++l) {
		uint32_t *px = seen ? layer_alone(rom, group, number, tiles, tw, th, l) : NULL;
		if (!px) return false;
		for (int i = 0; i < tw * th; ++i)
			for (int m = 0; m < 2; ++m) {
				uint64_t key = picture(px, tw, i % tw, i / tw, m);
				if (key) held(seen, key, true);
			}
		free(px);
	}
	return true;
}

static int panel_at(const uint8_t *panels, int A, int B) {
	int H = TILES_SRC_SPAN / 2;
	return A < -H || B < -H || A >= H || B >= H ? 0 : panels[(B + H) * TILES_SRC_SPAN + A + H];
}

/* Where tile i of map a lies (LENT_*). */
static int place(const AreaSrc *a, const uint8_t *panels, int i) {
	TileGrid g = { .tw = a->tw, .th = a->th, .ex = a->ex, .ey = a->ey };
	int phase, A, B, own = 0, other = 0;
	tile_class(&g, i % a->tw, i / a->tw, &phase, &A, &B);
	int p = panel_at(panels, A, B);
	if (p) return TILE_MATERIAL(p) == 3 ? LENT_OTHER : p & TILE_PAD ? LENT_PAD : LENT_OWN;
	/* (floor at height z is drawn z pixels up: the tile's middle that much
	 * lower on the ground) */
	bool ramp = false;
	for (int z = 0; z <= HIGHEST; z += 8) {
		int u = i % a->tw * 8 + 4 - a->tw * 4, v = 2 * (i / a->tw * 8 + 4 + z - a->th * 4), X = (u - v) / 2, Y = (u + v) / 2;
		if (area_src_walled_floor(a, X, Y) != 1) continue;
		int h = area_src_height(a, X, Y);
		if (z && h == z) return LENT_RAISED;
		ramp |= h == HEIGHT_UNEVEN;
	}
	if (ramp) return LENT_RAMP;
	for (int k = 0; k < 9; ++k) {
		int q = panel_at(panels, A + k % 3 - 1, B + k / 3 - 1);
		own += q && TILE_MATERIAL(q) != 3;
		other += TILE_MATERIAL(q) == 3;
	}
	return own ? LENT_BY_OWN : other ? LENT_BY_OTHER : LENT_AWAY;
}

uint32_t *lent_map(const AreaSrc *a, const uint8_t *panels, Lent *n) {
	size_t cells = (size_t)a->tw * a->th;
	uint16_t *tiles = calloc(cells * 2, sizeof *tiles);
	uint64_t *kinds = calloc(SLOTS, sizeof *kinds);
	uint32_t *out = malloc(cells * 64 * sizeof *out);
	memset(n, 0, sizeof *n);
	bool ok = seen && tiles && kinds && out;
	for (size_t i = 0; ok && i < cells * 64; ++i) out[i] = area_src_px(a, i);
	for (int l = 0; ok && l < a->layers; ++l) {
		memcpy(tiles + l * cells, a->tile[l], cells * sizeof *tiles);
		uint32_t *px = layer_alone(a->rom, a->group, a->number, tiles, a->tw, a->th, l);
		for (int i = 0; px && i < (int)cells; ++i) {
			uint64_t key = picture(px, a->tw, i % a->tw, i / a->tw, false);
			if (!key) continue;
			bool used = held(seen, key, false), known = held(kinds, key, true);
			n->drawn++;
			n->used += used;
			n->kinds += !known;
			n->kinds_used += !known && used;
			if (panels && !known && !used) n->left[place(a, panels, i)]++;
			/* (the tile's own pixels of this layer, where they show) */
			for (int k = 0; !used && k < 64; ++k) {
				size_t at = (size_t)(i / a->tw * 8 + k / 8) * a->tw * 8 + (size_t)(i % a->tw) * 8 + k % 8;
				if (px[at] >> 24 && (l || !a->front[at])) out[at] = 0xFF000000u | ((out[at] >> 2 & 0x3F3F3Fu) + (TINT >> 2 & 0x3F3F3Fu) * 3);
			}
		}
		ok = px != NULL;
		free(px);
	}
	free(tiles);
	free(kinds);
	if (!ok) { free(out); out = NULL; }
	return out;
}
