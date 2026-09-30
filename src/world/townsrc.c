/* Learning a town's ground from an original real-world map, and picking
 * a generated town's tiles from it (docs/OVERWORLD.md).
 *
 * Every 8-unit world cell of the source gets a material from the colours
 * its ground shows (real-world maps are 256-colour, and each material has
 * its own colour indices, which the town's style gives) and from the walls
 * (walkable or not). A tile is
 * keyed by the materials under 16 points of it, 8 points above it (the
 * slab's faces hang below the floor's edge) and its phase against the cell
 * lattice. A generated map's tiles are the source's tiles of the same key:
 * where a neighbour's source continues with a matching tile, that one (the
 * source is copied in coherent runs); else the one whose own pixels agree
 * best with the plan, of the same texture phase, the most common. The
 * source is also learned mirrored, which turns its north-east edge's
 * planting strip into a north-west one. */
#include "townsrc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "rom.h"

#define SAMPLES 16
#define UI_COLOURS 208   /* colour indices from here on are the game's UI palettes while it runs */
#define ABOVE 8

static const int8_t samp[SAMPLES][2] = {
	{ 0, 0 }, { 2, 0 }, { 5, 0 }, { 7, 0 }, { 0, 2 }, { 2, 2 }, { 5, 2 }, { 7, 2 },
	{ 0, 5 }, { 2, 5 }, { 5, 5 }, { 7, 5 }, { 0, 7 }, { 2, 7 }, { 5, 7 }, { 7, 7 },
};
/* points above the tile (x, y): a face hangs below the floor's edge, and
 * which side the floor lies on tells a south-west face from a south-east one */
static const int8_t above[ABOVE][2] = {
	{ 0, -2 }, { 7, -2 }, { 0, -6 }, { 7, -6 }, { 0, -10 }, { 7, -10 }, { 0, -14 }, { 7, -14 },
};

static int fdiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }
static int fmod_(int a, int b) { int m = a % b; return m < 0 ? m + b : m; }
/* (buildings stay apart from the sky: the plan has none, so no tile
 * beside one serves) */
static int keymat(int m) { return m; }

typedef struct {
	AreaSrc a;
	uint8_t *cell;         /* material per cell of the box */
	int cx0, cy0, cw, ch;
	int *entry;            /* book entry per tile, -1 none */
	uint8_t *walk;         /* per cell of the box: Lan walks there (the original only) */
} View;

typedef struct {
	uint64_t key, pk;      /* the tile's key; its own pixels' materials at the key's points */
	uint8_t px[64];        /* the colour index of each of its pixels */
	uint16_t e0, e1;       /* its map entries, front and back layer */
	uint16_t tx, ty;
	uint8_t view, tex;
	uint16_t freq;         /* entries alike in key, texture phase and tiles */
	uint16_t seen;         /* the original's tiles alike in tiles (1: a one-off) */
} Entry;

typedef struct { uint64_t key; int first, n; } Key;

struct TownBook {
	View v[2];
	int tw, th;
	Entry *e;
	int ne;
	Key *keys;
	int nkeys;
	TownProfile *prof;
	int nprof;
	int *part;             /* the front layer's piece per tile, -1 none */
	TownFoot *foot;
	int nparts;
	/* the colours each material shows inside itself (for the sky: its
	 * faces); anything else on a tile is foreign there */
	uint8_t allowed[TM_COUNT][256];
};

static int view_cell(const View *v, int cx, int cy) {
	int x = cx - v->cx0, y = cy - v->cy0;
	if (x < 0 || y < 0 || x >= v->cw || y >= v->ch) return TM_VOID;
	return v->cell[(size_t)y * v->cw + x];
}

/* the source's colour indices by material (TownMatOf) */
static TownMatOf mat_of_index;

static void classify(View *v) {
	const AreaSrc *a = &v->a;
	int W = a->tw * 8, H = a->th * 8;
	int r = (a->tw * 4 + a->th * 8) / 16 + 2;
	v->cx0 = v->cy0 = -r;
	v->cw = v->ch = 2 * r;
	v->cell = malloc((size_t)v->cw * v->ch);
	for (int y = 0; y < v->ch; ++y)
		for (int x = 0; x < v->cw; ++x) {
			int cx = x + v->cx0, cy = y + v->cy0, count[TM_COUNT] = { 0 };
			for (int dy = 1; dy < 8; dy += 2)
				for (int dx = 1; dx < 8; dx += 2) {
					int wx = cx * 8 + dx, wy = cy * 8 + dy;
					int X = area_px(a->tw, wx, wy), Y = area_py(a->th, wx, wy);
					count[X >= 0 && Y >= 0 && X < W && Y < H ? mat_of_index(a->idx[(size_t)Y * W + X]) : TM_VOID]++;
				}
			int m = 0;
			for (int k = 1; k < TM_COUNT; ++k) if (count[k] > count[m]) m = k;
			if (m == TM_MARK && count[TM_ROAD] >= 3) m = TM_ROAD;
			/* off the walkable ground: planters stay grass; where the sky
			 * shows it is the sky (with the slab's face hanging into it),
			 * else a building or an object */
			if (area_src_walled_floor(a, cx * 8 + 4, cy * 8 + 4) != 1 && m != TM_GRASS && m != TM_VOID)
				m = count[TM_VOID] >= 3 ? TM_VOID : TM_EDGE;
			v->cell[(size_t)y * v->cw + x] = (uint8_t)m;
		}
	/* the cells just past the walkable ground's south-west and south-east
	 * edges are where its faces hang: sky, not a building */
	for (int y = 0; y < v->ch; ++y)
		for (int x = 0; x < v->cw; ++x) {
			int cx = x + v->cx0, cy = y + v->cy0;
			if (v->cell[(size_t)y * v->cw + x] != TM_EDGE) continue;
			if (area_src_walled_floor(a, (cx + 1) * 8 + 4, cy * 8 + 4) == 1 || area_src_walled_floor(a, cx * 8 + 4, (cy - 1) * 8 + 4) == 1)
				v->cell[(size_t)y * v->cw + x] = TM_VOID;
		}
}

/* Where Lan walks in the original: floor its walls ring, less the wall
 * cells themselves (walls lifted by an event flag, value 0x80 and up, do
 * not count: the town has no events). */
static void learn_walk(View *v) {
	const AreaSrc *a = &v->a;
	v->walk = calloc((size_t)v->cw * v->ch, 1);
	for (int y = 0; y < v->ch; ++y)
		for (int x = 0; x < v->cw; ++x)
			v->walk[(size_t)y * v->cw + x] = area_src_walled_floor(a, (x + v->cx0) * 8 + 4, (y + v->cy0) * 8 + 4) == 1;
	for (int i = 0; i < a->nsec[0]; ++i) {
		const CoordCell *c = &a->sec[0][i];
		int x = fdiv(c->x, 8) - v->cx0, y = fdiv(c->y, 8) - v->cy0;
		if (c->value < 0x80 && x >= 0 && y >= 0 && x < v->cw && y < v->ch) v->walk[(size_t)y * v->cw + x] = 0;
	}
}

/* The front layer's pieces: 8-connected runs of its tiles, each standing
 * on the cell under its lowest pixels (where the game would sort it). */
static void learn_parts(TownBook *b) {
	const AreaSrc *a = &b->v[0].a;
	int tw = a->tw, th = a->th, W = tw * 8;
	b->part = malloc(sizeof(int) * (size_t)tw * th);
	for (int i = 0; i < tw * th; ++i) b->part[i] = -1;
	if (a->layers < 2) return;
	b->foot = malloc(sizeof(TownFoot) * (size_t)tw * th);
	int *stack = malloc(sizeof(int) * (size_t)tw * th);
	for (int start = 0; start < tw * th; ++start) {
		if (b->part[start] >= 0 || !(a->tile[1][start] & 0x3FF)) continue;
		int id = b->nparts++, top = 0, low = -1, xsum = 0, xn = 0;
		b->part[start] = id;
		stack[top++] = start;
		while (top) {
			int t = stack[--top], tx = t % tw, ty = t / tw;
			/* its lowest pixels */
			for (int y = 7; y >= 0; --y) {
				int Y = ty * 8 + y;
				if (Y < low) break;
				for (int x = 0; x < 8; ++x) {
					if (!a->front[(size_t)Y * W + tx * 8 + x]) continue;
					if (Y > low) { low = Y; xsum = 0; xn = 0; }
					xsum += tx * 8 + x;
					++xn;
				}
			}
			for (int dy = -1; dy <= 1; ++dy)
				for (int dx = -1; dx <= 1; ++dx) {
					int nx = tx + dx, ny = ty + dy;
					if (nx < 0 || ny < 0 || nx >= tw || ny >= th) continue;
					int n = ny * tw + nx;
					if (b->part[n] >= 0 || !(a->tile[1][n] & 0x3FF)) continue;
					b->part[n] = id;
					stack[top++] = n;
				}
		}
		int X = xn ? xsum / xn : 0, ax = X - tw * 4, by = (low - th * 4) * 2;
		b->foot[id].cx = (int16_t)fdiv(fdiv(ax - by, 2), 8);
		b->foot[id].cy = (int16_t)fdiv(fdiv(ax + by, 2), 8);
	}
	free(stack);
}

/* The colours inside each material: pixels of cells whose eight
 * neighbours are of the same material, the colours of at least 1 in 400
 * of them (a stray curb's corner is not the ground's); for the sky, 0 and
 * the colours of the faces (non-floor pixels up to 16 below a walkable
 * cell's). */
static void learn_palettes(TownBook *b) {
	const View *v = &b->v[0];
	const AreaSrc *a = &v->a;
	int W = a->tw * 8, H = a->th * 8;
	static int count[TM_COUNT][256];
	int total[TM_COUNT] = { 0 };
	memset(count, 0, sizeof count);
	memset(b->allowed, 0, sizeof b->allowed);
	b->allowed[TM_VOID][0] = 1;
	for (int y = 0; y < H; ++y)
		for (int x = 0; x < W; ++x) {
			int i = a->idx[(size_t)y * W + x];
			if (!i) continue;
			int ax = x - a->tw * 4, by = (y - a->th * 4) * 2;
			int wx = fdiv(ax - by, 2), wy = fdiv(ax + by, 2), cx = fdiv(wx, 8), cy = fdiv(wy, 8);
			int m = view_cell(v, cx, cy);
			if (m == TM_EDGE || m == TM_VOID) {
				/* a face: floor within 16 pixels above */
				for (int d = 2; d <= 16; d += 2) {
					int ay = (y - d - a->th * 4) * 2;
					int fx = fdiv(ax - ay, 2), fy = fdiv(ax + ay, 2);
					int fm = view_cell(v, fdiv(fx, 8), fdiv(fy, 8));
					if (fm != TM_VOID && fm != TM_EDGE) { b->allowed[TM_VOID][i] = 1; break; }
				}
				continue;
			}
			bool inner = true;
			for (int dy = -1; dy <= 1 && inner; ++dy)
				for (int dx = -1; dx <= 1 && inner; ++dx) inner = view_cell(v, cx + dx, cy + dy) == m;
			if (inner) { count[m][i]++; total[m]++; }
		}
	for (int m = 1; m < TM_COUNT; ++m)
		for (int i = 1; i < 256; ++i) b->allowed[m][i] = count[m][i] * 400 >= total[m] && count[m][i] > 0;
	/* a curb's colours are its own; a road is plain (its markings are
	 * where the source's crossings and lanes are) */
	for (int i = 1; i < 256; ++i) {
		if (mat_of_index(i) == TM_MARK) b->allowed[TM_MARK][i] = 1;
		if (mat_of_index(i) == TM_MARK) b->allowed[TM_ROAD][i] = 0;
	}
}

/* ---- keys ---- */

typedef int (*CellFn)(const void *ctx, int cx, int cy);

static int mat_px(CellFn f, const void *ctx, int X, int Y, int tw, int th) {
	int a = X - tw * 4, b = (Y - th * 4) * 2;
	int x = fdiv(a - b, 2), y = fdiv(a + b, 2);
	return keymat(f(ctx, fdiv(x, 8), fdiv(y, 8)));
}

static uint64_t key_at(CellFn f, const void *ctx, int tx, int ty, int tw, int th) {
	uint64_t k = (uint64_t)fmod_(fdiv(tx * 8 - tw * 4, 8), 2);
	for (int i = 0; i < SAMPLES; ++i)
		k |= (uint64_t)mat_px(f, ctx, tx * 8 + samp[i][0], ty * 8 + samp[i][1], tw, th) << (1 + 3 * i);
	for (int j = 0; j < ABOVE; ++j)
		if (mat_px(f, ctx, tx * 8 + above[j][0], ty * 8 + above[j][1], tw, th) != TM_VOID) k |= 1ull << (49 + j);
	return k;
}

static bool key_empty(uint64_t k) { return !(k >> 1); }
static uint64_t key_samples(uint64_t k) { return (k >> 1) & ((1ull << 48) - 1); }

static int agree(uint64_t pk, uint64_t k) {
	uint64_t s = key_samples(k);
	int n = 0;
	for (int i = 0; i < SAMPLES; ++i) n += ((pk >> (3 * i)) & 7) == ((s >> (3 * i)) & 7);
	return n;
}

static int tex_phase(int tx, int ty, int tw, int th) {
	return fmod_(fdiv(tx * 8 - tw * 4, 8), 8) | fmod_(fdiv(ty * 8 - th * 4, 8), 4) << 3;
}

static int view_cell_cb(const void *ctx, int cx, int cy) { return view_cell(ctx, cx, cy); }

/* ---- the book ---- */

static int cmp_key(const void *p, const void *q) {
	const Entry *a = p, *b = q;
	if (a->key != b->key) return a->key < b->key ? -1 : 1;
	if (a->view != b->view) return a->view - b->view;
	return a->ty != b->ty ? a->ty - b->ty : a->tx - b->tx;
}

static int cmp_alike(const void *p, const void *q) {
	const Entry *a = *(const Entry *const *)p, *b = *(const Entry *const *)q;
	if (a->key != b->key) return a->key < b->key ? -1 : 1;
	if (a->tex != b->tex) return a->tex - b->tex;
	if (a->e0 != b->e0) return a->e0 - b->e0;
	return a->e1 - b->e1;
}

static int cmp_tiles(const void *p, const void *q) {
	const Entry *a = *(const Entry *const *)p, *b = *(const Entry *const *)q;
	if (a->e0 != b->e0) return a->e0 - b->e0;
	return a->e1 - b->e1;
}

static void add_view_entries(TownBook *b, int vi) {
	View *v = &b->v[vi];
	const AreaSrc *a = &v->a;
	int W = a->tw * 8;
	for (int ty = 0; ty < a->th; ++ty)
		for (int tx = 0; tx < a->tw; ++tx) {
			uint64_t k = key_at(view_cell_cb, v, tx, ty, a->tw, a->th);
			if (key_empty(k)) continue;
			/* (tiles drawn in colours the running game gives its UI would
			 * show them wrong) */
			bool ui = false;
			for (int y = 0; y < 8 && !ui; ++y)
				for (int x = 0; x < 8 && !ui; ++x) ui = a->idx[(size_t)(ty * 8 + y) * W + tx * 8 + x] >= UI_COLOURS;
			if (ui) continue;
			Entry *e = &b->e[b->ne++];
			memset(e, 0, sizeof *e);
			e->key = k;
			for (int i = 0; i < SAMPLES; ++i)
				e->pk |= (uint64_t)mat_of_index(a->idx[(size_t)(ty * 8 + samp[i][1]) * W + tx * 8 + samp[i][0]]) << (3 * i);
			for (int y = 0; y < 8; ++y)
				for (int x = 0; x < 8; ++x) e->px[y * 8 + x] = a->idx[(size_t)(ty * 8 + y) * W + tx * 8 + x];
			for (int l = 0; l < 2; ++l) {
				uint16_t t = a->tile[l] ? a->tile[l][(size_t)ty * a->tw + tx] : 0;
				if (!(t & 0x3FF)) t = 0;
				if (l) e->e1 = t; else e->e0 = t;
			}
			e->tx = (uint16_t)tx;
			e->ty = (uint16_t)ty;
			e->view = (uint8_t)vi;
			e->tex = (uint8_t)tex_phase(tx, ty, a->tw, a->th);
		}
}

/* ---- edge profiles ---- */

static int find_profile(const TownProfile *list, int n, const TownProfile *p) {
	for (int i = 0; i < n; ++i) {
		const TownProfile *q = &list[i];
		if (q->o == p->o && q->a == p->a && q->b == p->b && q->nmid == p->nmid && q->ph == p->ph && !memcmp(q->mid, p->mid, p->nmid)) return i;
	}
	return -1;
}

#define MAX_PROFILES 4096

/* Counts the view's profiles, then adds them to the book's: all of the
 * original's, only those it lacks of the mirror's. */
static void learn_profiles(TownBook *b, int vi, bool only_new) {
	const View *v = &b->v[vi];
	int n = v->cw;
	uint8_t *line = malloc((size_t)n);
	TownProfile *mine = calloc(MAX_PROFILES, sizeof(TownProfile));
	int nmine = 0;
	for (int o = 0; o < 2; ++o)
		for (int u = 0; u < v->ch; ++u) {
			for (int k = 0; k < n; ++k)
				line[k] = (uint8_t)keymat(o == 0 ? view_cell(v, k + v->cx0, u + v->cy0) : view_cell(v, u + v->cx0, k + v->cy0));
			int i = 0;
			while (i < n - 2) {
				int a = line[i];
				if (line[i + 1] != a) { ++i; continue; }
				int j = i + 1;
				while (j < n && line[j] == a) ++j;
				for (int len = 0; len <= 3; ++len) {
					int k = j + len;
					if (k + 1 >= n) break;
					int bm = line[k];
					if (bm == a || line[k + 1] != bm) continue;
					bool clean = true;
					for (int m = j; m < k; ++m) clean &= line[m] != a && line[m] != bm;
					if (!clean) continue;
					TownProfile p = { (uint8_t)o, (uint8_t)a, (uint8_t)bm, (uint8_t)len, { 0 }, (uint8_t)fmod_((j + (o == 0 ? v->cx0 : v->cy0)) * 8, 32), (uint8_t)vi, 1 };
					memcpy(p.mid, line + j, (size_t)len);
					int at = find_profile(mine, nmine, &p);
					if (at >= 0) mine[at].count++;
					else if (nmine < MAX_PROFILES) mine[nmine++] = p;
					break;
				}
				i = j;
			}
		}
	for (int i = 0; i < nmine; ++i) {
		int at = find_profile(b->prof, b->nprof, &mine[i]);
		if (at < 0 && b->nprof < MAX_PROFILES) b->prof[b->nprof++] = mine[i];
		else if (at >= 0 && !only_new) b->prof[at].count += mine[i].count;
	}
	free(mine);
	free(line);
}

TownBook *townsrc_learn(int group, int number, TownMatOf mat_of) {
	TownBook *b = calloc(1, sizeof *b);
	mat_of_index = mat_of;
	if (!area_src_load(group, number, &b->v[0].a) || !b->v[0].a.idx) { free(b); return NULL; }
	area_src_mirror(&b->v[0].a, &b->v[1].a);
	b->tw = b->v[0].a.tw;
	b->th = b->v[0].a.th;
	for (int vi = 0; vi < 2; ++vi) classify(&b->v[vi]);
	learn_walk(&b->v[0]);
	learn_parts(b);
	learn_palettes(b);
	b->e = malloc(sizeof(Entry) * (size_t)b->tw * b->th * 2);
	for (int vi = 0; vi < 2; ++vi) add_view_entries(b, vi);
	qsort(b->e, (size_t)b->ne, sizeof(Entry), cmp_key);
	/* frequencies: entries alike in key, texture phase and tiles */
	Entry **by = malloc(sizeof(Entry *) * (size_t)b->ne);
	for (int i = 0; i < b->ne; ++i) by[i] = &b->e[i];
	qsort(by, (size_t)b->ne, sizeof(Entry *), cmp_alike);
	for (int i = 0; i < b->ne;) {
		int j = i;
		while (j < b->ne && !cmp_alike(&by[i], &by[j])) ++j;
		for (int k = i; k < j; ++k) by[k]->freq = (uint16_t)(j - i > 65535 ? 65535 : j - i);
		i = j;
	}
	/* how often the original shows each tile, anywhere */
	qsort(by, (size_t)b->ne, sizeof(Entry *), cmp_tiles);
	for (int i = 0; i < b->ne;) {
		int j = i, n = 0;
		while (j < b->ne && !cmp_tiles(&by[i], &by[j])) n += !by[j++]->view;
		for (int k = i; k < j; ++k) by[k]->seen = (uint16_t)(n > 65535 ? 65535 : n);
		i = j;
	}
	free(by);
	/* the unique keys, and each tile's entry */
	b->keys = malloc(sizeof(Key) * (size_t)(b->ne + 1));
	for (int i = 0; i < b->ne;) {
		int j = i;
		while (j < b->ne && b->e[j].key == b->e[i].key) ++j;
		b->keys[b->nkeys++] = (Key){ b->e[i].key, i, j - i };
		i = j;
	}
	for (int vi = 0; vi < 2; ++vi) {
		b->v[vi].entry = malloc(sizeof(int) * (size_t)b->tw * b->th);
		for (int i = 0; i < b->tw * b->th; ++i) b->v[vi].entry[i] = -1;
	}
	for (int i = 0; i < b->ne; ++i) b->v[b->e[i].view].entry[b->e[i].ty * b->tw + b->e[i].tx] = i;
	b->prof = calloc(MAX_PROFILES, sizeof(TownProfile));
	learn_profiles(b, 0, false);
	learn_profiles(b, 1, true);
	return b;
}

void townsrc_free(TownBook *b) {
	if (!b) return;
	for (int vi = 0; vi < 2; ++vi) {
		area_src_free(&b->v[vi].a);
		free(b->v[vi].cell);
		free(b->v[vi].entry);
		free(b->v[vi].walk);
	}
	free(b->part);
	free(b->foot);
	free(b->e);
	free(b->keys);
	free(b->prof);
	free(b);
}

void townsrc_size(const TownBook *b, int *tw, int *th) { *tw = b->tw; *th = b->th; }

void townsrc_slots(const TownBook *b, uint32_t *desc, uint32_t *coord_slot) {
	*desc = b->v[0].a.desc;
	*coord_slot = b->v[0].a.coord_slot;
}

const AreaSrc *townsrc_area(const TownBook *b) { return &b->v[0].a; }
int townsrc_mat(const TownBook *b, int cx, int cy) { return view_cell(&b->v[0], cx, cy); }

bool townsrc_walk(const TownBook *b, int cx, int cy) {
	const View *v = &b->v[0];
	int x = cx - v->cx0, y = cy - v->cy0;
	return x >= 0 && y >= 0 && x < v->cw && y < v->ch && v->walk[(size_t)y * v->cw + x];
}

int townsrc_parts(const TownBook *b, const int **part, const TownFoot **foot) {
	*part = b->part;
	*foot = b->foot;
	return b->nparts;
}

#define RW_OBJ_SPAWNERS 0x034654u /* RealWorldSpawnMapObjectJumptable */

int townsrc_objects(const TownBook *b, uint8_t (*rec)[20], int max) {
	/* the group's spawner loads its per-map table with its first
	 * "ldr r1, [pc, #imm]" */
	uint32_t fn = rom_u32(RW_OBJ_SPAWNERS + (uint32_t)b->v[0].a.group * 4);
	if (!rom_is_ptr(fn)) return 0;
	fn = rom_off(fn) & ~1u;
	uint32_t table = 0;
	for (uint32_t pc = fn; pc < fn + 32 && !table; pc += 2) {
		uint16_t op = rom_u16(pc);
		if ((op & 0xFF00) == 0x4900) table = rom_u32(((pc + 4) & ~3u) + (uint32_t)(op & 0xFF) * 4);
	}
	if (!rom_is_ptr(table)) return 0;
	uint32_t list = rom_u32(rom_off(table) + (uint32_t)b->v[0].a.number * 4);
	if (!rom_is_ptr(list)) return 0;
	int n = 0;
	for (uint32_t at = rom_off(list); n < max && at + 20 <= ROM_SIZE && R.data[at] != 0xFF; at += 20) memcpy(rec[n++], R.data + at, 20);
	return n;
}

/* ---- picking ---- */

static const Key *find_key(const TownBook *b, uint64_t k) {
	int lo = 0, hi = b->nkeys - 1;
	while (lo <= hi) {
		int m = (lo + hi) / 2;
		if (b->keys[m].key == k) return &b->keys[m];
		if (b->keys[m].key < k) lo = m + 1; else hi = m - 1;
	}
	return NULL;
}

/* The key of the same phase and faces whose materials agree best. */
static const Key *nearest_key(const TownBook *b, uint64_t k) {
	const Key *best = NULL;
	int bs = -1;
	uint64_t frame = (k & 1) | (k & (255ull << 49));
	for (int pass = 0; pass < 2 && !best; ++pass)
		for (int i = 0; i < b->nkeys; ++i) {
			uint64_t c = b->keys[i].key;
			if (!pass && ((c & 1) | (c & (255ull << 49))) != frame) continue;
			if (pass && (c & 1) != (k & 1)) continue;
			int s = agree(key_samples(c), k);
			/* (key_samples(c) is laid out as a pixel key) */
			if (s > bs) { bs = s; best = &b->keys[i]; }
		}
	return best;
}

/* A tile's fit: minus its pixels in colours the plan's material there
 * does not show (a curb's line on plain sidewalk, a house's wall in a
 * planting strip). */
static const TownBook *fit_book;
static int full_agree(const Entry *e, uint64_t k, const uint8_t want[64], bool uniform) {
	(void)k; (void)uniform;
	int n = 0;
	/* (half as much near another material, where curbs belong) */
	for (int i = 0; i < 64; ++i)
		if (!fit_book->allowed[want[i] & 0x7F][e->px[i]]) n -= want[i] & 0x80 ? 1 : 2;
	return n;
}

typedef struct { TownCell cell; } PlanCtx;
static int plan_cell(const void *ctx, int cx, int cy) { return ((const PlanCtx *)ctx)->cell(cx, cy); }

static uint32_t rng_next32(uint32_t *s) {
	uint32_t x = *s ? *s : 0x9E3779B9u;
	x ^= x << 13; x ^= x >> 17; x ^= x << 5;
	return *s = x;
}

void townsrc_synth(const TownBook *b, TownCell cell, int tw, int th, uint32_t seed, const TownPin *pins, uint16_t *map, uint8_t *miss, TownSynthStats *st) {
	const AreaSrc *sa = &b->v[0].a;
	PlanCtx ctx = { cell };
	fit_book = b;
	int *src = malloc(sizeof(int) * (size_t)tw * th);
	uint16_t *l0 = map, *l1 = map + (size_t)tw * th;
	TownSynthStats local = { 0 };
	if (!st) st = &local;
	memset(st, 0, sizeof *st);
	uint32_t rs = seed * 2654435761u + 1;
	for (int i = 0; i < tw * th; ++i) src[i] = -1;
	int dbg_tx = -1, dbg_ty = -1;
	if (getenv("CYBERWORLD_TOWN_TILE")) sscanf(getenv("CYBERWORLD_TOWN_TILE"), "%d,%d", &dbg_tx, &dbg_ty);
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) {
			size_t at = (size_t)ty * tw + tx;
			src[at] = -1;
			l0[at] = l1[at] = 0;
			if (miss) miss[at] = 0;
			const TownPin *pin = pins ? &pins[at] : NULL;
			if (pin && pin->x1 >= 0) l1[at] = sa->tile[1][(size_t)pin->y1 * sa->tw + pin->x1];
			if (pin && pin->x0 >= 0) {
				/* taken whole: what follows it continues it */
				size_t s = (size_t)pin->y0 * sa->tw + pin->x0;
				l0[at] = sa->tile[0][s];
				src[at] = b->v[0].entry[s];
				if (miss) miss[at] |= 8;
				continue;
			}
			uint64_t k = key_at(plan_cell, &ctx, tx, ty, tw, th);
			if (key_empty(k)) continue;
			st->picks++;
			const Key *K = find_key(b, k);
			if (K) st->exact++;
			else {
				K = nearest_key(b, k);
				st->near++;
				if (miss) miss[at] = 1;
				if (!K) continue;
			}
			/* what the plan puts under each of the tile's pixels; 0x80 set
			 * where another material is near */
			uint8_t want[64];
			bool uniform = true;
			for (int y = 0; y < 8; ++y)
				for (int x = 0; x < 8; ++x) {
					int X = tx * 8 + x, Y = ty * 8 + y;
					int a = X - tw * 4, bb = (Y - th * 4) * 2;
					int wx = fdiv(a - bb, 2), wy = fdiv(a + bb, 2), cx = fdiv(wx, 8), cy = fdiv(wy, 8);
					int m = keymat(cell(cx, cy));
					bool inner = true;
					for (int dy = -1; dy <= 1 && inner; ++dy)
						for (int dx = -1; dx <= 1 && inner; ++dx) inner = keymat(cell(cx + dx, cy + dy)) == m;
					want[y * 8 + x] = (uint8_t)(inner ? m : m | 0x80);
					uniform &= want[y * 8 + x] == want[0];
				}
			/* the original's tiles; the mirror's only where the original has
			 * none of this key (the NW planting strip): its faces are lit
			 * from the other side */
			int view = 1;
			for (int i = 0; i < K->n && view; ++i) if (!b->e[K->first + i].view) view = 0;
			int ba = -(1 << 30);
			for (int i = 0; i < K->n; ++i) {
				const Entry *e = &b->e[K->first + i];
				if (e->view != view) continue;
				int s = full_agree(e, k, want, uniform);
				if (s > ba) ba = s;
			}
			/* the hinted tile, if it is of this key and fits as well as the
			 * best (no crossing's stripes on a plain road) */
			int pick = -1;
			if (pin && pin->hx >= 0 && pin->hx < b->tw && pin->hy >= 0 && pin->hy < b->th) {
				int c = b->v[0].entry[pin->hy * b->tw + pin->hx];
				/* (and whose own pixels show the key's grounds as well as
				 * any: a cell's ground is its most, not all of it) */
				int best_pk = 0;
				for (int i = 0; c >= 0 && i < K->n; ++i) {
					int a = agree(b->e[K->first + i].pk, k);
					if (a > best_pk) best_pk = a;
				}
				if (c >= 0 && b->e[c].key == K->key && b->e[c].seen >= 2 && full_agree(&b->e[c], k, want, uniform) >= ba && agree(b->e[c].pk, k) >= best_pk) {
					pick = c;
					st->hinted++;
					if (miss) miss[at] |= 4;
				}
			}
			/* coherence: the source's own neighbour of a neighbour's source
			 * tile, first beside the tiles taken whole (on every side), then
			 * after the picked ones (left, above) */
			static const int nd[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			for (int pass = 0; pass < 2 && pick < 0; ++pass)
				for (int d = 0; d < 4 && pick < 0; ++d) {
					int nx = tx + nd[d][0], ny = ty + nd[d][1];
					if (nx < 0 || ny < 0 || nx >= tw || ny >= th) continue;
					size_t na = (size_t)ny * tw + nx;
					bool whole = pins && pins[na].x0 >= 0;
					int sx, sy, sv;
					if (pass == 0) {
						if (!whole) continue;
						sx = pins[na].x0; sy = pins[na].y0; sv = 0;
					} else {
						if (whole || src[na] < 0) continue;
						sx = b->e[src[na]].tx; sy = b->e[src[na]].ty; sv = b->e[src[na]].view;
					}
					int cx = sx - nd[d][0], cy = sy - nd[d][1];
					if (sv != view || cx < 0 || cy < 0 || cx >= b->tw || cy >= b->th) continue;
					int c = b->v[sv].entry[cy * b->tw + cx];
					/* (not into a one-off: a tile the source shows once is
					 * part of something, a corner, a drain) */
					if (c >= 0 && b->e[c].key == K->key && b->e[c].seen >= 2 && full_agree(&b->e[c], k, want, uniform) == ba) { pick = c; st->coherent++; }
				}
			if (pick < 0) {
				/* of those: the texture phase's if any, the most common, a
				 * random one of what ties */
				int tex = tex_phase(tx, ty, tw, th);
				bool any_tex = false;
				for (int i = 0; i < K->n && !any_tex; ++i) {
					const Entry *e = &b->e[K->first + i];
					any_tex = e->view == view && e->tex == tex && full_agree(e, k, want, uniform) == ba;
				}
				int best_f = -1, ties = 0;
				for (int i = 0; i < K->n; ++i) {
					const Entry *e = &b->e[K->first + i];
					if (e->view != view || (any_tex && e->tex != tex) || full_agree(e, k, want, uniform) != ba) continue;
					if (e->freq < best_f) continue;
					if (e->freq > best_f) { best_f = e->freq; ties = 0; }
					if (rng_next32(&rs) % (uint32_t)(++ties) == 0) pick = K->first + i;
				}
			}
			if (pick < 0) continue;
			if (dbg_tx == tx && dbg_ty == ty) {
				const Entry *e = &b->e[pick];
				fprintf(stderr, "tile %d,%d: key %016llx (%d candidates), best fit %d; picked source %d,%d view %d fit %d pk-agree %d, hint %d,%d\n", tx, ty,
					(unsigned long long)k, K->n, ba, e->tx, e->ty, e->view, full_agree(e, k, want, uniform), agree(e->pk, k), pin ? pin->hx : -9, pin ? pin->hy : -9);
			}
			src[at] = pick;
			if (miss && b->e[pick].view) miss[at] |= 2;   /* (from the mirror) */
			l0[at] = b->e[pick].e0;
			if (!pin || pin->x1 == -1) l1[at] = b->e[pick].e1;
		}
	free(src);
}
