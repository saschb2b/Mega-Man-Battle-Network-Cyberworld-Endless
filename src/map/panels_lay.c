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
typedef struct { uint16_t near, wide, up, down, on; } View;   /* (on: the panels it lies on, none beside) */

/* The floor's panels a map shows, w x h from (A0, B0), each with its shape
 * (0: no floor in or around it) and the panel laid on it (book, index; book
 * -1 none), or its pieces (`pieced`, -1 none; book and index then its first
 * piece's). */
typedef struct {
	const TileBook *books;
	const TileSeams *seams;
	const TileGrid *g;
	int A0, B0, w, h;
	uint32_t *shape;
	int8_t *book;
	uint16_t *index;
	int16_t *pieced;
	uint8_t *apart;   /* whether a piece drawn apart lies in or round the panel */
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

/* The tile laid at slot s of the floor's panel i, NULL none. */
static const PanelTile *slot_tile(const Laid *d, size_t i, int s) {
	int k = d->book[i], c = d->index[i];
	if (k < 0) return NULL;
	if (d->pieced[i] >= 0) {
		k = d->pieces[d->pieced[i]].book[s];
		c = d->pieces[d->pieced[i]].index[s];
		if (k < 0) return NULL;
	}
	const TilePanels *p = &d->books[k].panels;
	return p->panel[c].tile[s] == NO_TILE ? NULL : &p->tile[p->panel[c].tile[s]];
}

/* Whether panel c of book k is drawn along its edge towards the panel at
 * round8[way] as the floor's panel i is laid there: the very tiles, in
 * every slot that touches that neighbour, whatever map each is from. (A
 * map and its mirror image set the same tile where the art is the same on
 * both sides: a platform's rim.) */
static bool same_edge(const Laid *d, size_t i, int k, int c, int way) {
	const TilePanels *p = &d->books[k].panels;
	int bit = (round8[way][1] + 1) * 3 + round8[way][0] + 1, n = 0;
	for (int s = 0; s < PANEL_TILES; ++s) {
		if (!(d->view[s].near >> bit & 1)) continue;
		const PanelTile *a = slot_tile(d, i, s), *b = p->panel[c].tile[s] == NO_TILE ? NULL : &p->tile[p->panel[c].tile[s]];
		if (!a || !b || a->e0 != b->e0 || a->e1 != b->e1 || a->mask != b->mask) return false;
		n += a->mask != 0;
	}
	return n > 0;
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
		if (d->book[ni] < 0) continue;
		int beside = panel_at(p, t->A + round8[s][0], t->B + round8[s][1]);
		if (beside < 0) continue;
		if (d->book[ni] == k && beside == d->index[ni]) score += s < 4 ? BESIDE : BESIDE / 2;
		else if (d->book[ni] == k && !memcmp(p->panel[beside].tile, p->panel[d->index[ni]].tile, sizeof t->tile)) score += s < 4 ? ALIKE : ALIKE / 2;
		/* (piece by piece: or drawn alike along the edge the two share,
		 * round8[s ^ 2] or its corner from there) */
		else if (corners && same_edge(d, ni, k, beside, s < 4 ? s ^ 2 : 11 - s)) score += s < 4 ? ALIKE : ALIKE / 2;
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
				(uint16_t)(tl_tile_panels(g, tx, ty, 0, 0, WALL_BAND) & ~near), (uint16_t)tl_tile_panels(g, tx, ty, 0, 0, 0) };
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
	d->apart = calloc(n, 1);
	d->lend = calloc((size_t)nbooks + 1, sizeof *d->lend);
	if (!d->shape || !d->book || !d->index || !d->pieced || !d->apart || !d->lend) return false;
	memset(d->book, -1, n);
	memset(d->pieced, -1, n * sizeof *d->pieced);
	for (size_t i = 0; i < n; ++i) {
		unsigned oa = 0, ob = 0, op = 0;
		for (int k = 0; k < 9; ++k) {
			int m = floor(d->A0 + (int)(i % d->w) + k % 3 - 1, d->B0 + (int)(i / d->w) + k / 3 - 1, ctx);
			if (TILE_MATERIAL(m) == TILE_A) oa |= 1u << k;
			if (TILE_MATERIAL(m) == TILE_B) ob |= 1u << k;
			if (TILE_MATERIAL(m) && m & TILE_PAD) op |= 1u << k;
			if (TILE_MATERIAL(m) && m & TILE_APART) d->apart[i] = 1;
			/* (a walkway's end at a room, and what lies round it: none laid) */
			if (m & TILE_JOINS) oa = ob = 0x200;
		}
		d->shape[i] = oa & 0x200 ? 0 : PANEL_SHAPE(oa, ob, op);
	}
	views(books, nbooks, g, d);
	return true;
}

static void laid_close(Laid *d) {
	free(d->shape);
	free(d->book);
	free(d->index);
	free(d->pieced);
	free(d->apart);
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
static unsigned slots_like(const View view[PANEL_TILES], const TilePanel *t, uint32_t shape, bool tight) {
	uint32_t x = t->shape ^ shape;
	unsigned differ = ((x >> 18 | x >> 9 | x) & 0x1FF) | t->no, voids = ~(shape >> 18 | shape >> 9) & 0x1FF, like = 0;
	if (differ & 0x10) return 0;
	for (int s = 0; s < PANEL_TILES; ++s) {
		unsigned near = voids & 0x10 ? view[s].wide : view[s].near;
		/* (`tight`, lay_more: only the panels the tile lies on, where what
		 * differs beside it is something the map has and the floor has not:
		 * the tile then shows its own panel's art and none of a join) */
		if (tight && !(near & differ & ~voids)) near = view[s].on;
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
 * for the tiles then missing, as far as any has them; of a panel laid in
 * pieces already, the tiles it still misses. False where none has any. */
#define MOST 9   /* a tile more outweighs every neighbourhood alike */
static bool lay_pieces(const TileBook *books, int nbooks, const TileGrid *g, Laid *d, size_t i) {
	int A = d->A0 + (int)(i % d->w), B = d->B0 + (int)(i / d->w), at = d->pieced[i];
	unsigned left = 0xFFFF;
	bool any = false;
	for (int s = 0; at >= 0 && s < PANEL_TILES; ++s)
		if (d->pieces[at].book[s] >= 0) left &= ~(1u << s);
	while (left) {
		int best = -1, best_k = 0, best_c = 0, alike;
		unsigned best_like = 0;
		for (int k = 0; k < nbooks; ++k) {
			const TilePanels *p = &books[k].panels;
			if (!d->lend[k]) continue;
			for (int c = 0; c < p->n + p->npart; ++c) {
				unsigned like = slots_like(d->view, &p->panel[c], d->shape[i], false) & left;
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
		any = true;
	}
	return any;
}

/* A panel laid in pieces and still short of tiles takes more of the pieces
 * it has: a tile is lent on the first round only where it lies clear of
 * every panel that differs, by a margin for art that reaches past its
 * panel; of a panel laid here already that art is in the picture, and its
 * tiles that merely lie beside a panel that differs, not on it, are taken
 * too. Only for a piece drawn apart: every floor its map sets beside it is
 * away by design, and its own art is wanted bare; where floors join, a
 * panel's own art turns with its neighbour (a rim opens into a junction),
 * and Town Area's walkways took the lattice's. (BN4's Park Area's one
 * bottom corner has a walkway joined to its right: the tip's left tile is
 * clean in the map, its right one in the map's mirror image, each within
 * the margin of the walkway's panel.) */
static void lay_more(Laid *d, size_t i) {
	Pieced *pc = &d->pieces[d->pieced[i]];
	for (int s = 0; s < PANEL_TILES; ++s) {
		if (pc->book[s] >= 0) continue;
		for (int q = 0; q < PANEL_TILES && pc->book[s] < 0; ++q) {
			if (q == s || pc->book[q] < 0) continue;
			const TilePanel *t = &d->books[pc->book[q]].panels.panel[pc->index[q]];
			if (!(slots_like(d->view, t, d->shape[i], true) >> s & 1)) continue;
			pc->book[s] = pc->book[q];
			pc->index[s] = pc->index[q];
		}
	}
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
			int phase, A, B;
			tile_class(g, tx, ty, &phase, &A, &B);
			if (books[from >> 16].panels.panel[from & 0xFFFF].edge >> d->slot[phase] & 1) continue;   /* (a border's overhang, the learner's to judge) */
			uint64_t must, never, deep;
			tl_expect(g, tx, ty, floor, ctx, TILE_A, &must, &never, &deep);
			if (tl_misses(t->mask, must, never & ~tl_above_floor(g, tx, ty, floor, ctx, WALL_BAND)) > SLACK) lift(g, d, i, tx, ty);
		}
}

/* Whether two tiles side by side are each other's mirror image: no map
 * sets them so, and no line shows between them (a corner's tip, its left
 * half from a map and its right from the map's mirror image). */
static bool mirrored(const PanelTile *left, const PanelTile *right) {
	return ((left->e0 ^ right->e0) == 0x400 || !((left->e0 | right->e0) & 0x3FF)) &&
		((left->e1 ^ right->e1) == 0x400 || !((left->e1 | right->e1) & 0x3FF)) && left->mask;
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
					if (!u || !u->mask || (j == i && fj == fi) || (k == 2 && mirrored(t, u)) || seams_strip_seen(seams, t->side[k], u->side[k ^ 2], k & 1)) continue;
					changed = true;
					if (i > j) { lift(g, d, i, tx, ty); break; }
					lift(g, d, j, tx + (k == 2), ty + (k == 3));
				}
			}
	}
}

/* Whether tile u, set at the side `m` (left, above, right, below) of a
 * laid tile w, meets it in a strip a map shows. */
static bool meets(const TileSeams *seams, const PanelTile *u, const PanelTile *w, int m) {
	if (!seams || !w->mask) return true;
	/* (w lies at u's side m: the strip's first tile is the left or upper one) */
	return m < 2 ? seams_strip_seen(seams, w->side[m ^ 2], u->side[m], m & 1) : seams_strip_seen(seams, u->side[m], w->side[m ^ 2], m & 1);
}

/* Whether tile u meets every tile laid round (tx, ty) as the maps' do. */
static bool meets_all(const Laid *d, int tx, int ty, const PanelTile *u) {
	static const int side[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	for (int m = 0; m < 4; ++m) {
		size_t j = 0;
		uint32_t from = 0;
		const PanelTile *w = laid_tile(d->books, d->g, d, tx + side[m][0], ty + side[m][1], &j, &from);
		if (w && !meets(d->seams, u, w, m)) return false;
	}
	return true;
}

/* The pixels along a tile's left, upper, right and lower sides. */
static const uint64_t side_px[4] = { 0x0101010101010101ull, 0xFFull, 0x8080808080808080ull, 0xFFull << 56 };

/* The tile of a border's own art (TilePanel.edge) that goes on from a laid
 * tile of `from` into the tile at (tx, ty), which lies at its side k ^ 2:
 * the one its map sets there, where it meets all that lies round it; its
 * book and panel in *to. NULL none. (Any other tile of the border's art
 * that met its neighbours was tried there too: it mended fewer teeth than
 * it set pieces where none belonged.) */
static const PanelTile *goes_on(const Laid *d, int tx, int ty, int k, uint32_t from, uint32_t *to) {
	static const int side[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	int phase, A, B, An, Bn;
	tile_class(d->g, tx + side[k][0], ty + side[k][1], &phase, &An, &Bn);
	tile_class(d->g, tx, ty, &phase, &A, &B);
	int s = d->slot[phase];
	const TilePanels *p = &d->books[from >> 16].panels;
	const TilePanel *sp = &p->panel[from & 0xFFFF];
	int c = A == An && B == Bn ? (int)(from & 0xFFFF) : panel_at(p, sp->A + A - An, sp->B + B - Bn);
	if (c < 0 || p->panel[c].tile[s] == NO_TILE || !(p->panel[c].edge >> s & 1)) return NULL;
	const PanelTile *u = &p->tile[p->panel[c].tile[s]];
	*to = (from & 0xFFFF0000u) | (uint32_t)c;
	return u->mask && meets_all(d, tx, ty, u) ? u : NULL;
}

/* The floor's panel that tile (tx, ty) lies in, where a border may still
 * go on into that tile: nothing laid there, on a map's panel this floor
 * shows, not one laid whole (its tile is laid); its slot in *s. -1 else. */
static long open_tile(const Laid *d, int tx, int ty, int *s) {
	size_t i = 0;
	uint32_t from = 0;
	int phase, A, B;
	if (laid_tile(d->books, d->g, d, tx, ty, &i, &from)) return -1;
	tile_class(d->g, tx, ty, &phase, &A, &B);
	if (A < d->A0 || B < d->B0 || A >= d->A0 + d->w || B >= d->B0 + d->h || d->slot[phase] < 0) return -1;
	i = (size_t)(B - d->B0) * d->w + A - d->A0;
	*s = d->slot[phase];
	return d->book[i] >= 0 && d->pieced[i] < 0 ? -1 : (long)i;
}

/* The tile (tx, ty) of the floor's panel i, slot s, laid from a tile that
 * goes on from one laid beside it; whether one did. */
static bool follow_into(Laid *d, int tx, int ty, size_t i, int s) {
	static const int side[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	for (int k = 0; k < 4; ++k) {
		size_t j = 0;
		uint32_t from = 0, to = 0;
		const PanelTile *w = laid_tile(d->books, d->g, d, tx + side[k][0], ty + side[k][1], &j, &from);
		if (!w || !(w->mask & side_px[k ^ 2]) || !goes_on(d, tx, ty, k, from, &to)) continue;
		if (d->pieced[i] < 0) {
			int at = pieced_new(d);
			if (at < 0) return false;
			d->pieced[i] = (int16_t)at;
			d->book[i] = (int8_t)(to >> 16);
			d->index[i] = (uint16_t)to;
		}
		d->pieces[d->pieced[i]].book[s] = (int8_t)(to >> 16);
		d->pieces[d->pieced[i]].index[s] = (uint16_t)to;
		return true;
	}
	return false;
}

/* A border's art followed across the tiles' edges. Laid panel by panel and
 * piece by piece, a border ends wherever a panel or a piece does: its band
 * runs on into the tile beside, which nothing laid, and the border shows a
 * tooth at every such tile. So where a laid tile draws up to its edge and
 * nothing is laid beyond, the tile of the border's own art that goes on
 * from it is laid there (goes_on), and so on from that one. */
static void follow_edges(Laid *d) {
	for (bool changed = true; changed;) {
		changed = false;
		for (int ty = 0; ty < d->g->th; ++ty)
			for (int tx = 0, s = 0; tx < d->g->tw; ++tx) {
				long i = open_tile(d, tx, ty, &s);
				if (i >= 0 && follow_into(d, tx, ty, (size_t)i, s)) changed = true;
			}
	}
}

/* Whether tile (tx, ty) lies on a panel of the floor. */
static bool on_floor(const Laid *d, int tx, int ty) {
	int phase, A, B;
	if (tx < 0 || ty < 0 || tx >= d->g->tw || ty >= d->g->th) return false;
	tile_class(d->g, tx, ty, &phase, &A, &B);
	return (shape_at(d, A, B) >> 18 | shape_at(d, A, B) >> 9) & 0x10;
}

/* What is drawn off the floor hangs on it: a tile laid there that no row of
 * laid tiles that draw joins to a tile of the floor's panels is taken up
 * again. (A map's panel beside a floor holds what another floor near it
 * draws there too, a walkway's shading two panels away; lent alone, it lies
 * in the void.) Nothing without memory. */
static void lift_loose(Laid *d) {
	static const int side[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	int tw = d->g->tw, th = d->g->th;
	uint8_t *held = calloc((size_t)tw, (size_t)th);
	if (!held) return;
	/* (swept there and back until no more are held) */
	for (bool more = true, back = false; more; back = !back) {
		more = false;
		for (int n = 0; n < tw * th; ++n) {
			int at = back ? tw * th - 1 - n : n, tx = at % tw, ty = at / tw;
			size_t i = 0;
			uint32_t from = 0;
			const PanelTile *t = held[at] ? NULL : laid_tile(d->books, d->g, d, tx, ty, &i, &from);
			if (!t || !t->mask) continue;
			for (int k = 0; k < 4 && !held[at]; ++k) {
				int x = tx + side[k][0], y = ty + side[k][1];
				held[at] = on_floor(d, tx, ty) || on_floor(d, x, y) || (x >= 0 && y >= 0 && x < tw && y < th && held[y * tw + x]);
			}
			more |= held[at];
		}
	}
	for (int at = 0; at < tw * th; ++at) {
		size_t i = 0;
		uint32_t from = 0;
		const PanelTile *t = held[at] ? NULL : laid_tile(d->books, d->g, d, at % tw, at / tw, &i, &from);
		if (t && t->mask && d->pieced[i] >= 0) lift(d->g, d, i, at % tw, at / tw);
	}
	free(held);
}

int panels_lay(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g, TileFloor floor, const void *ctx, TilePick *pick) {
	bool any = false;
	if (nbooks > 127) nbooks = 127;
	for (int k = 0; k < nbooks; ++k) any |= lends(&books[k], g);
	Laid d;
	memset(&d, 0, sizeof d);
	d.books = books;
	d.seams = seams;
	d.g = g;
	if (!any || !laid_open(&d, books, nbooks, g, floor, ctx)) { laid_close(&d); return 0; }
	size_t n = (size_t)d.w * d.h;
	/* (the floor's plain middle is the first book's to lay by its period
	 * or to scatter: tiles_pick.c) */
	bool middles = nbooks && (books[0].patch || books[0].vary);
	/* the floor's panels, whole where a map has them; then what lies round
	 * the floor (its faces, legs and walls), each by the panels laid beside
	 * it: a void's neighbourhood does not say what floor it lies under */
	for (size_t i = 0; i < n; ++i) {
		bool is_floor = (d.shape[i] >> 18 | d.shape[i] >> 9) & 0x10;
		if (!is_floor || (middles && d.shape[i] == PANEL_SHAPE(0x1FF, 0, 0))) continue;
		if (!lay_one(books, nbooks, g, &d, i)) lay_pieces(books, nbooks, g, &d, i);
		if (d.pieced[i] >= 0 && d.apart[i]) lay_more(&d, i);
	}
	/* (round after round, while any tile is laid: each lies beside one laid
	 * before it, the first by a corner has none yet on its round, and one
	 * laid early takes what a later neighbour hands it: under a corner's
	 * tip, the half its map's mirror image has) */
	for (bool laid = true; laid;) {
		laid = false;
		for (size_t i = 0; i < n; ++i) {
			bool is_floor = (d.shape[i] >> 18 | d.shape[i] >> 9) & 0x10;
			if (is_floor || !(d.shape[i] >> 9) || !lay_pieces(books, nbooks, g, &d, i)) continue;
			if (d.apart[i]) lay_more(&d, i);
			laid = true;
		}
	}
	lift_strays(books, g, floor, ctx, &d);
	if (seams) take_up(books, seams, g, &d);
	follow_edges(&d);
	lift_loose(&d);
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
