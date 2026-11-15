/* The breaks in a generated map's picture, for the tile test (build.py
 * tiles). The picker's own counts (tiles.h: seams, tiles drawn off) say
 * how far a tile's pair was seen from its place; they do not say whether
 * the picture holds together. A floor whose tiles are each right for
 * their panels can still show a block of another shade in its middle or a
 * panel's frame cut off at a tile's edge, where two tiles meet that never
 * do in the original: so the picture itself is held to the originals',
 * edge by edge. */
#include "breaks.h"

#include <stdlib.h>
#include <string.h>

#define NONE 0x8000    /* a pixel nothing draws */
#define SHADE 3        /* steps of a 5-bit channel two pixels may differ by and look alike */

/* A strip from one of its sides: that side's 8 pixels, then the other's. */
typedef struct { uint16_t key[8], other[8]; } Strip;

/* by[v][side]: the strips across vertical (0) or horizontal (1) edges,
 * from their first side (0: left or upper) and from their second. */
struct Breaks {
	Strip *by[2][2];
	size_t n[2][2], cap[2][2];
	bool sorted;
};

bool breaks_measure;

static uint16_t px555(uint32_t c) {
	return c >> 24 ? (uint16_t)((c >> 19 & 31) | (c >> 11 & 31) << 5 | (c >> 3 & 31) << 10) : NONE;
}

/* The strip of picture `pic` (W pixels wide) across the edge between tile
 * (tx, ty) and the one to its right, or with `v` the one below; false
 * where nothing is drawn on either side. */
static bool strip_read(const uint32_t *pic, const AreaSrc *a, const uint8_t *plane, int W, int tx, int ty, int v, uint16_t first[8],
                       uint16_t second[8]) {
	bool drawn = false;
	for (int i = 0; i < 8; ++i) {
		size_t p = v ? (size_t)(ty * 8 + 7) * W + tx * 8 + i : (size_t)(ty * 8 + i) * W + tx * 8 + 7, q = p + (v ? (size_t)W : 1);
		/* (an ARGB picture, or a learned map's colour indices) */
		first[i] = px555(pic ? pic[p] : area_src_colour(a, plane, p));
		second[i] = px555(pic ? pic[q] : area_src_colour(a, plane, q));
		drawn |= first[i] != NONE || second[i] != NONE;
	}
	return drawn;
}

static bool push(Breaks *b, int v, int side, const uint16_t key[8], const uint16_t other[8]) {
	if (b->n[v][side] == b->cap[v][side]) {
		size_t cap = b->cap[v][side] ? b->cap[v][side] * 2 : 4096;
		Strip *s = realloc(b->by[v][side], cap * sizeof *s);
		if (!s) return false;
		b->by[v][side] = s;
		b->cap[v][side] = cap;
	}
	Strip *s = &b->by[v][side][b->n[v][side]++];
	memcpy(s->key, key, sizeof s->key);
	memcpy(s->other, other, sizeof s->other);
	return true;
}

void breaks_add(Breaks **bp, const AreaSrc *a) {
	if (!a) {
		for (int k = 0; *bp && k < 4; ++k) free((*bp)->by[k / 2][k % 2]);
		free(*bp);
		*bp = NULL;
		return;
	}
	if (!breaks_measure) return;
	if (!*bp) *bp = calloc(1, sizeof **bp);
	Breaks *b = *bp;
	if (!b) return;
	b->sorted = false;
	const uint8_t *planes[2] = { a->idx, a->idx0 };
	for (int k = 0; k < 2; ++k)
		for (int ty = 0; ty < a->th; ++ty)
			for (int tx = 0; tx < a->tw; ++tx)
				for (int v = 0; v < 2; ++v) {
					uint16_t first[8], second[8];
					if (v ? ty + 1 >= a->th : tx + 1 >= a->tw) continue;
					if (!strip_read(NULL, a, planes[k], a->tw * 8, tx, ty, v, first, second)) continue;
					if (!push(b, v, 0, first, second) || !push(b, v, 1, second, first)) return;
				}
}

static int cmp_strip(const void *x, const void *y) { return memcmp(x, y, sizeof(Strip)); }

/* Sorted by their key side, each once. */
static void sort(Breaks *b) {
	for (int v = 0; v < 2; ++v)
		for (int side = 0; side < 2; ++side) {
			Strip *s = b->by[v][side];
			size_t n = b->n[v][side], kept = 0;
			if (!n) continue;
			qsort(s, n, sizeof *s, cmp_strip);
			for (size_t i = 0; i < n; ++i)
				if (!kept || memcmp(&s[kept - 1], &s[i], sizeof *s)) s[kept++] = s[i];
			b->n[v][side] = kept;
		}
	b->sorted = true;
}

static bool alike(uint16_t x, uint16_t y) {
	if (x == y) return true;
	if (x == NONE || y == NONE) return false;
	for (int k = 0; k < 15; k += 5) {
		int d = (x >> k & 31) - (y >> k & 31);
		if (d > SHADE || d < -SHADE) return false;
	}
	return true;
}

/* The fewest pixels of `other` that differ from a strip's the maps show
 * beside `key` (8: none shows `key` on that side at all). */
static int nearest(const Breaks *b, int v, int side, const uint16_t key[8], const uint16_t other[8]) {
	const Strip *s = b->by[v][side];
	size_t lo = 0, hi = b->n[v][side];
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		if (memcmp(s[mid].key, key, sizeof s->key) < 0) lo = mid + 1; else hi = mid;
	}
	int best = 8;
	for (size_t i = lo; i < b->n[v][side] && best && !memcmp(s[i].key, key, sizeof s->key); ++i) {
		int d = 0;
		for (int k = 0; k < 8; ++k) d += !alike(s[i].other[k], other[k]);
		if (d < best) best = d;
	}
	return best;
}

void breaks_count(Breaks *b, const uint32_t *px, int tw, int th, uint8_t *out, int *breaks, int *nicks) {
	*breaks = *nicks = 0;
	memset(out, 0, (size_t)tw * th);
	if (!b) return;
	if (!b->sorted) sort(b);
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx)
			for (int v = 0; v < 2; ++v) {
				uint16_t first[8], second[8];
				if (v ? ty + 1 >= th : tx + 1 >= tw) continue;
				if (!strip_read(px, NULL, NULL, tw * 8, tx, ty, v, first, second)) continue;
				int d = nearest(b, v, 0, first, second), e = d ? nearest(b, v, 1, second, first) : 0;
				if (e < d) d = e;
				if (!d) continue;
				bool whole = d >= BREAK_PIXELS;
				out[ty * tw + tx] |= (uint8_t)(1u << (v + (whole ? 0 : 2)));
				++*(whole ? breaks : nicks);
			}
}
