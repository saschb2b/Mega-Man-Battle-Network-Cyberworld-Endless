/* The original maps' panels laid on a generated floor (panels.h) before
 * its tiles are picked one by one: whole where a map has the floor's panel,
 * else piece by piece, each tile from a panel like the floor's as far as
 * the tile shows. */
#include "panels.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define NO_TILE PANEL_NO_TILE
#define WALL_BAND 8   /* pixels over a floor's upper edge a wall on it may stand (tiles_learn.c) */

/* The book's panel at (A, B), -1 none. */
static int panel_at(const TilePanels *p, int A, int B) {
	int lo = 0, hi = p->n + p->npart;
	while (lo < hi) {
		int mid = (lo + hi) / 2;
		const TilePanel *t = &p->panel[p->at[mid]];
		if (t->B < B || (t->B == B && t->A < A)) lo = mid + 1; else hi = mid;
	}
	return lo < p->n + p->npart && p->panel[p->at[lo]].A == A && p->panel[p->at[lo]].B == B ? p->at[lo] : -1;
}

/* Its first whole panel of `shape` (they lie together), p->n none. */
static int first_of(const TilePanels *p, uint32_t shape) {
	int lo = 0, hi = p->n;
	while (lo < hi) {
		int mid = (lo + hi) / 2;
		if (p->panel[mid].shape < shape) lo = mid + 1; else hi = mid;
	}
	return lo;
}

/* A panel laid piece by piece: per slot, the panel its tile is from. */
typedef struct { int8_t book[PANEL_TILES]; uint16_t index[PANEL_TILES]; } Pieced;

/* What a tile shows, by its slot: the panels (bit k, as a shape's) it lies
 * on or beside, those within a tile of it (off the floor, where a map's art
 * may reach past its panel's edge: a walkway's end cap, a rounded turn),
 * those whose side faces could reach it from above, and those a wall on
 * whose upper edge could. */
#define REACH 8
typedef struct { uint16_t near, wide, up, down; } View;

/* The floor's panels a map shows, w x h from (A0, B0), each with its shape
 * (0: no floor in or around it) and the panel laid on it (book, index; book
 * -1 none), or its pieces (`pieced`, -1 none; book and index then its first
 * piece's). */
typedef struct {
	const TileBook *books;
	int A0, B0, w, h;
	uint32_t *shape;
	int8_t *book;
	uint16_t *index;
	int16_t *pieced;
	Pieced *pieces;
	int npieces, cap;
	View view[PANEL_TILES];   /* what each slot's tile shows */
	int8_t slot[64];          /* a phase's slot on this map, -1 none */
	bool *lend;               /* per book: whether its panels are laid here */
} Laid;

static uint32_t shape_at(const Laid *d, int A, int B) {
	return A < d->A0 || B < d->B0 || A >= d->A0 + d->w || B >= d->B0 + d->h ? 0 : d->shape[(size_t)(B - d->B0) * d->w + A - d->A0];
}

/* The panels round one: its sides, then its corners. */
static const int round8[8][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 } };

/* Whether a panel draws on both layers in some tile. */
static bool two_layers(const TilePanels *p, const TilePanel *t) {
	for (int k = 0; k < PANEL_TILES; ++k)
		if (t->tile[k] != NO_TILE && (p->tile[t->tile[k]].e0 & 0x3FF) && (p->tile[t->tile[k]].e1 & 0x3FF)) return true;
	return false;
}

/* How well panel c of book k suits the floor's panel (A, B): most where it
 * lies beside the panels laid beside it as its map lays them (16 a side),
 * or beside one drawn as theirs (12), and with `corners` half that corner
 * to corner; then, in *alike, by the panels around it having the floor's
 * neighbourhoods too (1 each). */
#define BESIDE 16
#define ALIKE 12
static int suits(const Laid *d, int k, int c, int A, int B, bool corners, int *alike) {
	const TilePanels *p = &d->books[k].panels;
	const TilePanel *t = &p->panel[c];
	int score = 0;
	for (int s = 0; s < (corners ? 8 : 4); ++s) {
		int nA = A + round8[s][0], nB = B + round8[s][1];
		if (nA < d->A0 || nB < d->B0 || nA >= d->A0 + d->w || nB >= d->B0 + d->h) continue;
		size_t ni = (size_t)(nB - d->B0) * d->w + nA - d->A0;
		if (d->book[ni] != k) continue;
		int beside = panel_at(p, t->A + round8[s][0], t->B + round8[s][1]);
		if (beside < 0) continue;
		if (beside == d->index[ni]) score += s < 4 ? BESIDE : BESIDE / 2;
		else if (!memcmp(p->panel[beside].tile, p->panel[d->index[ni]].tile, sizeof t->tile)) score += s < 4 ? ALIKE : ALIKE / 2;
	}
	*alike = 0;
	for (int j = 0; j < 9; ++j) {
		if (j == 4) continue;
		int beside = panel_at(p, t->A + j % 3 - 1, t->B + j / 3 - 1);
		uint32_t want = shape_at(d, A + j % 3 - 1, B + j / 3 - 1);
		*alike += beside >= 0 ? p->panel[beside].shape == want : !want;
	}
	return score;
}

/* The books whose panels this map can take: their floor drawn where it
 * draws its own (their faces may be another height: BN4's Town Area's
 * squares stand 17 pixels tall, its walkways 10). */
static bool lends(const TileBook *b, const TileGrid *g) {
	return b->panels.n && b->dv == g->dv;
}

/* What a panel's tiles show on this map, by their slots (a phase's place
 * among the map's phases in order, as a book numbers its own); and the
 * books that lend it panels: drawn as it draws its floor, on its lattice. */
static void views(const TileBook *books, int nbooks, const TileGrid *g, Laid *d) {
	View by_phase[64];
	bool seen[64] = { false };
	/* (faces and what hangs under them, as deep as any lending book's) */
	int reach = g->face + g->hang;
	for (int k = 0; k < nbooks; ++k)
		if (lends(&books[k], g) && books[k].face + books[k].hang > reach) reach = books[k].face + books[k].hang;
	/* (every phase lies within a panel's width and height of the middle) */
	for (int ty = g->th / 2 - 4; ty < g->th / 2 + 4; ++ty)
		for (int tx = g->tw / 2 - 8; tx < g->tw / 2 + 8; ++tx) {
			int phase, A, B;
			tile_class(g, tx, ty, &phase, &A, &B);
			if (seen[phase]) continue;
			seen[phase] = true;
			unsigned near = tl_tile_panels(g, tx, ty, TILE_OUT, 0, 0);
			by_phase[phase] = (View){ (uint16_t)near, (uint16_t)tl_tile_panels(g, tx, ty, REACH, 0, 0),
				(uint16_t)(tl_tile_panels(g, tx, ty, 0, reach + TILE_OUT, 0) & ~near),
				(uint16_t)(tl_tile_panels(g, tx, ty, 0, 0, WALL_BAND) & ~near) };
		}
	int slots = 0;
	for (int phase = 0; phase < 64; ++phase) {
		d->slot[phase] = (int8_t)(seen[phase] && slots < PANEL_TILES ? slots : -1);
		if (d->slot[phase] >= 0) d->view[slots++] = by_phase[phase];
	}
	for (int k = 0; k < nbooks; ++k)
		d->lend[k] = lends(&books[k], g) && !memcmp(books[k].panels.slot, d->slot, sizeof d->slot);
}

/* The panels a map shows and their shapes; false without memory. */
static bool laid_open(Laid *d, const TileBook *books, int nbooks, const TileGrid *g, TileFloor floor, const void *ctx) {
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
	d->pieced = malloc(n * sizeof *d->pieced);
	d->lend = calloc((size_t)nbooks + 1, sizeof *d->lend);
	if (!d->shape || !d->book || !d->index || !d->pieced || !d->lend) return false;
	memset(d->book, -1, n);
	memset(d->pieced, -1, n * sizeof *d->pieced);
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
	views(books, nbooks, g, d);
	return true;
}

static void laid_close(Laid *d) {
	free(d->shape);
	free(d->book);
	free(d->index);
	free(d->pieced);
	free(d->pieces);
	free(d->lend);
}

/* The panel that suits the floor's panel i best, of the books' with its
 * shape (the first book's first, then the first found); false none. */
static bool lay_one(const TileBook *books, int nbooks, const TileGrid *g, Laid *d, size_t i) {
	int A = d->A0 + (int)(i % d->w), B = d->B0 + (int)(i / d->w), best = -1, alike;
	for (int k = 0; k < nbooks; ++k) {
		const TilePanels *p = &books[k].panels;
		if (!d->lend[k]) continue;
		for (int c = first_of(p, d->shape[i]); c < p->n && p->panel[c].shape == d->shape[i]; ++c) {
			if (p->panel[c].rare || (g->single && two_layers(p, &p->panel[c]))) continue;
			int score = suits(d, k, c, A, B, false, &alike) + alike;
			if (score <= best) continue;
			best = score;
			d->book[i] = (int8_t)k;
			d->index[i] = (uint16_t)c;
		}
	}
	return best >= 0;
}

/* The slots of panel t whose tiles a floor's panel of `shape` can take:
 * those that show nothing of where the two neighbourhoods differ. A tile
 * shows the panels it lies on or beside; and with void among them, those
 * whose faces or walls could reach it there. */
static unsigned slots_like(const View view[PANEL_TILES], const TilePanel *t, uint32_t shape) {
	uint32_t x = t->shape ^ shape;
	unsigned differ = ((x >> 18 | x >> 9 | x) & 0x1FF) | t->no, voids = ~(shape >> 18 | shape >> 9) & 0x1FF, like = 0;
	if (differ & 0x10) return 0;
	for (int s = 0; s < PANEL_TILES; ++s) {
		unsigned near = voids & 0x10 ? view[s].wide : view[s].near;
		/* (floor of another kind is drawn as it likes: none of it anywhere near) */
		if ((view[s].wide | view[s].up | view[s].down) & t->no) continue;
		if (!(t->has >> s & 1) || near & differ) continue;
		if (near & voids && (view[s].up | view[s].down) & differ) continue;
		like |= 1u << s;
	}
	return like;
}

/* Room for one more panel in pieces, all its slots empty; -1 without memory. */
static int pieced_new(Laid *d) {
	if (d->npieces == d->cap) {
		int cap = d->cap ? d->cap * 2 : 32;
		Pieced *t = cap <= INT16_MAX ? realloc(d->pieces, (size_t)cap * sizeof *t) : NULL;
		if (!t) return -1;
		d->pieces = t;
		d->cap = cap;
	}
	memset(d->pieces[d->npieces].book, -1, sizeof d->pieces->book);
	return d->npieces++;
}

/* The floor's panel i laid piece by piece: of every panel that is like it
 * in some of its tiles, the one lying most as its map lays it beside what is
 * laid already, then the one with the most tiles still missing; and so on
 * for the tiles then missing, as far as any has them. False where none has
 * any. */
#define MOST 9   /* a tile more outweighs every neighbourhood alike */
static bool lay_pieces(const TileBook *books, int nbooks, const TileGrid *g, Laid *d, size_t i) {
	int A = d->A0 + (int)(i % d->w), B = d->B0 + (int)(i / d->w), at = -1;
	for (unsigned left = 0xFFFF; left;) {
		int best = -1, best_k = 0, best_c = 0, alike;
		unsigned best_like = 0;
		for (int k = 0; k < nbooks; ++k) {
			const TilePanels *p = &books[k].panels;
			if (!d->lend[k]) continue;
			for (int c = 0; c < p->n + p->npart; ++c) {
				unsigned like = slots_like(d->view, &p->panel[c], d->shape[i]) & left;
				if (!like || p->panel[c].rare || (g->single && two_layers(p, &p->panel[c]))) continue;
				int cont = suits(d, k, c, A, B, true, &alike), score = cont * 64 + __builtin_popcount(like) * MOST + alike;
				/* (off the floor, only as a map lays it beside what is laid) */
				if (!cont && !((d->shape[i] >> 18 | d->shape[i] >> 9) & 0x10)) continue;
				if (score <= best) continue;
				best = score;
				best_k = k;
				best_c = c;
				best_like = like;
			}
		}
		if (best < 0) break;
		if (at < 0) {
			if ((at = pieced_new(d)) < 0) return false;
			d->pieced[i] = (int16_t)at;
			d->book[i] = (int8_t)best_k;
			d->index[i] = (uint16_t)best_c;
		}
		for (int s = 0; s < PANEL_TILES; ++s)
			if (best_like >> s & 1) {
				d->pieces[at].book[s] = (int8_t)best_k;
				d->pieces[at].index[s] = (uint16_t)best_c;
			}
		left &= ~best_like;
	}
	return at >= 0;
}

/* The tile a laid panel sets at (tx, ty), NULL none; its panel in *at, and
 * in *from the panel of a map it is from. */
static const PanelTile *laid_tile(const TileBook *books, const TileGrid *g, const Laid *d, int tx, int ty, size_t *at, uint32_t *from) {
	if (tx < 0 || ty < 0 || tx >= g->tw || ty >= g->th) return NULL;
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	if (A < d->A0 || B < d->B0 || A >= d->A0 + d->w || B >= d->B0 + d->h) return NULL;
	*at = (size_t)(B - d->B0) * d->w + A - d->A0;
	int k = d->book[*at], c = d->index[*at], s = d->slot[phase];
	if (k < 0 || s < 0) return NULL;
	if (d->pieced[*at] >= 0) {
		const Pieced *pc = &d->pieces[d->pieced[*at]];
		if (pc->book[s] < 0) return NULL;
		k = pc->book[s];
		c = pc->index[s];
	}
	const TilePanels *p = &books[k].panels;
	if (p->panel[c].tile[s] == NO_TILE) return NULL;
	*from = (uint32_t)k << 16 | (uint32_t)c;
	return &p->tile[p->panel[c].tile[s]];
}

/* The tile at (tx, ty) of the floor's panel `at` taken up again: with its
 * whole panel, or where the panel is laid in pieces, alone. */
static void lift(const TileGrid *g, Laid *d, size_t at, int tx, int ty) {
	if (d->pieced[at] < 0) { d->book[at] = -1; return; }
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	d->pieces[d->pieced[at]].book[d->slot[phase]] = -1;
}

/* A piece's tiles are like the floor as far as its panels say; what a map
 * draws past a panel's edge they do not (BN4's Park Area ends a walkway at
 * its platform's corner in a cap a tile beyond its panel). So each is held
 * to the floor as a picked tile is: drawn on it and its faces, not away from
 * them, but for a wall on an upper edge. */
static void lift_strays(const TileBook *books, const TileGrid *g, TileFloor floor, const void *ctx, Laid *d) {
	for (int ty = 0; ty < g->th; ++ty)
		for (int tx = 0; tx < g->tw; ++tx) {
			size_t i = 0;
			uint32_t from = 0;
			const PanelTile *t = laid_tile(books, g, d, tx, ty, &i, &from);
			if (!t || d->pieced[i] < 0) continue;
			uint64_t must, never, deep;
			tl_expect(g, tx, ty, floor, ctx, TILE_A, &must, &never, &deep);
			if (tl_misses(t->mask, must, never & ~tl_above_floor(g, tx, ty, floor, ctx, WALL_BAND)) > SLACK) lift(g, d, i, tx, ty);
		}
}

/* Two tiles laid side by side that no map lays so must still meet as the
 * maps' tiles do: where a strip across their edge is one no map shows, the
 * later of the two is taken up again (picked one by one, to meet what
 * stays): its whole panel, or the tile alone in a panel laid in pieces. */
static void take_up(const TileBook *books, const TileSeams *seams, const TileGrid *g, Laid *d) {
	for (bool changed = true; changed;) {
		changed = false;
		for (int ty = 0; ty < g->th; ++ty)
			for (int tx = 0; tx < g->tw; ++tx) {
				size_t i = 0, j = 0;
				uint32_t fi = 0, fj = 0;
				const PanelTile *t = laid_tile(books, g, d, tx, ty, &i, &fi);
				if (!t || !t->mask) continue;
				for (int k = 2; k < 4; ++k) {
					const PanelTile *u = laid_tile(books, g, d, tx + (k == 2), ty + (k == 3), &j, &fj);
					if (!u || !u->mask || (j == i && fj == fi) || seams_strip_seen(seams, t->side[k], u->side[k ^ 2], k & 1)) continue;
					changed = true;
					if (i > j) { lift(g, d, i, tx, ty); break; }
					lift(g, d, j, tx + (k == 2), ty + (k == 3));
				}
			}
	}
}

int panels_lay(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g, TileFloor floor, const void *ctx, TilePick *pick) {
	bool any = false;
	if (nbooks > 127) nbooks = 127;
	for (int k = 0; k < nbooks; ++k) any |= lends(&books[k], g);
	Laid d;
	memset(&d, 0, sizeof d);
	d.books = books;
	if (!any || !laid_open(&d, books, nbooks, g, floor, ctx)) { laid_close(&d); return 0; }
	size_t n = (size_t)d.w * d.h;
	/* (the floor's plain middle is the first book's to lay by its period
	 * or to scatter: tiles_pick.c) */
	bool middles = nbooks && (books[0].patch || books[0].vary);
	/* the floor's panels, whole where a map has them; then what lies round
	 * the floor (its faces, legs and walls), each by the panels laid beside
	 * it: a void's neighbourhood does not say what floor it lies under */
	for (int pass = 0; pass < 2; ++pass)
		for (size_t i = 0; i < n; ++i) {
			bool is_floor = (d.shape[i] >> 18 | d.shape[i] >> 9) & 0x10;
			if (!(d.shape[i] >> 9) || is_floor == (pass == 1) || (middles && d.shape[i] == PANEL_SHAPE(0x1FF, 0, 0))) continue;
			if (!is_floor || !lay_one(books, nbooks, g, &d, i)) lay_pieces(books, nbooks, g, &d, i);
		}
	lift_strays(books, g, floor, ctx, &d);
	if (seams) take_up(books, seams, g, &d);
	int tiles = 0;
	for (int ty = 0; ty < g->th; ++ty)
		for (int tx = 0; tx < g->tw; ++tx) {
			size_t i = 0;
			uint32_t from = 0;
			const PanelTile *t = laid_tile(books, g, &d, tx, ty, &i, &from);
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
