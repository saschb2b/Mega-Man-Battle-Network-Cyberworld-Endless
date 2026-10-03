/* The NaviCust as a run's second build axis (navicust.h, docs/NAVICUST.md). */
#include "navicust.h"

#include <stdio.h>
#include <string.h>

#include "game.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"

/* The pool: BN6's programs a run can be offered, the act (0-5 of the first
 * cycle, 7 the second cycle) from which a guardian may offer each, its
 * build, and what it does. Left out: Humor and Poem (they answer the L
 * button, which the engine takes), Rush, Beat and Tango (VS battles only),
 * OilBody, Fish, Battery and Jungle (they steer the game's encounter roll,
 * which the engine replaces), Millions (its Mystery Data rule loops
 * forever on the engine's single-entry Mystery Data), SneakRun (it judges
 * the game's rolled battle, not the engine's) and NumbrOpn (its condition
 * is not yet known). The tiers follow the board: 4x4 until the act 2
 * guardian's ExpMemry, 5x4 until the act 4 guardian's, then 5x5. */
static const struct {
	uint8_t program, tier, build;
	const char *about;
} POOL[] = {
	{ 1, 0, BUILD_GUARD, "SuperArmor: I won't flinch when I'm hit." },
	{ 2, 0, BUILD_HAND, "Custom1: one more chip each turn." },
	{ 3, 4, BUILD_HAND, "Custom2: two more chips each turn." },
	{ 4, 0, BUILD_HAND, "MegFldr1: room for one more Mega chip in the folder." },
	{ 5, 2, BUILD_HAND, "MegFldr2: room for two more Mega chips in the folder." },
	{ 6, 4, BUILD_HAND, "GigFldr1: room for a second Giga chip in the folder." },
	{ 7, 0, BUILD_GUARD, "FstBarr: every battle starts with a Barrier on me." },
	{ 8, 0, BUILD_GUARD, "Shield: B and Left raises a shield." },
	{ 9, 2, BUILD_GUARD, "Reflect: B and Left raises a shield that shoots back." },
	{ 10, 2, BUILD_GUARD, "AntiDmg: B and Left, and a hit becomes my counterattack." },
	{ 11, 2, BUILD_FIELD, "FlotShoe: panels can't hurt me or hold me." },
	{ 12, 2, BUILD_FIELD, "AirShoes: I can stand over holes." },
	{ 13, 0, BUILD_GUARD, "UnderSht: a hit that would delete me leaves me 1 HP instead." },
	{ 14, 4, BUILD_HAND, "ChpShufl: one reshuffle in the Custom screen." },
	{ 21, 2, BUILD_FIELD, "Collect: viruses drop their chips more often." },
	{ 25, 0, BUILD_FIELD, "SlipRunr: with B, I slide instead of running: faster, but I keep going till something stops me." },
	{ 26, 2, BUILD_FIELD, "AutoHeal: a little HP back after every battle." },
	{ 27, 2, BUILD_BUSTER, "BustPack: buster attack, speed and charge up three each." },
	{ 28, 4, BUILD_GUARD, "BodyPack: SuperArmor, FlotShoe, AirShoes and UnderSht in one." },
	{ 29, 2, BUILD_HAND, "FldrPak1: Custom1 and MegFldr1 in one." },
	{ 30, 4, BUILD_HAND, "FldrPak2: Custom2 and MegFldr2 in one." },
	{ 31, 4, BUILD_FIELD, "BugStop: no bug can touch our NaviCust." },
	{ 35, 0, BUILD_BUSTER, "Attack+1: a stronger buster." },
	{ 36, 0, BUILD_BUSTER, "Speed+1: a faster buster." },
	{ 37, 0, BUILD_BUSTER, "Charge+1: a quicker charge shot." },
	{ 38, 4, BUILD_BUSTER, "AttckMAX: my buster's attack at its highest." },
	{ 39, 2, BUILD_BUSTER, "SpeedMAX: my buster's speed at its highest." },
	{ 40, 2, BUILD_BUSTER, "ChargMAX: my charge shot at its quickest." },
	{ 41, 0, BUILD_HP, "HP+50: fifty more max HP." },
	{ 42, 0, BUILD_HP, "HP+100: a hundred more max HP." },
	{ 43, 2, BUILD_HP, "HP+200: two hundred more max HP." },
	{ 44, 7, BUILD_HP, "HP+300: three hundred more max HP." },
	{ 45, 7, BUILD_HP, "HP+400: four hundred more max HP." },
	{ 46, 7, BUILD_HP, "HP+500: five hundred more max HP." },
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
const char *navicust_about(int program) { int i = find(program); return i < 0 ? NULL : POOL[i].about; }

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

const char *navicust_color_turns(int c) {
	static char buf[120];
	if (c < 1 || c > 6) return "";
	if (spins >> (c - 1) & 1) snprintf(buf, sizeof buf, "L and R turn it as we place it: we hold the %s Spin.", spin_names[c]);
	else snprintf(buf, sizeof buf, "L and R won't turn it: that takes the %s Spin, and we don't have it.", spin_names[c]);
	return buf;
}

const char *navicust_turn_words(int variant) {
	static char buf[160];
	int c = 0, held = 0;
	if (variant > 0 && variant < 47 * 4 && R.data && R.layout && R.layout->navicust_programs)
		c = R.data[R.layout->navicust_programs + (uint32_t)variant * 16 + 3];
	if (c >= 1 && c <= 6) return navicust_color_turns(c);
	for (int k = 1; k <= 6; ++k) held += spins >> (k - 1) & 1;
	if (!held) return "A program turns with L and R only once we hold a Spin of its color, and we have none yet.";
	if (held == 6) return "L and R turn a program as we place it from the list.";
	int k = snprintf(buf, sizeof buf, "L and R turn only "), n = 0;
	for (int i = 1; i <= 6; ++i)
		if (spins >> (i - 1) & 1)
			k += snprintf(buf + k, sizeof buf - (size_t)k, "%s%s", n++ == 0 ? "" : n == held ? " and " : ", ", spin_names[i]);
	snprintf(buf + k, sizeof buf - (size_t)k, " programs as we place them: we hold %s.", held == 1 ? "that Spin" : "those Spins");
	return buf;
}

/* (BN6's compile, bn6f sub_813BBD4: a program's shape lands at its
 * column and row less 3 on the 7x7 grid; the command line is the grid's
 * row 3, whatever the board's size; colours touch only inside its 5x5) */
#define CMD_ROW 3

/* what the rules find of part `p` (1-based): a bit each, by the order the
 * words take */
enum { CAUSE_PLUS = 1, CAUSE_OFF = 2, CAUSE_EDGE = 4 };
static int part_causes(const uint8_t *grid, const NaviPart *part, int p, int w, int h) {
	bool on_line = false, past = false;
	for (int y = 0; y < NAVICUST_GRID; ++y)
		for (int x = 0; x < NAVICUST_GRID; ++x) {
			if (grid[y * NAVICUST_GRID + x] != p) continue;
			on_line |= y == CMD_ROW;
			past |= x < 1 || x > w || y < 1 || y > h;
		}
	return (part->kind == 1 && on_line ? CAUSE_PLUS : 0) | (part->kind == 0 && !on_line ? CAUSE_OFF : 0) | (past ? CAUSE_EDGE : 0);
}

/* the first two programs of one colour side by side inside the 5x5 (their
 * indexes, 1-based, in *a and *b); false for none */
static bool same_colors(const uint8_t *grid, const NaviPart *parts, int n, int *a, int *b) {
	for (int y = 1; y <= 5; ++y)
		for (int x = 1; x <= 5; ++x) {
			int p = grid[y * NAVICUST_GRID + x];
			if (!p || p > n) continue;
			int q[2] = { x < 5 ? grid[y * NAVICUST_GRID + x + 1] : 0, y < 5 ? grid[(y + 1) * NAVICUST_GRID + x] : 0 };
			for (int k = 0; k < 2; ++k)
				if (q[k] && q[k] <= n && q[k] != p && parts[q[k] - 1].color == parts[p - 1].color) { *a = p; *b = q[k]; return true; }
		}
	return false;
}

const char *navicust_bug_cause(const uint8_t grid[NAVICUST_GRID * NAVICUST_GRID], const NaviPart *parts, int n, int w, int h) {
	static const char *const why[3] = {
		" is a plus part on the command line: plus parts go anywhere else.",
		" is off the command line: a program needs a block on it.",
		" goes past the board's edge.",
	};
	static char buf[300];
	int k = 0, said = 0;
	buf[0] = 0;
	for (int p = 1; p <= n && said < 2; ++p) {
		int c = part_causes(grid, &parts[p - 1], p, w, h);
		for (int r = 0; r < 3 && said < 2; ++r)
			if (c >> r & 1 && parts[p - 1].name) {
				k += snprintf(buf + k, sizeof buf - (size_t)k, "%s%s%s", k ? " " : "", parts[p - 1].name, why[r]);
				++said;
			}
	}
	int a, b;
	if (said < 2 && same_colors(grid, parts, n, &a, &b) && parts[a - 1].name && parts[b - 1].name) {
		k += snprintf(buf + k, sizeof buf - (size_t)k, "%s%s and %s, both %s, touch.", k ? " " : "", parts[a - 1].name, parts[b - 1].name,
			navicust_color_name(parts[a - 1].color));
		++said;
	}
	return said ? buf : NULL;
}

const char *navicust_bug_words(const uint8_t counts[NAVICUST_BUGS], bool after_run, const char *cause) {
	/* the game's bug types (its compile counts one per violation; the
	 * level is the count, up to 3) and what each does, in MegaMan's words */
	static const char *const name[NAVICUST_BUGS] = {
		[1] = "moving", [2] = "emotion", [3] = "panel", [4] = "Custom", [5] = "encounter", [6] = "reward",
		[7] = "buster", [9] = "HP",
	};
	static const char *const effect[NAVICUST_BUGS] = {
		[1] = "every step slides me as far as I can go",
		[2] = "my mood will swing in battle",
		[3] = "panels may crack under me as I move",
		[4] = "fewer chips each turn as a battle goes on",
		[5] = "more viruses will find us",
		[6] = "battles will pay zenny instead of chips",
		[7] = "my buster may misfire",
		[9] = "I'll lose HP in battle, faster with every hit",
		[11] = "five colors: something odd happens at the start of every battle",
		[12] = "six colors: something odd happens at the start of every battle, for longer",
	};
	static char buf[800];
	int k = 0, n = 0;
	for (int t = 1; t < NAVICUST_BUGS; ++t) n += counts[t] && effect[t];
	if (!n) return "";
	/* (after the RUN: its "OK! RUN complete!" and "Good job, Lan!" are the
	 * game's whatever the board, and a playtester read them as clean) */
	k += snprintf(buf + k, sizeof buf - (size_t)k, "@M Lan, %s has %s!", after_run ? "the RUN says OK, but our NaviCust" : "our NaviCust",
		n == 1 ? "a bug" : "bugs");
	for (int t = 1; t < NAVICUST_BUGS && k < (int)sizeof buf - 160; ++t) {
		if (!counts[t] || !effect[t]) continue;
		if (!name[t]) {
			k += snprintf(buf + k, sizeof buf - (size_t)k, "|@M %c%s!", effect[t][0] - 'a' + 'A', effect[t] + 1);
			continue;
		}
		int level = counts[t] > 3 ? 3 : counts[t];
		bool vowel = strchr("aeiouAEIOU", name[t][0]) || name[t][0] == 'H';   /* ("an HP bug") */
		const char *article = level == 1 ? "A light" : level == 3 ? "A bad" : vowel ? "An" : "A";
		k += snprintf(buf + k, sizeof buf - (size_t)k, "|@M %s %s bug: %s.", article, name[t], effect[t]);
	}
	/* (where to look, from the game's rules: a playtester told only what a
	 * bug did, of a program he had pushed over the edge, found the board
	 * looking clean again; the colours' count names itself) */
	bool placed = false;
	for (int t = 1; t < 11; ++t) placed |= counts[t] && effect[t];
	/* (and where it can be read from the board, what: a playtester told
	 * the four rules looked for the one he had broken) */
	if (placed && cause && k < (int)sizeof buf - 320) k += snprintf(buf + k, sizeof buf - (size_t)k, "|@M %s", cause);
	else if (placed && k < (int)sizeof buf - 220)
		k += snprintf(buf + k, sizeof buf - (size_t)k,
			"|@M Bugs come from a program over the board's edge or off the command line, a Plus part on it, "
			"or two of one color side by side.");
	if (k < (int)sizeof buf - 200)
		snprintf(buf + k, sizeof buf - (size_t)k, "|@M We can rearrange it in the PET, or live with it. %s", navicust_turn_words(0));
	return buf;
}
