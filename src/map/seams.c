/* Which tiles the original maps set side by side: a hash set of (direction,
 * first, second) over every pair of neighbouring tiles in the maps an area
 * learns from, mirror images and raised views included. */
#include "seams.h"

#include <stdlib.h>
#include <string.h>

static uint64_t mix(uint64_t x) {
	x ^= x >> 30; x *= 0xBF58476D1CE4E5B9ull;
	x ^= x >> 27; x *= 0x94D049BB133111EBull;
	x ^= x >> 31;
	return x ? x : 1;
}

static uint64_t hash(uint32_t first, uint32_t second, bool vertical) {
	return mix(((uint64_t)first << 32 | second) ^ (vertical ? 0x9E3779B97F4A7C15ull : 0));
}

static void put(TileSeams *s, uint64_t h) {
	unsigned i = (unsigned)h & (s->cap - 1);
	while (s->slot[i]) {
		if (s->slot[i] == h) return;
		i = (i + 1) & (s->cap - 1);
	}
	s->slot[i] = h;
	s->n++;
}

/* (false where memory ran out: the table kept as it was) */
static bool grow(TileSeams *s) {
	TileSeams t = { calloc(s->cap ? s->cap * 2 : 1u << 14, sizeof *t.slot), s->cap ? s->cap * 2 : 1u << 14, 0 };
	if (!t.slot) return false;
	for (unsigned i = 0; i < s->cap; ++i) if (s->slot[i]) put(&t, s->slot[i]);
	free(s->slot);
	*s = t;
	return true;
}

/* Tile i of map a as its neighbours see it. */
static uint32_t look(const AreaSrc *a, bool bg_in_map, int tx, int ty) {
	int W = a->tw * 8;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			size_t p = (size_t)(ty * 8 + y) * W + tx * 8 + x;
			if ((bg_in_map ? a->idx0 : a->idx)[p]) return a->tile[0][(size_t)ty * a->tw + tx];
		}
	return SEAM_VOID;
}

uint32_t seams_side(const uint16_t px[8]) {
	uint64_t a = (uint64_t)px[0] << 48 | (uint64_t)px[1] << 32 | (uint64_t)px[2] << 16 | px[3];
	uint64_t b = (uint64_t)px[4] << 48 | (uint64_t)px[5] << 32 | (uint64_t)px[6] << 16 | px[7];
	return (uint32_t)(mix(a ^ mix(b ^ 0xA0761D6478BD642Full)) >> 32);
}

/* A strip's hash, from its two sides'. */
static uint64_t strip_hash(uint32_t first, uint32_t second, bool vertical) {
	return mix(((uint64_t)first << 32 | second) ^ (vertical ? 0xD6E8FEB86659FD93ull : 0x2D358DCCAA6C78A5ull));
}

static uint16_t strip_px(uint32_t c) {
	return c >> 24 ? (uint16_t)((c >> 19 & 31) | (c >> 11 & 31) << 5 | (c >> 3 & 31) << 10) : STRIP_NONE;
}

/* The strip of picture `pic` (W pixels wide) across the edge between tile
 * (tx, ty) and the one to its right, or with `vertical` the one below,
 * into the set; none where nothing is drawn on either side. */
static void strip_put(TileSeams *s, const AreaSrc *a, const uint8_t *plane, int W, int tx, int ty, bool vertical) {
	uint16_t first[8], second[8];
	bool drawn = false;
	for (int i = 0; i < 8; ++i) {
		size_t p = vertical ? (size_t)(ty * 8 + 7) * W + tx * 8 + i : (size_t)(ty * 8 + i) * W + tx * 8 + 7;
		first[i] = strip_px(area_src_colour(a, plane, p));
		second[i] = strip_px(area_src_colour(a, plane, p + (vertical ? (size_t)W : 1)));
		drawn |= first[i] != STRIP_NONE || second[i] != STRIP_NONE;
	}
	if (drawn) put(s, strip_hash(seams_side(first), seams_side(second), vertical));
}

/* (a map's pairs left out where memory ran out: tools/heap3ds.py's 3DS
 * crashed here, learning a layer's area in a long run) */
void seams_add(TileSeams *s, const AreaSrc *a, bool bg_in_map) {
	uint32_t *k = calloc((size_t)a->tw * a->th, sizeof *k);
	if (!k) return;
	for (int ty = 0; ty < a->th; ++ty)
		for (int tx = 0; tx < a->tw; ++tx) k[ty * a->tw + tx] = look(a, bg_in_map, tx, ty);
	for (int ty = 0; ty < a->th; ++ty)
		for (int tx = 0; tx < a->tw; ++tx) {
			if (2 * (s->n + 6) >= s->cap && !grow(s)) { free(k); return; }
			uint32_t here = k[ty * a->tw + tx];
			if (tx + 1 < a->tw) put(s, hash(here, k[ty * a->tw + tx + 1], false));
			if (ty + 1 < a->th) put(s, hash(here, k[(ty + 1) * a->tw + tx], true));
			/* (and the strips across those edges, as shown and as the first
			 * layer alone draws them: a generated map sets most tiles so) */
			for (int v = 0; v < 2; ++v) {
				if (v ? ty + 1 >= a->th : tx + 1 >= a->tw) continue;
				strip_put(s, a, a->idx, a->tw * 8, tx, ty, v);
				strip_put(s, a, a->idx0, a->tw * 8, tx, ty, v);
			}
		}
	free(k);
}

void seams_free(TileSeams *s) {
	free(s->slot);
	memset(s, 0, sizeof *s);
}

static bool has(const TileSeams *s, uint64_t h) {
	for (unsigned i = (unsigned)h & (s->cap - 1); s->slot[i]; i = (i + 1) & (s->cap - 1))
		if (s->slot[i] == h) return true;
	return false;
}

bool seams_seen(const TileSeams *s, uint32_t first, uint32_t second, bool vertical) {
	if (first == SEAM_ANY || second == SEAM_ANY || !s->cap) return true;
	return has(s, hash(first, second, vertical));
}

bool seams_strip_seen(const TileSeams *s, uint32_t first, uint32_t second, bool vertical) {
	return !s->cap || has(s, strip_hash(first, second, vertical));
}
