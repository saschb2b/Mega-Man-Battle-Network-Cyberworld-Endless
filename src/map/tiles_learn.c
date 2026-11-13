/* An area's tiles learned from its original maps (tiles.c): the shapes its
 * floor's tiles take, its patches and their period, its variety. */
#include "tiles_learn.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "panels.h"
#include "tiles_parts.h"

static void learn_variety(TileBook *out);

/* pixels over a floor's upper edge a wall on it may stand (a panel's tile
 * the pixel test turned down for them is its own: panels.h) */
#define WALL_BAND 8

static void src_close(Src *s) {
	free(s->state);
	free(s->drawn);
}

/* A view of the area's original map, its panels measured as they are
 * asked after: `s` with its area and styles set; false where there is no
 * memory for it */
static bool src_open(Src *s) {
	s->state = malloc(SPAN * SPAN);
	s->drawn = malloc(SPAN * SPAN);
	if (!s->state || !s->drawn) {
		src_close(s);
		return false;
	}
	memset(s->state, -1, SPAN * SPAN);
	memset(s->drawn, -1, SPAN * SPAN);
	return true;
}

/* The panels' neighbourhoods, as legal.c asks after them (every ring of
 * walls inside the floor a hole: what the tiles are learned from leaves the
 * floors a layer keeps as they were), sorted, each once, and kept at the
 * size they hold (an area kept eight books of a whole map's tiles each,
 * 30 MB for the Undernet's, and a New 3DS ran out of memory entering it). */
static void learn_shapes(const Src *src, uint8_t *pads, TileBook *out) {
	Src walled = { src->a, src->styles, src->walk_styles, src->skip_styles, src->joint_hues, src->bg_in_map, NULL, NULL, false, NULL };
	bool own = src->inner_walls && src_open(&walled);
	uint8_t *walled_pads = own ? tl_find_pads(&walled) : pads;
	if (!own) walled = *src;
	out->nshapes = 0;
	out->shapes = walled_pads ? malloc(SPAN * SPAN * sizeof *out->shapes) : NULL;   /* (none: memory ran out) */
	for (int B = -SPAN / 2 + 1; out->shapes && walled_pads && B < SPAN / 2 - 1; ++B)
		for (int A = -SPAN / 2 + 1; A < SPAN / 2 - 1; ++A) {
			unsigned oa = 0, ob = 0;
			bool mixed = false;
			for (int k = 0; k < 9; ++k) {
				int st = tl_src_panel(&walled, A + k % 3 - 1, B + k / 3 - 1);
				if (st == OTHER) mixed = true;
				if (st == TILE_A) oa |= 1u << k;
				if (st == TILE_B) ob |= 1u << k;
			}
			if (mixed || !(oa | ob)) continue;
			out->shapes[out->nshapes++] = TILE_SHAPE(oa, ob, walled_pads[(B + SPAN / 2) * SPAN + A + SPAN / 2]);
		}
	if (own) {
		src_close(&walled);
		free(walled_pads);
	}
	if (!out->shapes) return;
	qsort(out->shapes, (size_t)out->nshapes, sizeof *out->shapes, tl_cmp_u32);
	int ns = 0;
	for (int i = 0; i < out->nshapes; ++i)
		if (!ns || out->shapes[ns - 1] != out->shapes[i]) out->shapes[ns++] = out->shapes[i];
	out->nshapes = ns;
	uint32_t *shapes = realloc(out->shapes, ((size_t)ns + 1) * sizeof *out->shapes);
	if (shapes) out->shapes = shapes;
}

/* Each class's first pair: its pairs lie together, the most common first
 * (in place: pairs dropped leave no more classes). */
void tl_index_classes(TileBook *b) {
	b->ncls = 0;
	for (int i = 0; i < b->n; ++i)
		if (!i || b->cand[i].key != b->cand[i - 1].key) b->cls[b->ncls++] = (TileClass){ b->cand[i].key, i };
}

/* One entry per pair, counted, the most common first, kept at the size
 * they hold, as the shapes are. */
static void keep_cands(TileCand *c, size_t n, TileBook *out) {
	qsort(c, n, sizeof *c, tl_cmp_cand);
	int nu = 0;
	for (size_t i = 0; i < n;) {
		size_t j = i;
		while (j < n && c[j].key == c[i].key && c[j].e0 == c[i].e0 && c[j].e1 == c[i].e1 && c[j].pad == c[i].pad) ++j;
		c[nu] = c[i];
		tiles_sides(c[nu].px, c[nu].mask, c[nu].side);
		c[nu++].count = (uint32_t)(j - i);
		i = j;
	}
	qsort(c, (size_t)nu, sizeof *c, tl_cmp_count);
	TileCand *kept_c = realloc(c, ((size_t)nu + 1) * sizeof *c);
	out->cand = kept_c ? kept_c : c;
	out->n = nu;
	int ncls = 0;
	for (int i = 0; i < nu; ++i) ncls += !i || out->cand[i].key != out->cand[i - 1].key;
	out->cls = malloc(((size_t)ncls + 1) * sizeof *out->cls);
	if (!out->cls) out->n = 0;   /* (a book of nothing) */
	tl_index_classes(out);
}

/* ---- the floor's middle, by its period ---- */

#define PATCH_MAX 6        /* panels a period spans at most, per axis */
#define PATCH_SHARE 95     /* % of the middle's tiles a period must explain */
#define PATCH_MIN 24       /* tiles of the middle a book needs for one */

typedef struct { int A, B, phase, cand; } PatchSample;

int tl_mod_of(int v, int m) { int r = v % m; return r < 0 ? r + m : r; }

/* The share (in % of `n`) of the samples the period (pa, pb) explains: per
 * panel of the period and phase, its most common pair's; that pair in
 * `best` (pa * pb * 64 entries, -1 none). */
static int patch_fit(const PatchSample *s, int n, int pa, int pb, int16_t *best) {
	int cells = pa * pb * 64;
	static int16_t cand[PATCH_MAX * PATCH_MAX * 64][4];
	static uint16_t cnt[PATCH_MAX * PATCH_MAX * 64][4];
	for (int i = 0; i < cells; ++i) for (int k = 0; k < 4; ++k) { cand[i][k] = -1; cnt[i][k] = 0; }
	for (int i = 0; i < n; ++i) {
		int at = (tl_mod_of(s[i].B, pb) * pa + tl_mod_of(s[i].A, pa)) * 64 + s[i].phase, k = 0;
		while (k < 4 && cand[at][k] >= 0 && cand[at][k] != s[i].cand) ++k;
		if (k == 4) continue;
		cand[at][k] = (int16_t)s[i].cand;
		++cnt[at][k];
	}
	int took = 0;
	for (int i = 0; i < cells; ++i) {
		int m = 0;
		for (int k = 1; k < 4; ++k) if (cnt[i][k] > cnt[i][m]) m = k;
		best[i] = cnt[i][m] ? cand[i][m] : -1;
		took += cnt[i][m];
	}
	return n ? took * 100 / n : 0;
}

#define STRETCH_MAX 8   /* panels of the largest stretch kept, per axis */
#define STRETCH_MIN 3

/* The largest rectangle of the middle's panels (up to STRETCH_MAX a side,
 * every tile of them seen) as the patch, its tiles as the original lays
 * them. */
/* The largest rectangle (STRETCH_MIN to STRETCH_MAX panels a side) of the
 * w x h panels whose tiles are all seen (tiles[k] == full): its corner and
 * size in r (a, b, w, h), its area returned (0 none). */
static int largest_whole(const uint8_t *tiles, int w, int h, int full, int r[4]) {
	int best = 0;
	for (int b = 0; b < h; ++b)
		for (int a = 0; a < w; ++a)
			for (int rh = STRETCH_MIN; rh <= STRETCH_MAX && b + rh <= h; ++rh)
				for (int rw = STRETCH_MIN; rw <= STRETCH_MAX && a + rw <= w; ++rw) {
					if (rw * rh <= best) continue;
					bool whole = true;
					for (int k = 0; k < rw * rh && whole; ++k) whole = tiles[(b + k / rw) * w + a + k % rw] == full;
					if (whole) { best = rw * rh; r[0] = a; r[1] = b; r[2] = rw; r[3] = rh; }
				}
	return best;
}

static void patch_stretch(const PatchSample *s, int n, TileBook *out) {
	int A0 = INT_MAX, B0 = INT_MAX, A1 = INT_MIN, B1 = INT_MIN;
	for (int i = 0; i < n; ++i) {
		A0 = s[i].A < A0 ? s[i].A : A0;
		A1 = s[i].A > A1 ? s[i].A : A1;
		B0 = s[i].B < B0 ? s[i].B : B0;
		B1 = s[i].B > B1 ? s[i].B : B1;
	}
	if (n < PATCH_MIN || A1 - A0 + 1 < STRETCH_MIN || B1 - B0 + 1 < STRETCH_MIN || A1 - A0 >= SPAN || B1 - B0 >= SPAN) return;
	int w = A1 - A0 + 1, h = B1 - B0 + 1, full = 0, r[4] = { 0 };
	/* per panel: its tiles' pairs by phase (-1 none) and how many it has */
	int16_t *at = malloc((size_t)w * h * 64 * sizeof *at);
	uint8_t *tiles = calloc((size_t)w * h, 1);
	if (at && tiles) {
		for (int i = 0; i < w * h * 64; ++i) at[i] = -1;
		for (int i = 0; i < n; ++i) {
			int k = (s[i].B - B0) * w + s[i].A - A0;
			tiles[k] += at[k * 64 + s[i].phase] < 0;
			at[k * 64 + s[i].phase] = (int16_t)s[i].cand;
		}
		/* (a panel of the middle has as many tiles as the most of them) */
		for (int k = 0; k < w * h; ++k) full = tiles[k] > full ? tiles[k] : full;
		if (largest_whole(tiles, w, h, full, r)) out->patch = malloc((size_t)r[2] * r[3] * 64 * sizeof *out->patch);
	}
	/* (the patch's panel (i, j) lies where A mod its width is i: the stretch's own) */
	for (int k = 0; out->patch && k < r[2] * r[3]; ++k) {
		int A = A0 + r[0] + k % r[2], B = B0 + r[1] + k / r[2];
		memcpy(&out->patch[(tl_mod_of(B, r[3]) * r[2] + tl_mod_of(A, r[2])) * 64], &at[((B - B0) * w + A - A0) * 64], 64 * sizeof *out->patch);
	}
	if (out->patch) { out->pa = r[2]; out->pb = r[3]; }
	free(at);
	free(tiles);
}

/* The platform floor's middle (panels with platform floor all round, not
 * pads, nor beside a skipped panel): its tiles' pairs and the smallest
 * period that lays them as the original does. */
/* Whether panel (A, B) of the source lies in its platform floor's middle:
 * platform floor all round, not a pad, nor beside a skipped panel. */
static bool src_middle(const Src *src, const uint8_t *pads, int A, int B) {
	if (A < -SPAN / 2 || B < -SPAN / 2 || A >= SPAN / 2 || B >= SPAN / 2 || pads[(B + SPAN / 2) * SPAN + A + SPAN / 2]) return false;
	for (int k = 0; k < 9; ++k)
		if (tl_src_panel(src, A + k % 3 - 1, B + k / 3 - 1) != TILE_A || tl_skipped(src, A + k % 3 - 1, B + k / 3 - 1)) return false;
	return true;
}

/* The pair learned for source tile i in the middle's class at `phase`, -1 none. */
static int middle_pair(const AreaSrc *a, const TileBook *out, size_t i, int phase) {
	uint32_t key = KEY(phase, 0x1FF, 0);
	for (int j = tl_first_class(out, key); j < out->ncls && out->cls[j].key == key; ++j) {
		int end = j + 1 < out->ncls ? out->cls[j + 1].first : out->n;
		for (int c = out->cls[j].first; c < end; ++c) {
			const TileCand *t = &out->cand[c];
			if (t->e0 == a->tile[0][i] && !t->pad && (!t->e1 || (a->layers > 1 && t->e1 == a->tile[1][i]))) return c;
		}
	}
	return -1;
}

/* The smallest period that explains PATCH_SHARE of the samples, as the
 * patch; false none. */
static bool patch_period(const PatchSample *s, int n, TileBook *out) {
	static int16_t best[PATCH_MAX * PATCH_MAX * 64];
	for (int area = 1; n >= PATCH_MIN && area <= PATCH_MAX * PATCH_MAX; ++area)
		for (int pa = 1; pa <= PATCH_MAX; ++pa) {
			int pb = area / pa;
			if (area % pa || pb > PATCH_MAX || patch_fit(s, n, pa, pb, best) < PATCH_SHARE) continue;
			out->patch = malloc((size_t)pa * pb * 64 * sizeof *out->patch);
			if (!out->patch) return false;
			memcpy(out->patch, best, (size_t)pa * pb * 64 * sizeof *best);
			out->pa = pa;
			out->pb = pb;
			return true;
		}
	return false;
}

static void learn_patch(const AreaSrc *a, const Src *src, const TileGrid *g, const uint8_t *pads, TileBook *out) {
	PatchSample *s = malloc(((size_t)a->tw * a->th + 1) * sizeof *s);
	if (!s) return;
	int n = 0;
	for (int ty = 0; ty < a->th; ++ty)
		for (int tx = 0; tx < a->tw; ++tx) {
			int phase, A, B;
			tile_class(g, tx, ty, &phase, &A, &B);
			int c = src_middle(src, pads, A, B) ? middle_pair(a, out, (size_t)ty * a->tw + tx, phase) : -1;
			if (c >= 0) s[n++] = (PatchSample){ A, B, phase, c };
		}
	/* none: its largest stretch, laid whole (Mr. Weather Comp's solar panels,
	 * whose lights cross the tiles' edges at random: each tile taking a
	 * look of its own cut them, the plain one alone left bands); and with
	 * none of those, its looks scattered */
	if (!patch_period(s, n, out)) patch_stretch(s, n, out);
	if (!out->patch) learn_variety(out);
	free(s);
}

/* Whether two pairs draw the same along the tile's edges. */
static bool same_edges(const TileCand *a, const TileCand *b) {
	if (a->e1 != b->e1) return false;
	for (int i = 0; i < 64; ++i) {
		int x = i % 8, y = i / 8;
		if (x && y && x < 7 && y < 7) continue;
		bool da = a->mask >> i & 1, db = b->mask >> i & 1;
		if (da != db || (da && a->px[i] != b->px[i])) return false;
	}
	return true;
}

/* The platform floor's middle where it has no period (patch): per phase,
 * its pairs keeping to the plain one's edges, and how often each is seen. */
static void learn_variety(TileBook *out) {
	int cap = out->n + 1, nv = 0;
	out->vary = malloc((size_t)cap * sizeof *out->vary);
	out->vary_weight = malloc((size_t)cap * sizeof *out->vary_weight);
	if (!out->vary || !out->vary_weight) { free(out->vary); free(out->vary_weight); out->vary = NULL; out->vary_weight = NULL; return; }
	for (int phase = 0; phase < 64; ++phase) {
		out->vary_first[phase] = nv;
		uint32_t key = KEY(phase, 0x1FF, 0);
		int j = tl_first_class(out, key);
		if (j >= out->ncls || out->cls[j].key != key) continue;
		int end = j + 1 < out->ncls ? out->cls[j + 1].first : out->n;
		const TileCand *plain = NULL;
		uint32_t all = 0, held = 0;   /* (the looks besides the plain one: seen, and held) */
		for (int c = out->cls[j].first; c < end; ++c) {
			const TileCand *t = &out->cand[c];
			if (t->pad) continue;
			if (!plain) { plain = t; out->vary[nv] = *t; out->vary_weight[nv++] = t->count; continue; }
			all += t->count;
			if (!same_edges(t, plain)) continue;
			held += t->count;
			out->vary[nv] = *t;
			out->vary_weight[nv++] = t->count;
		}
		/* the kept looks as often as all of them are seen: those reaching
		 * a tile's edge (a light across two tiles) would be cut here */
		for (int i = out->vary_first[phase] + 1; held && i < nv; ++i)
			out->vary_weight[i] = (uint32_t)((uint64_t)out->vary_weight[i] * all / held);
	}
	out->vary_first[64] = nv;
}

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

/* The colours of a map's pads (TILES_PADS_ALONE): what their tops show
 * well inside them, and what the map draws off the floor where a pad's side
 * face must be drawn, in at least one in SIDE_SHARE of the tiles the most
 * common such colour is in. Counted by tiles, not pixels: a side's outline
 * is a pixel or two in every tile, a hub or a road set against a pad many
 * pixels in a few. A byte per colour; NULL without memory. (The map's
 * pixels are kept: tiles_keep_begin.) */
#define SIDE_SHARE 4
static uint8_t *pad_colours(const Src *src, const TileGrid *g) {
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

/* A map's floor's own art, pixel by pixel. A border's art runs past the
 * line a tile may draw up to (a slab a tile wider than its floor, a wall
 * over an upper edge, an outline round a corner): judged alone, the tile
 * that holds only the overhang is turned down, and the border comes out cut
 * at that tile's edge, a tooth at every other tile. So the art is followed:
 * from the pixels the floor's tiles must draw (its tops and the faces under
 * its edges) to every pixel of the same colour beside one of its own, as
 * far as tiles with the floor in or round their panel reach, but not onto
 * floor of another kind. With `seeds`, a map's pads' colours, the art is
 * theirs alone: begun in those colours, and followed into any of them. A tile
 * may draw its floor's own art wherever it lies (panels.h, panels_own).
 * A byte per map pixel (1: its own); NULL without memory. */
enum { ART_NONE, ART_OWN, ART_OPEN, ART_LINE };   /* (open: could be, not reached; line: its outline, found) */

/* A map's picture and what of it is its floor's art so far. */
typedef struct { const AreaSrc *a; uint8_t *art; int W, H; } Art;

static int art_at(const Art *t, int x, int y) {
	return x < 0 || y < 0 || x >= t->W || y >= t->H ? ART_NONE : t->art[(size_t)y * t->W + x];
}

/* The map's floor pixel at (x, y), 0 where it draws none. */
static uint32_t art_px(const Art *t, int x, int y) {
	if (x < 0 || y < 0 || x >= t->W || y >= t->H) return 0;
	uint32_t c = area_src_floor_px(t->a, (size_t)y * t->W + x);
	return c >> 24 ? c : 0;
}

/* Where the art begins and how far it may go: the pixels the floor's tiles
 * must draw its own (with `seeds`, those in these colours), the others of
 * the tiles round the floor open to it, but none on floor of another kind. */
static void art_begin(const Src *src, const TileGrid *g, const uint8_t *seeds, Art *t) {
	for (int ty = 0; ty < t->a->th; ++ty)
		for (int tx = 0; tx < t->a->tw; ++tx) {
			if (!near_own_floor(src, g, tx, ty)) continue;
			uint64_t must, never, deep;
			tl_expect(g, tx, ty, tl_src_floor, src, TILE_A, &must, &never, &deep);
			for (int i = 0, pa, pb; i < 64; ++i) {
				int x = tx * 8 + i % 8, y = ty * 8 + i / 8;
				uint32_t c = art_px(t, x, y);
				if (!c) continue;
				tl_pixel_panel(g, x, y, &pa, &pb);
				if (tl_src_panel(src, pa, pb) && !own_floor(src, pa, pb)) continue;
				t->art[(size_t)y * t->W + x] = must >> i & 1 && (!seeds || seeds[colour_of(c)]) ? ART_OWN : ART_OPEN;
			}
		}
}

/* One sweep over the map, forwards (d -1: from the pixel left and above) or
 * back: each open pixel beside one of the art's in its own colour joins it,
 * and with `seeds`, in any of those colours (an outline is another colour
 * than what it lines). Whether any joined. */
static bool art_sweep(const Art *t, const uint8_t *seeds, int d) {
	bool joined = false;
	for (int k = 0; k < t->W * t->H; ++k) {
		int i = d > 0 ? t->W * t->H - 1 - k : k, x = i % t->W, y = i / t->W;
		if (t->art[i] != ART_OPEN) continue;
		uint32_t c = art_px(t, x, y);
		bool any = seeds && seeds[colour_of(c)];
		if ((art_at(t, x + d, y) == ART_OWN && (any || art_px(t, x + d, y) == c)) ||
			(art_at(t, x, y + d) == ART_OWN && (any || art_px(t, x, y + d) == c))) {
			t->art[i] = ART_OWN;
			joined = true;
		}
	}
	return joined;
}

/* The art's outline joins it: what is drawn between its art and nothing,
 * whatever its colour (a line a pixel thick, never where the floor's tiles
 * must draw, so no colour of theirs). */
static void art_outline(const Art *t) {
	for (int i = 0; i < t->W * t->H; ++i) {
		int x = i % t->W, y = i / t->W;
		if (t->art[i] != ART_OPEN) continue;
		bool own = art_at(t, x - 1, y) == ART_OWN || art_at(t, x + 1, y) == ART_OWN || art_at(t, x, y - 1) == ART_OWN || art_at(t, x, y + 1) == ART_OWN;
		bool bare = false;
		for (int k = 0; own && k < 9 && !bare; ++k) bare = !art_px(t, x + k % 3 - 1, y + k / 3 - 1);
		if (own && bare) t->art[i] = ART_LINE;
	}
	for (int i = 0; i < t->W * t->H; ++i)
		if (t->art[i] == ART_LINE) t->art[i] = ART_OWN;
}

static uint8_t *own_art(const Src *src, const TileGrid *g, const uint8_t *seeds) {
	Art t = { src->a, NULL, src->a->tw * 8, src->a->th * 8 };
	if (!(t.art = calloc((size_t)t.W, (size_t)t.H))) return NULL;
	art_begin(src, g, seeds, &t);
	/* (swept there and back until nothing joins: a band is followed in a
	 * sweep or two, and a queue of a map's pixels is four times its size) */
	for (int sweep = 0; sweep < 64 && art_sweep(&t, seeds, sweep & 1 ? 1 : -1); ++sweep) {}
	art_outline(&t);
	return t.art;
}

/* The pixels of tile (tx, ty) that are its floor's own art. */
static uint64_t art_of(const Src *src, int tx, int ty) {
	uint64_t own = 0;
	for (int i = 0; src->art && i < 64; ++i)
		if (src->art[(size_t)(ty * 8 + i / 8) * src->a->tw * 8 + tx * 8 + i % 8] == ART_OWN) own |= 1ull << i;
	return own;
}

/* What learning a map's tiles holds as it goes over them. */
typedef struct {
	const AreaSrc *a;
	const Src *src;
	const TileGrid *g;
	const uint8_t *pads;
	bool bg_in_map;
	PanelsLearn *whole;   /* its panels learned too (TILES_WHOLE), or NULL */
} Learning;

/* The platform and walkway floors round panel (A, B); true where floor of
 * another kind, or floor whose tiles are not learned, lies among them. */
static bool floors_around(const Src *src, int A, int B, unsigned *oa, unsigned *ob) {
	bool mixed = false;
	*oa = *ob = 0;
	for (int k = 0; k < 9; ++k) {
		int st = tl_src_panel(src, A + k % 3 - 1, B + k / 3 - 1);
		if (st == OTHER || tl_skipped(src, A + k % 3 - 1, B + k / 3 - 1)) mixed = true;
		if (st == TILE_A) *oa |= 1u << k;
		if (st == TILE_B) *ob |= 1u << k;
	}
	return mixed;
}

/* A tile that is no pair of the floor's, left to its panel, which may keep
 * it yet: a wall over a floor's upper edge is the panel's to draw, and so
 * is the floor's own art wherever it lies; on a map that lends its pads
 * alone, nothing but that art (the rest is its roads' and hubs'). */
static void to_panel(const Learning *L, PanelSpot at, int tx, int ty, uint64_t must, uint64_t never) {
	panels_loose(L->whole, at, tx, ty, must, never & ~tl_above_floor(L->g, tx, ty, tl_src_floor, L->src, WALL_BAND));
	panels_own(L->whole, art_of(L->src, tx, ty), L->src->styles & TILES_PADS_ALONE);
}

/* Tile (tx, ty) of the map: its pair in *t where it is one of the floor's
 * (true); and for the map's panels, the tile or what its panel lacks. */
static bool learn_tile(const Learning *L, int tx, int ty, TileCand *t) {
	const AreaSrc *a = L->a;
	int phase, A, B;
	tile_class(L->g, tx, ty, &phase, &A, &B);
	unsigned oa, ob;
	bool mixed = floors_around(L->src, A, B, &oa, &ob), off_floor = !((oa | ob) & 0x10);
	if (!(oa | ob) || (mixed && !L->whole)) return false;
	PanelSpot at = { A, B, phase, false };
	/* (a map that lends its pads alone: a tile of theirs draws their own art
	 * and no more, or is none of theirs) */
	bool only = L->src->styles & TILES_PADS_ALONE && L->src->art;
	uint64_t must, never, deep;
	tl_expect(L->g, tx, ty, tl_src_floor, L->src, TILE_A, &must, &never, &deep);
	/* (off the floor, filler the map hides under what it sets over it: no
	 * tile of the floor's, nor of its panel's) */
	bool filler = off_floor && tl_flat_tile(a, tx, ty);
	/* (floor of another kind in or beside its panel: no pair of the floor's,
	 * but its panel may lend the tile yet, in part) */
	if (mixed) {
		if (tl_src_panel(L->src, A, B) == OTHER || tl_skipped(L->src, A, B)) panels_bad(L->whole, A, B);
		else if (filler) panels_lacks(L->whole, A, B);
		else to_panel(L, at, tx, ty, must, never);
		return false;
	}
	if (filler) { panels_lacks(L->whole, A, B); return false; }
	/* The first layer alone where it draws the floor: what the originals
	 * set in front of their floors on the second (bridges, ornaments,
	 * spikes, floors overlapping others on screen) belongs to them, not to
	 * the floor. The second too where the floor needs it (the Judge Tree
	 * draws its panels' middles there), except where the map holds the
	 * background: there it is that background. */
	bool back = false;
	t->mask = tl_drawn(a, true, tx, ty, t->px);
	if (tl_misses(t->mask, must, never) > SLACK) {
		bool second = a->layers >= 2 && !L->bg_in_map;
		if (second) t->mask = tl_drawn(a, false, tx, ty, t->px);
		/* (made for something else here; a panel may keep it yet) */
		if (!second || tl_misses(t->mask, must, never) > SLACK) { to_panel(L, at, tx, ty, must, never); return false; }
		back = true;
	}
	if (only && tl_popcount64(t->mask & ~art_of(L->src, tx, ty)) > SLACK) { to_panel(L, at, tx, ty, must, never); return false; }
	size_t i = (size_t)ty * a->tw + tx;
	t->key = KEY(phase, oa, ob);
	t->e0 = a->tile[0][i];
	t->e1 = back ? a->tile[1][i] : 0;
	t->count = 1;
	t->other = 0;
	t->pad = tl_pad_look(L->src, L->pads, A, B);
	/* (nothing on the first layer: the second may draw the panel's there) */
	if (L->whole && !t->mask) to_panel(L, at, tx, ty, must, never);
	else panels_tile(L->whole, at, t->e0, t->e1, t->mask, t->px);
	return true;
}

void tiles_learn(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, uint16_t joint_hues, bool bg_in_map, TileBook *out) {
	memset(out, 0, sizeof *out);
	Src src = { a, styles, walk_styles, skip_styles, joint_hues, bg_in_map, NULL, NULL, styles & TILES_INNER_WALLS, NULL };
	if (!src_open(&src)) return;   /* (a book of nothing) */
	uint8_t *pads = tl_find_pads(&src);
	if (!pads) { src_close(&src); return; }   /* (a book of nothing) */
	src.pads = pads;
	TileGrid g = { a->tw, a->th, a->ex, a->ey, 0, 0, 0, false };
	tl_calibrate(a, &src, &g);
	out->dv = g.dv;
	out->face = g.face;
	out->hang = g.hang;
	TileCand *c = malloc(((size_t)a->tw * a->th + 1) * sizeof *c);
	if (!c) { src_close(&src); free(pads); return; }   /* (a book of nothing) */
	size_t n = 0;
	Learning L = { a, &src, &g, pads, bg_in_map, styles & TILES_WHOLE ? panels_begin() : NULL };
	/* (the original map's pixels kept as its tiles are tested: learning an
	 * area asked after each a thousand times, three quarters of the time) */
	tiles_keep_begin(&g, tl_src_floor, &src);
	/* (the floor's own art, for its panels' borders; from its pads' colours
	 * alone where it lends only them) */
	uint8_t *seeds = styles & TILES_PADS_ALONE ? pad_colours(&src, &g) : NULL;
	uint8_t *art = L.whole || seeds ? own_art(&src, &g, seeds) : NULL;
	free(seeds);
	src.art = art;
	for (int ty = 0; ty < a->th; ++ty)
		for (int tx = 0; tx < a->tw; ++tx) n += learn_tile(&L, tx, ty, &c[n]);
	tiles_keep_end();
	learn_shapes(&src, pads, out);
	keep_cands(c, n, out);
	tl_find_plain(out);
	learn_patch(a, &src, &g, pads, out);
	for (int i = 0; i < out->n; ++i) out->joins += KEY_A(out->cand[i].key) && KEY_B(out->cand[i].key);
	panels_end(L.whole, &src, pads, out);
	src_close(&src);
	free(pads);
	free(art);
}

void tiles_src_text(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, uint16_t joint_hues, bool bg_in_map, FILE *f) {
	Src src = { a, styles, walk_styles, skip_styles, joint_hues, bg_in_map, NULL, NULL, styles & TILES_INNER_WALLS, NULL };
	if (!src_open(&src)) return;
	uint8_t *pads = tl_find_pads(&src);
	if (!pads) { src_close(&src); return; }
	int H = SPAN / 2, a0 = H, a1 = -H, b0 = H, b1 = -H;
	for (int B = -H; B < H; ++B)
		for (int A = -H; A < H; ++A)
			if (tl_src_panel(&src, A, B)) {
				if (A < a0) a0 = A;
				if (A > a1) a1 = A;
				if (B < b0) b0 = B;
				if (B > b1) b1 = B;
			}
	/* rows down the grid's y (world -X), columns along its x (world +Y), as
	 * a generated layer's */
	for (int A = a1; A >= a0; --A) {
		for (int B = b0; B <= b1; ++B) {
			int st = tl_src_panel(&src, A, B);
			bool pad = pads[(B + H) * SPAN + A + H];
			/* (floor of another style by its hue bucket: 0-9, X, Y, G grey) */
			int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B, hue = st == OTHER ? tl_style_at(a, area_px(a->tw, X, Y), area_py(a->th, X, Y)) : 0;
			fputc(st == TILE_A ? (pad ? 'P' : 'a') : st == TILE_B ? (pad ? 'Q' : 'b') : st == OTHER ? "0123456789XYG"[hue] : '.', f);
		}
		fputc('\n', f);
	}
	/* (to lay the panels' grid over its picture) */
	fprintf(f, "(rows from A %d down, columns from B %d; the grid's origin %d, %d)\n", a1, b0, a->ex, a->ey);
	src_close(&src);
	free(pads);
}
