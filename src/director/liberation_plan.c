/* A liberation board's planner (liberation_plan.h; docs/LIBERATION.md, The
 * quality bar). It plays the board as a fair player would, with the team
 * it was made for: each phase every Navi in turn (support first: ToadMan's
 * LifeMelody, ShadowMan's SneakAttack, NumberMan's NumberCheck, then the
 * rest) moves over the clean panels and takes the best of what he could do
 * there, each move worth what it brings the run's dialectic at the board's
 * scale (press on toward a DarkHole, the Darkloid or a key; prepare: Order
 * Points, a heart, a rest, a guardian out of the way; a detour for loot or
 * the sealed region's route), less what the board shows it costs (a
 * guardian's reach where he would stand, a trap's harm, an Order Point).
 * Battles are won in one turn, in three or lost by the dark around (BN5's
 * text; tools/liberation.py's SKILL), the Darkloids' phase as liberation.c
 * plays it, each unit going for the nearest Navi. A decision's sensible
 * moves are those of different purpose worth four fifths of the best or
 * more: a decision with one is dead. A teammate deleted leaves the board;
 * MegaMan deleted loses the play. The par is the median of the plays won. */
#include "liberation_plan.h"

#include <stdlib.h>
#include <string.h>

#define PLAYS        12      /* plays of the board */
bool lib_plan_lax;
#define BARE_PLAYS   6       /* ... with its rewards stripped (the quality bar's 2) */
#define PHASE_CAP    45
#define SENSIBLE     80      /* percent of the best move's worth a sensible move has */
#define WIN_MIN      60      /* percent of plays won, at least */
#define DEAD_MAX     55      /* percent of decisions with one sensible move, at most */
#define DEAD_TEAM    60      /* ... with a team */
#define BARE_MAX     60      /* ... on the bare board */
#define LONGSWRD_LEAST 50    /* the Navis' commands taken, per hundred plays, at least */
#define LONGSWRD_MOST  40    /* ... the best move in at most this percent of decisions */
#define ROUTE_PLAYS  4       /* ... by a player going for the sealed region's route */
#define ROUTE_LEAST  2       /* the route's cost in phases over the par: at least (a choice, not a gift) */
#define ROUTE_MOST   10      /* ... and at most (a choice, not a wall) */

/* MegaMan's HP on the board by act (a run's: about a hundred more an act,
 * its HPMemory, docs/ECONOMY.md) and a virus battle's harm (BN5's numbers
 * for its missions, tools/liberation.py's ACTS) */
static const int mm_hp[6] = { 300, 300, 400, 500, 600, 700 };
static const int virus[6] = { 14, 14, 20, 28, 34, 40 };
static const int dir4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
static const int dir8[8][2] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };

/* The board's panels by index, and their neighbours */
static int16_t at[MAP_H][MAP_W];
static uint8_t px[MAP_W * MAP_H], py[MAP_W * MAP_H];
static int16_t nb[MAP_W * MAP_H][4];
static int np;

/* The maps a phase reads: each DarkHole's, the Darkloid's, the keys' (of
 * barriers walling a target off), the sealed region's */
enum { MAPS = LIB_HOLES + 3, MAP_BOSS = LIB_HOLES, MAP_KEY, MAP_ROUTE };
#define FAR 0x7F00
/* A move's purpose: a map's, or these */
enum { WHY_WIDEN = MAPS, WHY_ITEM, WHY_UNIT, WHY_SHIELD = WHY_UNIT + LIB_UNITS };
enum { CMD_LIB, CMD_LONG, CMD_PASS, CMD_FIGHT };   /* (CMD_LONG: the Navi's command, LongSwrd or his own) */

static struct {
	LibBoard b;
	int me, hp, max, op;   /* the Navi acting: his panel, his HP and max (his own kept in mhp between his turns) */
	int k, command;        /* ... his place in the team, his command (LIB_DO_*) */
	int mhp[LIB_TEAM], mmax[LIB_TEAM];
	bool acted[LIB_TEAM], melody[LIB_TEAM], weak[LIB_UNITS];
	bool shield, strip, lost;
	int taste;             /* percent: his care for the sealed region's route (0 the par's player) */
	uint32_t rs;
	/* its measures */
	int dead, moves, phases, strikes, best[4], longswrd, hurt, order, out;
	bool sealed;
} P;

static int16_t cost[MAPS][MAP_W * MAP_H];
static int here[MAPS], weight[MAPS];
static bool live[MAPS];
static int16_t stands[MAP_W * MAP_H], threat[MAP_W * MAP_H];
static int nstands;

typedef struct { int value, cls, cmd, stand, t, dir; } Option;
#define OPTIONS 512
static Option opt[OPTIONS];
static int nopt;

static uint32_t rnext(void) { P.rs ^= P.rs << 13; P.rs ^= P.rs >> 17; P.rs ^= P.rs << 5; return P.rs; }
static int rr(int n) { return n <= 1 ? 0 : (int)(rnext() % (uint32_t)n); }

static int K(int i) { return P.b.kind[py[i]][px[i]]; }
static int item(int i) { return P.b.item[py[i]][px[i]]; }
static int at_xy(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H ? at[y][x] : -1; }

static int8_t occ[MAP_W * MAP_H];   /* the unit on each panel, -1 none: kept by units_placed */

static void units_placed(void) {
	memset(occ, 0xFF, (size_t)np);
	for (int u = 0; u < P.b.nunits; ++u)
		if (!P.b.unit[u].gone) occ[at[P.b.unit[u].y][P.b.unit[u].x]] = (int8_t)u;
}

static int unit_on(int i) { return occ[i]; }

static void index_board(const LibBoard *b) {
	np = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			at[y][x] = b->kind[y][x] != LIB_VOID ? (int16_t)np : -1;
			if (at[y][x] >= 0) { px[np] = (uint8_t)x; py[np] = (uint8_t)y; ++np; }
		}
	for (int i = 0; i < np; ++i)
		for (int d = 0; d < 4; ++d) nb[i][d] = (int16_t)at_xy(px[i] + dir4[d][0], py[i] + dir4[d][1]);
}

/* ---- this phase's maps ---- */

/* Map m from its sources: the dark panels to liberate from each panel to
 * the nearest source (a 0-1 walk, barriers shut) */
static void build_map(int m, const int16_t *src, int nsrc) {
	static int16_t dq[MAP_W * MAP_H * 2];
	int16_t *c = cost[m];
	for (int i = 0; i < np; ++i) c[i] = FAR;
	int head = MAP_W * MAP_H, tail = head;
	for (int s = 0; s < nsrc; ++s) { c[src[s]] = 0; dq[tail++] = src[s]; }
	while (head < tail) {
		int i = dq[head++], w = lib_dark_kind(K(i)) ? 1 : 0;
		for (int d = 0; d < 4; ++d) {
			int n = nb[i][d];
			if (n < 0 || K(n) == LIB_BAR || c[i] + w >= c[n]) continue;
			c[n] = (int16_t)(c[i] + w);
			if (w) dq[tail++] = (int16_t)n;
			else dq[--head] = (int16_t)n;
		}
	}
	live[m] = true;
	here[m] = FAR;
	for (int s = 0; s < nstands; ++s) if (c[stands[s]] < here[m]) here[m] = c[stands[s]];
	live[m] = here[m] < FAR;
}

/* Where he may stand this phase: the clean panels joined to his, past no
 * unit (the team's Navis walk through one another, as BN5's do); the dark
 * ones too for BombLiberation's flier */
static void reach(void) {
	static uint8_t seen[MAP_W * MAP_H];
	units_placed();
	memset(seen, 0, (size_t)np);
	nstands = 0;
	stands[nstands++] = (int16_t)P.me;
	seen[P.me] = 1;
	for (int h = 0; h < nstands; ++h)
		for (int d = 0; d < 4; ++d) {
			int n = nb[stands[h]][d];
			if (n < 0 || seen[n] || unit_on(n) >= 0) continue;
			if (K(n) != LIB_CLEAN && K(n) != LIB_BONUS && !(P.command == LIB_DO_BOMB && lib_flown(&P.b, px[n], py[n]))) continue;
			seen[n] = 1;
			stands[nstands++] = (int16_t)n;
		}
	for (int s = 0; s < nstands; ++s) threat[stands[s]] = -1;
}

/* The board's units' harm at stand s next phase (lib_threat), once a
 * phase; none for KnightMan, whose StoneBody takes none */
static int threat_at(int s) {
	if (P.b.team.navi[P.k] == MATE_KNIGHTMAN) return 0;
	if (threat[s] < 0) threat[s] = (int16_t)lib_threat(&P.b, px[s], py[s]);
	return threat[s];
}

static int item_panels(int got, int number, int16_t *out) {
	int n = 0;
	for (int i = 0; i < np; ++i)
		if (K(i) == LIB_ITEM && (item(i) & 15) == got && (number < 0 || (item(i) >> 4) == number)) out[n++] = (int16_t)i;
	return n;
}

static void maps(void) {
	static int16_t src[MAP_W * MAP_H];
	memset(live, 0, sizeof live);
	int holes = 0, near = FAR;
	for (int h = 0; h < P.b.nholes; ++h) {
		int i = at[P.b.hole_y[h]][P.b.hole_x[h]];
		if (K(i) != LIB_HOLE) continue;
		++holes;
		src[0] = (int16_t)i;
		build_map(h, src, 1);
		if (live[h] && here[h] < near) near = here[h];
	}
	if (!holes) { src[0] = (int16_t)at[P.b.boss_y][P.b.boss_x]; build_map(MAP_BOSS, src, 1); near = here[MAP_BOSS]; }
	/* (a target walled off: the keys of the barriers before the sealed one) */
	bool walled = holes ? near >= FAR : !live[MAP_BOSS];
	int n = 0;
	for (int k = 1; walled && k <= P.b.nbars; ++k) if (k != P.b.sealed) n += item_panels(LIB_GOT_KEY, k, src + n);
	if (n) build_map(MAP_KEY, src, n);
	if (P.b.sealed && P.taste) {
		n = item_panels(LIB_GOT_KEY, P.b.sealed, src);
		if (!n) n = item_panels(LIB_GOT_ROUTE, -1, src);
		if (n) build_map(MAP_ROUTE, src, n);
	}
	for (int m = 0; m < MAPS; ++m)
		weight[m] = m < LIB_HOLES ? 100 * (near + 2) / (here[m] + 2) : m == MAP_ROUTE ? P.taste : 100;
}

/* ---- what a move is worth (hundredths) ---- */

static int item_worth(int i) {
	if (K(i) == LIB_TRAP) return -2000 * lib_trap_harm(P.b.act) / (P.hp > 0 ? P.hp : 1);
	if (K(i) != LIB_ITEM) return 0;
	switch (item(i) & 15) {
	case LIB_GOT_OP: return P.strip ? 0 : P.op < 6 ? 250 : 50;
	case LIB_GOT_HEART: return P.strip ? 0 : 300 * (P.max - P.hp) / P.max;
	case LIB_GOT_ZENNY: return P.strip ? 0 : 80;
	case LIB_GOT_CHIP: return P.strip ? 0 : 120;
	case LIB_GOT_KEY: return 50;
	default: return 400 * P.taste / 100;   /* (the route) */
	}
}

/* Liberating panel i: its way on (toward the map it brings nearest, from
 * panel `from` or from his reach's best), its item; its purpose */
static int panel_worth(int i, int from, int *why) {
	int best = 0, purpose = WHY_WIDEN;
	for (int m = 0; m < MAPS; ++m) {
		if (!live[m]) continue;
		int ref = from >= 0 ? cost[m][from] : here[m];
		int v = cost[m][i] < ref ? 300 * weight[m] / 100 : 0;
		if (v > best) { best = v; purpose = m; }
	}
	if (!best) best = 60;
	/* (the Darkloid's ring once his DarkHoles are gone: each dark panel
	 * of it makes his battle harder) */
	if (K(i) == LIB_RING) { best += 250; purpose = MAP_BOSS; }
	int it = item_worth(i);
	if (it > best) purpose = WHY_ITEM;
	if (why) *why = purpose;
	return best + it;
}

static bool freeable(int i) {
	int k = K(i);
	return k == LIB_DARK || k == LIB_ITEM || k == LIB_TRAP || (k == LIB_RING && !lib_holes_left(&P.b));
}

static int around_worth(int t) {
	int v = 0;
	for (int d = 0; d < 8; ++d) {
		int n = at_xy(px[t] + dir8[d][0], py[t] + dir8[d][1]);
		if (n >= 0 && freeable(n) && unit_on(n) < 0) v += panel_worth(n, -1, NULL);
	}
	return v;
}

/* A battle's risk: where its worst harm would take his last HP (a battle
 * lost so ends the run for MegaMan, a teammate's place on the board),
 * never worth it; short of that, the rest's worth weighs his HP */
static int virus_harm(void) { return virus[P.b.act < 0 ? 0 : P.b.act > 5 ? 5 : P.b.act]; }
static int harm_of(int d) { return virus_harm() * (100 + 25 * d) / 100; }
static int risk(int worst) { return worst >= P.hp ? (P.k ? 1500 : 3000) : 0; }

static int p_one(int d) { int p = 75 - 5 * d; return p < 8 ? 8 : p; }
static int p_fail(int d) { int p = 4 + 35 * d / 10; return p > 45 ? 45 : p; }
static int dark_at(int i) { return lib_dark_around(&P.b, px[i], py[i]); }
static int threat_cost(int s) { return threat_at(s) * 600 / (P.hp > 0 ? P.hp : 1); }
static int command_price(void) { return 300 - 40 * P.op; }

/* A move: its worth, and its class (its command, purpose and, for a
 * panel's, whether it is a safe one or a bold one, four or more dark
 * round it: the one-turn win's gamble) */
static void add(int value, int cmd, int why, int s, int t, int dir) {
	if (nopt >= OPTIONS) return;
	bool bold = t >= 0 && cmd != CMD_FIGHT && dark_at(t) >= 4;
	opt[nopt++] = (Option){ value, (cmd * 32 + why) * 2 + bold, cmd, s, t, dir };
}

/* The panels a liberating command frees with the one faced (t), beyond it:
 * LongSwrd's, ScrenDiv's or TomahawkSwing's shape, or a LifeMelody's line
 * (lib_command_panels), the free ones without a unit; into out */
static int also_freed(int command, int t, int dir, int16_t out[6]) {
	int xs[6], ys[6], n = lib_command_panels(&P.b, command, px[t], py[t], dir, xs, ys), m = 0;
	for (int j = 0; j < n; ++j) {
		int i = at_xy(xs[j], ys[j]);
		if (i >= 0 && i != t && freeable(i) && unit_on(i) < 0) out[m++] = (int16_t)i;
	}
	return m;
}

static bool liberating(int command) { return lib_command_battles(command); }

/* Liberation and the Navi's command on panel t from stand s, facing dir */
static void panel_options(int s, int t, int dir, int extra) {
	int d = dark_at(t), why;
	if (K(t) == LIB_HOLE) {
		for (int h = 0; h < P.b.nholes; ++h)
			if (at[P.b.hole_y[h]][P.b.hole_x[h]] == t) add(1050 - risk(virus_harm() * 9 / 4) + extra, CMD_LIB, h, s, t, dir);
		return;
	}
	int v = panel_worth(t, -1, &why), bonus = p_one(d) * 80 / 100 * around_worth(t) / 100, keep = 100 - 35 * d / 10;
	extra -= risk(harm_of(d) * 182 / 100);
	int16_t more[6];
	int lib = v;
	/* (a LifeMelody sung for him: his Liberation frees five ahead) */
	if (P.melody[P.k]) for (int j = 0, n = also_freed(LIB_DO_MELODY, t, dir, more); j < n; ++j) lib += panel_worth(more[j], t, NULL);
	add((lib + bonus) * keep / 100 + extra, CMD_LIB, why, s, t, dir);
	if (!P.op || !liberating(P.command)) return;
	int n = also_freed(P.command, t, dir, more), v2 = 0;
	if (!n) return;
	for (int j = 0; j < n; ++j) v2 += panel_worth(more[j], t, NULL);
	add((v + v2 + bonus) * keep / 100 - command_price() + extra, CMD_LONG, why, s, t, dir);
}

/* NumberCheck on the two by three ahead of stand s: its items taken and
 * its traps disarmed, no battle, nothing liberated */
static void number_option(int s, int t, int dir, int extra) {
	int16_t area[6];
	int xs[6], ys[6], n = lib_command_panels(&P.b, LIB_DO_NUMBER, px[t], py[t], dir, xs, ys), m = 0, v = 0;
	for (int j = 0; j < n; ++j) {
		int i = at_xy(xs[j], ys[j]);
		if (i >= 0 && (K(i) == LIB_ITEM || K(i) == LIB_TRAP)) area[m++] = (int16_t)i;
	}
	if (!m) return;
	for (int j = 0; j < m; ++j) v += K(area[j]) == LIB_TRAP ? -item_worth(area[j]) : item_worth(area[j]) * 9 / 10;
	add(v - command_price() + extra, CMD_LONG, WHY_ITEM, s, t, dir);
}

/* SneakAttack on a unit one or two ahead of stand s: weakened for its
 * battle (ShadowMan's strike never deletes) */
static void sneak_option(int s, int t, int dir, int extra) {
	for (int d = 0; d < 2 && t >= 0; ++d, t = nb[t][dir]) {
		int u = unit_on(t);
		if (u < 0 || P.weak[u] || (u == 0 && lib_holes_left(&P.b))) continue;
		add((u == 0 ? 700 : 350) - command_price() + extra, CMD_LONG, WHY_UNIT + u, s, t, dir);
		return;
	}
}

/* LifeMelody from stand s: the teammates beside it yet to act sung for */
static void melody_option(int s, int extra) {
	int sung = 0;
	for (int j = 0; j < P.b.team.n; ++j)
		if (j != P.k && !P.b.mate[j].out && !P.acted[j] && !P.melody[j] && abs(P.b.mate[j].x - px[s]) + abs(P.b.mate[j].y - py[s]) <= 1) ++sung;
	if (sung) add(320 * sung - command_price() + extra, CMD_LONG, WHY_WIDEN, s, -1, -1);
}

/* MagnetBarrier from stand s: the harm the board's units would do the team
 * next phase, none of it taken */
static void magnet_option(int s, int extra) {
	if (P.shield) return;
	int v = threat_at(s) * 600 / (P.hp > 0 ? P.hp : 1);
	for (int j = 0; j < P.b.team.n; ++j)
		if (j != P.k && !P.b.mate[j].out && P.mhp[j] > 0) v += lib_threat(&P.b, P.b.mate[j].x, P.b.mate[j].y) * 600 / P.mhp[j];
	if (v) add(v - command_price() + extra, CMD_LONG, WHY_SHIELD, s, -1, -1);
}

/* BombLiberation over stand s: its panel freed with no battle, its item
 * taken (a trap's harm with it) */
static void bomb_option(int s, int extra) {
	if (!lib_flown(&P.b, px[s], py[s])) return;
	int why, v = panel_worth(s, -1, &why);
	add(v - command_price() + extra, CMD_LONG, why, s, s, -1);
}

/* A fight with the unit on panel t from stand s */
static void fight_option(int s, int t, int u, int dir, int extra) {
	int d = dark_at(t), weak = P.weak[u] ? 15 : 0;
	if (u == 0) {
		if (lib_holes_left(&P.b)) return;
		int wc = 80 - 6 * d + weak;
		add((d <= 3 ? 3000 * wc / 100 : 300) - risk(lib_harm(UNIT_DARKLOID, P.b.act) * 5 / 2) + extra, CMD_FIGHT, MAP_BOSS, s, t, dir);
		return;
	}
	/* (his panel freed with him: the way on it, and his reach gone; a
	 * DarkHole's guardian back while it stands) */
	P.b.unit[u].gone = true;
	int without = lib_threat(&P.b, px[s], py[s]);
	P.b.unit[u].gone = false;
	int removed = (threat_at(s) - without) * 600 / (P.hp > 0 ? P.hp : 1), wc = 70 - 2 * d + weak;
	int v = (panel_worth(t, -1, NULL) + removed) * wc / 100;
	int h = P.b.unit[u].hole;
	if (h >= 0 && P.b.kind[P.b.hole_y[h]][P.b.hole_x[h]] == LIB_HOLE) v = v * 8 / 10;
	add(v - risk(virus_harm() * 3) + extra, CMD_FIGHT, WHY_UNIT + u, s, t, dir);
}

static void stand_options(int s) {
	bool any = false;
	for (int dir = 0; dir < 4 && !any; ++dir) any = nb[s][dir] >= 0 && (unit_on(nb[s][dir]) >= 0 || freeable(nb[s][dir]) || K(nb[s][dir]) == LIB_HOLE);
	int extra = -threat_cost(s) + (K(s) == LIB_BONUS && !P.strip ? 200 : 0);
	if (P.op && P.command == LIB_DO_MELODY) melody_option(s, extra);
	if (P.op && P.command == LIB_DO_MAGNET) magnet_option(s, extra);
	if (P.op && P.command == LIB_DO_BOMB) bomb_option(s, extra);
	for (int dir = 0; dir < 4; ++dir) {
		int t = nb[s][dir];
		if (t < 0) continue;
		if (P.op && P.command == LIB_DO_NUMBER) number_option(s, t, dir, extra);
		if (P.op && P.command == LIB_DO_SNEAK) sneak_option(s, t, dir, extra);
		if (!any) continue;
		int u = unit_on(t);
		if (u >= 0) fight_option(s, t, u, dir, extra);
		else if (freeable(t) || K(t) == LIB_HOLE) panel_options(s, t, dir, extra);
	}
}

static void options(void) {
	nopt = 0;
	for (int k = 0; k < nstands; ++k) stand_options(stands[k]);
	/* (a rest's worth grows as his HP falls: from about half his HP to a
	 * third it weighs against pressing on, the dialectic; below it wins.
	 * BN5's Pass gives a fifth of his HP back: romlab, mission 2) */
	int rest = P.hp < P.max ? 135 * P.max / (P.hp > 0 ? P.hp : 1) * 2 / 5 : 30;
	if (P.hp * 3 < P.max) rest = rest * 5 / 2;
	add(rest - threat_cost(P.me), CMD_PASS, 0, P.me, -1, -1);
}

/* The decision's sensible moves (of different purpose, four fifths of the
 * best's worth or more), counted; the move taken (each worth a little
 * more or less, as players differ) */
static const Option *decide(void) {
	int best = 0, pick = 0, pv = -(1 << 30);
	for (int i = 0; i < nopt; ++i) {
		if (opt[i].value > opt[best].value) best = i;
		int v = opt[i].value * (85 + rr(31)) / 100;
		if (v > pv) { pv = v; pick = i; }
	}
	uint8_t seen[256 / 8] = { 0 };
	int sensible = 0;
	for (int i = 0; i < nopt; ++i) {
		int c = opt[i].cls & 255;
		if (opt[best].value <= 0 || opt[i].value * 100 < opt[best].value * SENSIBLE || (seen[c >> 3] >> (c & 7) & 1)) continue;
		seen[c >> 3] |= (uint8_t)(1 << (c & 7));
		++sensible;
	}
	P.moves += sensible;
	/* (a strike at a DarkHole or the Darkloid beside him is the board's
	 * beat, not a dead decision on the way) */
	const Option *top = &opt[best];
	bool strike = (top->cmd == CMD_FIGHT && top->cls / 2 % 32 == MAP_BOSS) || (top->cmd == CMD_LIB && top->t >= 0 && K(top->t) == LIB_HOLE);
	P.dead += sensible <= 1 && !strike;
	P.strikes += strike;
	++P.phases;
	++P.best[opt[best].cmd];
	return &opt[pick];
}

/* ---- the moves played ---- */

/* The board's harm on Navi k: none shielded (a BonusPanel's, KnightMan's
 * StoneBody), never his last HP */
static void board_hurt(int k, int hp) {
	int *h = k == P.k ? &P.hp : &P.mhp[k];
	if (P.shield || hp <= 0 || lib_mate_shielded(&P.b, k) || P.b.team.navi[k] == MATE_KNIGHTMAN) return;
	P.hurt += *h - 1 < hp ? *h - 1 : hp;
	*h = *h - hp < 1 ? 1 : *h - hp;
}

static void battle_hurt(int hp) {
	P.hp -= hp;
	if (P.hp > 0) return;
	if (!P.k) P.lost = true;
	else { P.b.mate[P.k].out = true; ++P.out; P.hp = 0; }
}

static bool down(void) { return P.lost || P.b.mate[P.k].out; }

/* Panel i liberated: what it held given */
static void free_panel(int i) {
	int k = K(i), it = item(i);
	if (k == LIB_TRAP) board_hurt(P.k, lib_trap_harm(P.b.act));
	if (k == LIB_ITEM && !P.strip && (it & 15) == LIB_GOT_OP) P.op = P.op + (it >> 4) > LIB_OP_MAX ? LIB_OP_MAX : P.op + (it >> 4);
	if (k == LIB_ITEM && !P.strip && (it & 15) == LIB_GOT_HEART) P.hp = P.hp + P.max * 3 / 10 > P.max ? P.max : P.hp + P.max * 3 / 10;
	if (k == LIB_ITEM && (it & 15) == LIB_GOT_KEY) {
		lib_open_barrier(&P.b, it >> 4);
		P.sealed |= (it >> 4) == P.b.sealed;
	}
	P.b.kind[py[i]][px[i]] = LIB_CLEAN;
	P.b.item[py[i]][px[i]] = 0;
}

static void free_around(int t) {
	for (int d = 0; d < 8; ++d) {
		int n = at_xy(px[t] + dir8[d][0], py[t] + dir8[d][1]);
		if (n >= 0 && freeable(n) && unit_on(n) < 0) free_panel(n);
	}
}

static void liberate(int t, int dir, int command) {
	int d = dark_at(t), r = rr(100), harm = harm_of(d) * (30 + rr(111)) / 100;
	bool one = r < p_one(d), fail = r >= 100 - p_fail(d);
	battle_hurt(one ? harm * 3 / 10 : fail ? harm * 13 / 10 : harm);
	if (fail || down()) return;
	int16_t more[6];
	int n = command != LIB_DO_NONE && dir >= 0 ? also_freed(command, t, dir, more) : 0;
	free_panel(t);
	for (int j = 0; j < n; ++j) if (freeable(more[j])) free_panel(more[j]);
	if (one) free_around(t);
}

static void hole_fight(int t) {
	battle_hurt(virus_harm() * 3 / 2 * (50 + rr(101)) / 100);
	if (down() || rr(100) >= 75) return;
	P.b.kind[py[t]][px[t]] = LIB_CLEAN;
	free_around(t);
	for (int h = 0; h < P.b.nholes; ++h) {
		if (at[P.b.hole_y[h]][P.b.hole_x[h]] != t) continue;
		P.order = P.order * 8 + h + 1;
		for (int u = 1; u < P.b.nunits; ++u) if (P.b.unit[u].hole == h) P.b.unit[u].gone = true;
	}
}

static void unit_fight(int t, int u) {
	int d = dark_at(t), weak = P.weak[u] ? 15 : 0;
	if (u == 0) {
		battle_hurt(lib_harm(UNIT_DARKLOID, P.b.act) * (100 + rr(151)) / (weak ? 140 : 100));
		if (!down() && rr(100) < 80 - 6 * d + weak) P.b.unit[0].gone = true;
		return;
	}
	battle_hurt(virus_harm() * 2 * (50 + rr(101)) / (weak ? 160 : 100));
	if (down() || rr(100) >= 70 - 2 * d + weak) return;
	P.b.unit[u].gone = true;
	P.weak[u] = false;
	free_panel(t);
}

/* A BonusPanel: BN5's frees it without a battle or the Navi's action
 * (liberation_bonus.c), as walking onto it here; its roulette's five in
 * BN5's order (no harm this phase, HP full, five Order Points, a chip,
 * the nearest guardian deleted) stopped by the player's A on the slot
 * shown: the planner stops it where it helps most, missing one in five */
static void bonus(int s) {
	P.b.kind[py[s]][px[s]] = LIB_CLEAN;
	if (P.strip) return;
	int near = -1, nd = 1 << 30;
	for (int u = 1; u < P.b.nunits; ++u) {
		int dx = P.b.unit[u].x - px[s], dy = P.b.unit[u].y - py[s];
		if (!P.b.unit[u].gone && dx * dx + dy * dy < nd) { nd = dx * dx + dy * dy; near = u; }
	}
	int r = rr(5) == 0 ? rr(5) : P.hp * 2 < P.max ? 1 : near > 0 && nd <= 9 ? 4 : P.op + 5 <= LIB_OP_MAX ? 2 : 0;
	if (r == 0) P.shield = true;
	else if (r == 1) P.hp = P.max;
	else if (r == 2) P.op = P.op + 5 > LIB_OP_MAX ? LIB_OP_MAX : P.op + 5;
	else if (r == 4 && near > 0) P.b.unit[near].gone = true;
}

/* A command played: NumberCheck's items taken (each panel left dark),
 * SneakAttack's unit weakened, LifeMelody's teammates sung for, else the
 * command's battle */
static void command(const Option *o) {
	--P.op;
	++P.longswrd;
	if (P.command == LIB_DO_MELODY) {
		for (int j = 0; j < P.b.team.n; ++j)
			if (j != P.k && !P.b.mate[j].out && !P.acted[j] && abs(P.b.mate[j].x - px[o->stand]) + abs(P.b.mate[j].y - py[o->stand]) <= 1) P.melody[j] = true;
		return;
	}
	if (P.command == LIB_DO_SNEAK) {
		if (unit_on(o->t) >= 0) P.weak[unit_on(o->t)] = true;
		return;
	}
	if (P.command == LIB_DO_MAGNET) { P.shield = true; return; }
	if (P.command == LIB_DO_BOMB) { free_panel(o->stand); return; }
	if (P.command == LIB_DO_NUMBER) {
		int xs[6], ys[6], n = lib_command_panels(&P.b, LIB_DO_NUMBER, px[o->t], py[o->t], o->dir, xs, ys);
		for (int j = 0; j < n; ++j) {
			int i = at_xy(xs[j], ys[j]), k = i >= 0 ? K(i) : LIB_VOID;
			if (k != LIB_ITEM && k != LIB_TRAP) continue;
			if (k == LIB_ITEM) free_panel(i);
			P.b.kind[ys[j]][xs[j]] = LIB_DARK;
		}
		return;
	}
	liberate(o->t, o->dir, P.command);
}

static void take(const Option *o) {
	P.me = o->stand;
	if (K(o->stand) == LIB_BONUS) bonus(o->stand);
	switch (o->cmd) {
	case CMD_LIB:
		if (K(o->t) == LIB_HOLE) hole_fight(o->t);
		else liberate(o->t, o->dir, P.melody[P.k] ? LIB_DO_MELODY : LIB_DO_NONE);
		break;
	case CMD_LONG: command(o); break;
	case CMD_PASS: P.hp = P.hp + P.max / LIB_REST_PART > P.max ? P.max : P.hp + P.max / LIB_REST_PART; break;
	default: unit_fight(o->t, unit_on(o->t)); break;
	}
}

/* The Navi nearest (x, y) still on the board, -1 none */
static int nearest_mate(int x, int y) {
	int best = -1, bd = 1 << 30;
	for (int k = 0; k < P.b.team.n; ++k) {
		int d = abs(P.b.mate[k].x - x) + abs(P.b.mate[k].y - y);
		if (!P.b.mate[k].out && d < bd) { bd = d; best = k; }
	}
	return best;
}

/* A unit's blow: on every Navi in its reach where it hits them all
 * (BigBrute), else on the nearest */
static void strike(int ux, int uy, int reach_, bool all, int harm) {
	int near = nearest_mate(ux, uy);
	for (int k = 0; k < P.b.team.n; ++k) {
		if (P.b.mate[k].out || abs(P.b.mate[k].x - ux) + abs(P.b.mate[k].y - uy) > reach_ || (!all && k != near)) continue;
		board_hurt(k, harm);
	}
}

/* The Darkloids' phase: each guardian's steps toward the nearest Navi and
 * its blow, the Darkloid's, then a hole's deleted guardian back beside it
 * as BN5 sends him (lib_guardian_return, by the Navi nearest the hole) */
static void enemy(void) {
	for (int u = 1; u < P.b.nunits; ++u) {
		LibUnit *n = &P.b.unit[u];
		int k = nearest_mate(n->x, n->y);
		if (n->gone || k < 0) continue;
		const LibGuardRule *r = lib_guard_rule(n->kind);
		int mx = P.b.mate[k].x, my = P.b.mate[k].y;
		for (int s = 0; s < r->move && abs(n->x - mx) + abs(n->y - my) > r->reach; ++s)
			if (!lib_guardian_step(&P.b, u, mx, my)) break;
		strike(n->x, n->y, r->reach, r->all, lib_harm(n->kind, P.b.act));
	}
	if (!P.b.unit[0].gone) strike(P.b.boss_x, P.b.boss_y, lib_darkloid_reach(P.b.act), false, lib_harm(UNIT_DARKLOID, P.b.act));
	P.shield = false;
	for (int h = 0; h < P.b.nholes; ++h) {
		int k = nearest_mate(P.b.hole_x[h], P.b.hole_y[h]);
		if (k < 0) continue;
		int x, y, u = lib_guardian_return(&P.b, h, P.b.mate[k].x, P.b.mate[k].y, &x, &y);
		if (u < 0) continue;
		P.b.unit[u].x = (uint8_t)x; P.b.unit[u].y = (uint8_t)y; P.b.unit[u].gone = false;
		occ[at_xy(x, y)] = (int8_t)u;
	}
}

/* The team's order in a phase: support first (a melody before the Navi it
 * is for, a strike before the fight), then the rest as the team stands */
static int turn_order(int order[LIB_TEAM]) {
	static const uint8_t first[] = { MATE_TOADMAN, MATE_SHADOWMAN, MATE_NUMBERMAN };
	int n = 0;
	for (unsigned f = 0; f < sizeof first; ++f)
		for (int k = 1; k < P.b.team.n; ++k) if (P.b.team.navi[k] == first[f]) order[n++] = k;
	for (int k = 0; k < P.b.team.n; ++k) {
		bool in = false;
		for (int j = 0; j < n; ++j) in |= order[j] == k;
		if (!in) order[n++] = k;
	}
	return n;
}

/* Navi k's turn: he moves and takes the best of what he can do */
static void turn(int k) {
	P.k = k;
	P.command = lib_command_of(P.b.team.navi[k]);
	P.me = at[P.b.mate[k].y][P.b.mate[k].x];
	P.hp = P.mhp[k];
	P.max = P.mmax[k];
	reach();
	maps();
	options();
	take(decide());
	P.acted[k] = true;
	P.mhp[k] = P.hp;
	if (!P.b.mate[k].out) { P.b.mate[k].x = px[P.me]; P.b.mate[k].y = py[P.me]; }
}

/* One play: the phases it took to delete the Darkloid, 0 not done */
static int play(const LibBoard *b, uint32_t seed, bool strip, int taste) {
	memset(&P, 0, sizeof P);
	P.b = *b;
	P.rs = seed ? seed : 1;
	P.strip = strip;
	P.taste = taste;
	for (int k = 0; k < b->team.n; ++k) {
		P.mmax[k] = k == 0 || !b->team.hp[k] ? mm_hp[b->act < 0 ? 0 : b->act > 5 ? 5 : b->act] : b->team.hp[k];
		P.mhp[k] = P.mmax[k];
	}
	P.op = LIB_OP_START;
	int order[LIB_TEAM], n = turn_order(order);
	for (int phase = 1; phase <= PHASE_CAP; ++phase) {
		memset(P.acted, 0, sizeof P.acted);
		memset(P.melody, 0, sizeof P.melody);
		for (int i = 0; i < n; ++i) {
			if (P.b.mate[order[i]].out) continue;
			turn(order[i]);
			if (P.lost) return 0;
			if (P.b.unit[0].gone) return phase;
		}
		units_placed();
		enemy();
	}
	return 0;
}

static int cmp_int(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

/* The plays' measures, and the par from those won */
static void plays(LibBoard *b, uint32_t seed) {
	LibMeasure *m = &b->m;
	int won[PLAYS], orders[PLAYS], norders = 0;
	memset(m, 0, sizeof *m);
	for (int k = 0; k < PLAYS; ++k) {
		/* (no use playing on: too few can be won, or too many decisions dead) */
		if (!lib_plan_lax && ((m->wins + PLAYS - k) * 100 < PLAYS * WIN_MIN || (k >= 4 && m->dead * 100 > (m->phases - m->strikes) * (DEAD_TEAM + 10)))) break;
		int phases = play(b, seed + (uint32_t)k * 0x9E3779B9u, false, 0);
		++m->plays;
		if (phases) won[m->wins++] = phases;
		m->phases += P.phases; m->dead += P.dead; m->strikes += P.strikes; m->moves += P.moves; m->longswrd += P.longswrd; m->hurt += P.hurt;
		m->out += P.out;
		for (int c = 0; c < 4; ++c) m->best[c] += P.best[c];
		bool known = false;
		for (int o = 0; o < norders; ++o) known |= orders[o] == P.order;
		if (!known) orders[norders++] = P.order;
	}
	m->orders = norders;
	for (int k = 0; k < BARE_PLAYS; ++k) {
		play(b, seed ^ 0xB4BEu ^ (uint32_t)k * 0x85EBCA6Bu, true, 0);
		m->bare_phases += P.phases - P.strikes; m->bare_dead += P.dead;
	}
	/* (the route's player: the phases it costs him over the par's) */
	int route[ROUTE_PLAYS], nroute = 0;
	for (int k = 0; b->sealed && k < ROUTE_PLAYS; ++k) {
		int phases = play(b, seed ^ 0x5EA1u ^ (uint32_t)k * 0xC2B2AE35u, false, 100);
		if (phases && P.sealed) route[nroute++] = phases;
	}
	m->sealed = nroute;
	if (nroute) {
		qsort(route, (size_t)nroute, sizeof route[0], cmp_int);
		m->route = route[nroute / 2];
	}
	if (!m->wins) return;
	qsort(won, (size_t)m->wins, sizeof won[0], cmp_int);
	m->median = won[m->wins / 2];
	m->p25 = won[m->wins / 4];
	m->p75 = won[m->wins * 3 / 4];
}

int lib_plan(LibBoard *b, uint32_t seed) {
	index_board(b);
	plays(b, seed);
	const LibMeasure *m = &b->m;
	b->par = m->median;
	if (lib_plan_lax) return m->wins ? LIB_WHY_NONE : LIB_WHY_UNWON;
	if (m->wins * 100 < m->plays * WIN_MIN) return LIB_WHY_UNWON;
	/* (a team's: each Navi's turn a decision, a support Navi's often his
	 * one sensible move while the rest of his phase decides) */
	if (m->dead * 100 > (m->phases - m->strikes) * (b->team.n > 1 ? DEAD_TEAM : DEAD_MAX)) return LIB_WHY_DEAD;
	if (m->bare_dead * 100 > m->bare_phases * BARE_MAX) return LIB_WHY_BARE;
	/* (a team's route a phase dearer at least: its phases are fewer, one
	 * Navi's detour while the rest press on) */
	int least = b->team.n > 1 ? 1 : ROUTE_LEAST;
	if (b->sealed && (m->sealed * 2 < ROUTE_PLAYS || m->route - m->median < least || m->route - m->median > ROUTE_MOST)) return LIB_WHY_ROUTE;
	if (m->longswrd * 100 < m->plays * LONGSWRD_LEAST || m->best[CMD_LONG] * 100 > m->phases * LONGSWRD_MOST) return LIB_WHY_LONGSWRD;
	return LIB_WHY_NONE;
}

void lib_plan_par(LibBoard *b) {
	LibMeasure kept = b->m;
	index_board(b);
	plays(b, b->plan_seed);
	if (b->m.wins) b->par = b->m.median;
	b->m = kept;
}
