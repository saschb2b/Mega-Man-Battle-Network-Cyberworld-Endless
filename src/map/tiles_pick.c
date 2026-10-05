/* A tile picked for each cell of a generated map (tiles.c): the
 * candidates weighed by their picture against what the cell should show,
 * and why one is off, for the tile test (build.py tiles). */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiles_learn.h"
#include "tiles_parts.h"

/* How different two neighbourhoods are around a tile at `phase`: floor
 * against no floor counts most at the centre and the sides nearest the
 * tile, then the nearest corner; the other material half as much, except
 * at the centre, which should keep its own. */
/* The panel straight above on screen (A + 1, B - 1): where side faces are
 * taller than half a panel, its face hangs over this one's top. */
#define ABOVE_BIT (1u << 2)
#define TALL_FACE 16

/* (dev) CYBERWORLD_TILE_AT "x,y;x,y": the tiles whose picks are printed */
static bool tile_debug(int tx, int ty) {
	static const char *at;
	static bool read;
	if (!read) { read = true; at = getenv("CYBERWORLD_TILE_AT"); }
	if (!at) return false;
	char want[32];
	snprintf(want, sizeof want, "%d,%d", tx, ty);
	for (const char *p = at; (p = strstr(p, want)); p += strlen(want))
		if ((p == at || p[-1] == ';') && (p[strlen(want)] == ';' || !p[strlen(want)])) return true;
	return false;
}

/* what a pixel of a tile showing other floors than its own costs, beside
 * the neighbourhood's distance */
#define PICTURE_COST 1
/* what a pair in colours the area's own map never shows on its floors
 * costs outside the walkway floor's fields, where its other maps' surfaces
 * came along (TILES_MORE_COLOURS: Seaside's yellow panels) */
#define OTHER_COST 16
/* Whether the middle panel lies in a 2 x 2 block of the panels of `m`. */
static bool in_block(unsigned m) {
	return (m & 0x1B) == 0x1B || (m & 0x36) == 0x36 || (m & 0xD8) == 0xD8 || (m & 0x1B0) == 0x1B0;
}

/* The panels' weights at `phase`: 4 the centre and the two nearest sides
 * (and with `tall` faces the one above), 2 the nearest corner, 1 the rest;
 * a set of them weighed by two lookups, of its bits 0-4 and 5-8, each
 * phase's made once (a tile's pick weighs a thousand pairs' neighbourhoods,
 * and weighing them panel by panel was half of making a layer) */
static const uint8_t *weights(int phase, bool tall) {
	static uint8_t w[2][64][48];
	static uint64_t made[2];
	uint8_t *t = w[tall][phase];
	if (made[tall] >> phase & 1) return t;
	made[tall] |= 1ull << phase;
	unsigned near = tl_nearest(phase, false), corner = tl_nearest(phase, true) & ~near;
	if (tall) near |= ABOVE_BIT;
	for (unsigned i = 0; i < 48; ++i) {
		unsigned bits = i < 32 ? i : (i - 32) << 5;
		int sum = 0;
		for (int k = 0; k < 9; ++k)
			if (bits >> k & 1) sum += near >> k & 1 ? 4 : corner >> k & 1 ? 2 : 1;
		t[i] = (uint8_t)sum;
	}
	return t;
}

/* floor against none counts twice its panel's weight, the other material
 * its weight, but 6 at the centre */
static int distance(const uint8_t *w, unsigned a1, unsigned b1, unsigned a2, unsigned b2) {
	unsigned f1 = a1 | b1, f2 = a2 | b2, x = (f1 ^ f2) & 0x1FF, m = f1 & f2 & (a1 ^ a2) & 0x1FF;
	return 2 * (w[x & 31] + w[32 + (x >> 5)]) + w[m & 0x0F] + w[32 + (m >> 5)] + (m & 0x10 ? 6 : 0);
}

/* The hue bucket (0-11, 12 grey) of a BGR555 colour. */
static int hue_calc(uint16_t c) {
	int r = c & 31, g = c >> 5 & 31, b = c >> 10 & 31;
	int mx = r > g ? (r > b ? r : b) : (g > b ? g : b), mn = r < g ? (r < b ? r : b) : (g < b ? g : b);
	if (!mx || (mx - mn) * 4 <= mx) return 12;
	float d = (float)(mx - mn), h = mx == r ? (g - b) / d : mx == g ? 2 + (b - r) / d : 4 + (r - g) / d;
	h /= 6;
	if (h < 0) h += 1;
	int k = (int)(h * 12);
	return k > 11 ? 11 : k;
}

/* (every colour's hue worked out once: its division, per pixel of every
 * pair unplain compares, was a tenth of making a layer) */
static int hue_of(uint16_t c) {
	static uint8_t hue[1 << 15];
	static bool made;
	if (!made) {
		for (int i = 0; i < 1 << 15; ++i) hue[i] = (uint8_t)hue_calc((uint16_t)i);
		made = true;
	}
	return hue[c & 0x7FFF];
}

/* Pixels well inside material m's floor where `c` looks like none of its
 * plain looks: its pattern, in their colours or another shade of their hues
 * (the Green HP draws its walkways a shade darker than its platforms, in
 * tiles of their own), each colour of theirs one of its own and no two the
 * same. */
static int unplain(const TileBook *b, int m, int phase, const TileCand *c, uint64_t deep) {
	if (!deep || !b->nplain[m][phase]) return 0;
	int best = 64;
	for (int k = 0; k < b->nplain[m][phase]; ++k) {
		const TileCand *p = &b->cand[b->plain[m][phase][k]];
		uint16_t from[16], to[16];
		int n = 0, pairs = 0;
		for (int i = 0; i < 64; ++i) {
			if (!(deep >> i & 1)) continue;
			uint16_t a = p->px[i], t = c->px[i];
			int j = 0;
			while (j < pairs && from[j] != a && to[j] != t) ++j;
			if (j < pairs) n += from[j] != a || to[j] != t;
			else if (pairs < 16 && (a == t || hue_of(a) == hue_of(t))) { from[pairs] = a; to[pairs++] = t; }
			else ++n;
		}
		if (n < best) best = n;
	}
	return best;
}

/* unplain, kept (plain_memo) by the pair, the material and the deep pixels */
static int unplain_kept(const TileBook *b, int m, int phase, const TileCand *c, uint64_t deep) {
	if (!tl_plain_memo || !deep) return unplain(b, m, phase, c, deep);
	uint64_t k = ((uint64_t)(uintptr_t)c * 0x9E3779B97F4A7C15ull) ^ (deep * 0xC2B2AE3D27D4EB4Full) ^ (uint64_t)m;
	uint32_t h = (uint32_t)(k >> 48) & (MEMO_SIZE - 1);
	for (int i = 0; i < MEMO_PROBES; ++i) {
		PlainMemo *e = &tl_plain_memo[(h + (uint32_t)i) & (MEMO_SIZE - 1)];
		if (e->pair == c && e->deep == deep && e->m == m) return e->value;
		if (!e->pair) {
			e->pair = c; e->deep = deep; e->m = m;
			return e->value = unplain(b, m, phase, c, deep);
		}
	}
	return unplain(b, m, phase, c, deep);
}

bool tiles_measure;
int tiles_pick_off, tiles_pick_why, tiles_pick_other;

/* The panel under map pixel (px, py) at z 0, as bit k of the 3x3 around
 * (A, B); -1 beyond them. */
static int panel_k(const TileGrid *g, int px, int py, int A, int B) {
	int u2 = 2 * px + 1 - g->tw * 8, v2 = 2 * (py - g->dv) + 1 - g->th * 8;
	int X4 = u2 - 2 * v2, Y4 = u2 + 2 * v2;
	int dA = tl_floordiv(X4 - 4 * g->ex, 128) - A, dB = tl_floordiv(Y4 - 4 * g->ey, 128) - B;
	return dA < -1 || dA > 1 || dB < -1 || dB > 1 ? -1 : dA + 1 + 3 * (dB + 1);
}

/* Per pixel of a tile, the panels straight up from it (bit k of the 3x3
 * around the tile's own, 0xFF beyond them) and whether it would show each
 * one's top, its side face or what hangs under that. */
#define COL_MAX 4
typedef struct { uint8_t n, open, k[COL_MAX], part[COL_MAX]; } PixCol;

/* A tile's pixels by their columns: pixels whose columns go up through the
 * same panels show alike (comparing pairs' pictures pixel by pixel was a
 * quarter of a map's pick). */
typedef struct { int n; PixCol col[64]; uint8_t count[64]; } Columns;

/* (the same for every tile of a phase in a map: kept per phase) */
static const Columns *columns(const TileGrid *g, int tx, int ty) {
	static Columns cache[64];
	static uint64_t have;
	static TileGrid of;
	if (g->tw != of.tw || g->th != of.th || g->ex != of.ex || g->ey != of.ey || g->dv != of.dv || g->face != of.face || g->hang != of.hang) {
		of = *g;
		have = 0;
	}
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	Columns *cs = &cache[phase];
	if (have >> phase & 1) return cs;
	have |= 1ull << phase;
	cs->n = 0;
	for (int i = 0; i < 64; ++i) {
		PixCol c = { 0 };
		int last = -2;
		for (int d = 0; d <= g->face + g->hang; ++d) {
			int k = panel_k(g, tx * 8 + i % 8, ty * 8 + i / 8 - d, A, B);
			if (k == last) continue;
			if (c.n == COL_MAX) { c.open = 1; break; }
			last = k;
			c.k[c.n] = k < 0 ? 0xFF : (uint8_t)k;
			c.part[c.n++] = d == 0 ? 0 : d <= g->face ? 1 : 2;
			if (k < 0) break;
		}
		int j = 0;
		while (j < cs->n && memcmp(&cs->col[j], &c, sizeof c)) ++j;
		if (j == cs->n) { cs->col[j] = c; cs->count[j] = 0; ++cs->n; }
		++cs->count[j];
	}
	return cs;
}

/* What a pixel shows of floors (a, b): 0 nothing, else which panel, whether
 * its top or its side face, and its material; 0xFF what does not tell
 * (beyond the 3x3, or under a face, where legs hang alike). */
static uint8_t col_code(const PixCol *c, unsigned a, unsigned b) {
	for (int j = 0; j < c->n; ++j) {
		if (c->k[j] == 0xFF) return 0xFF;
		if (!((a | b) >> c->k[j] & 1)) continue;
		return c->part[j] == 2 ? 0xFF : (uint8_t)(1 + (c->k[j] << 2 | (c->part[j] == 1) << 1 | (b >> c->k[j] & 1)));
	}
	return c->open ? 0xFF : 0;
}

/* The pixels where floors (a, b) show otherwise than `mine` (what the
 * tile's own show, per column). */
static int picture_diff(const Columns *cs, const uint8_t mine[64], unsigned a, unsigned b) {
	int n = 0;
	for (int j = 0; j < cs->n; ++j) {
		if (mine[j] == 0xFF) continue;
		uint8_t t = col_code(&cs->col[j], a, b);
		if (t != 0xFF && t != mine[j]) n += cs->count[j];
	}
	return n;
}

/* How far pair c was seen from (oa, ob), off by n pixels of the tile's
 * picture (TILE_EXACT ... TILE_OFF_NEAR). */
#define OFF_EDGE_PX 3
static int off_by(unsigned oa, unsigned ob, const TileCand *c, int n) {
	if (KEY_A(c->key) == oa && KEY_B(c->key) == ob) return TILE_EXACT;
	return !n ? TILE_OFF_FAR : n <= OFF_EDGE_PX ? TILE_OFF_EDGE : TILE_OFF_NEAR;
}

/* A pair as its neighbours see it. */
static uint32_t look_of(const TileCand *c) { return c->mask ? c->e0 : SEAM_VOID; }

/* pixels along one horizontal tile edge drawn on one side only, past a
 * diagonal's */
static int cut(uint64_t upper, uint64_t lower) {
	int n = tl_popcount64(((upper >> 56) ^ lower) & 0xFF) - CUT_EDGE;
	return n > 0 ? n : 0;
}

int tiles_trouble(const TileSeams *s, uint32_t look, uint64_t mask, const TileNeighbours *n) {
	int t = 0;
	/* (inside the floor, where both tiles are drawn whole, the classes keep
	 * the look: pairs the originals happen not to show there are no seam) */
	for (int k = 0; k < 4; ++k) {
		if (mask == ~0ull && n->mask[k] == ~0ull) continue;
		uint32_t first = k < 2 ? n->look[k] : look, second = k < 2 ? look : n->look[k];
		if (!seams_seen(s, first, second, k & 1)) t += SEAM_COST;
	}
	if (n->look[1] != SEAM_ANY) t += cut(n->mask[1], mask);
	if (n->look[3] != SEAM_ANY) t += cut(mask, n->mask[3]);
	return t;
}

/* (measuring) Why a tile's pick is drawn off: how far the pairs showing
 * what it shows got (TILE_WHY_...). */
static int why_off(const TileBook *books, int nbooks, const TileGrid *g, int tx, int ty, int phase,
	unsigned oa, unsigned ob, bool pad, TileFloor floor, const void *ctx, bool single, int cm,
	const Columns *cols, const uint8_t mine[64]) {
	int why = TILE_WHY_UNSEEN;
	uint64_t must = 0, never = 0, deep = 0;
	int allowed = 0;
	TileGrid gk = *g;
	gk.face = -1;
	gk.side = g->face;
	for (int k = 0; k < nbooks; ++k) {
		const TileBook *b = &books[k];
		int face = b->face > FACE_TALL ? g->face : b->face, hang = b->face > FACE_TALL ? g->hang : b->hang;
		if (face != gk.face || hang != gk.hang) {
			gk.face = face; gk.hang = hang;
			tl_expect(&gk, tx, ty, floor, ctx, cm, &must, &never, &deep);
			allowed = tl_popcount64(deep) / 8;
		}
		for (int i = tl_first_of(b, KEY(phase, 0, 0)); i < b->n && KEY_PHASE(b->cand[i].key) == phase; ++i) {
			const TileCand *t = &b->cand[i];
			if ((single && t->e1) || (g->single && (t->e0 & 0x3FF) && (t->e1 & 0x3FF))) continue;
			if (picture_diff(cols, mine, KEY_A(t->key), KEY_B(t->key)) > OFF_EDGE_PX) continue;
			int w = TILE_WHY_PIXELS;
			if (tl_misses(t->mask, must, never) <= SLACK) {
				int u = pad || (g->rimmed && (oa | ob) != 0x1FF) ? 0 : unplain(b, cm - 1, phase, t, deep);
				w = u <= allowed ? TILE_WHY_RANKED : TILE_WHY_PLAIN;
			}
			if (w > why) why = w;
		}
	}
	return why;
}

/* The best pair of `books` for a tile of class (phase, oa, ob) at (tx, ty):
 * the nearest neighbourhood seen whose tile fits there and meets its
 * neighbours as the originals do, most common first, preferring those with
 * the same floors on the panels nearest the tile; failing that, the least
 * bad. Only pairs without a back tile when `single`. */
static const TileCand *best(const TileBook *books, int nbooks, const TileGrid *g, int tx, int ty, int phase,
	unsigned oa, unsigned ob, bool pad, TileFloor floor, const void *ctx, bool single,
	const TileSeams *seams, const TileNeighbours *n, int *dist, int *off) {
	int cm = ob & 0x10 ? TILE_B : TILE_A;   /* the centre panel's material */
	/* each book's tiles with the faces and legs its own map draws (a
	 * walkway's may be thinner than a platform's), but the floor where this
	 * map draws it: a tile from a map measured to draw it higher would step
	 * out of the edge beside it; and a raised view's faces, which reach down
	 * to the ground, with this map's (its floor lies on the ground here) */
	Look look;
	int reach = 0;
	for (int k = 0; k < nbooks; ++k) {
		int r = books[k].face > FACE_TALL ? tl_reach_of(g->face, g->hang) : tl_reach_of(books[k].face, books[k].hang);
		if (r > reach) reach = r;
	}
	tl_expect_look(g, tx, ty, floor, ctx, cm, reach, &look);
	uint64_t must = 0, never = 0, deep = look.deep;
	int allowed = tl_popcount64(deep) / 8, kface = -1, khang = -1;
	const TileCand *fit = NULL, *any = NULL, *shows = NULL;
	int fit_d = INT_MAX, any_score = INT_MAX, fit_k = -1, fit_p = 0, any_p = 0, shows_d = INT_MAX, shows_p = 0;
	/* the walkway floor in a 2 x 2 block with the tile's panel: a field of it
	 * (not a pad, which is its own island) */
	unsigned pads = 0;
	{
		int ph, A, B;
		tile_class(g, tx, ty, &ph, &A, &B);
		for (int k = 0; k < 9; ++k)
			if (floor(A + k % 3 - 1, B + k / 3 - 1, ctx) & TILE_PAD) pads |= 1u << k;
	}
	bool field = in_block(ob & ~pads);
	/* what the tile shows of its own neighbourhood: a pair costs a point for
	 * each pixel where its own showed other floors, faces or materials */
	const Columns *cols = columns(g, tx, ty);
	uint8_t mine[64];
	for (int j = 0; j < cols->n; ++j) mine[j] = col_code(&cols->col[j], oa, ob);
	uint32_t pic_key = UINT32_MAX;
	int pic = 0;
	for (int k = 0; k < nbooks; ++k) {
		const TileBook *b = &books[k];
		int face = b->face > FACE_TALL ? g->face : b->face, hang = b->face > FACE_TALL ? g->hang : b->hang;
		if (face != kface || hang != khang) {
			kface = face; khang = hang;
			/* (outlines beside its faces as deep as this map's) */
			tl_expect_for(&look, face, hang, g->face, &must, &never);
		}
		const uint8_t *w = weights(phase, b->face > TALL_FACE);
		for (int j = tl_first_class(b, KEY(phase, 0, 0)); j < b->ncls && KEY_PHASE(b->cls[j].key) == phase; ++j) {
			/* a class's pairs share its distance, which each only adds to:
			 * a class too far to win is passed whole (of the hundreds of
			 * classes a tile's pick meets, most) */
			uint32_t key = b->cls[j].key;
			int d0 = distance(w, oa, ob, KEY_A(key), KEY_B(key));
			if (d0 > fit_d && 4 * d0 >= any_score) continue;
			int end = j + 1 < b->ncls ? b->cls[j + 1].first : b->n;
			for (int i = b->cls[j].first; i < end; ++i) {
				const TileCand *c = &b->cand[i];
				/* (a mirror image's empty entry is tile 0 flipped) */
				if ((single && c->e1) || (g->single && (c->e0 & 0x3FF) && (c->e1 & 0x3FF))) continue;
				int d = d0, m = tl_misses(c->mask, must, never);
				/* a pad in the pads' look, other floor not */
				if (c->pad != pad) d += PAD_LOOK;
				if (c->other && !field) d += OTHER_COST;
				/* (what follows only adds: a pair that cannot win is left) */
				if (!(m <= SLACK && d <= fit_d) && m + 4 * d >= any_score) continue;
				/* (a pad's middle is not the usual floor: it has its own look; nor
				 * is a rimmed floor's edge) */
				int u = pad || (g->rimmed && (oa | ob) != 0x1FF) ? 0 : unplain_kept(b, cm - 1, phase, c, deep);
				/* a neighbour it never meets in the originals: a seam */
				if (seams) d += tiles_trouble(seams, look_of(c), c->mask, n);
				/* what it shows otherwise (once per class: pairs come by class) */
				if (c->key != pic_key) {
					pic_key = c->key;
					pic = picture_diff(cols, mine, KEY_A(c->key), KEY_B(c->key));
				}
				d += PICTURE_COST * pic;
				if (tiles_measure && tile_debug(tx, ty))
					fprintf(stderr, "tile %d,%d phase %d ours a %03x b %03x: book %d e0 %04x e1 %04x count %u pad %d other %d key a %03x b %03x: d %d (picture %d) misses %d unplain %d of %d\n",
						tx, ty, phase, oa, ob, k, c->e0, c->e1, c->count, c->pad, c->other, KEY_A(c->key), KEY_B(c->key), d, pic, m, u, allowed);
				/* ties go to the first book with the class (the area's own map
				 * before its others), so a floor keeps one look (the other
				 * floor's edge coming along where only one is costs its pixels) */
				if (m <= SLACK && u <= allowed && (d < fit_d || (d == fit_d && k == fit_k && c->count > fit->count))) { fit = c; fit_d = d; fit_k = k; fit_p = pic; }
				/* (and the nearest showing what the tile shows, however its
				 * middle looks: a join's own tile, a bridge plugging into the
				 * Graveyard's slabs, lightens the slab around it) */
				if (m <= SLACK && pic <= OFF_EDGE_PX && d < shows_d) { shows = c; shows_d = d; shows_p = pic; }
				if (m + u + 4 * d < any_score) { any = c; any_score = m + u + 4 * d; any_p = pic; }
			}
		}
	}
	/* a tile showing other floors than its own is worse than one whose
	 * middle is not the plain floor's */
	if (fit && fit_p > OFF_EDGE_PX && shows) { fit = shows; fit_d = shows_d; fit_p = shows_p; }
	*dist = fit ? fit_d : -1;
	const TileCand *c = fit ? fit : any;
	*off = !tiles_measure ? TILE_EXACT : c ? off_by(oa, ob, c, fit ? fit_p : any_p) : TILE_OFF_NEAR;
	if (*off != TILE_OFF_NEAR) return c;
	tiles_pick_why = why_off(books, nbooks, g, tx, ty, phase, oa, ob, pad, floor, ctx, single, cm, cols, mine);
	return c;
}

/* One material of a floor, the other taken for void. */
typedef struct { TileFloor floor; const void *ctx; int keep; } Only;

static int only(int A, int B, const void *ctx) {
	const Only *o = ctx;
	int m = o->floor(A, B, o->ctx);
	return TILE_MATERIAL(m) == o->keep ? m : TILE_VOID;
}

/* a pick with this distance (-1: none fitted) in the stats (the pairs in
 * other colours are counted over the map as drawn: tiles_pick_other) */
static void count(int dist, const TileCand *c) {
	tiles_stats.picks++;
	if (dist < 0) tiles_stats.fallbacks++;
	else if (dist) tiles_stats.near++;
	if (c && c->other) tiles_pick_other = 1;
}

/* Where the original never joins its two floors (areas whose floors are
 * not drawn apart in pieces, net_area.apart): each drawn as if the other
 * were not there, on the two layers, the one whose top covers the other's
 * side face in front. */
static bool apart(const TileBook *books, int nbooks, unsigned oa, unsigned ob) {
	if (!oa || !ob) return false;
	for (int k = 0; k < nbooks; ++k) if (books[k].joins) return false;
	return true;
}

bool tiles_shape_seen(const TileBook *books, int nbooks, uint32_t shape) {
	for (int k = 0; k < nbooks; ++k) {
		const TileBook *b = &books[k];
		int lo = 0, hi = b->nshapes;
		while (lo < hi) {
			int mid = (lo + hi) / 2;
			if (b->shapes[mid] < shape) lo = mid + 1; else hi = mid;
		}
		if (lo < b->nshapes && b->shapes[lo] == shape) return true;
	}
	return false;
}

/* The middle's pair at panel (A, B) and `phase`: by its period (patch), or
 * where it has none, one of its looks as often as the original draws it
 * (vary); NULL neither. */
static const TileCand *middle_pick(const TileBook *b, int A, int B, int phase) {
	if (b->patch) {
		int ci = b->patch[(tl_mod_of(B, b->pb) * b->pa + tl_mod_of(A, b->pa)) * 64 + phase];
		return ci >= 0 ? &b->cand[ci] : NULL;
	}
	if (!b->vary || b->vary_first[phase + 1] <= b->vary_first[phase]) return NULL;
	uint32_t total = 0, h = (uint32_t)A * 0x9E3779B1u ^ (uint32_t)B * 0x85EBCA77u ^ (uint32_t)phase * 0xC2B2AE3Du;
	h ^= h >> 15;
	h *= 0x2C1B3C6Du;
	h ^= h >> 12;
	for (int i = b->vary_first[phase]; i < b->vary_first[phase + 1]; ++i) total += b->vary_weight[i];
	uint32_t r = total ? h % total : 0;
	int i = b->vary_first[phase];
	while (i + 1 < b->vary_first[phase + 1] && r >= b->vary_weight[i]) r -= b->vary_weight[i++];
	return &b->vary[i];
}

bool tiles_pick(const TileBook *books, int nbooks, const TileGrid *g, int tx, int ty,
	TileFloor floor, const void *ctx, const TileSeams *seams, const TileNeighbours *n,
	uint16_t *e0, uint16_t *e1, uint32_t *look, uint64_t *mask) {
	int phase, A, B, dist, off, off2;
	tiles_pick_off = TILE_EXACT;
	tiles_pick_why = TILE_WHY_NONE;
	tiles_pick_other = 0;
	tile_class(g, tx, ty, &phase, &A, &B);
	unsigned oa, ob;
	tl_occupancy(floor, ctx, A, B, &oa, &ob);
	if (!(oa | ob)) return false;
	bool pad = floor(A, B, ctx) & TILE_PAD;
	/* the platform floor's middle, as the own map lays it */
	const TileCand *mc = oa == 0x1FF && !ob && !pad && nbooks ? middle_pick(&books[0], A, B, phase) : NULL;
	if (mc) {
		count(0, mc);
		*e0 = mc->e0;
		*e1 = mc->e1;
		*look = look_of(mc);
		*mask = mc->mask;
		return true;
	}
	if (apart(books, nbooks, oa, ob)) {
		Only oa_only = { floor, ctx, TILE_A }, ob_only = { floor, ctx, TILE_B };
		const TileCand *pa = best(books, nbooks, g, tx, ty, phase, oa, 0, pad, only, &oa_only, true, NULL, n, &dist, &off);
		count(dist, pa);
		const TileCand *pb = best(books, nbooks, g, tx, ty, phase, 0, ob, pad, only, &ob_only, true, NULL, n, &dist, &off2);
		count(dist, pb);
		if (pb && !pb->mask) pb = NULL;
		if (pa && !pa->mask) pa = NULL;
		if (pa || pb) {
			tiles_pick_off = off > off2 ? off : off2;
			/* the one in front on the second layer, which the game shows so */
			const TileCand *first = pa, *second = pb;
			if (!pa || (pb && !tiles_in_front(g, tx, ty, only, &ob_only, only, &oa_only))) { first = pb; second = pa; }
			*e0 = first->e0;
			*e1 = second ? second->e0 : 0;
			*look = SEAM_ANY;   /* two tiles over each other: no original to compare */
			*mask = (pa ? pa->mask : 0) | (pb ? pb->mask : 0);
			return true;
		}
	}
	const TileCand *c = best(books, nbooks, g, tx, ty, phase, oa, ob, pad, floor, ctx, false, seams, n, &dist, &off);
	count(dist, c);
	if (!c) return false;
	tiles_pick_off = off;
	*e0 = c->e0;
	*e1 = c->e1;
	*look = look_of(c);
	*mask = c->mask;
	return true;
}
