/* A whole tile map: every tile's pair in reading order, each knowing the
 * tiles picked before it; then again where a tile meets its neighbours badly
 * (tiles_trouble), now knowing all four, with the neighbours it meets badly,
 * until none changes. Where pieces of the floor stand apart (the Aquarium's
 * pads, Green's and CopyBot's platforms), each piece and the rest of the
 * floor are picked so, each as if the others were not there, and laid over
 * each other on the two tile layers. */
#include "tilemap.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "panels.h"

#define PASSES 3

/* The tiles picked so far: how each looks and what it draws. */
typedef struct {
	const TileBook *books;
	int nbooks;
	const TileSeams *seams;
	const TileGrid *g;
	TileFloor floor;
	const void *ctx;
	uint16_t *map;
	TilePick *pick;   /* (look SEAM_ANY: not picked yet) */
	bool *on;         /* on the floor: has a pair */
	uint8_t *off;     /* how far its pair was seen from its neighbourhood, why, and its colours (tiles_pick_off, _why, _other) */
	uint8_t *dirty;   /* to look at again: a neighbour has changed */
	uint8_t *whole;   /* set with its panel, as an original map lays it (panels.h) */
} Picked;

static void picked_close(Picked *p) {
	free(p->pick);
	free(p->on);
	free(p->off);
	free(p->dirty);
	free(p->whole);
}

/* Their memory, none picked yet; false, and none held, where it ran out. */
static bool picked_open(Picked *p, size_t cells) {
	p->pick = calloc(cells, sizeof *p->pick);
	p->on = calloc(cells, 1);
	p->off = calloc(cells, 1);
	p->dirty = malloc(cells);
	p->whole = calloc(cells, 1);
	if (!p->pick || !p->on || !p->off || !p->dirty || !p->whole) {
		picked_close(p);
		return false;
	}
	for (size_t i = 0; i < cells; ++i) p->pick[i].look = SEAM_ANY;
	memset(p->dirty, 1, cells);
	return true;
}

/* Tile (tx, ty)'s four neighbours: left, above, right, below. */
static void neighbours(const Picked *p, int tx, int ty, TileNeighbours *n) {
	static const int d[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	n->known = 0;
	for (int k = 0; k < 4; ++k) {
		int x = tx + d[k][0], y = ty + d[k][1];
		bool off = x < 0 || y < 0 || x >= p->g->tw || y >= p->g->th;
		const TilePick *t = off ? NULL : &p->pick[y * p->g->tw + x];
		n->look[k] = t ? t->look : SEAM_VOID;
		n->mask[k] = t ? t->mask : 0;
		if ((t && t->side) || !n->mask[k]) n->known |= 1u << k;
		n->side[k] = t && t->side ? t->side[k ^ 2] : 0;
	}
}

static bool pick_one(Picked *p, int tx, int ty) {
	size_t i = (size_t)ty * p->g->tw + tx, cells = (size_t)p->g->tw * p->g->th;
	TileNeighbours n;
	neighbours(p, tx, ty, &n);
	p->pick[i] = (TilePick){ 0, 0, SEAM_VOID, 0, NULL };
	bool on = tiles_pick(p->books, p->nbooks, p->g, tx, ty, p->floor, p->ctx, p->seams, &n, &p->pick[i]);
	if (!on) p->pick[i] = (TilePick){ 0, 0, SEAM_VOID, 0, NULL };
	p->map[i] = p->pick[i].e0;
	p->map[cells + i] = p->pick[i].e1;
	p->off[i] = (uint8_t)(tiles_pick_off | tiles_pick_why << 2 | tiles_pick_other << 5);
	return on;
}

/* The sides (bit k: left, above, right, below) where tile (tx, ty) meets its
 * neighbour badly; with `seams`, only where no original map sets the two
 * side by side (the seams the tile test counts: the breaks in the picture
 * it counts in the picture itself, breaks.h). */
static unsigned trouble(const Picked *p, int tx, int ty, bool seams) {
	size_t i = (size_t)ty * p->g->tw + tx;
	TileNeighbours n = { { 0 }, { 0 }, { 0 }, 0 };
	neighbours(p, tx, ty, &n);
	unsigned sides = 0;
	for (int k = 0; k < 4; ++k) {
		TileNeighbours one = { { SEAM_ANY, SEAM_ANY, SEAM_ANY, SEAM_ANY }, { 0 }, { 0 }, 0 };
		one.look[k] = n.look[k];
		one.mask[k] = n.mask[k];
		one.side[k] = n.side[k];
		one.known = seams ? 0 : n.known & 1u << k;
		if (tiles_trouble(p->seams, p->pick[i].look, p->pick[i].mask, p->pick[i].side, &one)) sides |= 1u << k;
	}
	return sides;
}

/* How many sides the tiles `at` (n of them) meet a neighbour badly on. */
static int troubles(const Picked *p, const size_t *at, int n) {
	int sum = 0;
	for (int k = 0; k < n; ++k) sum += __builtin_popcount(trouble(p, (int)(at[k] % (size_t)p->g->tw), (int)(at[k] / (size_t)p->g->tw), false));
	return sum;
}

/* The whole panel tile (tx, ty) lies in, tried tile by tile: kept so where
 * its tiles then meet their neighbours on fewer sides badly, else put back
 * as it was; `up` is scratch (all 0). True where it was taken up. */
#define PANEL_MOST 32
static bool take_up(Picked *p, int tx, int ty, uint8_t *up) {
	int tw = p->g->tw, n = 0;
	size_t cells = (size_t)tw * p->g->th, at[PANEL_MOST];
	TilePick was[PANEL_MOST];
	panels_mark(p->g, tx, ty, up);
	for (int y = ty - 4; y <= ty + 4; ++y)
		for (int x = tx - 8; x <= tx + 8; ++x) {
			if (x < 0 || y < 0 || x >= tw || y >= p->g->th || !up[(size_t)y * tw + x]) continue;
			up[(size_t)y * tw + x] = 0;
			if (n < PANEL_MOST && p->whole[(size_t)y * tw + x]) at[n++] = (size_t)y * tw + x;
		}
	int before = troubles(p, at, n);
	for (int k = 0; k < n; ++k) {
		was[k] = p->pick[at[k]];
		p->pick[at[k]].look = SEAM_ANY;
	}
	TileStats stats = tiles_stats;
	for (int k = 0; k < n; ++k) p->on[at[k]] = pick_one(p, (int)(at[k] % (size_t)tw), (int)(at[k] / (size_t)tw));
	if (troubles(p, at, n) < before) {
		for (int k = 0; k < n; ++k) p->whole[at[k]] = 0;
		return true;
	}
	tiles_stats = stats;
	for (int k = 0; k < n; ++k) {
		p->pick[at[k]] = was[k];
		p->on[at[k]] = false;
		p->off[at[k]] = 0;
		p->map[at[k]] = was[k].e0;
		p->map[cells + at[k]] = was[k].e1;
	}
	return false;
}

/* How many of tile (tx, ty)'s pixels must be floor. */
static int floor_px(const Picked *p, int tx, int ty) {
	uint64_t must, never, deep;
	tl_expect(p->g, tx, ty, p->floor, p->ctx, TILE_A, &must, &never, &deep);
	return tl_popcount64(must);
}

/* Whether tile (tx, ty) lies on a panel of the floor, by more than a
 * corner's tip (a road's bend rounds its tip off). */
#define TIP_PX 16
static bool on_floor(const Picked *p, int tx, int ty) {
	int phase, A, B;
	tile_class(p->g, tx, ty, &phase, &A, &B);
	unsigned on = tl_tile_panels(p->g, tx, ty, 0, 0, 0);
	for (int k = 0; k < 9; ++k)
		if (on >> k & 1 && TILE_MATERIAL(p->floor(A + k % 3 - 1, B + k / 3 - 1, p->ctx))) return floor_px(p, tx, ty) >= TIP_PX;
	return false;
}

/* The original's panels whole where the floor has them, then the rest
 * tile by tile; then each panel whose tiles meet those picked beside them
 * badly is tried tile by tile too (take_up). */
static void lay_whole(Picked *p) {
	int tw = p->g->tw, th = p->g->th;
	size_t cells = (size_t)tw * th;
	panels_lay(p->books, p->nbooks, p->seams, p->g, p->floor, p->ctx, p->pick);
	for (size_t i = 0; i < cells; ++i) p->whole[i] = p->pick[i].look != SEAM_ANY;
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) {
			size_t i = (size_t)ty * tw + tx;
			/* (thin books, a road's: off its floor nothing is picked alone,
			 * where its rim came out as blocks of plain floor) */
			if (!p->whole[i] && p->g->thin && !on_floor(p, tx, ty)) continue;
			if (!p->whole[i]) { p->on[i] = pick_one(p, tx, ty); continue; }
			p->map[i] = p->pick[i].e0;
			p->map[cells + i] = p->pick[i].e1;
		}
	uint8_t *up = p->g->thin ? NULL : calloc(cells, 1);
	for (int ty = 0; up && ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx)
			if (p->whole[ty * tw + tx] && trouble(p, tx, ty, false)) take_up(p, tx, ty, up);
	free(up);
	int whole = 0;
	for (size_t i = 0; i < cells; ++i) {
		whole += p->whole[i];
		p->dirty[i] = !p->whole[i];
	}
	tiles_stats.picks += whole;
	tiles_stats.whole += whole;
}

static void pick_all(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g,
	TileFloor floor, const void *ctx, uint16_t *map, uint8_t *left) {
	int tw = g->tw, th = g->th;
	size_t cells = (size_t)tw * th;
	Picked p = { books, nbooks, seams, g, floor, ctx, map, NULL, NULL, NULL, NULL, NULL };
	memset(map, 0, cells * 2 * sizeof *map);   /* (off the floor a tile keeps nothing) */
	memset(left, 0, cells);
	if (!picked_open(&p, cells)) return;   /* (no memory: a map of nothing) */
	tiles_keep_begin(g, floor, ctx);
	lay_whole(&p);
	TileStats first = tiles_stats;   /* (the passes below would count twice) */
	static const int d[5][2] = { { 0, 0 }, { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	for (int pass = 0; pass < PASSES; ++pass) {
		int changed = 0;
		for (int ty = 0; ty < th; ++ty)
			for (int tx = 0; tx < tw; ++tx) {
				size_t i = (size_t)ty * tw + tx;
				if (!p.dirty[i] || !p.on[i]) continue;
				p.dirty[i] = 0;
				unsigned sides = trouble(&p, tx, ty, false);
				/* the tile, then the neighbours it meets badly */
				for (int k = 0; sides && k < 5; ++k) {
					if (k && !(sides >> (k - 1) & 1)) continue;
					int x = tx + d[k][0], y = ty + d[k][1];
					size_t j = (size_t)y * tw + x;
					if (x < 0 || y < 0 || x >= tw || y >= th || !p.on[j]) continue;
					uint32_t was = p.pick[j].look;
					pick_one(&p, x, y);
					if (p.pick[j].look == was) continue;
					++changed;
					for (int m = 1; m < 5; ++m) {
						int u = x + d[m][0], v = y + d[m][1];
						if (u >= 0 && v >= 0 && u < tw && v < th) p.dirty[(size_t)v * tw + u] = 1;
					}
				}
			}
		if (!changed) break;
	}
	tiles_stats = first;
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) {
			unsigned sides = trouble(&p, tx, ty, true);
			left[ty * tw + tx] = (uint8_t)((sides >> 2 & 1) | (sides >> 3 & 1) << 1 | p.off[ty * tw + tx] << 2);   /* (off: bits 2-3, why: 4-6, other: 7) */
		}
	picked_close(&p);
	tiles_keep_end();
}

/* Where pieces stand apart, the floor in pieces: each piece (its TILE_APART
 * panels joined side by side) and the rest of the floor. `label` covers the
 * panels the map shows: 0 off the floor, 1 the rest, 2 and up the pieces. */
typedef struct {
	TileFloor floor;
	const void *ctx;
	int A0, B0, w, h;
	uint8_t *label;
	int lo, hi;   /* the pieces a view shows */
	int roads;    /* the first piece that is a road (TILE_ROAD), after the rooms: no room, a walkway of another look */
	bool joins;   /* ... its floor beside a room it does not show marked so (TILE_JOINS) */
	bool under;   /* ... a road: it runs on two panels under the floor it meets, of any piece (pieces_floor) */
} Pieces;

#define REST 1
#define FIRST_PIECE 2
#define ROAD_OPEN 254   /* (while the pieces are labelled: a road's panel, a room's, not yet in one) */
#define ROOM_OPEN 255

static int piece(const Pieces *p, int A, int B) {
	if (A < p->A0 || B < p->B0 || A >= p->A0 + p->w || B >= p->B0 + p->h) return 0;
	return p->label[(size_t)(B - p->B0) * p->w + A - p->A0];
}

static bool room(const Pieces *p, int l) { return l >= FIRST_PIECE && l < p->roads; }

static const int side[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };

/* Whether the road's panel (A, B) runs on under the floor at its side k:
 * under a room always; under a walkway only at the road's end, and
 * straight on where it can. A walkway that leaves a road's side ends at
 * the road's rim: run on under it, the road was a junction, which its
 * maps have none of, laid in pieces. */
static bool runs_under(const Pieces *p, int A, int B, int k) {
	int n = piece(p, A + side[k][0], B + side[k][1]), own = 0, on = k;
	if (!n || (n >= p->lo && n <= p->hi)) return false;
	if (room(p, n)) return true;
	for (int j = 0; j < 4; ++j) {
		int m = piece(p, A + side[j][0], B + side[j][1]);
		if (m < p->lo || m > p->hi) continue;
		++own;
		on = j ^ 2;   /* (straight on from the road behind) */
	}
	int ahead = piece(p, A + side[on][0], B + side[on][1]);
	return own <= 1 && (on == k || !ahead || (ahead >= p->lo && ahead <= p->hi));
}

/* A road runs on straight under the floor it meets, two panels: its maps
 * end a road in an arrow's head or at a walkway, never at a room, and its
 * rims, wider than a walkway's, are laid with its panels alone; so it is
 * laid as if it went on, the room or walkway drawn over it, and its end
 * lies well under them. The road's floor at panel (A, B) of another piece,
 * 0 none. */
static int under_floor(const Pieces *p, int A, int B) {
	for (int k = 0; k < 4; ++k) {
		int A1 = A + side[k][0], B1 = B + side[k][1], n1 = piece(p, A1, B1);
		if (n1 >= p->lo && n1 <= p->hi && runs_under(p, A1, B1, k ^ 2)) return TILE_MATERIAL(p->floor(A1, B1, p->ctx));
		int A2 = A1 + side[k][0], B2 = B1 + side[k][1], n2 = piece(p, A2, B2);
		if (n1 && (n1 < p->lo || n1 > p->hi) && n2 >= p->lo && n2 <= p->hi && runs_under(p, A2, B2, k ^ 2)) return TILE_MATERIAL(p->floor(A2, B2, p->ctx));
	}
	return 0;
}

/* The floor of pieces lo..hi alone, the rest taken for void; with
 * p->joins, a panel of theirs beside a room the view does not show (no
 * pad: a pad is where a walkway ends, and its maps draw that) is marked
 * TILE_JOINS. */
static int pieces_floor(int A, int B, const void *ctx) {
	const Pieces *p = ctx;
	int l = piece(p, A, B);
	if (!l || l < p->lo || l > p->hi) {
		int under = p->under && l ? under_floor(p, A, B) : 0;
		if (under) return under;
		/* (a room's panel just below a walkway's end on screen: the walkway
		 * runs on under the room's upper edge, whose wall then covers its
		 * end; ended at its own panel, picked tile by tile, it stopped in
		 * steps beside that wall) */
		static const int above[2][2] = { { 1, 0 }, { 0, -1 } };   /* (upper right, upper left) */
		for (int k = 0; p->joins && room(p, l) && k < 2; ++k) {
			int n = piece(p, A + above[k][0], B + above[k][1]);
			if (n && n >= p->lo && n <= p->hi && !(p->floor(A, B, p->ctx) & TILE_PAD))
				return TILE_MATERIAL(p->floor(A + above[k][0], B + above[k][1], p->ctx)) | TILE_JOINS;
		}
		return TILE_VOID;
	}
	int m = p->floor(A, B, p->ctx);
	for (int k = 0; p->joins && k < 4; ++k) {
		int n = piece(p, A + side[k][0], B + side[k][1]);
		if (n && (n < p->lo || n > p->hi) && !(p->floor(A + side[k][0], B + side[k][1], p->ctx) & TILE_PAD)) return m | TILE_JOINS;
	}
	return m;
}

/* Whether tile (tx, ty), whose centre lies in another piece's panel, also
 * lies on a panel of the view's own floor: it straddles that piece's edge. */
static bool straddles(const TileGrid *g, const Pieces *view, int tx, int ty) {
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	unsigned on = tl_tile_panels(g, tx, ty, 0, 0, 0);
	for (int k = 0; k < 9; ++k) {
		int l = piece(view, A + k % 3 - 1, B + k / 3 - 1);
		if (on >> k & 1 && l >= view->lo && l <= view->hi) return true;
	}
	return false;
}

/* Panel i and those of its kind (`open`) joined to it side by side, labelled l. */
static void label_from(Pieces *p, size_t i, uint8_t open, uint8_t l, int *queue) {
	int head = 0, tail = 0;
	queue[tail++] = (int)i;
	p->label[i] = l;
	while (head < tail) {
		int c = queue[head++], x = c % p->w, y = c / p->w;
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int nx = x + d[k][0], ny = y + d[k][1];
			if (nx < 0 || ny < 0 || nx >= p->w || ny >= p->h || p->label[ny * p->w + nx] != open) continue;
			p->label[ny * p->w + nx] = l;
			queue[tail++] = ny * p->w + nx;
		}
	}
}

/* Labels the floor's pieces; how many apart from the rest it found: the
 * rooms, then the roads (from p->roads), each joined to its own kind. */
static int label_pieces(const TileGrid *g, Pieces *p) {
	int phase, A, B, A1 = INT_MIN, B1 = INT_MIN;
	p->A0 = p->B0 = INT_MAX;
	for (int k = 0; k < 4; ++k) {   /* the map's corners reach furthest */
		tile_class(g, k & 1 ? g->tw - 1 : 0, k & 2 ? g->th - 1 : 0, &phase, &A, &B);
		if (A < p->A0) p->A0 = A;
		if (B < p->B0) p->B0 = B;
		if (A > A1) A1 = A;
		if (B > B1) B1 = B;
	}
	p->A0 -= 1; p->B0 -= 1;   /* (a tile looks a panel further) */
	p->w = A1 + 2 - p->A0;
	p->h = B1 + 2 - p->B0;
	size_t n = (size_t)p->w * p->h;
	p->label = calloc(n, 1);
	int *queue = malloc(n * sizeof *queue), pieces = 0;
	if (!p->label || !queue) {   /* (no memory: no pieces, all of it the rest's) */
		free(queue);
		free(p->label);
		p->label = NULL;
		p->w = p->h = 0;
		return 0;
	}
	for (size_t i = 0; i < n; ++i) {
		int m = p->floor(p->A0 + (int)(i % p->w), p->B0 + (int)(i / p->w), p->ctx);
		if (!TILE_MATERIAL(m)) continue;
		p->label[i] = g->apart && m & TILE_APART ? ROOM_OPEN : g->nroads && m & TILE_ROAD ? ROAD_OPEN : REST;
	}
	for (int pass = 0; pass < 2; ++pass) {
		uint8_t open = pass ? ROAD_OPEN : ROOM_OPEN;
		if (pass) p->roads = FIRST_PIECE + pieces;
		/* (past 252 pieces, the last takes them all) */
		for (size_t i = 0; i < n; ++i)
			if (p->label[i] == open) label_from(p, i, open, (uint8_t)(FIRST_PIECE + (pieces < 252 ? pieces++ : pieces - 1)), queue);
	}
	free(queue);
	return pieces;
}

static bool drawn(uint16_t e) { return e & 0x3FF; }

/* The entry of a tile's pair the game shows in front: the second layer's
 * (BG2 at priority 2, over the first's BG1 at priority 3). */
static uint16_t front_of(uint16_t e0, uint16_t e1) { return drawn(e1) ? e1 : e0; }

/* What laying one view's tiles over the map's so far goes by. */
typedef struct {
	const TileGrid *g;
	const Pieces *all, *view, *laid;   /* every piece, the view's, those laid before it */
	uint16_t *map, *one;               /* the map so far, the view's tiles */
	uint8_t *left, *one_left, *stacked;
} Stack;

/* Whether a road's tile at (tx, ty) is its own to draw: in a panel of its
 * floor; in another piece's, where it reaches back onto its own
 * (straddles); off the floor, where it lies on or beside its own or under
 * it (its rim, its side faces). What it draws of the panels it runs on
 * under (pieces_floor) stays out: their rims showed along a room's side. */
static bool road_draws(const TileGrid *g, const Pieces *view, int tx, int ty) {
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	int l = piece(view, A, B);
	if (l >= view->lo && l <= view->hi) return true;
	if (l) return straddles(g, view, tx, ty);
	unsigned on = tl_tile_panels(g, tx, ty, TILE_OUT + 1, g->face + g->hang, TILE_OUT + 1);
	for (int k = 0; k < 9; ++k) {
		int n = piece(view, A + k % 3 - 1, B + k / 3 - 1);
		if (on >> k & 1 && n >= view->lo && n <= view->hi) return true;
	}
	return false;
}

/* A road's tile laid: behind whatever is laid there (the rooms, laid
 * before it, stand in front of a road that runs on under them). */
static void stack_road(const Stack *s, int tx, int ty) {
	size_t cells = (size_t)s->g->tw * s->g->th, i = (size_t)ty * s->g->tw + tx;
	uint16_t *e0 = &s->map[i], *e1 = &s->map[cells + i], mine = front_of(s->one[i], s->one[cells + i]);
	if (!drawn(mine) || (drawn(*e0) && drawn(*e1)) || !road_draws(s->g, s->view, tx, ty)) return;
	unsigned was = s->left[i] >> 2, now = s->one_left[i] >> 2;
	s->left[i] = (uint8_t)((s->left[i] | s->one_left[i]) & 3) | (uint8_t)(((now & 3) > (was & 3) ? now : was) << 2);
	if (drawn(*e0) || drawn(*e1)) {
		*e1 = front_of(*e0, *e1);
		s->stacked[i] = 1;
	}
	*e0 = mine;
}

/* Whether the rest's tile at (tx, ty) lies under a room, where the area's
 * rooms stand over its walkways (LAY_ROOMS_OVER) and a walkway ends plainly
 * at one (TileGrid.plain_ends): on or just beside a room's panel, or where
 * its side faces reach. */
static bool under_rim(const Stack *s, int tx, int ty) {
	if (s->view->lo != REST || !s->g->plain_ends || !(s->g->rules & LAY_ROOMS_OVER)) return false;
	int phase, A, B;
	tile_class(s->g, tx, ty, &phase, &A, &B);
	unsigned near = tl_tile_panels(s->g, tx, ty, TILE_OUT, s->g->face + s->g->hang + TILE_OUT, 0);
	for (int k = 0; k < 9; ++k)
		if (near >> k & 1 && room(s->all, piece(s->all, A + k % 3 - 1, B + k / 3 - 1))) return true;
	return false;
}

/* The view's tile at (tx, ty) laid over what the map holds there: where two
 * draw a tile, the one whose top covers the other's side faces on the
 * second layer, which the game shows in front. */
static void stack_tile(const Stack *s, int tx, int ty) {
	size_t cells = (size_t)s->g->tw * s->g->th, i = (size_t)ty * s->g->tw + tx;
	uint16_t *e0 = &s->map[i], *e1 = &s->map[cells + i], n0 = s->one[i], n1 = s->one[cells + i];
	int phase, A, B;
	tile_class(s->g, tx, ty, &phase, &A, &B);
	/* (a walkway run on under a room: in the room's panels its tiles go
	 * behind the room's, and only those that reach back over the room's
	 * edge onto the walkway's own panel) */
	if (s->view->joins && room(s->all, piece(s->all, A, B))) {
		if (!(drawn(n0) || drawn(n1)) || (drawn(*e0) && drawn(*e1)) || !straddles(s->g, s->view, tx, ty)) return;
		uint16_t theirs = front_of(*e0, *e1);
		*e0 = front_of(n0, n1);
		*e1 = theirs;
		s->stacked[i] = 1;
		return;
	}
	unsigned was = s->left[i] >> 2, now = s->one_left[i] >> 2;
	s->left[i] = (uint8_t)((s->left[i] | s->one_left[i]) & 3) | (uint8_t)(((now & 3) > (was & 3) ? now : was) << 2);
	if (!drawn(n0) && !drawn(n1)) return;
	if (!drawn(*e0) && !drawn(*e1)) { *e0 = n0; *e1 = n1; return; }
	/* (a tile holds two pieces: of three, the one found behind goes) */
	uint16_t mine = front_of(n0, n1), theirs = front_of(*e0, *e1);
	/* (which lies in front is the floors' own to say, but where the
	 * area's rooms stand over its walkways) */
	Pieces own = *s->view;
	own.joins = false;
	if (!under_rim(s, tx, ty) && tiles_in_front(s->g, tx, ty, pieces_floor, &own, pieces_floor, s->laid)) { *e0 = theirs; *e1 = mine; }
	else if (!drawn(*e0) || !drawn(*e1)) { *e0 = mine; *e1 = theirs; }
	s->stacked[i] = 1;
}

/* Two tiles over each other: no original to compare them with. */
static void stacked_seams(const TileGrid *g, const uint8_t *stacked, uint8_t *left) {
	for (int ty = 0; ty < g->th; ++ty)
		for (int tx = 0; tx < g->tw; ++tx) {
			size_t i = (size_t)ty * g->tw + tx;
			if (stacked[i] || (tx + 1 < g->tw && stacked[i + 1])) left[i] &= (uint8_t)~1u;
			if (stacked[i] || (ty + 1 < g->th && stacked[i + g->tw])) left[i] &= (uint8_t)~2u;
		}
}

void tilemap_pick(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g,
	TileFloor floor, const void *ctx, uint16_t *map, uint8_t *left) {
	if (!g->apart && !g->nroads) {
		pick_all(books, nbooks, seams, g, floor, ctx, map, left);
		return;
	}
	/* Each piece as if no other floor were there, then the rest of the
	 * floor as if there were no pieces, laid over each other (stack_tile).
	 * A piece keeps its whole rim, and a walkway ends at it. The walkways
	 * marked as roads (TILE_ROAD) are pieces too, after the rooms, each
	 * picked from the books of the roads' look (g->roads), ending at a room
	 * as a walkway does and at a walkway plainly. */
	Pieces all = { floor, ctx, 0, 0, 0, 0, NULL, 0, 0, 0, false, false };
	int pieces = label_pieces(g, &all);
	size_t cells = (size_t)g->tw * g->th;
	uint16_t *one = calloc(cells * 2, sizeof *one);
	uint8_t *one_left = calloc(cells, 1), *stacked = calloc(cells, 1);
	memset(map, 0, cells * 2 * sizeof *map);
	memset(left, 0, cells);
	if (!one || !one_left || !stacked) {   /* (no memory: a map of nothing) */
		free(one);
		free(one_left);
		free(stacked);
		free(all.label);
		return;
	}
	/* (each piece in one layer's tiles: two can always share a tile, and
	 * none brings what the original drew over its floor on the other) */
	TileGrid one_layer = *g;
	one_layer.single = true;
	Pieces laid = all, view = all;   /* the pieces laid so far, the next one */
	laid.lo = FIRST_PIECE; laid.hi = FIRST_PIECE - 1;
	Stack stack = { g, &all, &view, &laid, map, one, left, one_left, stacked };
	for (int k = 0; k <= pieces; ++k) {
		int l = k < pieces ? FIRST_PIECE + k : REST;
		view.lo = view.hi = l;
		/* (where the area's panels are laid whole, g->wall: elsewhere a
		 * walkway's end at a room is picked as it always was) */
		bool road = l >= all.roads;
		view.joins = l == REST && g->plain_ends && g->wall;
		view.under = road;
		/* (a road in the books of its own look, drawn as they draw it) */
		TileGrid road_grid = one_layer;
		if (road) { road_grid.thin = true; road_grid.dv = g->roads[0].dv; road_grid.face = g->roads[0].face; road_grid.hang = g->roads[0].hang; }
		pick_all(road ? g->roads : books, road ? g->nroads : nbooks, seams, road ? &road_grid : &one_layer, pieces_floor, &view, one, one_left);
		for (int ty = 0; ty < g->th; ++ty)
			for (int tx = 0; tx < g->tw; ++tx) road ? stack_road(&stack, tx, ty) : stack_tile(&stack, tx, ty);
		if (l != REST) laid.hi = l;
		else laid.lo = REST;
	}
	stacked_seams(g, stacked, left);
	free(one);
	free(one_left);
	free(stacked);
	free(all.label);
}
