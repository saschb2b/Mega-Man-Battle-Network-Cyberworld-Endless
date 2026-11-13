/* The original maps' panels whole (panels.h): learned as a map's tiles are
 * told, and laid on a generated floor before its tiles are picked one by
 * one. */
#include "panels.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define H (SPAN / 2)
#define NO_TILE 0xFFFFu
#define TILE_HASH 8192   /* (a power of two, past any map's pairs) */

/* ---- learning ---- */

typedef struct { int16_t A, B; uint8_t phase; int tx, ty; uint64_t must, never; } Loose;

struct PanelsLearn {
	uint8_t bad[SPAN * SPAN];
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
	memset(l->bad, 0, sizeof l->bad);
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
	if (l && inside(A, B)) l->bad[(B + H) * SPAN + A + H] = 1;
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
	l->loose[l->nloose++] = (Loose){ (int16_t)A, (int16_t)B, (uint8_t)phase, tx, ty, must, never };
}

/* Whether `mask` draws what a loose tile must, and little beyond. */
static bool loose_fits(const Loose *t, uint64_t mask) {
	return tl_popcount64(t->must & ~mask) <= SLACK && tl_popcount64(mask & t->never) <= SLACK;
}

/* Whether the pixels of `mask` are all in colours `seen`. */
static bool colours_seen(const uint8_t *seen, uint64_t mask, const uint16_t px[64]) {
	for (int i = 0; i < 64; ++i)
		if (mask >> i & 1 && !seen[px[i] & 0x7FFF]) return false;
	return true;
}

/* The tiles the pixel test turned down, kept where they draw what they
 * must in the floor's own colours: on the first layer alone, or on both
 * where the map's second is no background. */
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
		bool ok = loose_fits(t, mask) && colours_seen(seen, mask, px);
		uint16_t e1 = 0;
		if (!ok && a->layers > 1 && !src->bg_in_map) {
			mask = tl_drawn(a, false, t->tx, t->ty, px);
			ok = loose_fits(t, mask) && colours_seen(seen, mask, px);
			e1 = a->tile[1][at];
		}
		if (ok) panels_tile(l, (PanelSpot){ t->A, t->B, t->phase }, a->tile[0][at], e1, mask, px);
		else panels_bad(l, t->A, t->B);
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

/* Panel (A, B)'s neighbourhood in the source. */
static uint32_t src_shape(const Src *src, const uint8_t *pads, int A, int B) {
	unsigned oa = 0, ob = 0, op = 0;
	for (int k = 0; k < 9; ++k) {
		int st = tl_skipped(src, A + k % 3 - 1, B + k / 3 - 1) ? TILE_VOID : tl_src_panel(src, A + k % 3 - 1, B + k / 3 - 1);
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

void panels_end(PanelsLearn *l, const Src *src, const uint8_t *pads, TileBook *out) {
	TilePanels *p = &out->panels;
	memset(p, 0, sizeof *p);
	memset(p->slot, -1, sizeof p->slot);
	if (!l) return;
	if (!l->failed) keep_loose(l, src, out);
	int n = 0;
	for (int i = 0; i < SPAN * SPAN && !l->failed && l->nslots == PANEL_TILES; ++i) {
		if (l->bad[i]) continue;
		bool whole = true;
		for (int k = 0; k < PANEL_TILES; ++k) whole &= l->tile[i][k] != NO_TILE;
		n += whole;
		if (!whole) l->bad[i] = 1;
	}
	if (n && n < (int)NO_TILE && (p->panel = malloc((size_t)n * sizeof *p->panel)) && (p->at = malloc((size_t)n * sizeof *p->at))) {
		for (int i = 0; i < SPAN * SPAN; ++i) {
			if (l->bad[i] || l->tile[i][0] == NO_TILE) continue;
			TilePanel *t = &p->panel[p->n++];
			t->A = (int16_t)(i % SPAN - H);
			t->B = (int16_t)(i / SPAN - H);
			t->shape = src_shape(src, pads, t->A, t->B);
			memcpy(t->tile, l->tile[i], sizeof t->tile);
		}
		qsort(p->panel, (size_t)p->n, sizeof *p->panel, cmp_shape);
		mark_rare(p);
		for (int i = 0; i < p->n; ++i) p->at[i] = (uint16_t)i;
		sort_base = p->panel;
		qsort(p->at, (size_t)p->n, sizeof *p->at, cmp_place);
		PanelTile *kept = realloc(l->tiles, ((size_t)l->ntiles + 1) * sizeof *kept);
		p->tile = kept ? kept : l->tiles;
		p->ntile = l->ntiles;
		p->px = l->px;
		l->tiles = NULL;
		l->px = NULL;
		memcpy(p->slot, l->slot, sizeof p->slot);
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
	int n = 0;
	for (int i = 0; i < p->n; ++i)
		if (!drop[i]) p->panel[n++] = p->panel[i];
	p->n = n;
	for (int i = 0; i < n; ++i) p->at[i] = (uint16_t)i;
	sort_base = p->panel;
	qsort(p->at, (size_t)n, sizeof *p->at, cmp_place);
}

void panels_other_colours(TilePanels *p, const uint8_t seen[TILE_COLOURS]) {
	uint8_t *other = p->px && p->n ? calloc((size_t)p->ntile, 1) : NULL, *drop = other ? calloc((size_t)p->n, 1) : NULL;
	if (drop) {
		for (int i = 0; i < p->ntile; ++i) other[i] = !colours_seen(seen, p->tile[i].mask, &p->px[(size_t)i * 64]);
		for (int i = 0; i < p->n; ++i)
			for (int k = 0; k < PANEL_TILES; ++k) drop[i] |= other[p->panel[i].tile[k]];
		leave_out(p, drop);
	} else p->n = 0;   /* (not known: none lent) */
	free(other);
	free(drop);
	panels_settle(p);
}

void panels_drop_pads(TilePanels *p) {
	uint8_t *drop = p->n ? calloc((size_t)p->n, 1) : NULL;
	if (!drop) { p->n = 0; return; }
	for (int i = 0; i < p->n; ++i) drop[i] = (p->panel[i].shape & 0x1FF) != 0;
	leave_out(p, drop);
	free(drop);
}

/* ---- laying ---- */

/* The book's panel at (A, B), -1 none. */
static int panel_at(const TilePanels *p, int A, int B) {
	int lo = 0, hi = p->n;
	while (lo < hi) {
		int mid = (lo + hi) / 2;
		const TilePanel *t = &p->panel[p->at[mid]];
		if (t->B < B || (t->B == B && t->A < A)) lo = mid + 1; else hi = mid;
	}
	return lo < p->n && p->panel[p->at[lo]].A == A && p->panel[p->at[lo]].B == B ? p->at[lo] : -1;
}

/* Its first panel of `shape` (they lie together), p->n none. */
static int first_of(const TilePanels *p, uint32_t shape) {
	int lo = 0, hi = p->n;
	while (lo < hi) {
		int mid = (lo + hi) / 2;
		if (p->panel[mid].shape < shape) lo = mid + 1; else hi = mid;
	}
	return lo;
}

/* The floor's panels a map shows, w x h from (A0, B0), each with its shape
 * (0: no floor in or around it) and the panel laid on it (book, index; book
 * -1 none). */
typedef struct {
	int A0, B0, w, h;
	uint32_t *shape;
	int8_t *book;
	uint16_t *index;
} Laid;

static uint32_t shape_at(const Laid *d, int A, int B) {
	return A < d->A0 || B < d->B0 || A >= d->A0 + d->w || B >= d->B0 + d->h ? 0 : d->shape[(size_t)(B - d->B0) * d->w + A - d->A0];
}

static const int side4[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };

/* Whether a panel draws on both layers in some tile. */
static bool two_layers(const TilePanels *p, const TilePanel *t) {
	for (int k = 0; k < PANEL_TILES; ++k)
		if ((p->tile[t->tile[k]].e0 & 0x3FF) && (p->tile[t->tile[k]].e1 & 0x3FF)) return true;
	return false;
}

/* How well panel c of book k suits the floor's panel (A, B): most where it
 * lies beside the panels laid beside it as its map lays them (16 a side),
 * or beside one drawn as theirs (12); then by the panels around it having
 * the floor's neighbourhoods too (1 each). */
#define BESIDE 16
#define ALIKE 12
static int suits(const TileBook *books, const Laid *d, int k, int c, int A, int B) {
	const TilePanels *p = &books[k].panels;
	const TilePanel *t = &p->panel[c];
	int score = 0;
	for (int s = 0; s < 4; ++s) {
		int nA = A + side4[s][0], nB = B + side4[s][1];
		if (nA < d->A0 || nB < d->B0 || nA >= d->A0 + d->w || nB >= d->B0 + d->h) continue;
		size_t ni = (size_t)(nB - d->B0) * d->w + nA - d->A0;
		if (d->book[ni] != k) continue;
		int beside = panel_at(p, t->A + side4[s][0], t->B + side4[s][1]);
		if (beside < 0) continue;
		if (beside == d->index[ni]) score += BESIDE;
		else if (!memcmp(p->panel[beside].tile, p->panel[d->index[ni]].tile, sizeof t->tile)) score += ALIKE;
	}
	for (int j = 0; j < 9; ++j) {
		if (j == 4) continue;
		int beside = panel_at(p, t->A + j % 3 - 1, t->B + j / 3 - 1);
		uint32_t want = shape_at(d, A + j % 3 - 1, B + j / 3 - 1);
		score += beside >= 0 ? p->panel[beside].shape == want : !want;
	}
	return score;
}

/* The panels a map shows and their shapes; false without memory. */
static bool laid_open(Laid *d, const TileGrid *g, TileFloor floor, const void *ctx) {
	int phase, A, B, A1 = INT_MIN, B1 = INT_MIN;
	d->A0 = d->B0 = INT_MAX;
	for (int k = 0; k < 4; ++k) {   /* the map's corners reach furthest */
		tile_class(g, k & 1 ? g->tw - 1 : 0, k & 2 ? g->th - 1 : 0, &phase, &A, &B);
		if (A < d->A0) d->A0 = A;
		if (B < d->B0) d->B0 = B;
		if (A > A1) A1 = A;
		if (B > B1) B1 = B;
	}
	d->w = A1 + 1 - d->A0;
	d->h = B1 + 1 - d->B0;
	size_t n = (size_t)d->w * d->h;
	d->shape = malloc(n * sizeof *d->shape);
	d->book = malloc(n);
	d->index = malloc(n * sizeof *d->index);
	if (!d->shape || !d->book || !d->index) return false;
	memset(d->book, -1, n);
	for (size_t i = 0; i < n; ++i) {
		unsigned oa = 0, ob = 0, op = 0;
		for (int k = 0; k < 9; ++k) {
			int m = floor(d->A0 + (int)(i % d->w) + k % 3 - 1, d->B0 + (int)(i / d->w) + k / 3 - 1, ctx);
			if (TILE_MATERIAL(m) == TILE_A) oa |= 1u << k;
			if (TILE_MATERIAL(m) == TILE_B) ob |= 1u << k;
			if (TILE_MATERIAL(m) && m & TILE_PAD) op |= 1u << k;
		}
		d->shape[i] = PANEL_SHAPE(oa, ob, op);
	}
	return true;
}

static void laid_close(Laid *d) {
	free(d->shape);
	free(d->book);
	free(d->index);
}

/* The books whose panels this map can take: drawn as it draws its floor. */
static bool lends(const TileBook *b, const TileGrid *g) {
	return b->panels.n && b->dv == g->dv && b->face == g->face && b->hang == g->hang;
}

/* The panel that suits the floor's panel i best, of the books' with its
 * shape (the first book's first, then the first found); false none. */
static bool lay_one(const TileBook *books, int nbooks, const TileGrid *g, Laid *d, size_t i) {
	int A = d->A0 + (int)(i % d->w), B = d->B0 + (int)(i / d->w), best = -1;
	for (int k = 0; k < nbooks; ++k) {
		const TilePanels *p = &books[k].panels;
		if (!lends(&books[k], g)) continue;
		for (int c = first_of(p, d->shape[i]); c < p->n && p->panel[c].shape == d->shape[i]; ++c) {
			if (p->panel[c].rare || (g->single && two_layers(p, &p->panel[c]))) continue;
			int score = suits(books, d, k, c, A, B);
			if (score <= best) continue;
			best = score;
			d->book[i] = (int8_t)k;
			d->index[i] = (uint16_t)c;
		}
	}
	return best >= 0;
}

/* The tile a laid panel sets at (tx, ty), NULL none; its panel in *at. */
static const PanelTile *laid_tile(const TileBook *books, const TileGrid *g, const Laid *d, int tx, int ty, size_t *at) {
	if (tx < 0 || ty < 0 || tx >= g->tw || ty >= g->th) return NULL;
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	if (A < d->A0 || B < d->B0 || A >= d->A0 + d->w || B >= d->B0 + d->h) return NULL;
	*at = (size_t)(B - d->B0) * d->w + A - d->A0;
	if (d->book[*at] < 0) return NULL;
	const TilePanels *p = &books[d->book[*at]].panels;
	return p->slot[phase] < 0 ? NULL : &p->tile[p->panel[d->index[*at]].tile[p->slot[phase]]];
}

/* Two panels laid side by side that no map lays so must still meet as the
 * maps' tiles do: where a strip across their tiles' edge is one no map
 * shows, the later of the two is taken up again (its tiles are picked one
 * by one, to meet what stays). */
static void take_up(const TileBook *books, const TileSeams *seams, const TileGrid *g, Laid *d) {
	for (bool changed = true; changed;) {
		changed = false;
		for (int ty = 0; ty < g->th; ++ty)
			for (int tx = 0; tx < g->tw; ++tx) {
				size_t i = 0, j = 0;
				const PanelTile *t = laid_tile(books, g, d, tx, ty, &i);
				if (!t || !t->mask) continue;
				for (int k = 2; k < 4; ++k) {
					const PanelTile *u = laid_tile(books, g, d, tx + (k == 2), ty + (k == 3), &j);
					if (!u || !u->mask || j == i || seams_strip_seen(seams, t->side[k], u->side[k ^ 2], k & 1)) continue;
					d->book[i > j ? i : j] = -1;
					changed = true;
					if (i > j) break;
				}
			}
	}
}

int panels_lay(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g, TileFloor floor, const void *ctx, TilePick *pick) {
	bool any = false;
	for (int k = 0; k < nbooks && k < 127; ++k) any |= lends(&books[k], g);
	Laid d = { 0, 0, 0, 0, NULL, NULL, NULL };
	if (!any || !laid_open(&d, g, floor, ctx)) { laid_close(&d); return 0; }
	size_t n = (size_t)d.w * d.h;
	/* (the floor's plain middle is the first book's to lay by its period
	 * or to scatter: tiles_pick.c) */
	bool middles = nbooks && (books[0].patch || books[0].vary);
	for (size_t i = 0; i < n; ++i) {
		if (!(d.shape[i] >> 9) || (middles && d.shape[i] == PANEL_SHAPE(0x1FF, 0, 0))) continue;
		lay_one(books, nbooks > 127 ? 127 : nbooks, g, &d, i);
	}
	if (seams) take_up(books, seams, g, &d);
	int tiles = 0;
	for (int ty = 0; ty < g->th; ++ty)
		for (int tx = 0; tx < g->tw; ++tx) {
			size_t i = 0;
			const PanelTile *t = laid_tile(books, g, &d, tx, ty, &i);
			if (!t) continue;
			size_t at = (size_t)ty * g->tw + tx;
			pick[at] = (TilePick){ t->e0, t->e1, t->mask ? t->e0 : SEAM_VOID, t->mask, t->side };
			++tiles;
		}
	laid_close(&d);
	return tiles;
}

void panels_mark(const TileGrid *g, int tx, int ty, uint8_t *mark) {
	int phase, A, B, a, b;
	tile_class(g, tx, ty, &phase, &A, &B);
	/* (a panel's tiles lie within four tiles of each other) */
	for (int y = ty - 4; y <= ty + 4; ++y)
		for (int x = tx - 8; x <= tx + 8; ++x) {
			if (x < 0 || y < 0 || x >= g->tw || y >= g->th) continue;
			tile_class(g, x, y, &phase, &a, &b);
			if (a == A && b == B) mark[(size_t)y * g->tw + x] = 1;
		}
}
