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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IN    2    /* pixels inside the floor's edge that must be drawn */
#define OUT   3    /* pixels beyond it that may be */
#define SLACK 2    /* stray pixels a tile may have */
#define DEEP  10   /* pixels inside the floor's edge where it must look plain */
#define HANG_BELOW_FACE 12   /* how far past a face legs may hang */
#define FACE_SOLID 12   /* pixels of a side face that must be drawn */
#define FACE_MAX 64     /* the tallest side face measured (the story comps' run to 30 and more) */
#define PLAIN_SHARE 8   /* a plain look is seen at least 1/8 as often as the most common */

static int floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

/* ---- the source's panels ---- */

/* The hue bucket (0-11, 12 grey) of the floor around map pixel (cx, cy). */
static int style_at(const AreaSrc *a, int cx, int cy) {
	long r = 0, g = 0, b = 0, n = 0;
	int W = a->tw * 8, H = a->th * 8;
	for (int y = cy - 4; y < cy + 4; ++y)
		for (int x = cx - 8; x < cx + 8; ++x) {
			if (x < 0 || y < 0 || x >= W || y >= H) continue;
			uint32_t c = area_src_floor_px(a, (size_t)y * W + x);
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
	uint32_t styles;
	uint16_t walk_styles, skip_styles;
	bool bg_in_map;
	int8_t *state;   /* SPAN x SPAN panel states, -1 until measured */
	int8_t *drawn;   /* ... whether each is floor of this view at all */
	bool inner_walls;   /* rings of walls inside the floor ring no holes (TILES_INNER_WALLS) */
} Src;

/* Whether panel (A, B) is floor of this view at all. */
static bool panel_drawn(const Src *s, int A, int B) {
	const AreaSrc *a = s->a;
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	/* floor of another height is drawn elsewhere: learned in its own view */
	if (a->hz && area_src_height(a, X, Y) != a->level) return 0;
	/* nor is a hole in it, however much of the faces around hangs over it
	 * (but for what the walls keep MegaMan off, on floor drawn whole) */
	int rings = area_src_rings(a, X, Y);
	if (rings >= 0 && !(rings & 1) && !(s->inner_walls && rings >= 2)) return 0;
	int px = area_px(a->tw, X, Y), py = area_py(a->th, X, Y);
	int W = a->tw * 8, H = a->th * 8;
	/* its middle and four points around it drawn: a panel, not the legs a
	 * computer's platforms hang into the gaps (where the background is part
	 * of the map, floor is what the first layer draws) */
	static const int probe[5][2] = { { 0, 0 }, { -12, 0 }, { 12, 0 }, { 0, -6 }, { 0, 6 } };
	for (int k = 0; k < 5; ++k) {
		int x = px + probe[k][0], y = py + probe[k][1];
		if (x < 0 || y < 0 || x >= W || y >= H || !(a->px[(size_t)y * W + x] >> 24)) return 0;
		if (s->bg_in_map && !(a->px0[(size_t)y * W + x] >> 24)) return 0;
	}
	return 1;
}

static bool drawn_cached(const Src *s, int A, int B) {
	int i = A + SPAN / 2, j = B + SPAN / 2;
	if (i < 0 || j < 0 || i >= SPAN || j >= SPAN) return panel_drawn(s, A, B);
	int8_t *d = &s->drawn[j * SPAN + i];
	if (*d < 0) *d = (int8_t)panel_drawn(s, A, B);
	return *d;
}

static int measure_panel(const Src *s, int A, int B) {
	if (!drawn_cached(s, A, B)) return 0;
	if (s->styles & TILES_BY_SHAPE) {
		/* platform floor lies in a 2 x 2 block of floor, walkways do not;
		 * a platform's edge, its rim, is walkway floor too */
		bool block = false;
		for (int db = -1; db <= 0; ++db)
			for (int da = -1; da <= 0; ++da)
				block |= drawn_cached(s, A + da, B + db) && drawn_cached(s, A + da + 1, B + db) &&
					drawn_cached(s, A + da, B + db + 1) && drawn_cached(s, A + da + 1, B + db + 1);
		if (!block) return TILE_B;
		for (int db = -1; db <= 1; ++db)
			for (int da = -1; da <= 1; ++da)
				if (!drawn_cached(s, A + da, B + db)) return TILE_B;
		return TILE_A;
	}
	const AreaSrc *a = s->a;
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	int st = style_at(a, area_px(a->tw, X, Y), area_py(a->th, X, Y));
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

/* The source's pads: panels of small platforms (at most PAD_PANELS in 2 x 2
 * blocks, joined to the rest by 1-wide bridges at most). SPAN x SPAN. */
#define PAD_PANELS 12
#define PAD_LOOK 8      /* how far a tile of the wrong look (pad or not) is */

static uint8_t *find_pads(const Src *s) {
	uint8_t *block = calloc(SPAN * SPAN, 1), *pad = calloc(SPAN * SPAN, 1);
	int H = SPAN / 2;
	for (int j = 0; j + 1 < SPAN; ++j)
		for (int i = 0; i + 1 < SPAN; ++i)
			if (src_panel(s, i - H, j - H) && src_panel(s, i + 1 - H, j - H) && src_panel(s, i - H, j + 1 - H) && src_panel(s, i + 1 - H, j + 1 - H))
				block[j * SPAN + i] = block[j * SPAN + i + 1] = block[(j + 1) * SPAN + i] = block[(j + 1) * SPAN + i + 1] = 1;
	static int q[SPAN * SPAN];
	for (int start = 0; start < SPAN * SPAN; ++start) {
		if (block[start] != 1) continue;
		int n = 0, h = 0;
		q[n++] = start;
		block[start] = 2;
		while (h < n) {
			int c = q[h++], x = c % SPAN, y = c / SPAN;
			static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
			for (int k = 0; k < 4; ++k) {
				int nx = x + d[k][0], ny = y + d[k][1];
				if (nx < 0 || ny < 0 || nx >= SPAN || ny >= SPAN || block[ny * SPAN + nx] != 1) continue;
				block[ny * SPAN + nx] = 2;
				q[n++] = ny * SPAN + nx;
			}
		}
		if (n <= PAD_PANELS)
			for (int k = 0; k < n; ++k) pad[q[k]] = 1;
	}
	free(block);
	return pad;
}

TileStats tiles_stats;

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
		int m = TILE_MATERIAL(floor(A + k % 3 - 1, B + k / 3 - 1, ctx));
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
	return TILE_MATERIAL(floor(floordiv(X4 - 4 * g->ex, 128), floordiv(Y4 - 4 * g->ey, 128), ctx));
}

/* The pixels of tile (tx, ty) that must be drawn (inside the floor and on
 * the side faces under it), those that must not (away from the floor and
 * what hangs under it, but for an outline a pixel beside a face) and those
 * well inside floor of material `cm`. */
#define WIN (8 + 2 * OUT)   /* the columns a tile's pixels look at */
#define FACE_SIDE 1         /* pixels beside a side face a tile may draw */
#define FACE_TALL 32        /* taller faces are a raised floor's, down to the ground */
static void expect(const TileGrid *g, int tx, int ty, TileFloor floor, const void *ctx, int cm, uint64_t *must, uint64_t *never, uint64_t *deep) {
	*must = *never = *deep = 0;
	/* (only its top need be solid: CopyBot's pods leave gaps lower down) */
	int solid = g->face - 2 < FACE_SOLID ? g->face - 2 : FACE_SOLID;
	int hangs = g->hang + OUT, reach = hangs > solid ? hangs : solid;
	/* (beside a raised view's faces, which reach down to the ground, their
	 * tops alone; and no deeper than the faces of the map being drawn) */
	int side = g->face <= FACE_TALL ? g->face : solid;
	if (g->side && g->side < side) side = g->side;
	/* per pixel and the OUT columns beside the tile, how far up the nearest
	 * floor is: 0 on the floor, past `reach` none */
	int up[8][WIN];
	for (int c = 0; c < WIN; ++c) {
		int px = tx * 8 - OUT + c, last = INT_MIN / 2;
		for (int py = ty * 8 - reach; py < ty * 8 + 8; ++py) {
			if (floor_px(g, floor, ctx, px, py)) last = py;
			if (py >= ty * 8) up[py - ty * 8][c] = py - last;
		}
	}
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			int px = tx * 8 + x, py = ty * 8 + y, a = up[y][x + OUT];
			bool in = a == 0, inner = in, near = false;
			for (int k = 0; k < 4; ++k) {
				static const int off[4][2] = { { -IN, 0 }, { IN, 0 }, { 0, -IN / 2 }, { 0, IN / 2 } };
				inner &= floor_px(g, floor, ctx, px + off[k][0], py + off[k][1]) != 0;
			}
			/* under a bottom edge: its side face, then nothing */
			bool face = !in && a <= solid;
			/* near the floor or under it (its faces, legs and pedestals), and
			 * beside its faces: the originals' outlines stand a pixel beyond a
			 * face's side at the panels' corners */
			near = a <= hangs || floor_px(g, floor, ctx, px, py + OUT / 2) != 0;
			for (int dx = -OUT; !near && dx <= OUT; ++dx) {
				int b = up[y][x + OUT + dx];
				near = b == 0 || (b <= side && dx >= -FACE_SIDE && dx <= FACE_SIDE);
			}
			uint64_t bit = 1ull << (y * 8 + x);
			if (inner || face) *must |= bit;
			else if (!near) *never |= bit;
			if (inner && floor_px(g, floor, ctx, px, py) == cm && floor_px(g, floor, ctx, px - DEEP, py) == cm && floor_px(g, floor, ctx, px + DEEP, py) == cm &&
				floor_px(g, floor, ctx, px, py - DEEP / 2) == cm && floor_px(g, floor, ctx, px, py + DEEP / 2) == cm) *deep |= bit;
		}
}

static int misses(uint64_t mask, uint64_t must, uint64_t never) {
	return __builtin_popcountll(must & ~mask) + __builtin_popcountll(never & mask);
}

bool tiles_in_front(const TileGrid *g, int tx, int ty, TileFloor fa, const void *ca, TileFloor fb, const void *cb) {
	/* votes: pixels on one floor's top within a side face's height under
	 * the other's */
	int va = 0, vb = 0;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			int px = tx * 8 + x, py = ty * 8 + y;
			bool ta = floor_px(g, fa, ca, px, py) != 0, tb = floor_px(g, fb, cb, px, py) != 0;
			if (ta == tb) continue;
			TileFloor fo = ta ? fb : fa;
			const void *co = ta ? cb : ca;
			for (int k = 1; k <= g->face; ++k)
				if (floor_px(g, fo, co, px, py - k)) { ++*(ta ? &va : &vb); break; }
		}
	if (va != vb) return va > vb;
	/* only faces meet here: the floor lower on screen in front (panel
	 * (A + k % 3 - 1, B + k / 3 - 1) lies k / 3 - k % 3 rows further down) */
	int phase, A, B, sa = 0, na = 0, sb = 0, nb = 0;
	tile_class(g, tx, ty, &phase, &A, &B);
	for (int k = 0; k < 9; ++k) {
		int down = k / 3 - k % 3;
		if (TILE_MATERIAL(fa(A + k % 3 - 1, B + k / 3 - 1, ca))) { sa += down; ++na; }
		if (TILE_MATERIAL(fb(A + k % 3 - 1, B + k / 3 - 1, cb))) { sb += down; ++nb; }
	}
	return sa * nb >= sb * na;
}

/* ---- learning ---- */

/* An opaque tile of one colour: filler the original hides under floor */
static bool flat_tile(const AreaSrc *a, int tx, int ty) {
	int W = a->tw * 8;
	uint32_t c = area_src_floor_px(a, (size_t)(ty * 8) * W + tx * 8);
	if (!(c >> 24)) return false;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x)
			if (area_src_floor_px(a, (size_t)(ty * 8 + y) * W + tx * 8 + x) != c) return false;
	return true;
}

/* The pixels tile (tx, ty) draws as the game shows it, or with `first`
 * its first layer alone (where the map holds the background too, and
 * where the second strays). */
static uint64_t drawn(const AreaSrc *a, bool first, int tx, int ty, uint16_t px[64]) {
	int W = a->tw * 8;
	const uint32_t *pic = first ? a->px0 : a->px;
	uint64_t m = 0;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			uint32_t c = pic[(size_t)(ty * 8 + y) * W + tx * 8 + x];
			px[y * 8 + x] = (uint16_t)((c >> 19 & 31) | (c >> 11 & 31) << 5 | (c >> 3 & 31) << 10);
			if (c >> 24) m |= 1ull << (y * 8 + x);
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
	if (x->e1 != y->e1) return x->e1 < y->e1 ? -1 : 1;
	return x->pad - y->pad;
}

static int cmp_u32(const void *a, const void *b) {
	uint32_t x = *(const uint32_t *)a, y = *(const uint32_t *)b;
	return (x > y) - (x < y);
}

static int cmp_count(const void *a, const void *b) {
	const TileCand *x = a, *y = b;
	if (x->key != y->key) return x->key < y->key ? -1 : 1;
	return (x->count < y->count) - (x->count > y->count);
}

/* Per material and phase, the looks of floor with the same floor all
 * around, common enough (off the pads, which have their own). */
static void find_plain(TileBook *b) {
	memset(b->nplain, 0, sizeof b->nplain);
	for (int i = 0; i < b->n; ++i) {
		const TileCand *c = &b->cand[i];
		if (c->pad) continue;
		int m = KEY_A(c->key) == 0x1FF ? 0 : KEY_B(c->key) == 0x1FF ? 1 : -1;
		if (m < 0) continue;
		int phase = KEY_PHASE(c->key), *np = &b->nplain[m][phase];
		/* candidates come most common first */
		if (*np < TILE_PLAIN && (!*np || c->count * PLAIN_SHARE >= b->cand[b->plain[m][phase][0]].count))
			b->plain[m][phase][(*np)++] = i;
	}
}

/* Whether panel (A, B) is floor whose tiles are not learned. */
static bool skipped(const Src *s, int A, int B) {
	if (!s->skip_styles || !drawn_cached(s, A, B)) return false;
	const AreaSrc *a = s->a;
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	return s->skip_styles >> style_at(a, area_px(a->tw, X, Y), area_py(a->th, X, Y)) & 1;
}

static void src_open(Src *s, const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, bool bg_in_map, bool inner_walls) {
	*s = (Src){ a, styles, walk_styles, skip_styles, bg_in_map, malloc(SPAN * SPAN), malloc(SPAN * SPAN), inner_walls };
	memset(s->state, -1, SPAN * SPAN);
	memset(s->drawn, -1, SPAN * SPAN);
}

static void src_close(Src *s) {
	free(s->state);
	free(s->drawn);
}

void tiles_learn(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, bool bg_in_map, TileBook *out) {
	memset(out, 0, sizeof *out);
	Src src;
	src_open(&src, a, styles, walk_styles, skip_styles, bg_in_map, styles & TILES_INNER_WALLS);
	uint8_t *pads = find_pads(&src);
	TileGrid g = { a->tw, a->th, a->ex, a->ey, 0, 0, 0, false };
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
				if (st == OTHER || skipped(&src, A + k % 3 - 1, B + k / 3 - 1)) mixed = true;
				if (st == TILE_A) oa |= 1u << k;
				if (st == TILE_B) ob |= 1u << k;
			}
			if (mixed || !(oa | ob)) continue;
			/* off the floor, filler would show as a hole */
			if (!((oa | ob) & 0x10) && flat_tile(a, tx, ty)) continue;
			TileCand *t = &c[n];
			uint64_t must, never, deep;
			expect(&g, tx, ty, src_floor, &src, TILE_A, &must, &never, &deep);
			/* The first layer alone where it draws the floor: what the
			 * originals set in front of their floors on the second (bridges,
			 * ornaments, spikes, floors overlapping others on screen) belongs
			 * to them, not to the floor. The second too where the floor needs
			 * it (the Judge Tree draws its panels' middles there), except where
			 * the map holds the background: there it is that background. */
			bool back = false;
			t->mask = drawn(a, true, tx, ty, t->px);
			if (misses(t->mask, must, never) > SLACK) {
				if (a->layers < 2 || bg_in_map) continue;
				t->mask = drawn(a, false, tx, ty, t->px);
				if (misses(t->mask, must, never) > SLACK) continue;   /* made for something else here */
				back = true;
			}
			size_t i = (size_t)ty * a->tw + tx;
			t->key = KEY(phase, oa, ob);
			t->e0 = a->tile[0][i];
			t->e1 = back ? a->tile[1][i] : 0;
			t->count = 1;
			t->other = 0;
			t->pad = !(styles & TILES_NO_PAD_LOOK) && A >= -SPAN / 2 && B >= -SPAN / 2 && A < SPAN / 2 && B < SPAN / 2 ? pads[(B + SPAN / 2) * SPAN + A + SPAN / 2] : 0;
			++n;
		}
	/* the panels' neighbourhoods, as legal.c asks after them (every ring of
	 * walls inside the floor a hole: what the tiles are learned from leaves
	 * the floors a layer keeps as they were) */
	Src walled = src;
	uint8_t *walled_pads = pads;
	if (src.inner_walls) {
		src_open(&walled, a, styles, walk_styles, skip_styles, bg_in_map, false);
		walled_pads = find_pads(&walled);
	}
	out->shapes = malloc(SPAN * SPAN * sizeof *out->shapes);
	out->nshapes = 0;
	for (int B = -SPAN / 2 + 1; B < SPAN / 2 - 1; ++B)
		for (int A = -SPAN / 2 + 1; A < SPAN / 2 - 1; ++A) {
			unsigned oa = 0, ob = 0;
			bool mixed = false;
			for (int k = 0; k < 9; ++k) {
				int st = src_panel(&walled, A + k % 3 - 1, B + k / 3 - 1);
				if (st == OTHER) mixed = true;
				if (st == TILE_A) oa |= 1u << k;
				if (st == TILE_B) ob |= 1u << k;
			}
			if (mixed || !(oa | ob)) continue;
			out->shapes[out->nshapes++] = TILE_SHAPE(oa, ob, walled_pads[(B + SPAN / 2) * SPAN + A + SPAN / 2]);
		}
	if (src.inner_walls) {
		src_close(&walled);
		free(walled_pads);
	}
	qsort(out->shapes, (size_t)out->nshapes, sizeof *out->shapes, cmp_u32);
	int ns = 0;
	for (int i = 0; i < out->nshapes; ++i)
		if (!ns || out->shapes[ns - 1] != out->shapes[i]) out->shapes[ns++] = out->shapes[i];
	out->nshapes = ns;
	/* one entry per pair, counted */
	qsort(c, n, sizeof *c, cmp_cand);
	int nu = 0;
	for (size_t i = 0; i < n;) {
		size_t j = i;
		while (j < n && c[j].key == c[i].key && c[j].e0 == c[i].e0 && c[j].e1 == c[i].e1 && c[j].pad == c[i].pad) ++j;
		c[nu] = c[i];
		c[nu++].count = (uint32_t)(j - i);
		i = j;
	}
	qsort(c, (size_t)nu, sizeof *c, cmp_count);
	out->cand = c;
	out->n = nu;
	find_plain(out);
	for (int i = 0; i < out->n; ++i) out->joins += KEY_A(out->cand[i].key) && KEY_B(out->cand[i].key);
	src_close(&src);
	free(pads);
}

void tiles_src_text(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, bool bg_in_map, FILE *f) {
	Src src;
	src_open(&src, a, styles, walk_styles, skip_styles, bg_in_map, styles & TILES_INNER_WALLS);
	uint8_t *pads = find_pads(&src);
	int H = SPAN / 2, a0 = H, a1 = -H, b0 = H, b1 = -H;
	for (int B = -H; B < H; ++B)
		for (int A = -H; A < H; ++A)
			if (src_panel(&src, A, B)) {
				if (A < a0) a0 = A;
				if (A > a1) a1 = A;
				if (B < b0) b0 = B;
				if (B > b1) b1 = B;
			}
	/* rows down the grid's y (world -X), columns along its x (world +Y), as
	 * a generated layer's */
	for (int A = a1; A >= a0; --A) {
		for (int B = b0; B <= b1; ++B) {
			int st = src_panel(&src, A, B);
			bool pad = pads[(B + H) * SPAN + A + H];
			/* (floor of another style by its hue bucket: 0-9, X, Y, G grey) */
			int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B, hue = st == OTHER ? style_at(a, area_px(a->tw, X, Y), area_py(a->th, X, Y)) : 0;
			fputc(st == TILE_A ? (pad ? 'P' : 'a') : st == TILE_B ? (pad ? 'Q' : 'b') : st == OTHER ? "0123456789XYG"[hue] : '.', f);
		}
		fputc('\n', f);
	}
	src_close(&src);
	free(pads);
}

void tiles_free(TileBook *b) {
	free(b->cand);
	free(b->shapes);
	memset(b, 0, sizeof *b);
}

void tiles_colours(const TileBook *b, uint8_t seen[TILE_COLOURS]) {
	for (int i = 0; i < b->n; ++i)
		for (int p = 0; p < 64; ++p)
			if (b->cand[i].mask >> p & 1) seen[b->cand[i].px[p] & 0x7FFF] = 1;
}

/* The first n pairs of `b` kept (still in order): the plain looks and joins
 * of those left. */
static void keep_first(TileBook *b, int n) {
	if (n == b->n) return;
	b->n = n;
	find_plain(b);
	b->joins = 0;
	for (int i = 0; i < b->n; ++i) b->joins += KEY_A(b->cand[i].key) && KEY_B(b->cand[i].key);
}

void tiles_other_colours(TileBook *b, const uint8_t seen[TILE_COLOURS], bool drop) {
	int n = 0;
	for (int i = 0; i < b->n; ++i) {
		TileCand *c = &b->cand[i];
		c->other = 0;
		for (int p = 0; p < 64 && !c->other; ++p) c->other = c->mask >> p & 1 && !seen[c->px[p] & 0x7FFF];
		if (!drop || !c->other) b->cand[n++] = *c;
	}
	keep_first(b, n);
}

void tiles_drop_pads(TileBook *b) {
	int n = 0;
	for (int i = 0; i < b->n; ++i)
		if (!b->cand[i].pad) b->cand[n++] = b->cand[i];
	keep_first(b, n);
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

/* The hue bucket (0-11, 12 grey) of a BGR555 colour. */
static int hue_of(uint16_t c) {
	int r = c & 31, g = c >> 5 & 31, b = c >> 10 & 31;
	int mx = r > g ? (r > b ? r : b) : (g > b ? g : b), mn = r < g ? (r < b ? r : b) : (g < b ? g : b);
	if (!mx || (mx - mn) * 4 <= mx) return 12;
	float d = (float)(mx - mn), h = mx == r ? (g - b) / d : mx == g ? 2 + (b - r) / d : 4 + (r - g) / d;
	h /= 6;
	if (h < 0) h += 1;
	int k = (int)(h * 12);
	return k > 11 ? 11 : k;
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

bool tiles_measure;
int tiles_pick_off, tiles_pick_why, tiles_pick_other;

/* The panel under map pixel (px, py) at z 0, as bit k of the 3x3 around
 * (A, B); -1 beyond them. */
static int panel_k(const TileGrid *g, int px, int py, int A, int B) {
	int u2 = 2 * px + 1 - g->tw * 8, v2 = 2 * (py - g->dv) + 1 - g->th * 8;
	int X4 = u2 - 2 * v2, Y4 = u2 + 2 * v2;
	int dA = floordiv(X4 - 4 * g->ex, 128) - A, dB = floordiv(Y4 - 4 * g->ey, 128) - B;
	return dA < -1 || dA > 1 || dB < -1 || dB > 1 ? -1 : dA + 1 + 3 * (dB + 1);
}

/* Per pixel of a tile, the panels straight up from it (bit k of the 3x3
 * around the tile's own, 0xFF beyond them) and whether it would show each
 * one's top, its side face or what hangs under that. */
#define COL_MAX 4
typedef struct { uint8_t n, open, k[COL_MAX], part[COL_MAX]; } PixCol;

/* (the same for every tile of a phase in a map: kept per phase) */
static const PixCol *columns(const TileGrid *g, int tx, int ty) {
	static PixCol cache[64][64];
	static uint64_t have;
	static TileGrid of;
	if (g->tw != of.tw || g->th != of.th || g->ex != of.ex || g->ey != of.ey || g->dv != of.dv || g->face != of.face || g->hang != of.hang) {
		of = *g;
		have = 0;
	}
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	PixCol *col = cache[phase];
	if (have >> phase & 1) return col;
	have |= 1ull << phase;
	for (int i = 0; i < 64; ++i) {
		PixCol *c = &col[i];
		int last = -2;
		c->n = 0;
		c->open = 0;
		for (int d = 0; d <= g->face + g->hang; ++d) {
			int k = panel_k(g, tx * 8 + i % 8, ty * 8 + i / 8 - d, A, B);
			if (k == last) continue;
			if (c->n == COL_MAX) { c->open = 1; break; }
			last = k;
			c->k[c->n] = k < 0 ? 0xFF : (uint8_t)k;
			c->part[c->n++] = d == 0 ? 0 : d <= g->face ? 1 : 2;
			if (k < 0) break;
		}
	}
	return col;
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

/* The pixels where floors (a, b) show otherwise than `mine`. */
static int picture_diff(const PixCol col[64], const uint8_t mine[64], unsigned a, unsigned b) {
	int n = 0;
	for (int i = 0; i < 64; ++i) {
		if (mine[i] == 0xFF) continue;
		uint8_t t = col_code(&col[i], a, b);
		n += t != 0xFF && t != mine[i];
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
	int n = __builtin_popcountll(((upper >> 56) ^ lower) & 0xFF) - CUT_EDGE;
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
	uint64_t must = 0, never = 0, deep = 0;
	int allowed = 0;
	TileGrid gk = *g;
	gk.face = -1;
	gk.side = g->face;   /* (outlines beside its faces as deep as this map's) */
	const TileCand *fit = NULL, *any = NULL;
	int fit_d = INT_MAX, any_score = INT_MAX, fit_k = -1, fit_p = 0, any_p = 0;
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
	const PixCol *col = columns(g, tx, ty);
	uint8_t mine[64];
	for (int i = 0; i < 64; ++i) mine[i] = col_code(&col[i], oa, ob);
	uint32_t pic_key = UINT32_MAX;
	int pic = 0;
	for (int k = 0; k < nbooks; ++k) {
		const TileBook *b = &books[k];
		int face = b->face > FACE_TALL ? g->face : b->face, hang = b->face > FACE_TALL ? g->hang : b->hang;
		if (face != gk.face || hang != gk.hang) {
			gk.face = face; gk.hang = hang;
			expect(&gk, tx, ty, floor, ctx, cm, &must, &never, &deep);
			allowed = __builtin_popcountll(deep) / 8;
		}
		for (int i = first_of(b, KEY(phase, 0, 0)); i < b->n && KEY_PHASE(b->cand[i].key) == phase; ++i) {
			const TileCand *c = &b->cand[i];
			/* (a mirror image's empty entry is tile 0 flipped) */
			if ((single && c->e1) || (g->single && (c->e0 & 0x3FF) && (c->e1 & 0x3FF))) continue;
			int d = distance(phase, b->face > TALL_FACE, oa, ob, KEY_A(c->key), KEY_B(c->key)), m = misses(c->mask, must, never);
			/* a pad in the pads' look, other floor not */
			if (c->pad != pad) d += PAD_LOOK;
			if (c->other && !field) d += OTHER_COST;
			/* (what follows only adds: a pair that cannot win is left) */
			if (!(m <= SLACK && d <= fit_d) && m + 4 * d >= any_score) continue;
			/* (a pad's middle is not the usual floor: it has its own look; nor
			 * is a rimmed floor's edge) */
			int u = pad || (g->rimmed && (oa | ob) != 0x1FF) ? 0 : unplain(b, cm - 1, phase, c, deep);
			/* a neighbour it never meets in the originals: a seam */
			if (seams) d += tiles_trouble(seams, look_of(c), c->mask, n);
			/* what it shows otherwise (once per class: pairs come by class) */
			if (c->key != pic_key) {
				pic_key = c->key;
				pic = picture_diff(col, mine, KEY_A(c->key), KEY_B(c->key));
			}
			d += PICTURE_COST * pic;
			if (tiles_measure && tile_debug(tx, ty))
				fprintf(stderr, "tile %d,%d phase %d ours a %03x b %03x: book %d e0 %04x e1 %04x count %u pad %d other %d key a %03x b %03x: d %d (picture %d) misses %d unplain %d of %d\n",
					tx, ty, phase, oa, ob, k, c->e0, c->e1, c->count, c->pad, c->other, KEY_A(c->key), KEY_B(c->key), d, pic, m, u, allowed);
			/* ties go to the first book with the class (the area's own map
			 * before its others), so a floor keeps one look (the other
			 * floor's edge coming along where only one is costs its pixels) */
			if (m <= SLACK && u <= allowed && (d < fit_d || (d == fit_d && k == fit_k && c->count > fit->count))) { fit = c; fit_d = d; fit_k = k; fit_p = pic; }
			if (m + u + 4 * d < any_score) { any = c; any_score = m + u + 4 * d; any_p = pic; }
		}
	}
	*dist = fit ? fit_d : -1;
	const TileCand *c = fit ? fit : any;
	*off = !tiles_measure ? TILE_EXACT : c ? off_by(oa, ob, c, fit ? fit_p : any_p) : TILE_OFF_NEAR;
	if (*off != TILE_OFF_NEAR) return c;
	/* (measuring) why: how far the pairs showing what it shows got */
	int why = TILE_WHY_UNSEEN;
	gk.face = -1;
	for (int k = 0; k < nbooks; ++k) {
		const TileBook *b = &books[k];
		int face = b->face > FACE_TALL ? g->face : b->face, hang = b->face > FACE_TALL ? g->hang : b->hang;
		if (face != gk.face || hang != gk.hang) {
			gk.face = face; gk.hang = hang;
			expect(&gk, tx, ty, floor, ctx, cm, &must, &never, &deep);
			allowed = __builtin_popcountll(deep) / 8;
		}
		for (int i = first_of(b, KEY(phase, 0, 0)); i < b->n && KEY_PHASE(b->cand[i].key) == phase; ++i) {
			const TileCand *t = &b->cand[i];
			if ((single && t->e1) || (g->single && (t->e0 & 0x3FF) && (t->e1 & 0x3FF))) continue;
			if (picture_diff(col, mine, KEY_A(t->key), KEY_B(t->key)) > OFF_EDGE_PX) continue;
			int w = TILE_WHY_PIXELS;
			if (misses(t->mask, must, never) <= SLACK) {
				int u = pad || (g->rimmed && (oa | ob) != 0x1FF) ? 0 : unplain(b, cm - 1, phase, t, deep);
				w = u <= allowed ? TILE_WHY_RANKED : TILE_WHY_PLAIN;
			}
			if (w > why) why = w;
		}
	}
	tiles_pick_why = why;
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

bool tiles_pick(const TileBook *books, int nbooks, const TileGrid *g, int tx, int ty,
	TileFloor floor, const void *ctx, const TileSeams *seams, const TileNeighbours *n,
	uint16_t *e0, uint16_t *e1, uint32_t *look, uint64_t *mask) {
	int phase, A, B, dist, off, off2;
	tiles_pick_off = TILE_EXACT;
	tiles_pick_why = TILE_WHY_NONE;
	tiles_pick_other = 0;
	tile_class(g, tx, ty, &phase, &A, &B);
	unsigned oa, ob;
	occupancy(floor, ctx, A, B, &oa, &ob);
	if (!(oa | ob)) return false;
	bool pad = floor(A, B, ctx) & TILE_PAD;
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
