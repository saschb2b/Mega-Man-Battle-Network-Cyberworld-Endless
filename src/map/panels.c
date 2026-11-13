/* The original maps' panels whole (panels.h), learned as a map's tiles are
 * told; laying them on a generated floor is panels_lay.c's. */
#include "panels.h"

#include <stdlib.h>
#include <string.h>

#define H (SPAN / 2)
#define NO_TILE PANEL_NO_TILE
#define TILE_HASH 8192   /* (a power of two, past any map's pairs) */

/* ---- learning ---- */

typedef struct { int16_t A, B; uint8_t phase, only; int tx, ty; uint64_t must, never, own; } Loose;

/* a panel's state: whole as far as known, lending in part, or not at all */
enum { WHOLE, PART, BAD };

struct PanelsLearn {
	uint8_t state[SPAN * SPAN];
	uint16_t edge[SPAN * SPAN];   /* per panel, the slots (as told) whose tile is a border's overhang */
	uint16_t tile[SPAN * SPAN][PANEL_TILES];
	PanelTile *tiles;
	uint16_t *px;   /* (their colours, 64 each) */
	int ntiles, cap;
	uint16_t hash[TILE_HASH];
	int8_t slot[64];
	int nslots;
	Loose *loose;
	int nloose, loose_cap;
	bool failed;   /* out of memory, or more phases than a panel has tiles */
};

PanelsLearn *panels_begin(void) {
	PanelsLearn *l = malloc(sizeof *l);
	if (!l) return NULL;
	memset(l->state, WHOLE, sizeof l->state);
	memset(l->edge, 0, sizeof l->edge);
	memset(l->tile, 0xFF, sizeof l->tile);
	memset(l->hash, 0xFF, sizeof l->hash);
	memset(l->slot, -1, sizeof l->slot);
	l->tiles = NULL;
	l->px = NULL;
	l->loose = NULL;
	l->ntiles = l->cap = l->nslots = l->nloose = l->loose_cap = 0;
	l->failed = false;
	return l;
}

static bool inside(int A, int B) { return A >= -H && B >= -H && A < H && B < H; }

void panels_bad(PanelsLearn *l, int A, int B) {
	if (l && inside(A, B)) l->state[(B + H) * SPAN + A + H] = BAD;
}

void panels_lacks(PanelsLearn *l, int A, int B) {
	if (l && inside(A, B) && l->state[(B + H) * SPAN + A + H] == WHOLE) l->state[(B + H) * SPAN + A + H] = PART;
}

/* The pair's index among the panels' tiles, added where it is new. */
static uint16_t tile_of(PanelsLearn *l, uint16_t e0, uint16_t e1, uint64_t mask, const uint16_t *px) {
	uint32_t h = ((uint32_t)e0 * 0x9E3779B1u ^ (uint32_t)e1 * 0x85EBCA77u) >> 19;
	for (;; h = (h + 1) & (TILE_HASH - 1)) {
		uint16_t i = l->hash[h & (TILE_HASH - 1)];
		if (i == NO_TILE) break;
		if (l->tiles[i].e0 == e0 && l->tiles[i].e1 == e1) return i;
	}
	if (l->ntiles >= TILE_HASH / 2) { l->failed = true; return NO_TILE; }
	if (l->ntiles == l->cap) {
		int cap = l->cap ? l->cap * 2 : 256;
		PanelTile *t = realloc(l->tiles, (size_t)cap * sizeof *t);
		if (t) l->tiles = t;
		uint16_t *c = realloc(l->px, (size_t)cap * 64 * sizeof *c);
		if (c) l->px = c;
		if (!t || !c) { l->failed = true; return NO_TILE; }
		l->cap = cap;
	}
	PanelTile *t = &l->tiles[l->ntiles];
	t->e0 = e0;
	t->e1 = e1;
	t->mask = mask;
	tiles_sides(px, mask, t->side);
	if (px) memcpy(&l->px[(size_t)l->ntiles * 64], px, 64 * sizeof *px);
	else memset(&l->px[(size_t)l->ntiles * 64], 0, 64 * sizeof *px);
	l->hash[h & (TILE_HASH - 1)] = (uint16_t)l->ntiles;
	return (uint16_t)l->ntiles++;
}

void panels_tile(PanelsLearn *l, PanelSpot at, uint16_t e0, uint16_t e1, uint64_t mask, const uint16_t *px) {
	int A = at.A, B = at.B, phase = at.phase;
	if (!l || l->failed || !inside(A, B)) return;
	if (l->slot[phase] < 0) {
		if (l->nslots == PANEL_TILES) { l->failed = true; return; }
		l->slot[phase] = (int8_t)l->nslots++;
	}
	if (!mask) e0 = e1 = 0;
	l->tile[(B + H) * SPAN + A + H][l->slot[phase]] = tile_of(l, e0, e1, mask, px);
	if (at.edge && mask) l->edge[(B + H) * SPAN + A + H] |= (uint16_t)(1u << l->slot[phase]);
}

void panels_loose(PanelsLearn *l, PanelSpot at, int tx, int ty, uint64_t must, uint64_t never) {
	int A = at.A, B = at.B, phase = at.phase;
	if (!l || l->failed || !inside(A, B)) return;
	if (l->nloose == l->loose_cap) {
		int cap = l->loose_cap ? l->loose_cap * 2 : 256;
		Loose *t = realloc(l->loose, (size_t)cap * sizeof *t);
		if (!t) { l->failed = true; return; }
		l->loose = t;
		l->loose_cap = cap;
	}
	l->loose[l->nloose++] = (Loose){ (int16_t)A, (int16_t)B, (uint8_t)phase, 0, tx, ty, must, never, 0 };
}

void panels_own(PanelsLearn *l, uint64_t own, bool only) {
	if (!l || l->failed || !l->nloose) return;
	l->loose[l->nloose - 1].own = own;
	l->loose[l->nloose - 1].only = only;
}

/* Whether the pixels of `mask` are all in colours `seen`. */
static bool colours_seen(const uint8_t *seen, uint64_t mask, const uint16_t px[64]) {
	for (int i = 0; i < 64; ++i)
		if (mask >> i & 1 && !seen[px[i] & 0x7FFF]) return false;
	return true;
}

/* Whether a tile fits where it lies: drawing what it must, and beyond what
 * it may only the floor's own art (*edge: it does) or a stray pixel, in the
 * floor's own colours; with t->only, nothing but the floor's own art. */
static bool tile_fits(const Loose *t, const uint8_t *seen, uint64_t mask, const uint16_t px[64], bool *edge) {
	uint64_t over = mask & (t->only ? ~0ull : t->never);
	*edge = tl_popcount64(over) > SLACK;
	return tl_popcount64(t->must & ~mask) <= SLACK && tl_popcount64(over & ~t->own) <= SLACK && (t->only || colours_seen(seen, mask, px));
}

/* The tiles the pixel test turned down, those that draw nothing on the
 * first layer and those off a pad's floor (TILES_PADS_ALONE), kept where
 * they fit: on the first layer alone, or on both where the map's second is
 * no background. */
static void keep_loose(PanelsLearn *l, const Src *src, const TileBook *out) {
	uint8_t *seen = calloc(TILE_COLOURS, 1);
	if (!seen) { l->failed = true; return; }
	tiles_colours(out, seen);
	const AreaSrc *a = src->a;
	for (int i = 0; i < l->nloose; ++i) {
		const Loose *t = &l->loose[i];
		size_t at = (size_t)t->ty * a->tw + t->tx;
		uint16_t px[64];
		uint64_t mask = tl_drawn(a, true, t->tx, t->ty, px);
		bool edge, ok = tile_fits(t, seen, mask, px, &edge);
		uint16_t e1 = 0;
		/* (and nothing on the first where the second draws: an edge's rim set
		 * there is the panel's, not a tile of nothing) */
		if ((!ok || !mask) && a->layers > 1 && !src->bg_in_map) {
			uint16_t both[64];
			uint64_t drawn = tl_drawn(a, false, t->tx, t->ty, both);
			if (!ok || drawn) {
				mask = drawn;
				memcpy(px, both, sizeof both);
				ok = tile_fits(t, seen, mask, px, &edge);
				e1 = a->tile[1][at];
			}
		}
		if (ok) panels_tile(l, (PanelSpot){ t->A, t->B, t->phase, edge }, a->tile[0][at], e1, mask, px);
		else panels_lacks(l, t->A, t->B);
	}
	free(seen);
}

static int cmp_shape(const void *x, const void *y) {
	const TilePanel *a = x, *b = y;
	if (a->shape != b->shape) return a->shape < b->shape ? -1 : 1;
	if (a->B != b->B) return a->B - b->B;
	return a->A - b->A;
}

static const TilePanel *sort_base;

static int cmp_place(const void *x, const void *y) {
	const TilePanel *a = &sort_base[*(const uint16_t *)x], *b = &sort_base[*(const uint16_t *)y];
	if (a->B != b->B) return a->B - b->B;
	return a->A - b->A;
}

/* Panel (A, B)'s neighbourhood in the source; in *no, the panels round it
 * that are floor of another kind. */
static uint32_t src_shape(const Src *src, const uint8_t *pads, int A, int B, uint16_t *no) {
	unsigned oa = 0, ob = 0, op = 0;
	*no = 0;
	for (int k = 0; k < 9; ++k) {
		bool skipped = tl_skipped(src, A + k % 3 - 1, B + k / 3 - 1);
		int st = skipped ? TILE_VOID : tl_src_panel(src, A + k % 3 - 1, B + k / 3 - 1);
		if (skipped || st == OTHER) *no |= (uint16_t)(1u << k);
		if (st == TILE_A) oa |= 1u << k;
		if (st == TILE_B) ob |= 1u << k;
		if ((st == TILE_A || st == TILE_B) && tl_pad_look(src, pads, A + k % 3 - 1, B + k / 3 - 1)) op |= 1u << k;
	}
	return PANEL_SHAPE(oa, ob, op);
}

/* Per shape, the panels whose look (their sixteen tiles) is a third as
 * common as its usual one, or less, marked rare. */
#define RARE 3
static void mark_rare(TilePanels *p) {
	for (int i = 0, j; i < p->n; i = j) {
		int most = 0;
		for (j = i; j < p->n && p->panel[j].shape == p->panel[i].shape; ++j) {
			int same = 0;
			for (int k = i; k < p->n && p->panel[k].shape == p->panel[i].shape; ++k)
				same += !memcmp(p->panel[k].tile, p->panel[j].tile, sizeof p->panel[j].tile);
			p->panel[j].rare = (uint16_t)(same > 0xFFFF ? 0xFFFF : same);
			if (same > most) most = same;
		}
		for (int k = i; k < j; ++k) p->panel[k].rare = p->panel[k].rare * RARE <= most && most > 1;
	}
}

/* The source's panel i of the span as the book's next (whole ones first:
 * p->n counts them, p->npart those after). */
static void keep_panel(const PanelsLearn *l, const Src *src, const uint8_t *pads, int i, TilePanel *t) {
	t->A = (int16_t)(i % SPAN - H);
	t->B = (int16_t)(i / SPAN - H);
	t->shape = src_shape(src, pads, t->A, t->B, &t->no);
	t->rare = t->has = 0;
	/* (its slots by their phases in order: the same in every book of a lattice) */
	t->edge = 0;
	for (int phase = 0, k = 0; phase < 64; ++phase)
		if (l->slot[phase] >= 0) {
			t->edge |= (uint16_t)((l->edge[i] >> l->slot[phase] & 1) << k);
			t->tile[k++] = l->tile[i][l->slot[phase]];
		}
	for (int k = 0; k < PANEL_TILES; ++k) t->has |= (uint16_t)((t->tile[k] != NO_TILE) << k);
}

void panels_end(PanelsLearn *l, const Src *src, const uint8_t *pads, TileBook *out) {
	TilePanels *p = &out->panels;
	memset(p, 0, sizeof *p);
	memset(p->slot, -1, sizeof p->slot);
	if (!l) return;
	if (!l->failed) keep_loose(l, src, out);
	int n = 0, npart = 0;
	for (int i = 0; i < SPAN * SPAN && !l->failed && l->nslots == PANEL_TILES; ++i) {
		int have = 0;
		for (int k = 0; k < PANEL_TILES; ++k) have += l->tile[i][k] != NO_TILE;
		uint16_t no = 0;
		if (!have) l->state[i] = BAD;
		if (l->state[i] == WHOLE) src_shape(src, pads, i % SPAN - H, i / SPAN - H, &no);
		if (l->state[i] == WHOLE && (have < PANEL_TILES || no)) l->state[i] = PART;
		n += l->state[i] == WHOLE;
		npart += l->state[i] == PART;
	}
	int all = n + npart;
	if (n && all < (int)NO_TILE && (p->panel = malloc((size_t)all * sizeof *p->panel)) && (p->at = malloc((size_t)all * sizeof *p->at))) {
		for (int i = 0; i < SPAN * SPAN; ++i)
			if (l->state[i] == WHOLE) keep_panel(l, src, pads, i, &p->panel[p->n++]);
		qsort(p->panel, (size_t)p->n, sizeof *p->panel, cmp_shape);
		mark_rare(p);
		for (int i = 0; i < SPAN * SPAN; ++i)
			if (l->state[i] == PART) keep_panel(l, src, pads, i, &p->panel[p->n + p->npart++]);
		for (int i = 0; i < all; ++i) p->at[i] = (uint16_t)i;
		sort_base = p->panel;
		qsort(p->at, (size_t)all, sizeof *p->at, cmp_place);
		PanelTile *kept = realloc(l->tiles, ((size_t)l->ntiles + 1) * sizeof *kept);
		p->tile = kept ? kept : l->tiles;
		p->ntile = l->ntiles;
		p->px = l->px;
		l->tiles = NULL;
		l->px = NULL;
		for (int phase = 0, k = 0; phase < 64; ++phase)
			if (l->slot[phase] >= 0) p->slot[phase] = (int8_t)k++;
	} else {
		free(p->panel);
		free(p->at);
		p->panel = NULL;
		p->at = NULL;
	}
	free(l->tiles);
	free(l->px);
	free(l->loose);
	free(l);
}

void panels_settle(TilePanels *p) {
	free(p->px);
	p->px = NULL;
}

/* The panels marked in `drop` left out. */
static void leave_out(TilePanels *p, const uint8_t *drop) {
	int n = 0, all = p->n + p->npart, whole = 0;
	for (int i = 0; i < all; ++i) {
		if (drop[i]) continue;
		whole += i < p->n;
		p->panel[n++] = p->panel[i];
	}
	p->n = whole;
	p->npart = n - whole;
	for (int i = 0; i < n; ++i) p->at[i] = (uint16_t)i;
	sort_base = p->panel;
	qsort(p->at, (size_t)n, sizeof *p->at, cmp_place);
}

void panels_other_colours(TilePanels *p, const uint8_t seen[TILE_COLOURS]) {
	int all = p->n + p->npart;
	uint8_t *other = p->px && all ? calloc((size_t)p->ntile, 1) : NULL, *drop = other ? calloc((size_t)all, 1) : NULL;
	if (drop) {
		for (int i = 0; i < p->ntile; ++i) other[i] = !colours_seen(seen, p->tile[i].mask, &p->px[(size_t)i * 64]);
		for (int i = 0; i < all; ++i)
			for (int k = 0; k < PANEL_TILES; ++k) drop[i] |= p->panel[i].tile[k] != NO_TILE && other[p->panel[i].tile[k]];
		leave_out(p, drop);
	} else p->n = p->npart = 0;   /* (not known: none lent) */
	free(other);
	free(drop);
	panels_settle(p);
}

void panels_drop_pads(TilePanels *p) {
	int all = p->n + p->npart;
	uint8_t *drop = all ? calloc((size_t)all, 1) : NULL;
	if (!drop) { p->n = p->npart = 0; return; }
	for (int i = 0; i < all; ++i) drop[i] = (p->panel[i].shape & 0x1FF) != 0;
	leave_out(p, drop);
	free(drop);
}
