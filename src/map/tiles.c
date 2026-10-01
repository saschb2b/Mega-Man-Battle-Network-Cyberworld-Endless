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

/* How many pixels of the top of the panel around map pixel (cx, cy) have
 * a hue in `buckets` (bit per bucket 0-11), strongly coloured. */
static int hue_pixels(const AreaSrc *a, int cx, int cy, unsigned buckets) {
	int W = a->tw * 8, H = a->th * 8, n = 0;
	for (int dy = -8; dy <= 8; ++dy)
		for (int dx = -16; dx <= 16; ++dx) {
			int x = cx + dx, y = cy + dy;
			if (2 * abs(dy) + abs(dx) > 16 || x < 0 || y < 0 || x >= W || y >= H) continue;
			uint32_t c = area_src_floor_px(a, (size_t)y * W + x);
			if (!(c >> 24)) continue;
			int r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255;
			int mx = r > g ? (r > b ? r : b) : (g > b ? g : b), mn = r < g ? (r < b ? r : b) : (g < b ? g : b);
			if (mx < 64 || (mx - mn) * 100 < 40 * mx) continue;
			float d = (float)(mx - mn), hue = mx == r ? (g - b) / d : mx == g ? 2 + (b - r) / d : 4 + (r - g) / d;
			hue /= 6;
			if (hue < 0) hue += 1;
			int k = (int)(hue * 12);
			n += buckets >> (k > 11 ? 11 : k) & 1;
		}
	return n;
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

/* (bits set: libgcc's call, on a machine without the instruction, was a
 * twentieth of making a layer) */
static inline int popcount64(uint64_t x) {
	x -= x >> 1 & 0x5555555555555555ull;
	x = (x & 0x3333333333333333ull) + (x >> 2 & 0x3333333333333333ull);
	x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0Full;
	return (int)(x * 0x0101010101010101ull >> 56);
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
static void px_panel(const TileGrid *g, int px, int py, int *A, int *B) {
	int u2 = 2 * px + 1 - g->tw * 8, v2 = 2 * (py - g->dv) + 1 - g->th * 8;
	int X4 = u2 - 2 * v2, Y4 = u2 + 2 * v2;
	*A = floordiv(X4 - 4 * g->ex, 128);
	*B = floordiv(Y4 - 4 * g->ey, 128);
}

static int floor_px_raw(const TileGrid *g, TileFloor floor, const void *ctx, int px, int py) {
	int A, B;
	px_panel(g, px, py, &A, &B);
	return TILE_MATERIAL(floor(A, B, ctx));
}

/* floor_px_raw pixel after pixel, the last panel's floor kept in `last`
 * (A, B and its floor; A INT_MIN at first): a map's calibration asks after
 * each of its pixels, and asking their panel's floor anew for each was 40%
 * of its time */
static int floor_px_on(const TileGrid *g, TileFloor floor, const void *ctx, int px, int py, int last[3]) {
	int A, B;
	px_panel(g, px, py, &A, &B);
	if (A != last[0] || B != last[1]) {
		last[0] = A;
		last[1] = B;
		last[2] = TILE_MATERIAL(floor(A, B, ctx));
	}
	return last[2];
}

/* floor_px's answers kept per map pixel through a map's pick
 * (tiles_keep_begin): each tile's test asks after a thousand pixels, most
 * of them its neighbours', and again as the pick goes back over it */
static struct {
	int8_t *px;   /* (-1: not asked yet) */
	int x0, y0, w, h;
	TileFloor floor;
	const void *ctx;
	int tw, th, ex, ey, dv;
} kept;

/* Whether the kept pixels are this grid's and floor's. */
static bool keeps(const TileGrid *g, TileFloor floor, const void *ctx) {
	return kept.px && floor == kept.floor && ctx == kept.ctx && g->dv == kept.dv && g->ex == kept.ex && g->ey == kept.ey &&
		g->tw == kept.tw && g->th == kept.th;
}

/* floor_px where `k` says whether the kept pixels serve (a tile's test
 * asks keeps once, then a thousand times here) */
static inline int floor_px_k(const TileGrid *g, TileFloor floor, const void *ctx, bool k, int px, int py) {
	if (k) {
		unsigned x = (unsigned)(px - kept.x0), y = (unsigned)(py - kept.y0);
		if (x < (unsigned)kept.w && y < (unsigned)kept.h) {
			int8_t *m = &kept.px[y * kept.w + x];
			if (*m < 0) *m = (int8_t)floor_px_raw(g, floor, ctx, px, py);
			return *m;
		}
	}
	return floor_px_raw(g, floor, ctx, px, py);
}

static int floor_px(const TileGrid *g, TileFloor floor, const void *ctx, int px, int py) {
	return floor_px_k(g, floor, ctx, keeps(g, floor, ctx), px, py);
}

/* unplain's answers through a map's pick as well: a pair and the tile's
 * deep pixels decide them, and a map's tiles share them (the pick's passes
 * ask again besides) */
#define MEMO_SIZE (1 << 16)
#define MEMO_PROBES 8
typedef struct { const void *pair; uint64_t deep; int m, value; } PlainMemo;
static PlainMemo *plain_memo;

void tiles_keep_begin(const TileGrid *g, TileFloor floor, const void *ctx) {
	tiles_keep_end();
	plain_memo = calloc(MEMO_SIZE, sizeof *plain_memo);
	/* (the map and a margin round it: a tile looks 40 pixels up at most) */
	kept.x0 = -64;
	kept.y0 = -96;
	kept.w = g->tw * 8 + 128;
	kept.h = g->th * 8 + 160;
	if (!(kept.px = malloc((size_t)kept.w * kept.h))) return;
	memset(kept.px, 0xFF, (size_t)kept.w * kept.h);
	kept.floor = floor;
	kept.ctx = ctx;
	kept.tw = g->tw; kept.th = g->th; kept.ex = g->ex; kept.ey = g->ey; kept.dv = g->dv;
}

void tiles_keep_end(void) {
	free(kept.px);
	kept.px = NULL;
	free(plain_memo);
	plain_memo = NULL;
}

#define WIN (8 + 2 * OUT)   /* the columns a tile's pixels look at */
#define FACE_SIDE 1         /* pixels beside a side face a tile may draw */
#define FACE_TALL 32        /* taller faces are a raised floor's, down to the ground */
/* What a tile's pixel test finds whatever the faces (expect_look), then the
 * test for one book's faces and legs (expect_for): a tile's pick tests it
 * for each book's, and looking at its floor again for each was a third of
 * making a layer. */
#define NO_FLOOR (INT_MAX / 2)
typedef struct {
	int up[8][WIN];   /* per pixel and the OUT columns beside the tile, how far up the nearest floor is (NO_FLOOR: none as far as looked) */
	uint64_t inner, below, deep;
} Look;

/* how far up a tile's pixels look for floor, for faces and legs so deep */
static int reach_of(int face, int hang) {
	int solid = face - 2 < FACE_SOLID ? face - 2 : FACE_SOLID, hangs = hang + OUT;
	return hangs > solid ? hangs : solid;
}

static void expect_look(const TileGrid *g, int tx, int ty, TileFloor floor, const void *ctx, int cm, int reach, Look *l) {
	bool k = keeps(g, floor, ctx);
	int last[WIN];
	for (int c = 0; c < WIN; ++c) last[c] = -NO_FLOOR;
	for (int py = ty * 8 - reach; py < ty * 8 + 8; ++py)
		for (int c = 0; c < WIN; ++c) {
			if (floor_px_k(g, floor, ctx, k, tx * 8 - OUT + c, py)) last[c] = py;
			if (py >= ty * 8) l->up[py - ty * 8][c] = last[c] == -NO_FLOOR ? NO_FLOOR : py - last[c];
		}
	l->inner = l->below = l->deep = 0;
	for (int y = 0; y < 8; ++y)
		for (int x = 0; x < 8; ++x) {
			int px = tx * 8 + x, py = ty * 8 + y;
			uint64_t bit = 1ull << (y * 8 + x);
			if (floor_px_k(g, floor, ctx, k, px, py + OUT / 2)) l->below |= bit;
			if (l->up[y][x + OUT]) continue;   /* (not on the floor) */
			bool inner = true;
			for (int j = 0; j < 4 && inner; ++j) {
				static const int off[4][2] = { { -IN, 0 }, { IN, 0 }, { 0, -IN / 2 }, { 0, IN / 2 } };
				inner = floor_px_k(g, floor, ctx, k, px + off[j][0], py + off[j][1]) != 0;
			}
			if (!inner) continue;
			l->inner |= bit;
			if (floor_px_k(g, floor, ctx, k, px, py) == cm && floor_px_k(g, floor, ctx, k, px - DEEP, py) == cm && floor_px_k(g, floor, ctx, k, px + DEEP, py) == cm &&
				floor_px_k(g, floor, ctx, k, px, py - DEEP / 2) == cm && floor_px_k(g, floor, ctx, k, px, py + DEEP / 2) == cm) l->deep |= bit;
		}
}

/* The pixels of the tile that must be drawn (inside the floor and on the
 * side faces under it) and those that must not (away from the floor and
 * what hangs under it, but for an outline a pixel beside a face), for side
 * faces `face` deep over legs `hang` (a map drawn with faces `map_face`:
 * outlines beside them no deeper). The look reached at least as far. */
static void expect_for(const Look *l, int face, int hang, int map_face, uint64_t *must, uint64_t *never) {
	/* (only its top need be solid: CopyBot's pods leave gaps lower down) */
	int solid = face - 2 < FACE_SOLID ? face - 2 : FACE_SOLID;
	int hangs = hang + OUT, reach = reach_of(face, hang);
	/* (beside a raised view's faces, which reach down to the ground, their
	 * tops alone; and no deeper than the faces of the map being drawn) */
	int side = face <= FACE_TALL ? face : solid;
	if (map_face && map_face < side) side = map_face;
	*must = l->inner;
	*never = 0;
	for (int y = 0; y < 8; ++y) {
		/* (floor further up than this reach is none) */
		int row[WIN];
		for (int c = 0; c < WIN; ++c) row[c] = l->up[y][c] <= reach + y ? l->up[y][c] : NO_FLOOR;
		for (int x = 0; x < 8; ++x) {
			uint64_t bit = 1ull << (y * 8 + x);
			int a = row[x + OUT];
			/* under a bottom edge: its side face, then nothing */
			if (a && a <= solid) { *must |= bit; continue; }
			if (l->inner & bit) continue;
			/* near the floor or under it (its faces, legs and pedestals), and
			 * beside its faces: the originals' outlines stand a pixel beyond a
			 * face's side at the panels' corners */
			bool near = a <= hangs || (l->below & bit);
			for (int dx = -OUT; !near && dx <= OUT; ++dx) {
				int b = row[x + OUT + dx];
				near = b == 0 || (b <= side && dx >= -FACE_SIDE && dx <= FACE_SIDE);
			}
			if (!near) *never |= bit;
		}
	}
}

/* The pixels of tile (tx, ty) that must be drawn, those that must not, and
 * those well inside floor of material `cm`. */
static void expect(const TileGrid *g, int tx, int ty, TileFloor floor, const void *ctx, int cm, uint64_t *must, uint64_t *never, uint64_t *deep) {
	Look l;
	expect_look(g, tx, ty, floor, ctx, cm, reach_of(g->face, g->hang), &l);
	expect_for(&l, g->face, g->hang, g->side, must, never);
	*deep = l.deep;
}

static int misses(uint64_t mask, uint64_t must, uint64_t never) {
	return popcount64(must & ~mask) + popcount64(never & mask);
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
	for (int x = 0; x < W; ++x) {
		/* (each pixel asked after once, here, then as the next one's above) */
		int last[3] = { INT_MIN, 0, 0 };
		bool above, here = floor_px_on(g, src_floor, src, x, 0, last);
		for (int y = 1; y + 8 < H; ++y) {
			above = here;
			here = floor_px_on(g, src_floor, src, x, y, last);
			if (!above && here && !(a->px[(size_t)(y - 1) * W + x] >> 24))
				for (int d = 0; d <= 8; ++d)
					if (a->px[(size_t)(y + d) * W + x] >> 24) { top[d]++; break; }
			if (above && !here) {
				int d = 0;
				while (d < FACE_MAX && y + d < H && a->px[(size_t)(y + d) * W + x] >> 24) ++d;
				face[d]++;
			}
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
	if (s->skip_styles & SKIP_ANY_PIXEL) return hue_pixels(a, area_px(a->tw, X, Y), area_py(a->th, X, Y), s->skip_styles & 0xFFFu) >= 8;
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

/* The panels' neighbourhoods, as legal.c asks after them (every ring of
 * walls inside the floor a hole: what the tiles are learned from leaves the
 * floors a layer keeps as they were), sorted, each once, and kept at the
 * size they hold (an area kept eight books of a whole map's tiles each,
 * 30 MB for the Undernet's, and a New 3DS ran out of memory entering it). */
static void learn_shapes(const Src *src, const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, bool bg_in_map,
	uint8_t *pads, TileBook *out) {
	Src walled = *src;
	uint8_t *walled_pads = pads;
	if (src->inner_walls) {
		src_open(&walled, a, styles, walk_styles, skip_styles, bg_in_map, false);
		walled_pads = find_pads(&walled);
	}
	out->nshapes = 0;
	out->shapes = malloc(SPAN * SPAN * sizeof *out->shapes);
	for (int B = -SPAN / 2 + 1; out->shapes && B < SPAN / 2 - 1; ++B)
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
	if (src->inner_walls) {
		src_close(&walled);
		free(walled_pads);
	}
	if (!out->shapes) return;
	qsort(out->shapes, (size_t)out->nshapes, sizeof *out->shapes, cmp_u32);
	int ns = 0;
	for (int i = 0; i < out->nshapes; ++i)
		if (!ns || out->shapes[ns - 1] != out->shapes[i]) out->shapes[ns++] = out->shapes[i];
	out->nshapes = ns;
	uint32_t *shapes = realloc(out->shapes, ((size_t)ns + 1) * sizeof *out->shapes);
	if (shapes) out->shapes = shapes;
}

/* Each class's first pair: its pairs lie together, the most common first
 * (in place: pairs dropped leave no more classes). */
static void index_classes(TileBook *b) {
	b->ncls = 0;
	for (int i = 0; i < b->n; ++i)
		if (!i || b->cand[i].key != b->cand[i - 1].key) b->cls[b->ncls++] = (TileClass){ b->cand[i].key, i };
}

/* One entry per pair, counted, the most common first, kept at the size
 * they hold, as the shapes are. */
static void keep_cands(TileCand *c, size_t n, TileBook *out) {
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
	TileCand *kept_c = realloc(c, ((size_t)nu + 1) * sizeof *c);
	out->cand = kept_c ? kept_c : c;
	out->n = nu;
	int ncls = 0;
	for (int i = 0; i < nu; ++i) ncls += !i || out->cand[i].key != out->cand[i - 1].key;
	out->cls = malloc(((size_t)ncls + 1) * sizeof *out->cls);
	if (!out->cls) out->n = 0;   /* (a book of nothing) */
	index_classes(out);
}

/* ---- the floor's middle, by its period ---- */

static int first_class(const TileBook *b, uint32_t key);
static void learn_variety(TileBook *out);

#define PATCH_MAX 6        /* panels a period spans at most, per axis */
#define PATCH_SHARE 95     /* % of the middle's tiles a period must explain */
#define PATCH_MIN 24       /* tiles of the middle a book needs for one */

typedef struct { int A, B, phase, cand; } PatchSample;

static int mod_of(int v, int m) { int r = v % m; return r < 0 ? r + m : r; }

/* The share (in % of `n`) of the samples the period (pa, pb) explains: per
 * panel of the period and phase, its most common pair's; that pair in
 * `best` (pa * pb * 64 entries, -1 none). */
static int patch_fit(const PatchSample *s, int n, int pa, int pb, int16_t *best) {
	int cells = pa * pb * 64;
	static int16_t cand[PATCH_MAX * PATCH_MAX * 64][4];
	static uint16_t cnt[PATCH_MAX * PATCH_MAX * 64][4];
	for (int i = 0; i < cells; ++i) for (int k = 0; k < 4; ++k) { cand[i][k] = -1; cnt[i][k] = 0; }
	for (int i = 0; i < n; ++i) {
		int at = (mod_of(s[i].B, pb) * pa + mod_of(s[i].A, pa)) * 64 + s[i].phase, k = 0;
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
	int w = A1 - A0 + 1, h = B1 - B0 + 1, full = 0, r[4];
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
		memcpy(&out->patch[(mod_of(B, r[3]) * r[2] + mod_of(A, r[2])) * 64], &at[((B - B0) * w + A - A0) * 64], 64 * sizeof *out->patch);
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
		if (src_panel(src, A + k % 3 - 1, B + k / 3 - 1) != TILE_A || skipped(src, A + k % 3 - 1, B + k / 3 - 1)) return false;
	return true;
}

/* The pair learned for source tile i in the middle's class at `phase`, -1 none. */
static int middle_pair(const AreaSrc *a, const TileBook *out, size_t i, int phase) {
	uint32_t key = KEY(phase, 0x1FF, 0);
	for (int j = first_class(out, key); j < out->ncls && out->cls[j].key == key; ++j) {
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
		int j = first_class(out, key);
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
	if (!c) { src_close(&src); free(pads); return; }   /* (a book of nothing) */
	size_t n = 0;
	/* (the original map's pixels kept as its tiles are tested: learning an
	 * area asked after each a thousand times, three quarters of the time) */
	tiles_keep_begin(&g, src_floor, &src);
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
	tiles_keep_end();
	learn_shapes(&src, a, styles, walk_styles, skip_styles, bg_in_map, pads, out);
	keep_cands(c, n, out);
	find_plain(out);
	learn_patch(a, &src, &g, pads, out);
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
	free(b->cls);
	free(b->shapes);
	free(b->patch);
	free(b->vary);
	free(b->vary_weight);
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
	index_classes(b);
	find_plain(b);
	b->joins = 0;
	for (int i = 0; i < b->n; ++i) b->joins += KEY_A(b->cand[i].key) && KEY_B(b->cand[i].key);
}

/* The middle's pairs where `moved` moved them (-1 dropped). */
static void remap_patch(TileBook *b, const int *moved) {
	for (int i = 0; b->patch && i < b->pa * b->pb * 64; ++i)
		if (b->patch[i] >= 0) b->patch[i] = (int16_t)moved[b->patch[i]];
}

void tiles_other_colours(TileBook *b, const uint8_t seen[TILE_COLOURS], bool drop) {
	int n = 0, *moved = malloc(((size_t)b->n + 1) * sizeof *moved);
	for (int i = 0; i < b->n; ++i) {
		TileCand *c = &b->cand[i];
		c->other = 0;
		for (int p = 0; p < 64 && !c->other; ++p) c->other = c->mask >> p & 1 && !seen[c->px[p] & 0x7FFF];
		if (moved) moved[i] = !drop || !c->other ? n : -1;
		if (!drop || !c->other) b->cand[n++] = *c;
	}
	if (moved) remap_patch(b, moved);
	else { free(b->patch); b->patch = NULL; }
	free(moved);
	keep_first(b, n);
}

void tiles_drop_pads(TileBook *b) {
	int n = 0, *moved = malloc(((size_t)b->n + 1) * sizeof *moved);
	for (int i = 0; i < b->n; ++i) {
		if (moved) moved[i] = b->cand[i].pad ? -1 : n;
		if (!b->cand[i].pad) b->cand[n++] = b->cand[i];
	}
	if (moved) remap_patch(b, moved);
	else { free(b->patch); b->patch = NULL; }
	free(moved);
	keep_first(b, n);
}

/* ---- picking ---- */

static int first_class(const TileBook *b, uint32_t key) {
	int lo = 0, hi = b->ncls;
	while (lo < hi) {
		int mid = (lo + hi) / 2;
		if (b->cls[mid].key < key) lo = mid + 1; else hi = mid;
	}
	return lo;
}

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
	unsigned near = nearest(phase, false), corner = nearest(phase, true) & ~near;
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
	if (!plain_memo || !deep) return unplain(b, m, phase, c, deep);
	uint64_t k = ((uint64_t)(uintptr_t)c * 0x9E3779B97F4A7C15ull) ^ (deep * 0xC2B2AE3D27D4EB4Full) ^ (uint64_t)m;
	uint32_t h = (uint32_t)(k >> 48) & (MEMO_SIZE - 1);
	for (int i = 0; i < MEMO_PROBES; ++i) {
		PlainMemo *e = &plain_memo[(h + (uint32_t)i) & (MEMO_SIZE - 1)];
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
	int dA = floordiv(X4 - 4 * g->ex, 128) - A, dB = floordiv(Y4 - 4 * g->ey, 128) - B;
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
	int n = popcount64(((upper >> 56) ^ lower) & 0xFF) - CUT_EDGE;
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
			expect(&gk, tx, ty, floor, ctx, cm, &must, &never, &deep);
			allowed = popcount64(deep) / 8;
		}
		for (int i = first_of(b, KEY(phase, 0, 0)); i < b->n && KEY_PHASE(b->cand[i].key) == phase; ++i) {
			const TileCand *t = &b->cand[i];
			if ((single && t->e1) || (g->single && (t->e0 & 0x3FF) && (t->e1 & 0x3FF))) continue;
			if (picture_diff(cols, mine, KEY_A(t->key), KEY_B(t->key)) > OFF_EDGE_PX) continue;
			int w = TILE_WHY_PIXELS;
			if (misses(t->mask, must, never) <= SLACK) {
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
		int r = books[k].face > FACE_TALL ? reach_of(g->face, g->hang) : reach_of(books[k].face, books[k].hang);
		if (r > reach) reach = r;
	}
	expect_look(g, tx, ty, floor, ctx, cm, reach, &look);
	uint64_t must = 0, never = 0, deep = look.deep;
	int allowed = popcount64(deep) / 8, kface = -1, khang = -1;
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
			expect_for(&look, face, hang, g->face, &must, &never);
		}
		const uint8_t *w = weights(phase, b->face > TALL_FACE);
		for (int j = first_class(b, KEY(phase, 0, 0)); j < b->ncls && KEY_PHASE(b->cls[j].key) == phase; ++j) {
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
				int d = d0, m = misses(c->mask, must, never);
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
		int ci = b->patch[(mod_of(B, b->pb) * b->pa + mod_of(A, b->pa)) * 64 + phase];
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
	occupancy(floor, ctx, A, B, &oa, &ob);
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
