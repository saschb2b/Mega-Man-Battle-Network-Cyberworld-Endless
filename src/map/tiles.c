/* Every tile of an original map is classified by where its centre falls
 * inside a panel (in 4-unit steps) and which of the 3x3 panels around it are
 * floor, keeping only panels of the area's chosen style. A generated map
 * takes, tile by tile, a pair seen with the same or the nearest neighbours.
 *
 * Classes alone let through pieces made for one place in the original: a
 * corner cut for a bridge, decoration hanging off an edge, a hole where
 * another layer covered it. So each tile also has a pixel test: where the
 * floor and the side faces under it are, it must be drawn; away from them,
 * it must be empty; and well inside the floor it must look like one of the
 * area's usual floor panels. Tiles of the original that fail the first two
 * in their own place are not learned. */
#include "tiles.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define IN    2    /* pixels inside the floor's edge that must be drawn */
#define OUT   3    /* pixels beyond it that may be */
#define SLACK 2    /* stray pixels a tile may have */
#define DEEP  10   /* pixels inside the floor's edge where it must look plain */
#define HANG_BELOW_FACE 12   /* how far past a face legs may hang */
#define FACE_MAX 64     /* the tallest side face measured (the story comps' run to 30 and more) */
#define PLAIN_SHARE 8   /* a plain look is seen at least 1/8 as often as the most common */

static int floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

/* ---- the source's panels ---- */

static int style_at(const AreaSrc *a, int cx, int cy) {
	long r = 0, g = 0, b = 0, n = 0;
	int W = a->tw * 8, H = a->th * 8;
	for (int y = cy - 4; y < cy + 4; ++y)
		for (int x = cx - 8; x < cx + 8; ++x) {
			if (x < 0 || y < 0 || x >= W || y >= H) continue;
			uint32_t c = a->px[(size_t)y * W + x];
			r += (c >> 16) & 255; g += (c >> 8) & 255; b += c & 255; ++n;
		}
	if (!n) return 12;
	float fr = (float)r / n, fg = (float)g / n, fb = (float)b / n;
	float mx = fr > fg ? (fr > fb ? fr : fb) : (fg > fb ? fg : fb);
	float mn = fr < fg ? (fr < fb ? fr : fb) : (fg < fb ? fg : fb);
	if (mx <= 0 || (mx - mn) / mx <= 0.25f) return 12;
	float d = mx - mn, hue = mx == fr ? (fg - fb) / d : mx == fg ? 2 + (fb - fr) / d : 4 + (fr - fg) / d;
	hue /= 6;
	if (hue < 0) hue += 1;
	int k = (int)(hue * 12);
	return k > 11 ? 11 : k;
}

#define SPAN 128   /* panels cached per axis, centred on the world origin */
#define OTHER 3    /* a source panel of neither material */

typedef struct {
	const AreaSrc *a;
	uint16_t styles, walk_styles;
	bool bg_in_map;
	int8_t *state;   /* SPAN x SPAN panel states, -1 until measured */
} Src;

static int measure_panel(const Src *s, int A, int B) {
	const AreaSrc *a = s->a;
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	/* floor of another height is drawn elsewhere: learned in its own view */
	if (a->hz && area_src_height(a, X, Y) != a->level) return 0;
	/* nor is a hole in it, however much of the faces around hangs over it */
	if (!area_src_walled_floor(a, X, Y)) return 0;
	int px = area_px(a->tw, X, Y), py = area_py(a->th, X, Y);
	int W = a->tw * 8, H = a->th * 8;
	/* its middle and four points around it drawn: a panel, not the legs a
	 * computer's platforms hang into the gaps (where the background is part
	 * of the map, floor is what the front layer draws) */
	static const int probe[5][2] = { { 0, 0 }, { -12, 0 }, { 12, 0 }, { 0, -6 }, { 0, 6 } };
	for (int k = 0; k < 5; ++k) {
		int x = px + probe[k][0], y = py + probe[k][1];
		if (x < 0 || y < 0 || x >= W || y >= H || !(a->px[(size_t)y * W + x] >> 24)) return 0;
		if (s->bg_in_map && !a->front[(size_t)y * W + x]) return 0;
	}
	int st = style_at(a, px, py);
	return s->styles >> st & 1 ? TILE_A : s->walk_styles >> st & 1 ? TILE_B : OTHER;
}

/* Panel state in the source map: 0 empty, TILE_A platform floor, TILE_B
 * walkway floor, OTHER floor of another style */
static int src_panel(const Src *s, int A, int B) {
	int i = A + SPAN / 2, j = B + SPAN / 2;
	if (i < 0 || j < 0 || i >= SPAN || j >= SPAN) return measure_panel(s, A, B);
	int8_t *st = &s->state[j * SPAN + i];
	if (*st < 0) *st = (int8_t)measure_panel(s, A, B);
	return *st;
}

/* as the pixel test sees it: floor of another style is floor too */
static int src_floor(int A, int B, const void *ctx) {
	int st = src_panel(ctx, A, B);
	return st == OTHER ? TILE_A : st;
}

/* ---- classes ---- */

void tile_class(const TileGrid *g, int tx, int ty, int *phase, int *A, int *B) {
	int u = tx * 8 + 4 - g->tw * 4, v = ty * 8 + 4 - g->th * 4;
	int X = (u - 2 * v) / 2, Y = (u + 2 * v) / 2;
	*A = floordiv(X - g->ex, 32);
	*B = floordiv(Y - g->ey, 32);
	int fx = X - g->ex - 32 * *A, fy = Y - g->ey - 32 * *B;
	*phase = (fx / 4) * 8 + fy / 4;
}

/* The panels nearest a tile at `phase`: the centre and the two nearest
 * sides, and with `corner` the nearest diagonal too. Bit k is the panel at
 * (A + k % 3 - 1, B + k / 3 - 1). */
static unsigned nearest(int phase, bool corner) {
	int fx = (phase / 8) * 4, fy = (phase % 8) * 4;
	int da = fx < 16 ? 0 : 2, db = fy < 16 ? 0 : 6;   /* the near column and row */
	unsigned m = 0x010 | 1u << (da + 3) | 1u << (1 + db);
	return corner ? m | 1u << (da + db) : m;
}

/* The 3x3 panels' materials: bit k of *a (platform) or *b (walkway). */
static void occupancy(TileFloor floor, const void *ctx, int A, int B, unsigned *a, unsigned *b) {
	*a = *b = 0;
	for (int k = 0; k < 9; ++k) {
		int m = floor(A + k % 3 - 1, B + k / 3 - 1, ctx);
		if (m == TILE_A) *a |= 1u << k;
		if (m == TILE_B) *b |= 1u << k;
	}
}

/* A class: phase, then the platform and walkway panels around. */
#define KEY(phase, a, b) ((uint32_t)(phase) << 18 | (uint32_t)(a) << 9 | (uint32_t)(b))
#define KEY_PHASE(k) ((int)((k) >> 18))
#define KEY_A(k) (((k) >> 9) & 0x1FFu)
#define KEY_B(k) ((k) & 0x1FFu)

/* ---- the pixel test ---- */

/* The floor map pixel (px, py) shows at z 0 (0 none, else its material):
 * the world point under its centre, in quarter units (X = (u - 2v) / 2,
 * Y = (u + 2v) / 2), with the floor drawn dv pixels below it. */
static int floor_px(const TileGrid *g, TileFloor floor, const void *ctx, int px, int py) {
	int u2 = 2 * px + 1 - g->tw * 8, v2 = 2 * (py - g->dv) + 1 - g->th * 8;
	int X4 = u2 - 2 * v2, Y4 = u2 + 2 * v2;
	return floor(floordiv(X4 - 4 * g->ex, 128), floordiv(Y4 - 4 * g->ey, 128), ctx);
}

/* The pixels of tile (tx, ty) that must be drawn (inside the floor and on
 * the side faces under it), those that must not (off both) and those well
 * inside floor of material `cm`. */
static void expect(const TileGrid *g, int tx, int ty, TileFloor floor, const void *ctx, int cm, uint64_t *must, uint64_t *never, uint64_t *deep) {
	*must = *never = *deep = 0;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			int px = tx * 8 + x, py = ty * 8 + y;
			int mat = floor_px(g, floor, ctx, px, py);
			bool in = mat != 0;
			bool inner = in, near = in;
			for (int k = 0; k < 8; ++k) {
				static const int off[8][2] = { { -IN, 0 }, { IN, 0 }, { 0, -IN / 2 }, { 0, IN / 2 },
					{ -OUT, 0 }, { OUT, 0 }, { 0, -OUT / 2 }, { 0, OUT / 2 } };
				bool f = floor_px(g, floor, ctx, px + off[k][0], py + off[k][1]) != 0;
				if (k < 4) inner &= f; else near |= f;
			}
			/* under a bottom edge: its side face, then nothing */
			bool face = false;
			for (int k = 1; !in && !face && k <= g->face - 2; ++k) face = floor_px(g, floor, ctx, px, py - k) != 0;
			for (int k = OUT / 2 + 1; !near && k <= g->hang + OUT; ++k) near = floor_px(g, floor, ctx, px, py - k) != 0;
			uint64_t bit = 1ull << (y * 8 + x);
			if (inner || face) *must |= bit;
			else if (!near) *never |= bit;
			if (inner && mat == cm && floor_px(g, floor, ctx, px - DEEP, py) == cm && floor_px(g, floor, ctx, px + DEEP, py) == cm &&
				floor_px(g, floor, ctx, px, py - DEEP / 2) == cm && floor_px(g, floor, ctx, px, py + DEEP / 2) == cm) *deep |= bit;
		}
}

static int misses(uint64_t mask, uint64_t must, uint64_t never) {
	return __builtin_popcountll(must & ~mask) + __builtin_popcountll(never & mask);
}

/* ---- learning ---- */

/* An opaque tile of one colour: filler the original hides under floor */
static bool flat_tile(const AreaSrc *a, int tx, int ty) {
	int W = a->tw * 8;
	uint32_t c = a->px[(size_t)(ty * 8) * W + tx * 8];
	if (!(c >> 24)) return false;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x)
			if (a->px[(size_t)(ty * 8 + y) * W + tx * 8 + x] != c) return false;
	return true;
}

/* the pixels tile (tx, ty) draws: its front layer where the map holds the
 * background too */
static uint64_t drawn(const AreaSrc *a, bool bg_in_map, int tx, int ty, uint16_t px[64]) {
	int W = a->tw * 8;
	uint64_t m = 0;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			size_t i = (size_t)(ty * 8 + y) * W + tx * 8 + x;
			uint32_t c = a->px[i];
			px[y * 8 + x] = (uint16_t)((c >> 19 & 31) | (c >> 11 & 31) << 5 | (c >> 3 & 31) << 10);
			if (bg_in_map ? a->front[i] : c >> 24) m |= 1ull << (y * 8 + x);
		}
	return m;
}

/* How the map draws its floor: how far below the panels' top edges it
 * starts (the most common distance to the first drawn pixel under an edge)
 * how tall the side face under a bottom edge is (the most common run of
 * drawn pixels there) and how far below it the map still draws often (legs
 * and pedestals: the longest run seen at least a quarter as often). */
static void calibrate(const AreaSrc *a, const Src *src, TileGrid *g) {
	int top[9] = { 0 }, face[FACE_MAX + 1] = { 0 }, W = a->tw * 8, H = a->th * 8;
	g->dv = g->face = g->hang = 0;
	for (int x = 0; x < W; ++x)
		for (int y = 1; y + 8 < H; ++y) {
			bool above = floor_px(g, src_floor, src, x, y - 1), here = floor_px(g, src_floor, src, x, y);
			if (!above && here && !(a->px[(size_t)(y - 1) * W + x] >> 24))
				for (int d = 0; d <= 8; ++d)
					if (a->px[(size_t)(y + d) * W + x] >> 24) { top[d]++; break; }
			if (above && !here) {
				int d = 0;
				while (d < FACE_MAX && y + d < H && a->px[(size_t)(y + d) * W + x] >> 24) ++d;
				face[d]++;
			}
		}
	for (int d = 1; d <= 8; ++d) if (top[d] > top[g->dv]) g->dv = d;
	/* (edges with nothing drawn under them, beside panels of other styles,
	 * say nothing of the faces) */
	int run = g->dv + 1, deep = 0;
	for (int d = g->dv + 1; d < FACE_MAX; ++d) if (face[d] > face[run]) run = d;
	for (int d = g->dv + 1; d < FACE_MAX; ++d) if (4 * face[d] >= face[run]) deep = d;
	g->face = run > g->dv ? run - g->dv : 0;
	g->hang = deep > g->dv ? deep - g->dv : 0;
	/* legs and pedestals, not the faces of floors raised far above */
	if (g->hang > g->face + HANG_BELOW_FACE) g->hang = g->face + HANG_BELOW_FACE;
}

static int cmp_cand(const void *a, const void *b) {
	const TileCand *x = a, *y = b;
	if (x->key != y->key) return x->key < y->key ? -1 : 1;
	if (x->e0 != y->e0) return x->e0 < y->e0 ? -1 : 1;
	return (x->e1 > y->e1) - (x->e1 < y->e1);
}

static int cmp_count(const void *a, const void *b) {
	const TileCand *x = a, *y = b;
	if (x->key != y->key) return x->key < y->key ? -1 : 1;
	return (x->count < y->count) - (x->count > y->count);
}

/* Per material and phase, the looks of floor with the same floor all
 * around, common enough. */
static void find_plain(TileBook *b) {
	memset(b->nplain, 0, sizeof b->nplain);
	for (int i = 0; i < b->n; ++i) {
		const TileCand *c = &b->cand[i];
		int m = KEY_A(c->key) == 0x1FF ? 0 : KEY_B(c->key) == 0x1FF ? 1 : -1;
		if (m < 0) continue;
		int phase = KEY_PHASE(c->key), *np = &b->nplain[m][phase];
		/* candidates come most common first */
		if (*np < TILE_PLAIN && (!*np || c->count * PLAIN_SHARE >= b->cand[b->plain[m][phase][0]].count))
			b->plain[m][phase][(*np)++] = i;
	}
}

void tiles_learn(const AreaSrc *a, uint16_t styles, uint16_t walk_styles, bool bg_in_map, TileBook *out) {
	memset(out, 0, sizeof *out);
	Src src = { a, styles, walk_styles, bg_in_map, malloc(SPAN * SPAN) };
	memset(src.state, -1, SPAN * SPAN);
	TileGrid g = { a->tw, a->th, a->ex, a->ey, 0, 0, 0 };
	calibrate(a, &src, &g);
	out->dv = g.dv;
	out->face = g.face;
	out->hang = g.hang;
	TileCand *c = malloc(((size_t)a->tw * a->th + 1) * sizeof *c);
	size_t n = 0;
	for (int ty = 0; ty < a->th; ++ty)
		for (int tx = 0; tx < a->tw; ++tx) {
			int phase, A, B;
			tile_class(&g, tx, ty, &phase, &A, &B);
			unsigned oa = 0, ob = 0;
			bool mixed = false;
			for (int k = 0; k < 9; ++k) {
				int st = src_panel(&src, A + k % 3 - 1, B + k / 3 - 1);
				if (st == OTHER) mixed = true;
				if (st == TILE_A) oa |= 1u << k;
				if (st == TILE_B) ob |= 1u << k;
			}
			if (mixed || !(oa | ob)) continue;
			/* off the floor, filler would show as a hole */
			if (!((oa | ob) & 0x10) && flat_tile(a, tx, ty)) continue;
			TileCand *t = &c[n];
			uint64_t must, never, deep;
			t->mask = drawn(a, bg_in_map, tx, ty, t->px);
			expect(&g, tx, ty, src_floor, &src, TILE_A, &must, &never, &deep);
			if (misses(t->mask, must, never) > SLACK) continue;   /* made for something else here */
			size_t i = (size_t)ty * a->tw + tx;
			t->key = KEY(phase, oa, ob);
			t->e0 = a->tile[0][i];
			/* where the map holds the background, its back layer is that
			 * background's pieces, not floor */
			t->e1 = a->layers > 1 && !bg_in_map ? a->tile[1][i] : 0;
			t->count = 1;
			++n;
		}
	/* one entry per pair, counted */
	qsort(c, n, sizeof *c, cmp_cand);
	int nu = 0;
	for (size_t i = 0; i < n;) {
		size_t j = i;
		while (j < n && c[j].key == c[i].key && c[j].e0 == c[i].e0 && c[j].e1 == c[i].e1) ++j;
		c[nu] = c[i];
		c[nu++].count = (uint32_t)(j - i);
		i = j;
	}
	qsort(c, (size_t)nu, sizeof *c, cmp_count);
	out->cand = c;
	out->n = nu;
	find_plain(out);
	for (int i = 0; i < out->n; ++i) out->joins += KEY_A(out->cand[i].key) && KEY_B(out->cand[i].key);
	free(src.state);
}

void tiles_free(TileBook *b) {
	free(b->cand);
	memset(b, 0, sizeof *b);
}

/* ---- picking ---- */

static int first_of(const TileBook *b, uint32_t key) {
	int lo = 0, hi = b->n;
	while (lo < hi) {
		int mid = (lo + hi) / 2;
		if (b->cand[mid].key < key) lo = mid + 1; else hi = mid;
	}
	return lo;
}

/* How different two neighbourhoods are around a tile at `phase`: floor
 * against no floor counts most at the centre and the sides nearest the
 * tile, then the nearest corner; the other material half as much, except
 * at the centre, which should keep its own. */
/* The panel straight above on screen (A + 1, B - 1): where side faces are
 * taller than half a panel, its face hangs over this one's top. */
#define ABOVE_BIT (1u << 2)
#define TALL_FACE 16

static int distance(int phase, bool tall, unsigned a1, unsigned b1, unsigned a2, unsigned b2) {
	unsigned near = nearest(phase, false), corner = nearest(phase, true) & ~near;
	if (tall) near |= ABOVE_BIT;
	int d = 0;
	for (int k = 0; k < 9; ++k) {
		unsigned bit = 1u << k;
		int w = near & bit ? 4 : corner & bit ? 2 : 1;
		bool f1 = (a1 | b1) & bit, f2 = (a2 | b2) & bit;
		if (f1 != f2) d += 2 * w;
		else if (f1 && ((a1 ^ a2) & bit)) d += k == 4 ? 6 : w;
	}
	return d;
}

/* Pixels well inside material m's floor where `c` looks like none of its
 * plain looks. */
static int unplain(const TileBook *b, int m, int phase, const TileCand *c, uint64_t deep) {
	if (!deep || !b->nplain[m][phase]) return 0;
	int best = 64;
	for (int k = 0; k < b->nplain[m][phase]; ++k) {
		const TileCand *p = &b->cand[b->plain[m][phase][k]];
		int n = 0;
		for (int i = 0; i < 64; ++i)
			if (deep >> i & 1 && c->px[i] != p->px[i]) ++n;
		if (n < best) best = n;
	}
	return best;
}

/* The best pair of `books` for a tile of class (phase, oa, ob) at (tx, ty):
 * the nearest neighbourhood seen whose tile fits there, most common first,
 * preferring those with the same floors on the panels nearest the tile;
 * failing that, the least bad. Only pairs without a back tile when `single`. */
static const TileCand *best(const TileBook *books, int nbooks, const TileGrid *g, int tx, int ty, int phase,
	unsigned oa, unsigned ob, TileFloor floor, const void *ctx, bool single) {
	int cm = ob & 0x10 ? TILE_B : TILE_A;   /* the centre panel's material */
	uint64_t must, never, deep;
	expect(g, tx, ty, floor, ctx, cm, &must, &never, &deep);
	int allowed = __builtin_popcountll(deep) / 8;
	const TileCand *fit = NULL, *same = NULL, *any = NULL;
	int fit_d = INT_MAX, same_d = INT_MAX, any_score = INT_MAX, fit_k = -1, same_k = -1;
	unsigned near = nearest(phase, true);
	for (int k = 0; k < nbooks; ++k) {
		const TileBook *b = &books[k];
		for (int i = first_of(b, KEY(phase, 0, 0)); i < b->n && KEY_PHASE(b->cand[i].key) == phase; ++i) {
			const TileCand *c = &b->cand[i];
			if (single && c->e1) continue;
			int d = distance(phase, g->face > TALL_FACE, oa, ob, KEY_A(c->key), KEY_B(c->key)), m = misses(c->mask, must, never);
			int u = unplain(b, cm - 1, phase, c, deep);
			/* ties go to the first book with the class (the area's own map
			 * before its others), so a floor keeps one look */
			if (m <= SLACK && u <= allowed && (d < fit_d || (d == fit_d && k == fit_k && c->count > fit->count))) { fit = c; fit_d = d; fit_k = k; }
			/* the other floor's edge must not come along where only one is */
			bool alike = !((oa ^ KEY_A(c->key)) & (oa | ob) & (KEY_A(c->key) | KEY_B(c->key)) & near);
			if (alike && m <= SLACK && u <= allowed && (d < same_d || (d == same_d && k == same_k && c->count > same->count))) { same = c; same_d = d; same_k = k; }
			if (m + u + 4 * d < any_score) { any = c; any_score = m + u + 4 * d; }
		}
	}
	if (same) fit = same;
	return fit ? fit : any;
}

/* One material of a floor, the other taken for void. */
typedef struct { TileFloor floor; const void *ctx; int keep; } Only;

static int only(int A, int B, const void *ctx) {
	const Only *o = ctx;
	int m = o->floor(A, B, o->ctx);
	return m == o->keep ? m : TILE_VOID;
}

bool tiles_pick(const TileBook *books, int nbooks, const TileGrid *g, int tx, int ty,
	TileFloor floor, const void *ctx, uint16_t *e0, uint16_t *e1) {
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	unsigned oa, ob;
	occupancy(floor, ctx, A, B, &oa, &ob);
	if (!(oa | ob)) return false;
	int joins = 0;
	for (int k = 0; k < nbooks; ++k) joins += books[k].joins;
	if (oa && ob && !joins) {
		/* the original never joins its two floors (Green's planks reach its
		 * raised grass by ramps): the walkway's end over the platform's edge,
		 * each drawn as if the other were not there, on the two layers */
		Only oa_only = { floor, ctx, TILE_A }, ob_only = { floor, ctx, TILE_B };
		const TileCand *pa = best(books, nbooks, g, tx, ty, phase, oa, 0, only, &oa_only, true);
		const TileCand *pb = best(books, nbooks, g, tx, ty, phase, 0, ob, only, &ob_only, true);
		if (pb && !pb->mask) pb = NULL;
		if (pa && !pa->mask) pa = NULL;
		if (pa || pb) {
			*e0 = pb ? pb->e0 : pa->e0;
			*e1 = pb && pa ? pa->e0 : 0;
			return true;
		}
	}
	const TileCand *c = best(books, nbooks, g, tx, ty, phase, oa, ob, floor, ctx, false);
	if (!c) return false;
	*e0 = c->e0;
	*e1 = c->e1;
	return true;
}
