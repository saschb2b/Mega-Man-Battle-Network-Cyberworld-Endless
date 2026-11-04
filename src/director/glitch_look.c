/* A Glitch layer's other looks (glitch.h; docs/game/glitch-layer.md): the
 * layer's own floor picked again in other areas' tiles (netmap_look), and
 * flipped in by BN6's own renderer. A flip writes the look's tile sets
 * into VRAM and its colours into BN6's palette buffer in one step, and its
 * tile map into the buffer BN6 draws the map from (and the screen maps
 * where the picture shows it) in the next: the frame between, one area's
 * tile map read with the other's tiles, is the corruption a GBA really
 * shows, kept as the flip's edge. The way back writes what the look
 * covered, read before it came. Sprites, the walls and the layer's
 * objects never change; a map entered again (after a battle) is BN6's own
 * afresh. All of it in the director's update, never in a draw. */
#include "glitch.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "bn6_map.h"
#include "director_state.h"
#include "emu.h"
#include "netmap.h"
#include "run.h"

#define LOOKS 3
#define VRAM 0x06000000u
#define PALETTE_SIZE 0x1A0    /* the 13 banks BN6 copies at a map's entry (bn6f decompAndCopyMapTiles_8030472) */
#define GBA_BG2HOFS 0x04000018u
#define GBA_BG2VOFS 0x0400001Au

static struct {
	NetmapLook look[LOOKS];
	int n;
	int shown;            /* the look on screen, -1 the layer's own */
	int step;             /* 0 none; 1 the tiles written, the map next */
	int want;             /* the look the step leads to */
	/* (what the looks cover, as it was before one came) */
	uint32_t lo, hi;      /* the VRAM they write */
	uint32_t *vram;
	uint8_t pal[PALETTE_SIZE];
	uint16_t *map;
	size_t map_bytes;
	bool kept;
} L;

void glitch_looks_free(void) {
	for (int k = 0; k < L.n; ++k) netmap_look_free(&L.look[k]);
	free(L.vram);
	free(L.map);
	memset(&L, 0, sizeof L);
	L.shown = -1;
}

/* (the areas a look may come from: those that draw the run's areas this
 * run, BN6's and another net's, the layer's own left out; in an order of
 * the layer's seed) */
static int candidates(int *out, int max, int own) {
	int n = 0;
	for (int b = 0; b < BIOME_COUNT && n < max; ++b) {
		int a[2] = { run_dress(b), b };
		for (int i = 0; i < 2; ++i) {
			bool seen = a[i] == own;
			for (int j = 0; j < n; ++j) seen |= out[j] == a[i];
			if (!seen && n < max) out[n++] = a[i];
		}
	}
	uint32_t h = run.layer_seed | 1;
	for (int i = n - 1; i > 0; --i) {
		h ^= h << 13; h ^= h >> 17; h ^= h << 5;
		int j = (int)(h % (uint32_t)(i + 1)), t = out[i];
		out[i] = out[j];
		out[j] = t;
	}
	return n;
}

void glitch_looks_make(int own) {
	glitch_looks_free();
	int areas[2 * BIOME_COUNT], n = candidates(areas, 2 * BIOME_COUNT, own), got[LOOKS];
	uint32_t t0 = SDL_GetTicks();
	for (int i = 0; i < n && L.n < LOOKS; ++i)
		if (netmap_look(areas[i], &L.look[L.n])) got[L.n++] = areas[i];
	printf("glitch: %d looks of other areas (", L.n);
	for (int k = 0; k < L.n; ++k) printf("%s%d", k ? ", " : "", got[k]);
	printf(") in %u ms\n", (unsigned)(SDL_GetTicks() - t0));
	L.lo = 0xFFFFFFFFu;
	for (int k = 0; k < L.n; ++k)
		for (int b = 0; b < 2; ++b) {
			const AreaVram *g = &L.look[k].gfx;
			if (!g->tiles[b]) continue;
			if (g->vram[b] < L.lo) L.lo = g->vram[b];
			if (g->vram[b] + g->n[b] > L.hi) L.hi = (uint32_t)(g->vram[b] + g->n[b]);
		}
	L.hi = (L.hi + 3) & ~3u;
}

int glitch_looks(void) { return L.n; }

int glitch_look_shown(void) { return L.shown; }

/* (VRAM takes words: a byte written there lands in both halves) */
static void vram_write(uint32_t at, const uint8_t *p, size_t n) {
	for (size_t k = 0; k + 3 < n; k += 4) emu_write32(VRAM + at + (uint32_t)k, (uint32_t)p[k] | (uint32_t)p[k + 1] << 8 | (uint32_t)p[k + 2] << 16 | (uint32_t)p[k + 3] << 24);
}

/* What the looks cover, read as it is (the layer's own, its colours
 * cycled as far as they are) */
static void keep_own(int tw, int th) {
	free(L.vram);
	free(L.map);
	L.vram = malloc(L.hi - L.lo);
	L.map_bytes = (size_t)tw * th * 4;
	L.map = malloc(L.map_bytes);
	L.kept = L.vram && L.map;
	if (!L.kept) return;
	for (uint32_t k = 0; k < (L.hi - L.lo) / 4; ++k) L.vram[k] = emu_read32(VRAM + L.lo + 4 * k);
	for (uint32_t k = 0; k < PALETTE_SIZE; k += 2) { uint16_t c = emu_read16(BN6_BG_PALETTE + k); memcpy(L.pal + k, &c, 2); }
	for (size_t k = 0; k < L.map_bytes / 2; ++k) L.map[k] = emu_read16(BN6_MAP_TILEMAP + BN6_MAP_TILEMAP_HEAD + 2 * (uint32_t)k);
}

/* The screen maps where the picture shows the tile map (and a tile round
 * it), both layers: BG1 and BG2 scroll alike (bn6_map.h) */
static void screen_refresh(int tw, int th) {
	int px = bn6_player_x(), py = bn6_player_y();
	int u = px + py + 4 * tw - 120, v = (py - px) / 2 + 4 * th - 80;
	int cu = (emu_read16(GBA_BG2HOFS) & 0x1FF) - 376 + 4 * tw, cv = (emu_read16(GBA_BG2VOFS) & 0x1FF) + 176 + 4 * th;
	u += ((cu - u) % 256 + 256 + 128) % 256 - 128;
	v += ((cv - v) % 256 + 256 + 128) % 256 - 128;
	int tx0 = (u >> 3) - 1, ty0 = (v >> 3) - 1, ox = ((376 - 4 * tw) >> 3) & 31, oy = ((-176 - 4 * th) >> 3) & 31;
	static uint16_t row[32];
	for (int layer_k = 0; layer_k < 2; ++layer_k) {
		uint32_t screen = layer_k ? BN6_BG2_SCREEN : BN6_BG1_SCREEN, from = BN6_MAP_TILEMAP + BN6_MAP_TILEMAP_HEAD + 2u * (uint32_t)(layer_k * tw * th);
		for (int ty = ty0; ty < ty0 + 22; ++ty) {
			if (ty < 0 || ty >= th) continue;
			for (int k = 0; k < 32; ++k) {
				int tx = tx0 + k;
				row[(tx + ox) & 31] = tx >= 0 && tx < tw ? emu_read16(from + 2u * (uint32_t)(ty * tw + tx)) : 0;
			}
			emu_write(screen + 64u * (uint32_t)((ty + oy) & 31), row, sizeof row);
		}
	}
}

void glitch_look_flip(int k) {
	if (k >= L.n || k == L.shown || !on_map()) return;
	int tw = emu_read8(BN6_MAP_TILEMAP), th = emu_read8(BN6_MAP_TILEMAP + 1);
	if (k >= 0 && (tw != L.look[k].tw || th != L.look[k].th)) return;
	if (L.shown < 0) keep_own(tw, th);
	if (!L.kept) return;
	/* (the tiles and colours first: the frame between keeps the map) */
	if (k >= 0) {
		const AreaVram *g = &L.look[k].gfx;
		for (int b = 0; b < 2; ++b) if (g->tiles[b]) vram_write(g->vram[b], g->tiles[b], g->n[b]);
		emu_write(BN6_BG_PALETTE, g->pal, g->npal < PALETTE_SIZE ? g->npal : PALETTE_SIZE);
	} else {
		vram_write(L.lo, (const uint8_t *)L.vram, L.hi - L.lo);
		emu_write(BN6_BG_PALETTE, L.pal, PALETTE_SIZE);
	}
	L.want = k;
	L.step = 1;
}

/* (a map entered again, after a battle: BN6's own tile map back in its
 * buffer, whatever look was shown) */
static void map_entered(void) {
	if (L.shown < 0 || L.step) return;
	const uint16_t *map = L.look[L.shown].map;
	for (size_t k = 0; k < L.map_bytes / 2; k += 97)
		if (emu_read16(BN6_MAP_TILEMAP + BN6_MAP_TILEMAP_HEAD + 2 * (uint32_t)k) != map[k]) { L.shown = -1; return; }
}

void glitch_look_update(void) {
	if (!on_map()) return;
	map_entered();
	if (L.step != 1) return;
	int tw = emu_read8(BN6_MAP_TILEMAP), th = emu_read8(BN6_MAP_TILEMAP + 1);
	const uint16_t *map = L.want >= 0 ? L.look[L.want].map : L.map;
	if ((size_t)tw * th * 4 == L.map_bytes) {
		emu_write(BN6_MAP_TILEMAP + BN6_MAP_TILEMAP_HEAD, map, L.map_bytes);
		screen_refresh(tw, th);
	}
	L.shown = L.want;
	L.step = 0;
}
