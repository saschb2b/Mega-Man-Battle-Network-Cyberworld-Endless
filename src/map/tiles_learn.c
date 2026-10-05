/* An area's tiles learned from its original maps (tiles.c): the shapes its
 * floor's tiles take, its patches and their period, its variety. */
#include "tiles_learn.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiles_parts.h"

static void learn_variety(TileBook *out);

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
	out->shapes = malloc(SPAN * SPAN * sizeof *out->shapes);
	for (int B = -SPAN / 2 + 1; out->shapes && B < SPAN / 2 - 1; ++B)
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

void tiles_learn(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, uint16_t joint_hues, bool bg_in_map, TileBook *out) {
	memset(out, 0, sizeof *out);
	Src src = { a, styles, walk_styles, skip_styles, joint_hues, bg_in_map, NULL, NULL, styles & TILES_INNER_WALLS, NULL };
	if (!src_open(&src)) return;   /* (a book of nothing) */
	uint8_t *pads = tl_find_pads(&src);
	src.pads = pads;
	TileGrid g = { a->tw, a->th, a->ex, a->ey, 0, 0, 0, false };
	tl_calibrate(a, &src, &g);
	out->dv = g.dv;
	out->face = g.face;
	out->hang = g.hang;
	TileCand *c = malloc(((size_t)a->tw * a->th + 1) * sizeof *c);
	if (!c) { src_close(&src); free(pads); return; }   /* (a book of nothing) */
	size_t n = 0;
	/* (the original map's pixels kept as its tiles are tested: learning an
	 * area asked after each a thousand times, three quarters of the time) */
	tiles_keep_begin(&g, tl_src_floor, &src);
	for (int ty = 0; ty < a->th; ++ty)
		for (int tx = 0; tx < a->tw; ++tx) {
			int phase, A, B;
			tile_class(&g, tx, ty, &phase, &A, &B);
			unsigned oa = 0, ob = 0;
			bool mixed = false;
			for (int k = 0; k < 9; ++k) {
				int st = tl_src_panel(&src, A + k % 3 - 1, B + k / 3 - 1);
				if (st == OTHER || tl_skipped(&src, A + k % 3 - 1, B + k / 3 - 1)) mixed = true;
				if (st == TILE_A) oa |= 1u << k;
				if (st == TILE_B) ob |= 1u << k;
			}
			if (mixed || !(oa | ob)) continue;
			/* off the floor, filler would show as a hole */
			if (!((oa | ob) & 0x10) && tl_flat_tile(a, tx, ty)) continue;
			TileCand *t = &c[n];
			uint64_t must, never, deep;
			tl_expect(&g, tx, ty, tl_src_floor, &src, TILE_A, &must, &never, &deep);
			/* The first layer alone where it draws the floor: what the
			 * originals set in front of their floors on the second (bridges,
			 * ornaments, spikes, floors overlapping others on screen) belongs
			 * to them, not to the floor. The second too where the floor needs
			 * it (the Judge Tree draws its panels' middles there), except where
			 * the map holds the background: there it is that background. */
			bool back = false;
			t->mask = tl_drawn(a, true, tx, ty, t->px);
			if (tl_misses(t->mask, must, never) > SLACK) {
				if (a->layers < 2 || bg_in_map) continue;
				t->mask = tl_drawn(a, false, tx, ty, t->px);
				if (tl_misses(t->mask, must, never) > SLACK) continue;   /* made for something else here */
				back = true;
			}
			size_t i = (size_t)ty * a->tw + tx;
			t->key = KEY(phase, oa, ob);
			t->e0 = a->tile[0][i];
			t->e1 = back ? a->tile[1][i] : 0;
			t->count = 1;
			t->other = 0;
			t->pad = tl_pad_look(&src, pads, A, B);
			++n;
		}
	tiles_keep_end();
	learn_shapes(&src, pads, out);
	keep_cands(c, n, out);
	tl_find_plain(out);
	learn_patch(a, &src, &g, pads, out);
	for (int i = 0; i < out->n; ++i) out->joins += KEY_A(out->cand[i].key) && KEY_B(out->cand[i].key);
	src_close(&src);
	free(pads);
}

void tiles_src_text(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, uint16_t joint_hues, bool bg_in_map, FILE *f) {
	Src src = { a, styles, walk_styles, skip_styles, joint_hues, bg_in_map, NULL, NULL, styles & TILES_INNER_WALLS, NULL };
	if (!src_open(&src)) return;
	uint8_t *pads = tl_find_pads(&src);
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
	src_close(&src);
	free(pads);
}
