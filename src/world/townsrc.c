/* Learning a town's ground from an original real-world map, and picking
 * a generated town's tiles from it (docs/OVERWORLD.md).
 *
 * Every 8-unit world cell of the source gets a material from the colours
 * its ground shows (real-world maps are 256-colour, and each material has
 * its own colour indices) and from the walls (walkable or not). A tile is
 * keyed by the materials under 16 points of it, 3 points above it (the
 * slab's faces hang below the floor's edge) and its phase against the cell
 * lattice. A generated map's tiles are the source's tiles of the same key:
 * where a neighbour's source continues with a matching tile, that one (the
 * source is copied in coherent runs); else the one whose own pixels agree
 * best with the plan, of the same texture phase, the most common. The
 * source is also learned mirrored, which turns its north-east edge's
 * planting strip into a north-west one. */
#include "townsrc.h"

#include <stdlib.h>
#include <string.h>

#include "area_src.h"

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

/* Central Town's colour indices by material (the source: group 0x01, map 0,
 * whose palette the town keeps). */
static int mat_of_index(int i) {
	if (i == 0) return TM_VOID;
	if (i == 10) return TM_ROAD;
	if (i >= 1 && i <= 3) return TM_MARK;
	if (i == 4) return TM_SIDE;
	if (i >= 5 && i <= 8) return TM_COBB;
	if (i >= 80 && i <= 84) return TM_BRICK;
	if ((i >= 32 && i <= 39) || i == 15 || i == 17 || i == 18 || i == 20 || i == 97 || i == 98)
		return TM_GRASS;   /* (the pinks 106-109 are the houses' walls) */
	return TM_EDGE;
}

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
} View;

typedef struct {
	uint64_t key, pk;      /* the tile's key; its own pixels' materials at the key's points */
	uint8_t px[64];        /* the colour index of each of its pixels */
	uint16_t e0, e1;       /* its map entries, front and back layer */
	uint16_t tx, ty;
	uint8_t view, tex;
	uint16_t freq;         /* entries alike in key, texture phase and tiles */
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
	/* the colours each material shows inside itself (for the sky: its
	 * faces); anything else on a tile is foreign there */
	uint8_t allowed[TM_COUNT][256];
};

static int view_cell(const View *v, int cx, int cy) {
	int x = cx - v->cx0, y = cy - v->cy0;
	if (x < 0 || y < 0 || x >= v->cw || y >= v->ch) return TM_VOID;
	return v->cell[(size_t)y * v->cw + x];
}

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

/* The colours inside each material: pixels of cells whose eight
 * neighbours are of the same material; for the sky, 0 and the colours
 * of the faces (non-floor pixels up to 16 below a walkable cell's). */
static void learn_palettes(TownBook *b) {
	const View *v = &b->v[0];
	const AreaSrc *a = &v->a;
	int W = a->tw * 8, H = a->th * 8;
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
			if (inner) b->allowed[m][i] = 1;
		}
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

TownBook *townsrc_learn(int group, int number) {
	TownBook *b = calloc(1, sizeof *b);
	if (!area_src_load(group, number, &b->v[0].a) || !b->v[0].a.idx) { free(b); return NULL; }
	area_src_mirror(&b->v[0].a, &b->v[1].a);
	b->tw = b->v[0].a.tw;
	b->th = b->v[0].a.th;
	for (int vi = 0; vi < 2; ++vi) classify(&b->v[vi]);
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
	}
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

int townsrc_profiles(const TownBook *b, const TownProfile **out) {
	*out = b->prof;
	return b->nprof;
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

void townsrc_synth(const TownBook *b, TownCell cell, int tw, int th, uint32_t seed, uint16_t *map, uint8_t *miss, TownSynthStats *st) {
	PlanCtx ctx = { cell };
	fit_book = b;
	int *src = malloc(sizeof(int) * (size_t)tw * th);
	uint16_t *l0 = map, *l1 = map + (size_t)tw * th;
	TownSynthStats local = { 0 };
	if (!st) st = &local;
	memset(st, 0, sizeof *st);
	uint32_t rs = seed * 2654435761u + 1;
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) {
			size_t at = (size_t)ty * tw + tx;
			src[at] = -1;
			l0[at] = l1[at] = 0;
			if (miss) miss[at] = 0;
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
			/* coherence: the source tile after the left neighbour's, or below the upper one's */
			int pick = -1;
			for (int d = 0; d < 2 && pick < 0; ++d) {
				int n = d == 0 ? (tx ? src[at - 1] : -1) : (ty ? src[at - tw] : -1);
				if (n < 0) continue;
				const Entry *ne = &b->e[n];
				int nx = ne->tx + (d == 0), ny = ne->ty + (d == 1);
				if (nx >= b->tw || ny >= b->th || ne->view != view) continue;
				int c = b->v[ne->view].entry[ny * b->tw + nx];
				if (c >= 0 && b->e[c].key == K->key && full_agree(&b->e[c], k, want, uniform) == ba) { pick = c; st->coherent++; }
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
			src[at] = pick;
			if (miss && b->e[pick].view) miss[at] |= 2;   /* (from the mirror) */
			l0[at] = b->e[pick].e0;
			l1[at] = b->e[pick].e1;
		}
	free(src);
}
