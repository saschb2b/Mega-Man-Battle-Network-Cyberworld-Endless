/* The NaviCust as a run's second build axis (navicust.h, docs/NAVICUST.md). */
#include "navicust.h"

#include "game.h"
#include "pacing.h"
#include "rom.h"

/* The pool: BN6's programs a run can be offered, the act (0-5 of the first
 * cycle, 7 the second cycle) from which a guardian may offer each, its
 * build (what it does: navicust_words.c). Left out: Humor and Poem (they answer the L
 * button, which the engine takes), Rush, Beat and Tango (VS battles only),
 * OilBody, Fish, Battery and Jungle (they steer the game's encounter roll,
 * which the engine replaces), Millions (its Mystery Data rule loops
 * forever on the engine's single-entry Mystery Data), SneakRun (it judges
 * the game's rolled battle, not the engine's) and NumbrOpn (its condition
 * is not yet known). The tiers follow the board: 4x4 until the act 2
 * guardian's ExpMemry, 5x4 until the act 4 guardian's, then 5x5. */
static const struct {
	uint8_t program, tier, build;
} POOL[] = {
	{ 1, 0, BUILD_GUARD },
	{ 2, 0, BUILD_HAND },
	{ 3, 4, BUILD_HAND },
	{ 4, 0, BUILD_HAND },
	{ 5, 2, BUILD_HAND },
	{ 6, 4, BUILD_HAND },
	{ 7, 0, BUILD_GUARD },
	{ 8, 0, BUILD_GUARD },
	{ 9, 2, BUILD_GUARD },
	{ 10, 2, BUILD_GUARD },
	{ 11, 2, BUILD_FIELD },
	{ 12, 2, BUILD_FIELD },
	{ 13, 0, BUILD_GUARD },
	{ 14, 4, BUILD_HAND },
	{ 21, 2, BUILD_FIELD },
	{ 25, 0, BUILD_FIELD },
	{ 26, 2, BUILD_FIELD },
	{ 27, 2, BUILD_BUSTER },
	{ 28, 4, BUILD_GUARD },
	{ 29, 2, BUILD_HAND },
	{ 30, 4, BUILD_HAND },
	{ 31, 4, BUILD_FIELD },
	{ 35, 0, BUILD_BUSTER },
	{ 36, 0, BUILD_BUSTER },
	{ 37, 0, BUILD_BUSTER },
	{ 38, 4, BUILD_BUSTER },
	{ 39, 2, BUILD_BUSTER },
	{ 40, 2, BUILD_BUSTER },
	{ 41, 0, BUILD_HP },
	{ 42, 0, BUILD_HP },
	{ 43, 2, BUILD_HP },
	{ 44, 7, BUILD_HP },
	{ 45, 7, BUILD_HP },
	{ 46, 7, BUILD_HP },
};
#define POOL_N ((int)(sizeof POOL / sizeof *POOL))

#define PACK_MAX 10      /* programs the packing takes at once */
#define COMMAND_ROW 2    /* the command line, the board's third row */

static int find(int program) {
	for (int i = 0; i < POOL_N; ++i)
		if (POOL[i].program == program) return i;
	return -1;
}

bool navicust_in_pool(int program) { return find(program) >= 0; }
int navicust_build(int program) { int i = find(program); return i < 0 ? -1 : POOL[i].build; }
int navicust_tier(int program) { int i = find(program); return i < 0 ? -1 : POOL[i].tier; }

/* the act a guardian at `depth` closes, as the pool's tiers count it */
static int reached(int depth) { return pacing_loop(depth) > 0 ? 7 : pacing_act(depth); }

bool navicust_offerable(int program, int depth) {
	int i = find(program);
	return i >= 0 && POOL[i].tier <= reached(depth);
}

int navicust_draft(int depth, NaviProgram out[NAVICUST_DRAFT]) {
	/* three builds of the five, in a random order, one program of each:
	 * the choice is a direction, not three of a kind */
	int builds[BUILD_COUNT] = { BUILD_BUSTER, BUILD_HAND, BUILD_GUARD, BUILD_FIELD, BUILD_HP };
	for (int i = BUILD_COUNT - 1; i > 0; --i) {
		int j = rng_range(0, i), t = builds[i];
		builds[i] = builds[j];
		builds[j] = t;
	}
	int n = 0, top = reached(depth);
	for (int b = 0; b < BUILD_COUNT && n < NAVICUST_DRAFT; ++b) {
		int fit[POOL_N], nfit = 0;
		for (int i = 0; i < POOL_N; ++i)
			if (POOL[i].build == builds[b] && POOL[i].tier <= top) fit[nfit++] = i;
		if (!nfit) continue;
		out[n].program = POOL[fit[rng_range(0, nfit - 1)]].program;
		out[n++].color = 0;
	}
	return n;
}

/* A colour variant of `program` that fits beside `have`, chosen at random
 * among those that do: its colour, 0 for none. */
static int fitting_color(int program, const uint8_t *have, int nhave, int w, int h) {
	NaviShape s[PACK_MAX];
	int n = 0;
	for (int i = 0; i < nhave && n < PACK_MAX - 1; ++i)
		if (navicust_shape(have[i], &s[n])) ++n;
	int colors[4], ncolors = 0;
	for (int v = 0; v < 4; ++v) {
		if (!navicust_shape(program * 4 + v, &s[n])) continue;
		if (navicust_pack(s, n + 1, w, h)) colors[ncolors++] = s[n].color;
	}
	return ncolors ? colors[rng_range(0, ncolors - 1)] : 0;
}

int navicust_draft_fitting(int depth, NaviProgram out[NAVICUST_DRAFT], const uint8_t *have, int nhave, int w, int h) {
	if (!R.data) return navicust_draft(depth, out);
	/* as navicust_draft, three builds in a random order; in each, its
	 * programs in a random order until one fits (a playtester drafted a
	 * SuprArmr that could not share his 4x4 board with the gift's Custom1,
	 * and nothing said so) */
	int builds[BUILD_COUNT] = { BUILD_BUSTER, BUILD_HAND, BUILD_GUARD, BUILD_FIELD, BUILD_HP };
	for (int i = BUILD_COUNT - 1; i > 0; --i) {
		int j = rng_range(0, i), t = builds[i];
		builds[i] = builds[j];
		builds[j] = t;
	}
	int n = 0, top = reached(depth);
	for (int b = 0; b < BUILD_COUNT && n < NAVICUST_DRAFT; ++b) {
		int fit[POOL_N], nfit = 0;
		for (int i = 0; i < POOL_N; ++i)
			if (POOL[i].build == builds[b] && POOL[i].tier <= top) fit[nfit++] = i;
		for (int i = nfit - 1; i > 0; --i) {
			int j = rng_range(0, i), t = fit[i];
			fit[i] = fit[j];
			fit[j] = t;
		}
		for (int i = 0; i < nfit; ++i) {
			int c = fitting_color(POOL[fit[i]].program, have, nhave, w, h);
			if (!c) continue;
			out[n].program = POOL[fit[i]].program;
			out[n++].color = (uint8_t)c;
			break;
		}
	}
	return n;
}

int navicust_color(int program) {
	/* the program records: 16 bytes per colour variant (program * 4 + v),
	 * the colour at +3, 0 where the variant is absent */
	if (!R.data || !R.layout || !R.layout->navicust_programs) return 0;
	int colors[4], n = 0;
	for (int v = 0; v < 4; ++v) {
		int c = R.data[R.layout->navicust_programs + (uint32_t)(program * 4 + v) * 16 + 3];
		if (c >= 1 && c <= 6) colors[n++] = c;
	}
	return n ? colors[rng_range(0, n - 1)] : 0;
}

bool navicust_code_text(const uint8_t raw[10], char out[12]) {
	/* (BN6's bytes are the offsets of its joypad masks: 0 L, 2 R, 4 A, 6 B,
	 * bn6f sub_813C334) */
	static const char letters[] = "LRAB";
	int k = 0;
	for (int i = 0; i < 10; ++i) {
		if (raw[i] > 6 || raw[i] & 1) return false;
		if (i == 5) out[k++] = ' ';
		out[k++] = letters[raw[i] / 2];
	}
	out[k] = 0;
	return true;
}

bool navicust_code(int program, char out[12]) {
	if (!R.data || !R.layout || !R.layout->navicust_codes || program <= 0 || program >= NAVICUST_PROGRAMS) return false;
	return navicust_code_text(R.data + R.layout->navicust_codes + (uint32_t)program * 10, out);
}

bool navicust_shape(int variant, NaviShape *out) { return navicust_shape_as(variant, false, out); }

bool navicust_shape_as(int variant, bool compressed, NaviShape *out) {
	/* the record's +1 the kind, +3 the colour, +8 a pointer to the shape:
	 * 49 bytes, a row of seven after another, +0xC to the compressed one
	 * (docs/ROM_DATA.md) */
	if (!R.data || !R.layout || !R.layout->navicust_programs || variant <= 0 || variant >= 47 * 4) return false;
	const uint8_t *rec = R.data + R.layout->navicust_programs + (uint32_t)variant * 16, *p = rec + (compressed ? 12 : 8);
	uint32_t at = (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
	if (compressed && ((at >> 25) != 4 || (at & 0x1FFFFFF) + 49 > ROM_SIZE)) return navicust_shape_as(variant, false, out);
	if (rec[3] < 1 || rec[3] > 6 || (at >> 25) != 4 || (at & 0x1FFFFFF) + 49 > ROM_SIZE) return false;
	const uint8_t *g = R.data + (at & 0x1FFFFFF);
	int cells = 0;
	for (int y = 0; y < 7; ++y)
		for (int x = 0; x < 7; ++x) cells += (out->cell[y][x] = g[y * 7 + x] != 0);
	out->kind = rec[1] <= 2 ? rec[1] : NAVI_EITHER;
	out->color = rec[3];
	return cells > 0;
}

void navicust_board(int expmemry, int *w, int *h) {
	*w = expmemry >= 1 ? 5 : 4;
	*h = expmemry >= 2 ? 5 : 4;
}

/* ---- packing: every placement of every turn, depth first (a board of 25
 * cells and a handful of programs) ---- */

typedef struct { int n; int8_t x[49], y[49]; int w, h; } Turn;
typedef struct { Turn turn[4]; int nturns; int kind, color; } Piece;

static void turn_of(const NaviShape *s, int k, Turn *t) {
	/* turned k quarters clockwise, then moved to its corner */
	int n = 0, minx = 7, miny = 7, maxx = 0, maxy = 0;
	int8_t xs[49], ys[49];
	for (int y = 0; y < 7; ++y)
		for (int x = 0; x < 7; ++x) {
			if (!s->cell[y][x]) continue;
			int rx = x, ry = y;
			for (int q = 0; q < k; ++q) { int tx = 6 - ry; ry = rx; rx = tx; }
			xs[n] = (int8_t)rx; ys[n] = (int8_t)ry; ++n;
			if (rx < minx) minx = rx;
			if (ry < miny) miny = ry;
			if (rx > maxx) maxx = rx;
			if (ry > maxy) maxy = ry;
		}
	t->n = n;
	for (int i = 0; i < n; ++i) { t->x[i] = (int8_t)(xs[i] - minx); t->y[i] = (int8_t)(ys[i] - miny); }
	t->w = maxx - minx + 1;
	t->h = maxy - miny + 1;
}

static bool same_turn(const Turn *a, const Turn *b) {
	if (a->n != b->n || a->w != b->w || a->h != b->h) return false;
	for (int i = 0; i < a->n; ++i) {
		bool found = false;
		for (int j = 0; j < b->n && !found; ++j) found = a->x[i] == b->x[j] && a->y[i] == b->y[j];
		if (!found) return false;
	}
	return true;
}

static int8_t board[7][7];   /* the piece standing on each cell, -1 none */
static unsigned spins = 0x3F;

void navicust_set_spins(unsigned mask) { spins = mask & 0x3F; }
unsigned navicust_spins(void) { return spins; }

static bool place(const Piece *p, const Turn *t, int ox, int oy, int w, int h, int id) {
	bool on_line = false;
	for (int i = 0; i < t->n; ++i) {
		int x = ox + t->x[i], y = oy + t->y[i];
		if (x < 0 || y < 0 || x >= w || y >= h || board[y][x] >= 0) return false;
		on_line |= y == COMMAND_ROW;
	}
	if ((p->kind == NAVI_PART && !on_line) || (p->kind == NAVI_PLUS && on_line)) return false;
	for (int i = 0; i < t->n; ++i) board[oy + t->y[i]][ox + t->x[i]] = (int8_t)id;
	return true;
}

static void lift(const Turn *t, int ox, int oy) {
	for (int i = 0; i < t->n; ++i) board[oy + t->y[i]][ox + t->x[i]] = -1;
}

/* (two programs of one colour side by side, program or plus parts alike:
 * a bug. BN6's compile, bn6f sub_813BD24 and sub_813BE38, gathers each
 * program's neighbours and counts every one of its colour, whatever its
 * kind; plus parts were spared here, and a draft offered a blue ChargMAX
 * beside a blue HP+100 on a 5x4 board, where no placement runs clean,
 * session 64) */
static bool touches_kin(const Piece *pieces, const Turn *t, int ox, int oy, int w, int h, int id) {
	static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int i = 0; i < t->n; ++i)
		for (int k = 0; k < 4; ++k) {
			int x = ox + t->x[i] + d[k][0], y = oy + t->y[i] + d[k][1];
			if (x < 0 || y < 0 || x >= w || y >= h) continue;
			int o = board[y][x];
			if (o >= 0 && o != id && pieces[o].color == pieces[id].color) return true;
		}
	return false;
}

static bool pack_from(const Piece *pieces, const int *order, int k, int n, int w, int h) {
	if (k == n) return true;
	int id = order[k];
	const Piece *p = &pieces[id];
	for (int r = 0; r < p->nturns; ++r) {
		const Turn *t = &p->turn[r];
		for (int oy = 0; oy + t->h <= h; ++oy)
			for (int ox = 0; ox + t->w <= w; ++ox) {
				if (!place(p, t, ox, oy, w, h, id)) continue;
				bool ok = !touches_kin(pieces, t, ox, oy, w, h, id) && pack_from(pieces, order, k + 1, n, w, h);
				lift(t, ox, oy);
				if (ok) return true;
			}
	}
	return false;
}

/* A shape as a piece, with the turns the NaviCust gives it: all four where
 * `turns` (a board laid out under another build), else only with its
 * colour's Spin, as the NaviCust's L and R (bn6f sub_8136364 checks key
 * item 0x4F + the record's colour). */
static void piece_of(const NaviShape *s, bool turns, Piece *p) {
	p->kind = s->kind;
	p->color = s->color;
	p->nturns = 0;
	for (int k = 0; k < 4; ++k) {
		if (k && !turns && !(s->color >= 1 && spins >> (s->color - 1) & 1)) break;
		Turn t;
		turn_of(s, k, &t);
		bool again = false;
		for (int j = 0; j < p->nturns && !again; ++j) again = same_turn(&t, &p->turn[j]);
		if (!again) p->turn[p->nturns++] = t;
	}
}

/* (shapes before `nfree` turn whatever their colour) */
static bool pack(const NaviShape *shapes, int n, int w, int h, int nfree) {
	if (n <= 0) return true;
	if (n > PACK_MAX || w > 7 || h > 7) return false;
	static Piece pieces[PACK_MAX];
	int order[PACK_MAX], cells = 0;
	for (int i = 0; i < n; ++i) {
		piece_of(&shapes[i], i < nfree, &pieces[i]);
		cells += pieces[i].turn[0].n;
		order[i] = i;
	}
	if (cells > w * h) return false;
	/* (the biggest first: the fewest ways to place) */
	for (int i = 1; i < n; ++i)
		for (int j = i; j > 0 && pieces[order[j]].turn[0].n > pieces[order[j - 1]].turn[0].n; --j) {
			int t = order[j]; order[j] = order[j - 1]; order[j - 1] = t;
		}
	for (int y = 0; y < 7; ++y) for (int x = 0; x < 7; ++x) board[y][x] = -1;
	return pack_from(pieces, order, 0, n, w, h);
}

bool navicust_pack(const NaviShape *shapes, int n, int w, int h) {
	/* the last shape is the one to fit; those before it are the board's.
	 * A board laid out while a Spin was held that the profile lacks (a run
	 * from a build that gave all six) may hold them turned: where they fit
	 * together only so, they may turn, and the new one only with its
	 * Spin */
	int nfree = n > 1 && spins != 0x3F && !pack(shapes, n - 1, w, h, 0) ? n - 1 : 0;
	return pack(shapes, n, w, h, nfree);
}

bool navicust_fits_free(const uint8_t grid[NAVICUST_GRID * NAVICUST_GRID], const NaviPart *parts, int n, const NaviShape *shape, int w, int h) {
	static Piece pieces[NAVICUST_GRID * NAVICUST_GRID + 1];
	if (n < 0 || n >= NAVICUST_GRID * NAVICUST_GRID || w > 5 || h > 5) return false;
	for (int y = 0; y < 7; ++y) for (int x = 0; x < 7; ++x) board[y][x] = -1;
	/* the board's programs where they stand (the grid's from (1, 1)) */
	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x) {
			int p = grid[(y + 1) * NAVICUST_GRID + x + 1];
			if (p && p <= n) board[y][x] = (int8_t)(p - 1);
		}
	for (int i = 0; i < n; ++i) pieces[i] = (Piece){ .kind = parts[i].kind, .color = parts[i].color };
	const Piece *p = &pieces[n];
	piece_of(shape, false, &pieces[n]);
	for (int r = 0; r < p->nturns; ++r) {
		const Turn *t = &p->turn[r];
		for (int oy = 0; oy + t->h <= h; ++oy)
			for (int ox = 0; ox + t->w <= w; ++ox) {
				if (!place(p, t, ox, oy, w, h, n)) continue;
				bool ok = !touches_kin(pieces, t, ox, oy, w, h, n);
				lift(t, ox, oy);
				if (ok) return true;
			}
	}
	return false;
}

int navicust_skip_frags(int depth) { return 10 + 5 * (pacing_act(depth) + 7 * pacing_loop(depth)); }

bool navicust_expmemry(int depth) { return depth == 6 || depth == 12; }

/* (BN6's own rule: a program turns only where its colour's Spin is held;
 * MegaMan had said L and R turn a program, and a playtester with no Spin
 * spent twenty calls pressing them, docs/NAVICUST.md 8) */
static const char *const spin_names[7] = { "", "white", "yellow", "pink", "red", "blue", "green" };

const char *navicust_color_name(int c) { return c >= 1 && c <= 6 ? spin_names[c] : ""; }
