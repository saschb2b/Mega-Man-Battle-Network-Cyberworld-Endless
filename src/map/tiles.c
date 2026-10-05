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
 * in their own place are not learned.
 *
 * Here the pixel tests and what a panel should look like; learning an
 * area's tiles is tiles_learn.c's, picking one for a cell tiles_pick.c's. */
#include "tiles.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "tiles_learn.h"
#include "tiles_parts.h"

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

#define IN    2    /* pixels inside the floor's edge that must be drawn */
/* (OUT, the pixels beyond it that may be: tiles_parts.h) */
#define DEEP  10   /* pixels inside the floor's edge where it must look plain */
#define HANG_BELOW_FACE 12   /* how far past a face legs may hang */
#define FACE_SOLID 12   /* pixels of a side face that must be drawn */
#define FACE_MAX 64     /* the tallest side face measured (the story comps' run to 30 and more) */
#define PLAIN_SHARE 8   /* a plain look is seen at least 1/8 as often as the most common */

int tl_floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

/* ---- the source's panels ---- */

/* The hue bucket (0-11, 12 grey) of the floor around map pixel (cx, cy). */
int tl_style_at(const AreaSrc *a, int cx, int cy) {
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

/* How many pixels of the top of the panel around map pixel (cx, cy) are
 * pale: bright and weakly coloured. */
static int pale_pixels(const AreaSrc *a, int cx, int cy) {
	int W = a->tw * 8, H = a->th * 8, n = 0;
	for (int dy = -8; dy <= 8; ++dy)
		for (int dx = -16; dx <= 16; ++dx) {
			int x = cx + dx, y = cy + dy;
			if (2 * abs(dy) + abs(dx) > 16 || x < 0 || y < 0 || x >= W || y >= H) continue;
			uint32_t c = area_src_floor_px(a, (size_t)y * W + x);
			if (!(c >> 24)) continue;
			int r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255;
			int mx = r > g ? (r > b ? r : b) : (g > b ? g : b), mn = r < g ? (r < b ? r : b) : (g < b ? g : b);
			n += mx >= 150 && (mx - mn) * 100 < 40 * mx;
		}
	return n;
}

#define PALE_PIXELS 40   /* SKIP_PALE: Mr. Weather Comp's fan belts 120 and more, its solar panels under 20 */
#define JOINT_PIXELS 4   /* a walkway panel showing this many of its area's joint hues is a joint: the Undernet's gems */

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

/* Whether panel (A, B) lies in a 2 x 2 block of floor. */
static bool in_block_of_floor(const Src *s, int A, int B) {
	for (int db = -1; db <= 0; ++db)
		for (int da = -1; da <= 0; ++da)
			if (drawn_cached(s, A + da, B + db) && drawn_cached(s, A + da + 1, B + db) &&
				drawn_cached(s, A + da, B + db + 1) && drawn_cached(s, A + da + 1, B + db + 1)) return true;
	return false;
}

static int measure_panel(const Src *s, int A, int B) {
	if (!drawn_cached(s, A, B)) return 0;
	if (s->styles & TILES_BY_SHAPE) {
		/* (where the area names hues, a panel of another is neither:
		 * Nebula Area's pale arrows on its paths) */
		unsigned hues = (s->styles | s->walk_styles) & 0x1FFFu;
		if (hues) {
			const AreaSrc *a = s->a;
			int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
			if (!(hues >> tl_style_at(a, area_px(a->tw, X, Y), area_py(a->th, X, Y)) & 1)) return OTHER;
		}
		/* platform floor lies in a 2 x 2 block of floor, walkways do not;
		 * a platform's edge, its rim, is walkway floor too */
		if (!in_block_of_floor(s, A, B)) return TILE_B;
		for (int db = -1; db <= 1; ++db)
			for (int da = -1; da <= 1; ++da)
				if (!drawn_cached(s, A + da, B + db)) return TILE_B;
		return TILE_A;
	}
	const AreaSrc *a = s->a;
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	int st = tl_style_at(a, area_px(a->tw, X, Y), area_py(a->th, X, Y));
	if (s->styles >> st & 1) return TILE_A;
	if (!(s->walk_styles >> st & 1)) return OTHER;
	/* (TILES_WALK_NARROW: in a block, a field of the walkways' hue) */
	return s->styles & TILES_WALK_NARROW && in_block_of_floor(s, A, B) ? TILE_A : TILE_B;
}

/* Panel state in the source map: 0 empty, TILE_A platform floor, TILE_B
 * walkway floor, OTHER floor of another style */
int tl_src_panel(const Src *s, int A, int B) {
	int i = A + SPAN / 2, j = B + SPAN / 2;
	if (i < 0 || j < 0 || i >= SPAN || j >= SPAN) return measure_panel(s, A, B);
	int8_t *st = &s->state[j * SPAN + i];
	if (*st < 0) *st = (int8_t)measure_panel(s, A, B);
	return *st;
}

/* as the pixel test sees it: floor of another style is floor too */
int tl_src_floor(int A, int B, const void *ctx) {
	int st = tl_src_panel(ctx, A, B);
	return st == OTHER ? TILE_A : st;
}

/* The source's pads: panels of small platforms (at most PAD_PANELS in 2 x 2
 * blocks, joined to the rest by 1-wide bridges at most). SPAN x SPAN. */
#define PAD_PANELS 12

uint8_t *tl_find_pads(const Src *s) {
	uint8_t *block = calloc(SPAN * SPAN, 1), *pad = calloc(SPAN * SPAN, 1);
	int H = SPAN / 2;
	for (int j = 0; j + 1 < SPAN; ++j)
		for (int i = 0; i + 1 < SPAN; ++i)
			if (tl_src_panel(s, i - H, j - H) && tl_src_panel(s, i + 1 - H, j - H) && tl_src_panel(s, i - H, j + 1 - H) && tl_src_panel(s, i + 1 - H, j + 1 - H))
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
	*A = tl_floordiv(X - g->ex, 32);
	*B = tl_floordiv(Y - g->ey, 32);
	int fx = X - g->ex - 32 * *A, fy = Y - g->ey - 32 * *B;
	*phase = (fx / 4) * 8 + fy / 4;
}

/* The panels nearest a tile at `phase`: the centre and the two nearest
 * sides, and with `corner` the nearest diagonal too. Bit k is the panel at
 * (A + k % 3 - 1, B + k / 3 - 1). */
unsigned tl_nearest(int phase, bool corner) {
	int fx = (phase / 8) * 4, fy = (phase % 8) * 4;
	int da = fx < 16 ? 0 : 2, db = fy < 16 ? 0 : 6;   /* the near column and row */
	unsigned m = 0x010 | 1u << (da + 3) | 1u << (1 + db);
	return corner ? m | 1u << (da + db) : m;
}

/* The 3x3 panels' materials: bit k of *a (platform) or *b (walkway). */
void tl_occupancy(TileFloor floor, const void *ctx, int A, int B, unsigned *a, unsigned *b) {
	*a = *b = 0;
	for (int k = 0; k < 9; ++k) {
		int m = TILE_MATERIAL(floor(A + k % 3 - 1, B + k / 3 - 1, ctx));
		if (m == TILE_A) *a |= 1u << k;
		if (m == TILE_B) *b |= 1u << k;
	}
}

/* ---- the pixel test ---- */

/* The floor map pixel (px, py) shows at z 0 (0 none, else its material):
 * the world point under its centre, in quarter units (X = (u - 2v) / 2,
 * Y = (u + 2v) / 2), with the floor drawn dv pixels below it. */
static void px_panel(const TileGrid *g, int px, int py, int *A, int *B) {
	int u2 = 2 * px + 1 - g->tw * 8, v2 = 2 * (py - g->dv) + 1 - g->th * 8;
	int X4 = u2 - 2 * v2, Y4 = u2 + 2 * v2;
	*A = tl_floordiv(X4 - 4 * g->ex, 128);
	*B = tl_floordiv(Y4 - 4 * g->ey, 128);
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
PlainMemo *tl_plain_memo;

void tiles_keep_begin(const TileGrid *g, TileFloor floor, const void *ctx) {
	tiles_keep_end();
	tl_plain_memo = calloc(MEMO_SIZE, sizeof *tl_plain_memo);
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
	free(tl_plain_memo);
	tl_plain_memo = NULL;
}

#define FACE_SIDE 1         /* pixels beside a side face a tile may draw */
/* What a tile's pixel test finds whatever the faces (expect_look), then the
 * test for one book's faces and legs (expect_for): a tile's pick tests it
 * for each book's, and looking at its floor again for each was a third of
 * making a layer. */
#define NO_FLOOR (INT_MAX / 2)

/* how far up a tile's pixels look for floor, for faces and legs so deep */
int tl_reach_of(int face, int hang) {
	int solid = face - 2 < FACE_SOLID ? face - 2 : FACE_SOLID, hangs = hang + OUT;
	return hangs > solid ? hangs : solid;
}

void tl_expect_look(const TileGrid *g, int tx, int ty, TileFloor floor, const void *ctx, int cm, int reach, Look *l) {
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
void tl_expect_for(const Look *l, int face, int hang, int map_face, uint64_t *must, uint64_t *never) {
	/* (only its top need be solid: CopyBot's pods leave gaps lower down) */
	int solid = face - 2 < FACE_SOLID ? face - 2 : FACE_SOLID;
	int hangs = hang + OUT, reach = tl_reach_of(face, hang);
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
void tl_expect(const TileGrid *g, int tx, int ty, TileFloor floor, const void *ctx, int cm, uint64_t *must, uint64_t *never, uint64_t *deep) {
	Look l;
	tl_expect_look(g, tx, ty, floor, ctx, cm, tl_reach_of(g->face, g->hang), &l);
	tl_expect_for(&l, g->face, g->hang, g->side, must, never);
	*deep = l.deep;
}

int tl_misses(uint64_t mask, uint64_t must, uint64_t never) {
	return tl_popcount64(must & ~mask) + tl_popcount64(never & mask);
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
bool tl_flat_tile(const AreaSrc *a, int tx, int ty) {
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
uint64_t tl_drawn(const AreaSrc *a, bool first, int tx, int ty, uint16_t px[64]) {
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
void tl_calibrate(const AreaSrc *a, const Src *src, TileGrid *g) {
	int top[9] = { 0 }, face[FACE_MAX + 1] = { 0 }, W = a->tw * 8, H = a->th * 8;
	g->dv = g->face = g->hang = 0;
	for (int x = 0; x < W; ++x) {
		/* (each pixel asked after once, here, then as the next one's above) */
		int last[3] = { INT_MIN, 0, 0 };
		bool above, here = floor_px_on(g, tl_src_floor, src, x, 0, last);
		for (int y = 1; y + 8 < H; ++y) {
			above = here;
			here = floor_px_on(g, tl_src_floor, src, x, y, last);
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

int tl_cmp_cand(const void *a, const void *b) {
	const TileCand *x = a, *y = b;
	if (x->key != y->key) return x->key < y->key ? -1 : 1;
	if (x->e0 != y->e0) return x->e0 < y->e0 ? -1 : 1;
	if (x->e1 != y->e1) return x->e1 < y->e1 ? -1 : 1;
	return x->pad - y->pad;
}

int tl_cmp_u32(const void *a, const void *b) {
	uint32_t x = *(const uint32_t *)a, y = *(const uint32_t *)b;
	return (x > y) - (x < y);
}

int tl_cmp_count(const void *a, const void *b) {
	const TileCand *x = a, *y = b;
	if (x->key != y->key) return x->key < y->key ? -1 : 1;
	return (x->count < y->count) - (x->count > y->count);
}

/* Per material and phase, the looks of floor with the same floor all
 * around, common enough (off the pads, which have their own). */
void tl_find_plain(TileBook *b) {
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

/* Whether a side of panel (A, B) touches walkway floor. */
static bool beside_walkway(const Src *s, int A, int B) {
	return tl_src_panel(s, A + 1, B) == TILE_B || tl_src_panel(s, A - 1, B) == TILE_B || tl_src_panel(s, A, B + 1) == TILE_B || tl_src_panel(s, A, B - 1) == TILE_B;
}

/* Whether panel (A, B) is a walkway's joint: its top shows the area's
 * joint hues (the Undernet's yellow gems). */
static bool joint_panel(const Src *s, int A, int B) {
	if (!s->joint_hues || tl_src_panel(s, A, B) != TILE_B) return false;
	const AreaSrc *a = s->a;
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	return hue_pixels(a, area_px(a->tw, X, Y), area_py(a->th, X, Y), s->joint_hues) >= JOINT_PIXELS;
}

/* Whether panel (A, B)'s tiles are in its pads' look: a pad of the map, or
 * a walkway's joint, in a look of its own as a pad's (a generated walkway's
 * straight runs took the gems' pieces). */
bool tl_pad_look(const Src *s, const uint8_t *pads, int A, int B) {
	bool in = A >= -SPAN / 2 && B >= -SPAN / 2 && A < SPAN / 2 && B < SPAN / 2;
	return (!(s->styles & TILES_NO_PAD_LOOK) && in && pads[(B + SPAN / 2) * SPAN + A + SPAN / 2]) || joint_panel(s, A, B);
}

/* Whether panel (A, B) is floor whose tiles are not learned. */
bool tl_skipped(const Src *s, int A, int B) {
	if (!s->skip_styles || !drawn_cached(s, A, B)) return false;
	if (s->skip_styles & SKIP_OFF_PADS && s->pads && A >= -SPAN / 2 && B >= -SPAN / 2 && A < SPAN / 2 && B < SPAN / 2 &&
		s->pads[(B + SPAN / 2) * SPAN + A + SPAN / 2]) return false;
	const AreaSrc *a = s->a;
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	if (s->skip_styles & SKIP_PALE) return tl_src_panel(s, A, B) == TILE_A && !beside_walkway(s, A, B) &&
		pale_pixels(a, area_px(a->tw, X, Y), area_py(a->th, X, Y)) >= PALE_PIXELS;
	if (s->skip_styles & SKIP_ANY_PIXEL) return hue_pixels(a, area_px(a->tw, X, Y), area_py(a->th, X, Y), s->skip_styles & 0xFFFu) >= 8;
	return s->skip_styles >> tl_style_at(a, area_px(a->tw, X, Y), area_py(a->th, X, Y)) & 1;
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
	tl_index_classes(b);
	tl_find_plain(b);
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

int tl_first_class(const TileBook *b, uint32_t key) {
	int lo = 0, hi = b->ncls;
	while (lo < hi) {
		int mid = (lo + hi) / 2;
		if (b->cls[mid].key < key) lo = mid + 1; else hi = mid;
	}
	return lo;
}

int tl_first_of(const TileBook *b, uint32_t key) {
	int lo = 0, hi = b->n;
	while (lo < hi) {
		int mid = (lo + hi) / 2;
		if (b->cand[mid].key < key) lo = mid + 1; else hi = mid;
	}
	return lo;
}
