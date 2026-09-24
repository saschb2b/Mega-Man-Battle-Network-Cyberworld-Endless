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

static void grow(TileSeams *s) {
	TileSeams t = { calloc(s->cap ? s->cap * 2 : 1u << 14, sizeof *t.slot), s->cap ? s->cap * 2 : 1u << 14, 0 };
	for (unsigned i = 0; i < s->cap; ++i) if (s->slot[i]) put(&t, s->slot[i]);
	free(s->slot);
	*s = t;
}

/* Tile i of map a as its neighbours see it. */
static uint32_t look(const AreaSrc *a, bool bg_in_map, int tx, int ty) {
	int W = a->tw * 8;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			size_t p = (size_t)(ty * 8 + y) * W + tx * 8 + x;
			if (bg_in_map ? a->front[p] : a->px[p] >> 24) return a->tile[0][(size_t)ty * a->tw + tx];
		}
	return SEAM_VOID;
}

void seams_add(TileSeams *s, const AreaSrc *a, bool bg_in_map) {
	uint32_t *k = malloc((size_t)a->tw * a->th * sizeof *k);
	for (int ty = 0; ty < a->th; ++ty)
		for (int tx = 0; tx < a->tw; ++tx) k[ty * a->tw + tx] = look(a, bg_in_map, tx, ty);
	for (int ty = 0; ty < a->th; ++ty)
		for (int tx = 0; tx < a->tw; ++tx) {
			if (2 * (s->n + 2) >= s->cap) grow(s);
			uint32_t here = k[ty * a->tw + tx];
			if (tx + 1 < a->tw) put(s, hash(here, k[ty * a->tw + tx + 1], false));
			if (ty + 1 < a->th) put(s, hash(here, k[(ty + 1) * a->tw + tx], true));
		}
	free(k);
}

void seams_free(TileSeams *s) {
	free(s->slot);
	memset(s, 0, sizeof *s);
}

bool seams_seen(const TileSeams *s, uint32_t first, uint32_t second, bool vertical) {
	if (first == SEAM_ANY || second == SEAM_ANY || !s->cap) return true;
	uint64_t h = hash(first, second, vertical);
	for (unsigned i = (unsigned)h & (s->cap - 1); s->slot[i]; i = (i + 1) & (s->cap - 1))
		if (s->slot[i] == h) return true;
	return false;
}

int seams_unseen(const TileSeams *s, uint32_t look, const uint32_t nb[4]) {
	return !seams_seen(s, nb[0], look, false) + !seams_seen(s, nb[1], look, true) +
		!seams_seen(s, look, nb[2], false) + !seams_seen(s, look, nb[3], true);
}
