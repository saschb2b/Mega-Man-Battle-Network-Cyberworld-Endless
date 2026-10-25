/* A liberation board's planner (liberation_plan.h; docs/LIBERATION.md, The
 * quality bar). It plays the board as a fair player would, MegaMan alone:
 * each phase he moves over the clean panels and takes the best of what he
 * could do there, each move worth what it brings the run's dialectic at
 * the board's scale (press on toward a DarkHole, the Darkloid or a key;
 * prepare: Order Points, a heart, a rest, a guardian out of the way; a
 * detour for loot or the sealed region's route), less what the board
 * shows it costs (a guardian's reach where he would stand, a trap's harm,
 * an Order Point). Battles are won in one turn, in three or lost by the
 * dark around (BN5's text; tools/liberation.py's SKILL), the Darkloids'
 * phase as liberation.c plays it. A phase's sensible moves are those of
 * different purpose worth four fifths of the best or more: a phase with
 * one is dead. The par is the median of the plays won. */
#include "liberation_plan.h"

#include <stdlib.h>
#include <string.h>

#define PLAYS        12      /* plays of the board */
bool lib_plan_lax;
#define BARE_PLAYS   6       /* ... with its rewards stripped (the quality bar's 2) */
#define PHASE_CAP    45
#define SENSIBLE     80      /* percent of the best move's worth a sensible move has */
#define WIN_MIN      60      /* percent of plays won, at least */
#define DEAD_MAX     55      /* percent of phases with one sensible move, at most */
#define BARE_MAX     60      /* ... on the bare board */
#define LONGSWRD_LEAST 50    /* LongSwrd taken, per hundred plays, at least */
#define LONGSWRD_MOST  40    /* ... the best move in at most this percent of phases */
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
enum { WHY_WIDEN = MAPS, WHY_ITEM, WHY_UNIT };
enum { CMD_LIB, CMD_LONG, CMD_PASS, CMD_FIGHT };

static struct {
	LibBoard b;
	int me, hp, max, op;
	bool shield, strip, lost;
	int taste;             /* percent: his care for the sealed region's route (0 the par's player) */
	int back_in[LIB_UNITS];
	uint32_t rs;
	/* its measures */
	int dead, moves, phases, strikes, best[4], longswrd, hurt, order, norder;
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

/* Where he may stand this phase: the clean panels joined to his, past no unit */
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
			if (n < 0 || seen[n] || (K(n) != LIB_CLEAN && K(n) != LIB_BONUS) || unit_on(n) >= 0) continue;
			seen[n] = 1;
			stands[nstands++] = (int16_t)n;
		}
	for (int s = 0; s < nstands; ++s) threat[stands[s]] = -1;
}

/* The board's units' harm at stand s next phase (lib_threat), once a phase */
static int threat_at(int s) {
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
 * lost so ends the run), never worth it; short of that, the rest's worth
 * weighs his HP */
static int virus_harm(void) { return virus[P.b.act < 0 ? 0 : P.b.act > 5 ? 5 : P.b.act]; }
static int harm_of(int d) { return virus_harm() * (100 + 25 * d) / 100; }
static int risk(int worst) { return worst >= P.hp ? 3000 : 0; }

static int p_one(int d) { int p = 75 - 5 * d; return p < 8 ? 8 : p; }
static int p_fail(int d) { int p = 4 + 35 * d / 10; return p > 45 ? 45 : p; }
static int dark_at(int i) { return lib_dark_around(&P.b, px[i], py[i]); }
static int threat_cost(int s) { return threat_at(s) * 600 / (P.hp > 0 ? P.hp : 1); }

/* A move: its worth, and its class (its command, purpose and, for a
 * panel's, whether it is a safe one or a bold one, four or more dark
 * round it: the one-turn win's gamble) */
static void add(int value, int cmd, int why, int s, int t, int dir) {
	if (nopt >= OPTIONS) return;
	bool bold = t >= 0 && cmd != CMD_FIGHT && dark_at(t) >= 4;
	opt[nopt++] = (Option){ value, (cmd * 32 + why) * 2 + bold, cmd, s, t, dir };
}

/* Liberation and LongSwrd on panel t from stand s, facing dir */
static void panel_options(int s, int t, int dir, int extra) {
	int d = dark_at(t), why;
	if (K(t) == LIB_HOLE) {
		for (int h = 0; h < P.b.nholes; ++h)
			if (at[P.b.hole_y[h]][P.b.hole_x[h]] == t) add(1050 - risk(virus_harm() * 9 / 4) + extra, CMD_LIB, h, s, t, dir);
		return;
	}
	int v = panel_worth(t, -1, &why), bonus = p_one(d) * 80 / 100 * around_worth(t) / 100, keep = 100 - 35 * d / 10;
	extra -= risk(harm_of(d) * 182 / 100);
	add((v + bonus) * keep / 100 + extra, CMD_LIB, why, s, t, dir);
	int t2 = nb[t][dir];
	if (!P.op || t2 < 0 || !freeable(t2) || unit_on(t2) >= 0) return;
	int v2 = panel_worth(t2, t, NULL), price = 300 - 40 * P.op;
	add((v + v2 + bonus) * keep / 100 - price + extra, CMD_LONG, why, s, t, dir);
}

/* A fight with the unit on panel t from stand s */
static void fight_option(int s, int t, int u, int dir, int extra) {
	int d = dark_at(t);
	if (u == 0) {
		if (lib_holes_left(&P.b)) return;
		int wc = 80 - 6 * d;
		add((d <= 3 ? 3000 * wc / 100 : 300) - risk(lib_harm(UNIT_DARKLOID, P.b.act) * 5 / 2) + extra, CMD_FIGHT, MAP_BOSS, s, t, dir);
		return;
	}
	/* (his panel freed with him: the way on it, and his reach gone; a
	 * DarkHole's guardian back while it stands) */
	P.b.unit[u].gone = true;
	int without = lib_threat(&P.b, px[s], py[s]);
	P.b.unit[u].gone = false;
	int removed = (threat_at(s) - without) * 600 / (P.hp > 0 ? P.hp : 1), wc = 70 - 2 * d;
	int v = (panel_worth(t, -1, NULL) + removed) * wc / 100;
	int h = P.b.unit[u].hole;
	if (h >= 0 && P.b.kind[P.b.hole_y[h]][P.b.hole_x[h]] == LIB_HOLE) v = v * 8 / 10;
	add(v - risk(virus_harm() * 3) + extra, CMD_FIGHT, WHY_UNIT + u, s, t, dir);
}

static void options(void) {
	nopt = 0;
	for (int k = 0; k < nstands; ++k) {
		int s = stands[k], extra = 0;
		bool any = false;
		for (int dir = 0; dir < 4 && !any; ++dir) any = nb[s][dir] >= 0 && (unit_on(nb[s][dir]) >= 0 || freeable(nb[s][dir]) || K(nb[s][dir]) == LIB_HOLE);
		if (!any) continue;
		extra = -threat_cost(s) + (K(s) == LIB_BONUS && !P.strip ? 200 : 0);
		for (int dir = 0; dir < 4; ++dir) {
			int t = nb[s][dir];
			if (t < 0) continue;
			int u = unit_on(t);
			if (u >= 0) fight_option(s, t, u, dir, extra);
			else if (freeable(t) || K(t) == LIB_HOLE) panel_options(s, t, dir, extra);
		}
	}
	/* (a rest's worth grows as his HP falls: from about half his HP to a
	 * third it weighs against pressing on, the dialectic; below it wins) */
	int rest = P.hp < P.max ? 135 * P.max / (P.hp > 0 ? P.hp : 1) : 30;
	add(rest - threat_cost(P.me), CMD_PASS, 0, P.me, -1, -1);
}

/* The phase's sensible moves (of different purpose, four fifths of the
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
	 * beat, not a dead phase on the way) */
	const Option *top = &opt[best];
	bool strike = (top->cmd == CMD_FIGHT && top->cls / 2 % 32 == MAP_BOSS) || (top->cmd == CMD_LIB && top->t >= 0 && K(top->t) == LIB_HOLE);
	P.dead += sensible <= 1 && !strike;
	P.strikes += strike;
	++P.phases;
	++P.best[opt[best].cmd];
	return &opt[pick];
}

/* ---- the moves played ---- */

static void board_hurt(int hp) {
	if (P.shield || hp <= 0) return;
	P.hurt += P.hp - 1 < hp ? P.hp - 1 : hp;
	P.hp = P.hp - hp < 1 ? 1 : P.hp - hp;
}

static void battle_hurt(int hp) {
	P.hp -= hp;
	if (P.hp <= 0) P.lost = true;
}

/* Panel i liberated: what it held given */
static void free_panel(int i) {
	int k = K(i), it = item(i);
	if (k == LIB_TRAP) board_hurt(lib_trap_harm(P.b.act));
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

static void liberate(int t, int dir) {
	int d = dark_at(t), r = rr(100), harm = harm_of(d) * (30 + rr(111)) / 100;
	bool one = r < p_one(d), fail = r >= 100 - p_fail(d);
	battle_hurt(one ? harm * 3 / 10 : fail ? harm * 13 / 10 : harm);
	if (fail || P.lost) return;
	free_panel(t);
	int t2 = dir >= 0 ? nb[t][dir] : -1;
	if (t2 >= 0 && freeable(t2) && unit_on(t2) < 0) free_panel(t2);
	if (one) free_around(t);
}

static void hole_fight(int t) {
	battle_hurt(virus_harm() * 3 / 2 * (50 + rr(101)) / 100);
	if (P.lost || rr(100) >= 75) return;
	P.b.kind[py[t]][px[t]] = LIB_CLEAN;
	free_around(t);
	for (int h = 0; h < P.b.nholes; ++h) {
		if (at[P.b.hole_y[h]][P.b.hole_x[h]] != t) continue;
		P.order = P.order * 8 + h + 1;
		for (int u = 1; u < P.b.nunits; ++u) if (P.b.unit[u].hole == h) P.b.unit[u].gone = true;
	}
}

static void unit_fight(int t, int u) {
	int d = dark_at(t);
	if (u == 0) {
		battle_hurt(lib_harm(UNIT_DARKLOID, P.b.act) * (100 + rr(151)) / 100);
		if (!P.lost && rr(100) < 80 - 6 * d) P.b.unit[0].gone = true;
		return;
	}
	battle_hurt(virus_harm() * 2 * (50 + rr(101)) / 100);
	if (P.lost || rr(100) >= 70 - 2 * d) return;
	P.b.unit[u].gone = true;
	P.back_in[u] = 2;
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

static void take(const Option *o) {
	P.me = o->stand;
	if (K(o->stand) == LIB_BONUS) bonus(o->stand);
	switch (o->cmd) {
	case CMD_LIB:
		if (K(o->t) == LIB_HOLE) hole_fight(o->t);
		else liberate(o->t, -1);
		break;
	case CMD_LONG: --P.op; ++P.longswrd; liberate(o->t, o->dir); break;
	case CMD_PASS: P.hp = P.hp + P.max / 2 > P.max ? P.max : P.hp + P.max / 2; break;
	default: unit_fight(o->t, unit_on(o->t)); break;
	}
}

/* The Darkloids' phase: a hole's deleted guardian back the phase after
 * next, each guardian's steps and blow, the Darkloid's */
static void enemy(void) {
	int mx = px[P.me], my = py[P.me];
	for (int u = 1; u < P.b.nunits; ++u) {
		LibUnit *n = &P.b.unit[u];
		if (!n->gone || n->hole < 0 || P.b.kind[P.b.hole_y[n->hole]][P.b.hole_x[n->hole]] != LIB_HOLE || --P.back_in[u] > 0) continue;
		for (int f = 0; f < 4; ++f) {
			int x = P.b.hole_x[n->hole] + dir4[f][0], y = P.b.hole_y[n->hole] + dir4[f][1], i = at_xy(x, y);
			if (i < 0 || !lib_dark_kind(K(i)) || K(i) == LIB_HOLE || unit_on(i) >= 0 || i == P.me) continue;
			n->x = (uint8_t)x; n->y = (uint8_t)y; n->gone = false;
			occ[i] = (int8_t)u;
			break;
		}
	}
	for (int u = 1; u < P.b.nunits; ++u) {
		LibUnit *n = &P.b.unit[u];
		if (n->gone) continue;
		const LibGuardRule *r = lib_guard_rule(n->kind);
		for (int s = 0; s < r->move && abs(n->x - mx) + abs(n->y - my) > r->reach; ++s)
			if (!lib_guardian_step(&P.b, u, mx, my)) break;
		if (abs(n->x - mx) + abs(n->y - my) <= r->reach) board_hurt(lib_harm(n->kind, P.b.act));
	}
	if (!P.b.unit[0].gone && abs(P.b.boss_x - mx) + abs(P.b.boss_y - my) <= lib_darkloid_reach(P.b.act)) board_hurt(lib_harm(UNIT_DARKLOID, P.b.act));
	P.shield = false;
}

/* One play: the phases it took to delete the Darkloid, 0 not done */
static int play(const LibBoard *b, uint32_t seed, bool strip, int taste) {
	memset(&P, 0, sizeof P);
	P.b = *b;
	P.rs = seed ? seed : 1;
	P.strip = strip;
	P.taste = taste;
	P.me = at[b->start_y][b->start_x];
	P.hp = P.max = mm_hp[b->act < 0 ? 0 : b->act > 5 ? 5 : b->act];
	P.op = LIB_OP_START;
	for (int phase = 1; phase <= PHASE_CAP; ++phase) {
		reach();
		maps();
		options();
		take(decide());
		if (P.lost) return 0;
		if (P.b.unit[0].gone) return phase;
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
		/* (no use playing on: too few can be won, or too many phases dead) */
		if (!lib_plan_lax && ((m->wins + PLAYS - k) * 100 < PLAYS * WIN_MIN || (k >= 4 && m->dead * 100 > (m->phases - m->strikes) * (DEAD_MAX + 15)))) break;
		int phases = play(b, seed + (uint32_t)k * 0x9E3779B9u, false, 0);
		++m->plays;
		if (phases) won[m->wins++] = phases;
		m->phases += P.phases; m->dead += P.dead; m->strikes += P.strikes; m->moves += P.moves; m->longswrd += P.longswrd; m->hurt += P.hurt;
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
	if (m->dead * 100 > (m->phases - m->strikes) * DEAD_MAX) return LIB_WHY_DEAD;
	if (m->bare_dead * 100 > m->bare_phases * BARE_MAX) return LIB_WHY_BARE;
	if (b->sealed && (m->sealed * 2 < ROUTE_PLAYS || m->route - m->median < ROUTE_LEAST || m->route - m->median > ROUTE_MOST)) return LIB_WHY_ROUTE;
	if (m->longswrd * 100 < m->plays * LONGSWRD_LEAST || m->best[CMD_LONG] * 100 > m->phases * LONGSWRD_MOST) return LIB_WHY_LONGSWRD;
	return LIB_WHY_NONE;
}
