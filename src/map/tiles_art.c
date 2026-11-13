/* A map's floor's own art and the floor each pixel of it belongs to
 * (tiles_art.h): what a border draws past its tile's line, kept with the
 * piece of floor it hangs on. */
#include "tiles_art.h"

#include <stdlib.h>
#include <string.h>

#include "netmap.h"

/* A colour of the map's picture as a tile's pixels hold them. */
static unsigned colour_of(uint32_t c) { return (c >> 19 & 31) | (c >> 11 & 31) << 5 | (c >> 3 & 31) << 10; }

/* Whether panel (A, B) is floor whose tiles are learned. */
static bool own_floor(const Src *src, int A, int B) {
	int st = tl_src_panel(src, A, B);
	return (st == TILE_A || st == TILE_B) && !tl_skipped(src, A, B);
}

/* Whether tile (tx, ty) has floor whose tiles are learned in or round its
 * panel. */
static bool near_own_floor(const Src *src, const TileGrid *g, int tx, int ty) {
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	for (int k = 0; k < 9; ++k)
		if (own_floor(src, A + k % 3 - 1, B + k / 3 - 1)) return true;
	return false;
}

/* (in at least one in SIDE_SHARE of the tiles the most common such colour
 * is in; counted by tiles, not pixels: a side's outline is a pixel or two
 * in every tile, a hub or a road set against a pad many pixels in a few) */
#define SIDE_SHARE 4
uint8_t *tl_pad_colours(const Src *src, const TileGrid *g) {
	const AreaSrc *a = src->a;
	int W = a->tw * 8;
	uint16_t *n = calloc(TILE_COLOURS, 2 * sizeof *n), *last = n ? n + TILE_COLOURS : NULL;
	uint8_t *own = n ? calloc(TILE_COLOURS, 1) : NULL;
	if (!own) { free(n); return NULL; }
	for (int ty = 0, tile = 1; ty < a->th; ++ty)
		for (int tx = 0; tx < a->tw; ++tx, ++tile) {
			if (!near_own_floor(src, g, tx, ty)) continue;
			uint64_t must, never, deep, deep_b;
			tl_expect(g, tx, ty, tl_src_floor, src, TILE_B, &must, &never, &deep_b);
			tl_expect(g, tx, ty, tl_src_floor, src, TILE_A, &must, &never, &deep);
			for (int i = 0, pa, pb; i < 64; ++i) {
				uint32_t c = area_src_floor_px(a, (size_t)(ty * 8 + i / 8) * W + tx * 8 + i % 8);
				if (!(c >> 24)) continue;
				tl_pixel_panel(g, tx * 8 + i % 8, ty * 8 + i / 8, &pa, &pb);
				if ((deep | deep_b) >> i & 1 && own_floor(src, pa, pb)) own[colour_of(c)] = 1;
				/* (each colour once a tile: `last`, the tile it was last seen in) */
				if (!(must >> i & 1) || tl_src_panel(src, pa, pb) || last[colour_of(c)] == (uint16_t)tile) continue;
				last[colour_of(c)] = (uint16_t)tile;
				++n[colour_of(c)];
			}
		}
	int most = 0;
	for (int i = 0; i < TILE_COLOURS; ++i) most = n[i] > most ? n[i] : most;
	for (int i = 0; i < TILE_COLOURS; ++i) own[i] = own[i] || (n[i] && n[i] * SIDE_SHARE >= most);
	free(n);
	return own;
}

/* A map's floor's own art, pixel by pixel, and the floor each pixel of it
 * belongs to. A border's art runs past the line a tile may draw up to (a
 * slab a tile wider than its floor, a wall over an upper edge, an outline
 * round a corner): judged alone, the tile that holds only the overhang is
 * turned down, and the border comes out cut at that tile's edge, a tooth at
 * every other tile. So the art is followed: from the pixels the floor's
 * tiles must draw (its tops and the faces under its edges) to every pixel
 * of the same colour beside one of its own, as far as tiles with the floor
 * in or round their panel reach, but not onto floor of another kind; then
 * what is drawn straight under it, and its outline. With `seeds`, a map's
 * pads' colours, the art is theirs alone: begun in those colours, and
 * followed into any of them.
 *
 * Each pixel the tiles must draw is its floor panel's, and each pixel that
 * joins takes the panel of the one it joined: so a map's art is told apart
 * by the piece of floor it hangs on, a platform's rim from the walkway's
 * that runs past it. A tile may draw the art of the floors in and round
 * its own panel wherever it lies (panels.h, panels_own), and no other's. */
enum { ART_NONE, ART_OWN, ART_OPEN, ART_LINE, ART_MAY };   /* (open: could be, not reached; line: joining, found; may: open, and where a tile may draw) */

/* A map's picture, what of it is its floor's art so far (`art`), and the
 * panel each such pixel belongs to (`of`, panel_code). */
typedef struct { const AreaSrc *a; uint8_t *art, *of; int W, H; } Art;

/* A panel as a pixel's byte: its place in a lattice of PIECES by PIECES
 * panels, from 1 (0: no art). Two panels PIECES apart share a number, and
 * no tile draws both. */
#define PIECES 15
#define OTHERS 0xFF   /* (in `of` while the art is followed: floor of another kind's) */
static int wrap(int v) { return (v % PIECES + PIECES) % PIECES; }
static uint8_t panel_code(int A, int B) { return (uint8_t)(1 + wrap(A) + PIECES * wrap(B)); }

/* Where the panel of `code` lies round panel (A, B): k as a shape's bits
 * (panel (A + k % 3 - 1, B + k / 3 - 1)), 9 further off. */
static int code_round(uint8_t code, int A, int B) {
	int da = wrap((code - 1) % PIECES - A + 1), db = wrap((code - 1) / PIECES - B + 1);
	return da > 2 || db > 2 ? 9 : db * 3 + da;
}

static int art_at(const Art *t, int x, int y) {
	return x < 0 || y < 0 || x >= t->W || y >= t->H ? ART_NONE : t->art[(size_t)y * t->W + x];
}

/* The map's floor pixel at (x, y), 0 where it draws none. */
static uint32_t art_px(const Art *t, int x, int y) {
	if (x < 0 || y < 0 || x >= t->W || y >= t->H) return 0;
	uint32_t c = area_src_floor_px(t->a, (size_t)y * t->W + x);
	return c >> 24 ? c : 0;
}

/* The floor panel a pixel the map's tiles must draw belongs to: the one it
 * lies on, or for a face's pixel the first straight above it (as far up as
 * the tallest slab stands: BN4's Town Area's squares, 17 pixels where the
 * map's walkways are 10). OTHERS where that is none of the floor's: a pixel
 * that floor of another kind must draw, off its own panel. */
static uint8_t owner_at(const Src *src, const TileGrid *g, int x, int y) {
	for (int up = 0, pa, pb; up <= 2 * FACE_TALL && y - up >= 0; ++up) {
		tl_pixel_panel(g, x, y - up, &pa, &pb);
		if (own_floor(src, pa, pb)) return panel_code(pa, pb);
	}
	return OTHERS;
}

/* Where the art begins and how far it may go: the pixels the floor's tiles
 * must draw its own (with `seeds`, those in these colours), the others of
 * the tiles round the floor open to it; and floor of another kind begins
 * art of its own, which is none of the floor's. */
static void art_begin(const Src *src, const TileGrid *g, const uint8_t *seeds, Art *t) {
	for (int ty = 0; ty < t->a->th; ++ty)
		for (int tx = 0; tx < t->a->tw; ++tx) {
			bool near = near_own_floor(src, g, tx, ty);
			uint64_t must = 0, never = 0, deep;
			if (near) tl_expect(g, tx, ty, tl_src_floor, src, TILE_A, &must, &never, &deep);
			for (int i = 0, pa, pb; i < 64; ++i) {
				int x = tx * 8 + i % 8, y = ty * 8 + i / 8;
				size_t at = (size_t)y * t->W + x;
				uint32_t c = art_px(t, x, y);
				if (!c) continue;
				tl_pixel_panel(g, x, y, &pa, &pb);
				/* (floor of another kind, wherever it lies: its art is followed as
				 * the floor's own is, and what is nearer to it is its, not ours) */
				if (tl_src_panel(src, pa, pb) && !own_floor(src, pa, pb)) {
					t->art[at] = ART_OWN;
					t->of[at] = OTHERS;
					continue;
				}
				if (!near) continue;
				t->art[at] = must >> i & 1 && (!seeds || seeds[colour_of(c)]) ? ART_OWN : never >> i & 1 ? ART_OPEN : ART_MAY;
				if (t->art[at] == ART_OWN) t->of[at] = owner_at(src, g, x, y);
			}
		}
}

/* Pixel i joins the art as its pixel `from` did. */
static void art_join(const Art *t, size_t i, size_t from, int as) {
	t->art[i] = (uint8_t)as;
	t->of[i] = t->of[from];
}

/* Whether open pixel i may join the art from its pixel `from`: in its own
 * colour, and with `seeds`, in any of those colours (an outline is another
 * colour than what it lines). Floor of another kind takes whatever is drawn
 * beside its art: nothing says which of its pixels its tiles must draw, so
 * its rim is not begun in its own colours as the floor's is, and would go
 * to the floor whose rim of the same colour it runs into (BN5's Oran Area:
 * a walkway's grey rim off a room's corner, laid as spikes on every room). */
static bool art_joins(const Art *t, const uint8_t *seeds, int i, int from) {
	if (t->art[i] != ART_OPEN && t->art[i] != ART_MAY) return false;
	if (t->of[from] == OTHERS) return true;
	uint32_t c = art_px(t, i % t->W, i / t->W);
	return (seeds && seeds[colour_of(c)]) || c == art_px(t, from % t->W, from / t->W);
}

/* The art followed from the pixels it begins in: each open pixel beside one
 * of the art's that it may join (art_joins) joins it, nearest first, so a
 * pixel between two floors goes to the nearer one's art and not to the one
 * a sweep over the map comes from. (Swept there and back instead, a
 * walkway's rim running past a room came out the room's: BN5's Oran Area.)
 * False without memory. */
static bool art_follow(const Art *t, const uint8_t *seeds) {
	int n = 0, head = 0, tail = 0, W = t->W, side[4] = { -1, 1, -t->W, t->W };
	for (int i = 0; i < W * t->H; ++i) n += t->art[i] == ART_OPEN || t->art[i] == ART_MAY;
	int *queue = malloc(((size_t)n + 1) * sizeof *queue);
	if (!queue) return false;
	for (int i = 0; i < W * t->H; ++i)
		for (int k = 0; k < 4; ++k) {
			int from = i + side[k];
			/* (a row's end is not beside the next row's start) */
			if (from < 0 || from >= W * t->H || (k < 2 && from / W != i / W) || t->art[from] != ART_OWN || !art_joins(t, seeds, i, from)) continue;
			art_join(t, (size_t)i, (size_t)from, ART_LINE);
			queue[tail++] = i;
			break;
		}
	while (head < tail) {
		int from = queue[head++];
		t->art[from] = ART_OWN;
		for (int k = 0; k < 4; ++k) {
			int i = from + side[k];
			if (i < 0 || i >= W * t->H || (k < 2 && i / W != from / W) || !art_joins(t, seeds, i, from)) continue;
			art_join(t, (size_t)i, (size_t)from, ART_LINE);
			queue[tail++] = i;
		}
	}
	free(queue);
	return true;
}

/* What is drawn straight under the art joins it, whatever its colours: a
 * slab's shading under its rim, in bands a pixel thick that no colour of
 * their own joins across a step. As far down as a tile may draw there (a
 * face and what hangs under it), and past that line as many pixels as a
 * tile may draw beyond its floor's edge, no further (BN5's Undernet hangs
 * lava under its floors). */
static void art_under(const Art *t) {
	for (int x = 0; x < t->W; ++x)
		for (int y = 1, run = 0; y < t->H; ++y) {
			size_t i = (size_t)y * t->W + x;
			if (t->art[i] == ART_OWN) { run = 0; continue; }
			bool under = run || t->art[i - t->W] == ART_OWN || t->art[i - t->W] == ART_LINE;
			if (t->art[i] == ART_MAY && under && run <= TILE_OUT) { art_join(t, i, i - t->W, ART_LINE); continue; }
			run = t->art[i] == ART_OPEN && under && run < TILE_OUT ? run + 1 : TILE_OUT + 1;
			if (run <= TILE_OUT) art_join(t, i, i - t->W, ART_LINE);
		}
	for (int i = 0; i < t->W * t->H; ++i)
		if (t->art[i] == ART_LINE) t->art[i] = ART_OWN;
}

/* The art's outline joins it: what is drawn between its art and nothing,
 * whatever its colour, a line a pixel thick at a time and as many lines as
 * a tile may draw pixels beyond its floor's edge (a rim's highlight, its
 * shade and its outline; never where the floor's tiles must draw, so no
 * colour of theirs). BN4's Park Area's platform ends in such a rim 13 pixels
 * past its right corner, where a tile may draw 3. */
static void art_outline(const Art *t) {
	static const int side[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
	for (int line = 0; line < TILE_OUT; ++line) {
		for (int i = 0; i < t->W * t->H; ++i) {
			int x = i % t->W, y = i / t->W, from = -1;
			if (t->art[i] != ART_OPEN && t->art[i] != ART_MAY) continue;
			for (int k = 0; k < 4 && from < 0; ++k)
				if (art_at(t, x + side[k][0], y + side[k][1]) == ART_OWN) from = i + side[k][0] + side[k][1] * t->W;
			bool bare = false;
			/* (nothing within the lines still to come) */
			for (int n = TILE_OUT - line, dy = -n; from >= 0 && dy <= n && !bare; ++dy)
				for (int dx = -n; dx <= n && !bare; ++dx) bare = !art_px(t, x + dx, y + dy);
			if (bare) art_join(t, (size_t)i, (size_t)from, ART_LINE);
		}
		for (int i = 0; i < t->W * t->H; ++i)
			if (t->art[i] == ART_LINE) t->art[i] = ART_OWN;
	}
}

/* Whether a map's floors all lie at one height. Where they do not, a view
 * of one height takes the others' floors for nothing at all, and what is
 * drawn under its art may be theirs (BN5's Oran Area: a walkway's rim off a
 * raised room's corner). */
static bool one_level(const AreaSrc *a) {
	int count[256] = { 0 };
	for (int i = 0; a->hz && i < a->hw * a->hh; ++i) count[a->hz[i]]++;
	for (int z = 8; z < HEIGHT_UNEVEN; z += 8)
		if (count[z] >= NETMAP_LEVEL_MIN_CELLS) return false;
	return !a->level;
}

uint8_t *tl_own_art(const Src *src, const TileGrid *g, const uint8_t *seeds) {
	Art t = { src->a, NULL, NULL, src->a->tw * 8, src->a->th * 8 };
	t.art = calloc((size_t)t.W, (size_t)t.H);
	t.of = calloc((size_t)t.W, (size_t)t.H);
	if (!t.art || !t.of) { free(t.art); free(t.of); return NULL; }
	art_begin(src, g, seeds, &t);
	if (!art_follow(&t, seeds)) { free(t.art); free(t.of); return NULL; }
	if (!seeds && one_level(src->a)) art_under(&t);
	art_outline(&t);
	for (int i = 0; i < t.W * t.H; ++i)
		if (t.art[i] != ART_OWN || t.of[i] == OTHERS) t.of[i] = 0;
	free(t.art);
	return t.of;
}

/* The panel round tile (tx, ty)'s own that pixel i of it is the art of
 * (code_round), -1 no art. */
static int art_round(const Src *src, int A, int B, int tx, int ty, int i) {
	uint8_t code = src->art[(size_t)(ty * 8 + i / 8) * src->a->tw * 8 + tx * 8 + i % 8];
	return code ? code_round(code, A, B) : -1;
}

uint64_t tl_art_of(const Src *src, const TileGrid *g, int tx, int ty) {
	uint64_t own = 0;
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	for (int i = 0; src->art && i < 64; ++i) {
		int k = art_round(src, A, B, tx, ty, i);
		if (k >= 0 && k < 9) own |= 1ull << i;
	}
	return own;
}
