/* A liberation's board from the dark layer's floor (liberation_board.h):
 * BN5's kinds placed by BN5's rules, as tools/liberation.py's `generate`
 * places them on its own floors (docs/LIBERATION.md, The generator): the
 * arrival's start zone kept clean, the Darkloid's 3x3 on the far side,
 * DarkHoles in rings of dark panels each with its guardian, dark panels
 * thickening with the walk from the start and gathered by one majority
 * pass, BonusPanels at dead ends, items in the dark's corners first; then
 * the act's rules of the staircase (docs/LIBERATION.md, The quality bar):
 * barriers and their keys with the sealed region's route behind one,
 * roaming guardians, trap panels on the way with a way round them, the
 * barriers' chain. The planner (liberation_plan.c) then plays it, sets the
 * par, and keeps it only where it is a set of real decisions. Its own
 * random numbers, the game's left alone. */
#include "liberation_board.h"

#include <stdlib.h>
#include <string.h>

#include "liberation_plan.h"

/* Per act from the second (0-based 1-5), BN5's mission it stands for
 * (docs/LIBERATION.md, What a mission is made of): its team, DarkHoles,
 * roaming guardians, the guardians' kinds, the dark share's range
 * (percent), item panels, BonusPanels, the staircase's rules, chip and
 * zenny items, traps. A smaller team than BN5's meets a DarkHole less for
 * every two Navis short, and dark five points thinner for each. */
static const struct { int team, holes, roam, kinds, dark_lo, dark_hi, items, bonus, rules, loot, traps; } spec[6] = {
	{ 2, 2, 0, 1, 40, 55, 5, 2, 0, 1, 0 },    /* (act 1 has none: as act 2's, for a test's) */
	{ 2, 2, 0, 1, 40, 55, 5, 2, 0, 1, 0 },
	{ 3, 3, 0, 1, 45, 58, 6, 3, LIB_RULE_BARRIERS, 2, 0 },
	{ 4, 4, 1, 2, 52, 66, 7, 3, LIB_RULE_BARRIERS | LIB_RULE_REACH, 2, 0 },
	{ 5, 5, 2, 3, 58, 68, 8, 3, LIB_RULE_BARRIERS | LIB_RULE_REACH | LIB_RULE_TOLLS, 3, 4 },
	{ 5, 5, 2, 3, 55, 68, 8, 3, LIB_RULE_BARRIERS | LIB_RULE_REACH | LIB_RULE_TOLLS | LIB_RULE_MAZE, 3, 4 },
};
#define MAKE_TRIES 16
/* The dark panels (beyond the DarkHoles' rings) a lone MegaMan meets by
 * act, a teammate's share more each: BN5's mission 1 has 33 for two Navis
 * on a board of 100, our layers twice its floor (the par a lone board's
 * 10 to 20 phases, BN5's 10 to 20 minutes) */
static const int dark_panels[6] = { 14, 14, 18, 18, 22, 26 };

static const int dir4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
static const int dir8[8][2] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };

/* ---- the units' rules ---- */

static const LibGuardRule rules[4] = { { 0, 0, false }, { 2, 1, true }, { 3, 3, false }, { 2, 1, false } };
static const int harm[4][5] = { { 40, 50, 80, 80, 100 }, { 20, 40, 50, 60, 80 }, { 20, 30, 40, 60, 70 }, { 50, 50, 70, 100, 120 } };
static const int darkloid_reach[5] = { 2, 2, 3, 2, 3 };

static int tier_of(int act) { int t = act - 1; return t < 0 ? 0 : t > 4 ? 4 : t; }

const LibGuardRule *lib_guard_rule(int kind) {
	static LibGuardRule darkloid;
	if (kind == UNIT_DARKLOID) return &darkloid;
	return &rules[kind > 0 && kind < 4 ? kind : 1];
}

int lib_harm(int kind, int act) { return harm[kind >= 0 && kind < 4 ? kind : 1][tier_of(act)]; }

/* (BN5's: 50 in missions 5 and 6, 100 later) */
int lib_trap_harm(int act) { return act >= 5 ? 100 : 50; }

int lib_zenny(int act) { return (2 + (act < 1 ? 1 : act)) * 100; }

/* ---- its random numbers ---- */
static uint32_t rs;
static uint32_t rnext(void) { rs ^= rs << 13; rs ^= rs >> 17; rs ^= rs << 5; return rs; }
static int rrange(int n) { return n <= 1 ? 0 : (int)(rnext() % (uint32_t)n); }
static int rpercent(void) { return rrange(100); }

static bool on_board(const LibBoard *b, int x, int y) {
	return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && b->kind[y][x] != LIB_VOID;
}

bool lib_dark_kind(int k) { return k == LIB_DARK || k == LIB_ITEM || k == LIB_HOLE || k == LIB_RING || k == LIB_TRAP; }

bool lib_walkable(const LibBoard *b, int x, int y) {
	return on_board(b, x, y) && (b->kind[y][x] == LIB_CLEAN || b->kind[y][x] == LIB_BONUS);
}

int lib_holes_left(const LibBoard *b) {
	int n = 0;
	for (int i = 0; i < b->nholes; ++i) n += b->kind[b->hole_y[i]][b->hole_x[i]] == LIB_HOLE;
	return n;
}

bool lib_liberable(const LibBoard *b, int x, int y) {
	if (!on_board(b, x, y)) return false;
	int k = b->kind[y][x];
	return k == LIB_DARK || k == LIB_ITEM || k == LIB_TRAP || k == LIB_HOLE || (k == LIB_RING && !lib_holes_left(b));
}

int lib_dark_around(const LibBoard *b, int x, int y) {
	int n = 0;
	for (int d = 0; d < 8; ++d) {
		int nx = x + dir8[d][0], ny = y + dir8[d][1];
		if (on_board(b, nx, ny)) n += lib_dark_kind(b->kind[ny][nx]);
	}
	return n;
}

int lib_open_barrier(LibBoard *b, int n) {
	int opened = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (b->kind[y][x] == LIB_BAR && b->item[y][x] == n) { b->kind[y][x] = LIB_CLEAN; b->item[y][x] = 0; ++opened; }
	return opened;
}

static int manhattan(int ax, int ay, int bx, int by) { return abs(ax - bx) + abs(ay - by); }

/* Walks from (sx, sy) over the board's floor, barriers shut where `bars`
 * (and the panels in `wall`, a candidate's): into dist (-1 unreached); the
 * farthest */
static int walk_from(const LibBoard *b, int sx, int sy, int16_t dist[MAP_H][MAP_W], bool bars, const uint8_t *wall) {
	static uint16_t q[MAP_W * MAP_H];
	memset(dist, 0xFF, sizeof(int16_t) * MAP_W * MAP_H);
	int head = 0, tail = 0, far = 0;
	dist[sy][sx] = 0;
	q[tail++] = (uint16_t)(sy * MAP_W + sx);
	while (head < tail) {
		int c = q[head++], x = c % MAP_W, y = c / MAP_W;
		if (dist[y][x] > far) far = dist[y][x];
		for (int d = 0; d < 4; ++d) {
			int nx = x + dir4[d][0], ny = y + dir4[d][1];
			if (!on_board(b, nx, ny) || dist[ny][nx] >= 0) continue;
			if ((bars && b->kind[ny][nx] == LIB_BAR) || (wall && wall[ny * MAP_W + nx])) continue;
			dist[ny][nx] = (int16_t)(dist[y][x] + 1);
			q[tail++] = (uint16_t)(ny * MAP_W + nx);
		}
	}
	return far;
}

static int walk(const LibBoard *b, int sx, int sy, int16_t dist[MAP_H][MAP_W]) { return walk_from(b, sx, sy, dist, false, NULL); }

void lib_cost_map(const LibBoard *b, int16_t cost[MAP_H][MAP_W], bool keys) {
	static uint16_t dq[MAP_W * MAP_H * 2];
	memset(cost, 0x7F, sizeof(int16_t) * MAP_W * MAP_H);
	int head = MAP_W * MAP_H, tail = head;
	bool holes = lib_holes_left(b) > 0;
	for (int h = 0; h < b->nholes; ++h) {
		int x = b->hole_x[h], y = b->hole_y[h];
		if (holes && b->kind[y][x] == LIB_HOLE) { cost[y][x] = 0; dq[tail++] = (uint16_t)(y * MAP_W + x); }
	}
	if (!holes) { cost[b->boss_y][b->boss_x] = 0; dq[tail++] = (uint16_t)(b->boss_y * MAP_W + b->boss_x); }
	if (keys)
		for (int y = 0; y < MAP_H; ++y)
			for (int x = 0; x < MAP_W; ++x)
				if (b->kind[y][x] == LIB_ITEM && (b->item[y][x] & 15) == LIB_GOT_KEY && b->item[y][x] >> 4 != b->sealed) {
					cost[y][x] = 0;
					dq[tail++] = (uint16_t)(y * MAP_W + x);
				}
	while (head < tail) {
		int c = dq[head++], x = c % MAP_W, y = c / MAP_W;
		int w = lib_dark_kind(b->kind[y][x]) ? 1 : 0;
		for (int d = 0; d < 4; ++d) {
			int nx = x + dir4[d][0], ny = y + dir4[d][1];
			if (!on_board(b, nx, ny) || b->kind[ny][nx] == LIB_BAR) continue;
			int nc = cost[y][x] + w;
			if (nc >= cost[ny][nx]) continue;
			cost[ny][nx] = (int16_t)nc;
			if (w) dq[tail++] = (uint16_t)(ny * MAP_W + nx);
			else dq[--head] = (uint16_t)(ny * MAP_W + nx);
		}
	}
}

static int unit_at(const LibBoard *b, int x, int y) {
	for (int u = 0; u < b->nunits; ++u)
		if (!b->unit[u].gone && b->unit[u].x == x && b->unit[u].y == y) return u;
	return -1;
}

/* Unit u's next step from (ux, uy) toward (x, y), as the Darkloids' phase
 * takes it: over the dark (Bladia onto clean panels too), past no other
 * unit, nearer, never onto (x, y); -1 none */
static int step_toward(const LibBoard *b, int u, int ux, int uy, int x, int y) {
	int best = -1, bd = manhattan(ux, uy, x, y);
	for (int f = 0; f < 4; ++f) {
		int sx = ux + dir4[f][0], sy = uy + dir4[f][1];
		if (!on_board(b, sx, sy) || (sx == x && sy == y)) continue;
		int k = b->kind[sy][sx], o = unit_at(b, sx, sy);
		bool ok = k == LIB_DARK || k == LIB_ITEM || k == LIB_TRAP || (b->unit[u].kind == UNIT_BLADIA && k == LIB_CLEAN);
		int d = manhattan(sx, sy, x, y);
		if (ok && (o < 0 || o == u) && d < bd) { bd = d; best = f; }
	}
	return best;
}

bool lib_guardian_step(LibBoard *b, int u, int x, int y) {
	LibUnit *n = &b->unit[u];
	int f = step_toward(b, u, n->x, n->y, x, y);
	if (f < 0) return false;
	n->x = (uint8_t)(n->x + dir4[f][0]);
	n->y = (uint8_t)(n->y + dir4[f][1]);
	if (n->kind == UNIT_BLADIA && b->kind[n->y][n->x] == LIB_CLEAN) b->kind[n->y][n->x] = LIB_DARK;
	return true;
}

int lib_threat(const LibBoard *b, int x, int y) {
	int hp = 0;
	for (int u = 1; u < b->nunits; ++u) {
		const LibUnit *n = &b->unit[u];
		if (n->gone) continue;
		const LibGuardRule *r = lib_guard_rule(n->kind);
		int ux = n->x, uy = n->y;
		for (int s = 0; s < r->move && manhattan(ux, uy, x, y) > r->reach; ++s) {
			int f = step_toward(b, u, ux, uy, x, y);
			if (f < 0) break;
			ux += dir4[f][0]; uy += dir4[f][1];
		}
		if (manhattan(ux, uy, x, y) <= r->reach) hp += lib_harm(n->kind, b->act);
	}
	if (!b->unit[0].gone && manhattan(b->boss_x, b->boss_y, x, y) <= lib_darkloid_reach(b->act)) hp += lib_harm(UNIT_DARKLOID, b->act);
	return hp;
}

int lib_darkloid_reach(int act) { return darkloid_reach[tier_of(act)]; }

static int floor_around(const LibBoard *b, int x, int y) {
	int n = 0;
	for (int d = 0; d < 8; ++d) n += on_board(b, x + dir8[d][0], y + dir8[d][1]);
	return n;
}

static int neighbours(const LibBoard *b, int x, int y) {
	int n = 0;
	for (int d = 0; d < 4; ++d) n += on_board(b, x + dir4[d][0], y + dir4[d][1]);
	return n;
}

static bool unit_on(const LibBoard *b, int x, int y) {
	for (int u = 0; u < b->nunits; ++u)
		if (b->unit[u].x == x && b->unit[u].y == y) return true;
	return false;
}

/* The Darkloid's 3x3 at the far side: a panel with its eight on the floor,
 * as far a walk as the board goes (BN5 sets him across from the start) */
static bool place_darkloid(LibBoard *b, int16_t reach[MAP_H][MAP_W], int far) {
	int best = -1, bx = 0, by = 0;
	for (int y = 1; y < MAP_H - 1; ++y)
		for (int x = 1; x < MAP_W - 1; ++x) {
			if (reach[y][x] < 0 || floor_around(b, x, y) < 8 || reach[y][x] * 10 < far * 6) continue;
			int score = reach[y][x] * (85 + rrange(16));
			if (score > best) { best = score; bx = x; by = y; }
		}
	if (best < 0) return false;
	for (int d = 0; d < 8; ++d) b->kind[by + dir8[d][1]][bx + dir8[d][0]] = LIB_RING;
	b->kind[by][bx] = LIB_BOSS;
	b->boss_x = bx; b->boss_y = by;
	b->unit[b->nunits++] = (LibUnit){ UNIT_DARKLOID, (uint8_t)bx, (uint8_t)by, -1, false };
	return true;
}

/* DarkHoles: whole rings of floor, a third of the walk on and more, six
 * apart and four off the Darkloid, the farthest first; each ring dark and
 * its guardian on it, BigBrute the first (BN5's mission 1) */
static bool place_holes(LibBoard *b, int16_t reach[MAP_H][MAP_W], int far, int want, int kinds) {
	static uint16_t cand[MAP_W * MAP_H];
	static int score[MAP_W * MAP_H];
	int n = 0;
	for (int y = 1; y < MAP_H - 1; ++y)
		for (int x = 1; x < MAP_W - 1; ++x) {
			if (b->kind[y][x] != LIB_CLEAN || reach[y][x] * 10 < far * 3 || manhattan(x, y, b->boss_x, b->boss_y) < 4) continue;
			bool ring = true;
			for (int d = 0; d < 8 && ring; ++d) ring = b->kind[y + dir8[d][1]][x + dir8[d][0]] == LIB_CLEAN;
			if (!ring) continue;
			score[n] = reach[y][x] * (60 + rrange(41));
			cand[n++] = (uint16_t)(y * MAP_W + x);
		}
	b->nholes = 0;
	while (b->nholes < want) {
		int best = -1;
		for (int i = 0; i < n; ++i) {
			if (score[i] < 0) continue;
			int x = cand[i] % MAP_W, y = cand[i] / MAP_W;
			bool apart = true;
			for (int h = 0; h < b->nholes && apart; ++h) apart = manhattan(x, y, b->hole_x[h], b->hole_y[h]) >= 6;
			if (!apart) { score[i] = -1; continue; }
			if (best < 0 || score[i] > score[best]) best = i;
		}
		if (best < 0) return false;
		score[best] = -1;
		int x = cand[best] % MAP_W, y = cand[best] / MAP_W, h = b->nholes++;
		b->hole_x[h] = (uint8_t)x; b->hole_y[h] = (uint8_t)y;
		b->kind[y][x] = LIB_HOLE;
		for (int d = 0; d < 8; ++d) b->kind[y + dir8[d][1]][x + dir8[d][0]] = LIB_DARK;
		const int *s = dir4[rrange(4)];
		int kind = h == 0 ? UNIT_BRUTE : UNIT_BRUTE + rrange(kinds);
		b->unit[b->nunits++] = (LibUnit){ (uint8_t)kind, (uint8_t)(x + s[0]), (uint8_t)(y + s[1]), (int8_t)h, false };
	}
	return true;
}

/* Dark panels thickening with the walk, past the start's two; then one
 * pass of a majority rule, so they gather (the prototype's numbers) */
static void place_dark(LibBoard *b, int16_t reach[MAP_H][MAP_W], int far, int lo, int hi) {
	static uint8_t was[MAP_H][MAP_W];
	memcpy(was, b->kind, sizeof was);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (was[y][x] != LIB_CLEAN || reach[y][x] <= 2) continue;
			int t = reach[y][x] * 100 / (far ? far : 1);
			int p = lo + (hi - lo) * 2 * t / 100 - 25 + t / 4;
			/* (the dark gathers on the fields: a one-wide way's panel half
			 * as often, BN5's walkways near the start clean) */
			if (neighbours(b, x, y) <= 2) p /= 2;
			if (rpercent() < p) b->kind[y][x] = LIB_DARK;
		}
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (was[y][x] != LIB_CLEAN || reach[y][x] <= 2) continue;
			int darks = 0, floors = 0;
			for (int d = 0; d < 8; ++d) {
				int nx = x + dir8[d][0], ny = y + dir8[d][1];
				if (!on_board(b, nx, ny)) continue;
				++floors;
				darks += b->kind[ny][nx] == LIB_DARK;
			}
			if (floors && darks * 10 > floors * 6 && rpercent() < 70) b->kind[y][x] = LIB_DARK;
			else if (floors && darks * 10 < floors * 2 && rpercent() < 50) b->kind[y][x] = LIB_CLEAN;
		}
}

/* The dark thinned to the act's count: the loneliest dark panel cleaned
 * first, again and again (the dark left in fields, as BN5's gathers), the
 * DarkHoles' rings and what is near the Darkloid kept */
static void prune_dark(LibBoard *b, int want) {
	static uint16_t lone[MAP_W * MAP_H];
	for (;;) {
		int n = 0, fewest = 9, count = 0;
		for (int y = 0; y < MAP_H; ++y)
			for (int x = 0; x < MAP_W; ++x) {
				if (b->kind[y][x] != LIB_DARK) continue;
				++count;
				bool kept = manhattan(x, y, b->boss_x, b->boss_y) <= 2;
				for (int h = 0; h < b->nholes && !kept; ++h) kept = abs(b->hole_x[h] - x) <= 1 && abs(b->hole_y[h] - y) <= 1;
				if (kept) continue;
				int d = lib_dark_around(b, x, y);
				if (d < fewest) { fewest = d; n = 0; }
				if (d == fewest) lone[n++] = (uint16_t)(y * MAP_W + x);
			}
		if (count <= want || !n) return;
		int c = lone[rrange(n)];
		b->kind[c / MAP_W][c % MAP_W] = LIB_CLEAN;
	}
}

/* ---- the way on, and what lies off it ---- */

/* The panels on a cheapest way from the start to each DarkHole and on to
 * the Darkloid (within one dark panel of the cheapest): where a player
 * must go; into way */
static void way_on(const LibBoard *b, uint8_t way[MAP_H][MAP_W]) {
	static int16_t from[MAP_H][MAP_W], to[MAP_H][MAP_W];
	static uint16_t dq[MAP_W * MAP_H * 2];
	memset(way, 0, MAP_W * MAP_H);
	lib_cost_map(b, to, false);
	/* (from the start: the dark panels liberated to reach each panel, it
	 * among them; to the targets: those after it, the target among them) */
	memset(from, 0x7F, sizeof from);
	int head = MAP_W * MAP_H, tail = head;
	from[b->start_y][b->start_x] = 0;
	dq[tail++] = (uint16_t)(b->start_y * MAP_W + b->start_x);
	while (head < tail) {
		int c = dq[head++], x = c % MAP_W, y = c / MAP_W;
		for (int d = 0; d < 4; ++d) {
			int nx = x + dir4[d][0], ny = y + dir4[d][1];
			if (!on_board(b, nx, ny) || b->kind[ny][nx] == LIB_BAR) continue;
			int w = lib_dark_kind(b->kind[ny][nx]) ? 1 : 0, nc = from[y][x] + w;
			if (nc >= from[ny][nx]) continue;
			from[ny][nx] = (int16_t)nc;
			if (w) dq[tail++] = (uint16_t)(ny * MAP_W + nx);
			else dq[--head] = (uint16_t)(ny * MAP_W + nx);
		}
	}
	int best = to[b->start_y][b->start_x];
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (on_board(b, x, y) && from[y][x] < 0x7F00 && to[y][x] < 0x7F00 && from[y][x] + to[y][x] <= best + 1) way[y][x] = 1;
	/* (and the Darkloid's approach: his ring, the holes) */
	for (int h = 0; h < b->nholes; ++h) way[b->hole_y[h]][b->hole_x[h]] = 1;
	for (int d = 0; d < 8; ++d) way[b->boss_y + dir8[d][1]][b->boss_x + dir8[d][0]] = 1;
}

/* Panel (x, y)'s walk to the nearest panel of the way (0 on it) */
static int off_way(const uint8_t way[MAP_H][MAP_W], int x, int y) {
	int best = 99;
	for (int dy = -3; dy <= 3; ++dy)
		for (int dx = -3; dx <= 3; ++dx) {
			int nx = x + dx, ny = y + dy;
			if (nx >= 0 && ny >= 0 && nx < MAP_W && ny < MAP_H && way[ny][nx] && abs(dx) + abs(dy) < best) best = abs(dx) + abs(dy);
		}
	return best;
}

/* ---- barriers and their keys (the staircase's act 3 on) ---- */

static uint8_t region[MAP_H][MAP_W];   /* the panels behind each barrier: its number */

/* Whether every target (each DarkHole, the Darkloid, each key but the
 * sealed barrier's when `sealed_shut`) can be reached from the start,
 * opening each barrier whose key is reached, in turn */
static bool keys_solve(const LibBoard *b, bool sealed_shut) {
	static LibBoard t;
	static int16_t dist[MAP_H][MAP_W];
	t = *b;
	for (bool more = true; more;) {
		more = false;
		walk_from(&t, t.start_x, t.start_y, dist, true, NULL);
		for (int y = 0; y < MAP_H; ++y)
			for (int x = 0; x < MAP_W; ++x) {
				if (dist[y][x] < 0 || t.kind[y][x] != LIB_ITEM || (t.item[y][x] & 15) != LIB_GOT_KEY) continue;
				int n = t.item[y][x] >> 4;
				if (sealed_shut && n == b->sealed) continue;
				t.kind[y][x] = LIB_DARK;
				more |= lib_open_barrier(&t, n) > 0;
			}
	}
	for (int h = 0; h < b->nholes; ++h) if (dist[b->hole_y[h]][b->hole_x[h]] < 0) return false;
	if (dist[b->boss_y][b->boss_x] < 0) return false;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (dist[y][x] < 0 && t.kind[y][x] == LIB_ITEM && (t.item[y][x] & 15) == LIB_GOT_KEY && (t.item[y][x] >> 4) != b->sealed) return false;
	return true;
}

/* Whether (x, y) is a DarkHole's or on its ring */
static bool by_hole(const LibBoard *b, int x, int y) {
	for (int h = 0; h < b->nholes; ++h)
		if (abs(b->hole_x[h] - x) <= 1 && abs(b->hole_y[h] - y) <= 1) return true;
	return false;
}

/* The wall for candidate panel c: c, and across a two-wide way its side's
 * panel too */
static void wall_at(const LibBoard *b, int c, uint8_t wall[MAP_W * MAP_H]) {
	int x = c % MAP_W, y = c / MAP_W;
	memset(wall, 0, MAP_W * MAP_H);
	wall[c] = 1;
	if (neighbours(b, x, y) != 3) return;
	for (int d = 0; d < 4; ++d) {
		int nx = x + dir4[d][0], ny = y + dir4[d][1], k = on_board(b, nx, ny) ? b->kind[ny][nx] : LIB_VOID;
		if ((k == LIB_CLEAN || k == LIB_DARK) && neighbours(b, nx, ny) == 3 && !unit_on(b, nx, ny) && !by_hole(b, nx, ny)) { wall[ny * MAP_W + nx] = 1; return; }
	}
}

/* The panels a wall cuts off by itself (`side` the walk with it shut, the
 * other barriers open): how many; -1 where the Darkloid is cut off or
 * something of the board's lies in them (a unit, an item, a DarkHole,
 * another barrier or its region) */
static int cut_size(const LibBoard *b, int16_t side[MAP_H][MAP_W], const uint8_t *wall) {
	if (side[b->boss_y][b->boss_x] < 0) return -1;
	int size = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!on_board(b, x, y) || side[y][x] >= 0 || wall[y * MAP_W + x]) continue;
			int k = b->kind[y][x];
			if (region[y][x] || k == LIB_BAR || k == LIB_ITEM || k == LIB_HOLE || unit_on(b, x, y)) return -1;
			++size;
		}
	return size;
}

/* A wall of one or two panels that cuts a region off the start's side
 * by itself, for barrier n, the sealed region's: five to sixteen panels
 * with nothing of the board's in them; the region marked; false where
 * none */
static bool find_wall(LibBoard *b, int16_t reach[MAP_H][MAP_W], int n, uint8_t wall_out[MAP_W * MAP_H]) {
	static int16_t side[MAP_H][MAP_W];
	static uint8_t wall[MAP_W * MAP_H];
	static uint16_t cand[MAP_W * MAP_H];
	int nc = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			int k = b->kind[y][x];
			if ((k == LIB_CLEAN || k == LIB_DARK) && reach[y][x] > 3 && neighbours(b, x, y) <= 3 && !unit_on(b, x, y) && !region[y][x] && !by_hole(b, x, y))
				cand[nc++] = (uint16_t)(y * MAP_W + x);
		}
	for (int tries = 0; tries < 400 && nc; ++tries) {
		wall_at(b, cand[rrange(nc)], wall);
		walk_from(b, b->start_x, b->start_y, side, false, wall);
		int size = cut_size(b, side, wall);
		if (size < 5 || size > 16) continue;
		for (int y = 0; y < MAP_H; ++y)
			for (int x = 0; x < MAP_W; ++x)
				if (on_board(b, x, y) && side[y][x] < 0 && !wall[y * MAP_W + x]) region[y][x] = (uint8_t)n;
		memcpy(wall_out, wall, sizeof wall);
		return true;
	}
	return false;
}

/* Its key on a dark panel the walk reaches with barrier n shut, off the
 * way by `off` panels at least: an item */
static bool place_key(LibBoard *b, int16_t reach[MAP_H][MAP_W], const uint8_t way[MAP_H][MAP_W], int n, int off) {
	static uint16_t spots[MAP_W * MAP_H];
	int ns = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (b->kind[y][x] == LIB_DARK && !region[y][x] && reach[y][x] >= 4 && !unit_on(b, x, y) && off_way(way, x, y) >= off)
				spots[ns++] = (uint16_t)(y * MAP_W + x);
	if (!ns) return false;
	int c = spots[rrange(ns)];
	b->kind[c / MAP_W][c % MAP_W] = LIB_ITEM;
	b->item[c / MAP_W][c % MAP_W] = (uint8_t)(LIB_GOT_KEY | n << 4);
	return true;
}

/* Barrier n: its wall, its key, its number on its panels */
static void wall_up(LibBoard *b, const uint8_t *wall, int n) {
	for (int c = 0; c < MAP_W * MAP_H; ++c)
		if (wall[c]) { b->kind[c / MAP_W][c % MAP_W] = LIB_BAR; b->item[c / MAP_W][c % MAP_W] = (uint8_t)n; }
	b->nbars = n;
}

static bool place_sealed(LibBoard *b, int16_t reach[MAP_H][MAP_W], const uint8_t way[MAP_H][MAP_W]) {
	static uint8_t wall[MAP_W * MAP_H];
	int n = b->nbars + 1;
	if (!find_wall(b, reach, n, wall) || !place_key(b, reach, way, n, 2)) return false;
	wall_up(b, wall, n);
	return true;
}

/* Barrier n's key on a dark panel the walk reaches round it (`side`), off
 * the way; past the barrier before where `open_` (the walk with it open)
 * reaches it sooner: an item; false where none */
static bool across_key(LibBoard *b, int16_t side[MAP_H][MAP_W], int16_t (*open_)[MAP_W], const uint8_t way[MAP_H][MAP_W], int n) {
	static uint16_t spots[MAP_W * MAP_H];
	int ns = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (b->kind[y][x] != LIB_DARK || region[y][x] || side[y][x] < 4 || unit_on(b, x, y) || off_way(way, x, y) < 1) continue;
			if (open_ && side[y][x] <= open_[y][x]) continue;
			spots[ns++] = (uint16_t)(y * MAP_W + x);
		}
	if (!ns) return false;
	int k = spots[rrange(ns)];
	b->kind[k / MAP_W][k % MAP_W] = LIB_ITEM;
	b->item[k / MAP_W][k % MAP_W] = (uint8_t)(LIB_GOT_KEY | n << 4);
	return true;
}

/* A barrier across the way (act 4 on): a panel or two of a cheapest way
 * walled, every target still reached round it, so its key (off the way)
 * is a detour against the longer way round; the chain's (act 6) key past
 * the barrier before it, the way to it crossing that one's wall */
static bool place_across(LibBoard *b, int16_t reach[MAP_H][MAP_W], const uint8_t way[MAP_H][MAP_W], int past) {
	static uint8_t wall[MAP_W * MAP_H];
	static int16_t side[MAP_H][MAP_W], open_[MAP_H][MAP_W];
	static uint16_t cand[MAP_W * MAP_H];
	int nc = 0, n = b->nbars + 1;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			int k = b->kind[y][x];
			if (way[y][x] && (k == LIB_CLEAN || k == LIB_DARK) && reach[y][x] > 3 && !unit_on(b, x, y) && !region[y][x] && !by_hole(b, x, y)
				&& manhattan(x, y, b->boss_x, b->boss_y) > 2) cand[nc++] = (uint16_t)(y * MAP_W + x);
		}
	for (int tries = 0; tries < 60 && nc; ++tries) {
		int i = rrange(nc), c = cand[i];
		cand[i] = cand[--nc];
		memset(wall, 0, sizeof wall);
		wall[c] = 1;
		walk_from(b, b->start_x, b->start_y, side, true, wall);
		bool round = side[b->boss_y][b->boss_x] >= 0;
		for (int h = 0; h < b->nholes && round; ++h) round = side[b->hole_y[h]][b->hole_x[h]] >= 0;
		if (!round) continue;
		/* (its key: off the way; the chain's, past the barrier before) */
		if (past) walk_from(b, b->start_x, b->start_y, open_, false, wall);
		if (!across_key(b, side, past ? open_ : NULL, way, n)) continue;
		wall_up(b, wall, n);
		return true;
	}
	return false;
}

/* The sealed region's route: an item at the region's far end from its
 * barrier, the region's panels dark (a detour of phases) */
static void place_route(LibBoard *b, int n) {
	int best = -1;
	b->route_x = b->route_y = -1;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (region[y][x] != n) continue;
			if (b->kind[y][x] == LIB_CLEAN && rpercent() < 80) b->kind[y][x] = LIB_DARK;
			int far = 0;
			for (int yy = 0; yy < MAP_H; ++yy)
				for (int xx = 0; xx < MAP_W; ++xx)
					if (b->kind[yy][xx] == LIB_BAR && b->item[yy][xx] == n) far = manhattan(x, y, xx, yy);
			if (far > best) { best = far; b->route_x = x; b->route_y = y; }
		}
	if (b->route_x < 0) return;
	b->kind[b->route_y][b->route_x] = LIB_ITEM;
	b->item[b->route_y][b->route_x] = LIB_GOT_ROUTE;
}

/* The act's barriers: act 3's one, the sealed region's (its key off the
 * way, never needed); from act 4 one across the way first, a DarkHole
 * behind it; act 6's chain, each key behind the barrier before */
static bool place_barriers(LibBoard *b, int16_t reach[MAP_H][MAP_W]) {
	static uint8_t way[MAP_H][MAP_W];
	memset(region, 0, sizeof region);
	if (!(b->rules & LIB_RULE_BARRIERS)) return true;
	way_on(b, way);
	if (!place_sealed(b, reach, way)) return false;
	b->sealed = b->nbars;
	place_route(b, b->sealed);
	if (b->route_x < 0) return false;
	if ((b->rules & LIB_RULE_REACH) && !place_across(b, reach, way, 0)) return false;
	/* (the chain where the floor gives one, else a second barrier across the way) */
	if ((b->rules & LIB_RULE_MAZE) && !place_across(b, reach, way, 1) && !place_across(b, reach, way, 0)) return false;
	return keys_solve(b, true);
}

/* ---- what else lies on it ---- */

/* BonusPanels at dead ends (BN5's "?" at the ends of its stubs) */
static void place_bonus(LibBoard *b, int16_t reach[MAP_H][MAP_W], int want) {
	for (int got = 0, tries = 0; got < want && tries < 400; ++tries) {
		int x = rrange(MAP_W), y = rrange(MAP_H), k = b->kind[y][x];
		if ((k != LIB_CLEAN && k != LIB_DARK) || reach[y][x] <= 2 || neighbours(b, x, y) != 1 || region[y][x] == b->sealed) continue;
		b->kind[y][x] = LIB_BONUS;
		++got;
	}
}

/* Items in the dark, its corners and dead ends first (BN5 tucks them
 * there): one of Order Points always, then hearts and Order Points */
static void place_items(LibBoard *b, int want) {
	static uint16_t darks[MAP_W * MAP_H];
	int n = 0;
	for (int few = 1; few <= 4 && n < want * 2; ++few)
		for (int y = 0; y < MAP_H && n < want * 2; ++y)
			for (int x = 0; x < MAP_W && n < want * 2; ++x)
				if (b->kind[y][x] == LIB_DARK && neighbours(b, x, y) == few && !region[y][x] && rpercent() < 60) darks[n++] = (uint16_t)(y * MAP_W + x);
	/* (every other one, then the rest: spread over the corners found) */
	for (int i = 0, k = 0; k < want && i < n * 2; ++i) {
		int j = i < (n + 1) / 2 ? i * 2 : (i - (n + 1) / 2) * 2 + 1;
		if (j >= n) continue;
		int x = darks[j] % MAP_W, y = darks[j] / MAP_W;
		static const uint8_t pool[7] = { LIB_GOT_HEART, LIB_GOT_HEART, LIB_GOT_HEART, LIB_GOT_OP | 2 << 4, LIB_GOT_OP | 2 << 4,
			LIB_GOT_OP | 3 << 4, LIB_GOT_OP | 3 << 4 };
		b->kind[y][x] = LIB_ITEM;
		b->item[y][x] = k == 0 ? (uint8_t)(LIB_GOT_OP | (2 + rrange(2)) << 4) : pool[rrange(7)];
		++k;
	}
}

/* The act's chips and zenny, off the way (a detour's worth) */
static void place_loot(LibBoard *b, int loot) {
	static uint16_t darks[MAP_W * MAP_H];
	static uint8_t way[MAP_H][MAP_W];
	way_on(b, way);
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (b->kind[y][x] == LIB_DARK && !region[y][x] && off_way(way, x, y) >= 1 && !unit_on(b, x, y)) darks[n++] = (uint16_t)(y * MAP_W + x);
	for (int k = 0; k < loot && k < LIB_LOOT && n; ++k) {
		int i = rrange(n), x = darks[i] % MAP_W, y = darks[i] / MAP_W;
		darks[i] = darks[--n];
		b->kind[y][x] = LIB_ITEM;
		/* (chips by their loot index, layer_objs.c's scripts; zenny lib_zenny's) */
		b->item[y][x] = k % 2 ? (uint8_t)LIB_GOT_ZENNY : (uint8_t)(LIB_GOT_CHIP | b->nloot++ << 4);
	}
}

/* Traps (act 5 on): dark panels on the way that a way round, a dark panel
 * or two longer, avoids: the toll or the detour, shown */
static void place_traps(LibBoard *b, int want) {
	static uint8_t way[MAP_H][MAP_W];
	static int16_t cost[MAP_H][MAP_W], with[MAP_H][MAP_W];
	way_on(b, way);
	lib_cost_map(b, cost, false);
	static uint16_t cand[MAP_W * MAP_H];
	int nc = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (way[y][x] && b->kind[y][x] == LIB_DARK && !unit_on(b, x, y) && !region[y][x] && manhattan(x, y, b->start_x, b->start_y) >= 4 && !by_hole(b, x, y))
				cand[nc++] = (uint16_t)(y * MAP_W + x);
	while (nc && b->traps < want) {
		int i = rrange(nc), x = cand[i] % MAP_W, y = cand[i] / MAP_W;
		cand[i] = cand[--nc];
		/* (the way round it: the board with it walled) */
		b->kind[y][x] = LIB_BAR;
		lib_cost_map(b, with, false);
		bool round = with[b->start_y][b->start_x] <= cost[b->start_y][b->start_x] + 2;
		b->kind[y][x] = round ? LIB_TRAP : LIB_DARK;
		b->traps += round;
	}
}

/* Roaming guardians on dark panels past two fifths of the walk */
static void place_roamers(LibBoard *b, int16_t reach[MAP_H][MAP_W], int far, int want, int kinds) {
	for (int got = 0, tries = 0; got < want && b->nunits < LIB_UNITS && tries < 400; ++tries) {
		int x = rrange(MAP_W), y = rrange(MAP_H);
		if (b->kind[y][x] != LIB_DARK || reach[y][x] * 10 <= far * 4 || region[y][x] || unit_on(b, x, y)) continue;
		b->unit[b->nunits++] = (LibUnit){ (uint8_t)(UNIT_BRUTE + (kinds > 1 ? 1 + rrange(kinds - 1) : 0)), (uint8_t)x, (uint8_t)y, -1, false };
		++got;
	}
}

/* BN5's guarantees (docs/LIBERATION.md, Guarantees): something to
 * liberate in the first phase, no guardian within three of the start nor
 * threatening it */
static bool fair(const LibBoard *b, int16_t reach[MAP_H][MAP_W]) {
	for (int u = 1; u < b->nunits; ++u)
		if (reach[b->unit[u].y][b->unit[u].x] <= 3) return false;
	if (lib_threat(b, b->start_x, b->start_y)) return false;
	static int16_t clean[MAP_H][MAP_W];
	static uint16_t q[MAP_W * MAP_H];
	memset(clean, 0xFF, sizeof clean);
	int head = 0, tail = 0;
	clean[b->start_y][b->start_x] = 0;
	q[tail++] = (uint16_t)(b->start_y * MAP_W + b->start_x);
	while (head < tail) {
		int c = q[head++], x = c % MAP_W, y = c / MAP_W;
		for (int d = 0; d < 4; ++d) {
			int nx = x + dir4[d][0], ny = y + dir4[d][1];
			if (!on_board(b, nx, ny)) continue;
			if (lib_liberable(b, nx, ny)) return true;
			if (clean[ny][nx] < 0 && lib_walkable(b, nx, ny)) { clean[ny][nx] = 0; q[tail++] = (uint16_t)(ny * MAP_W + nx); }
		}
	}
	return false;
}

static int arrival(int *x, int *y) {
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_WARP_IN) { *x = (int)layer.obj[i].x; *y = (int)layer.obj[i].y; return 1; }
	return 0;
}

/* The board's floor: the layer's flat floor the arrival reaches, walked
 * into reach; the farthest walk, -1 no arrival on it */
static int board_floor(LibBoard *b, int16_t reach[MAP_H][MAP_W]) {
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			b->kind[y][x] = layer.cell[y][x] == C_PATH && !layer.level[y][x] ? LIB_CLEAN : LIB_VOID;
			b->panels += b->kind[y][x] != LIB_VOID;
		}
	if (!arrival(&b->start_x, &b->start_y) || !on_board(b, b->start_x, b->start_y)) return -1;
	int far = walk(b, b->start_x, b->start_y, reach);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (reach[y][x] < 0 && b->kind[y][x] != LIB_VOID) { b->kind[y][x] = LIB_VOID; --b->panels; }
	return far;
}

/* One try at the board: its floor and kinds, LIB_WHY_NONE where it keeps
 * BN5's guarantees */
static int make_once(LibBoard *b, int act, int team) {
	int a = act < 0 ? 0 : act > 5 ? 5 : act, short_by = spec[a].team - team < 0 ? 0 : spec[a].team - team;
	int holes = spec[a].holes - short_by / 2 < 1 ? 1 : spec[a].holes - short_by / 2, thin = 5 * short_by;
	int roam = spec[a].roam - short_by / 2 < 1 ? 1 : spec[a].roam - short_by / 2;
	memset(b, 0, sizeof *b);
	b->act = act;
	b->rules = spec[a].rules;
	b->route_x = b->route_y = -1;
	static int16_t reach[MAP_H][MAP_W];
	int far = board_floor(b, reach);
	if (far < 0) return LIB_WHY_FLOOR;
	if (!place_darkloid(b, reach, far)) return LIB_WHY_DARKLOID;
	if (!place_holes(b, reach, far, holes, spec[a].kinds)) return LIB_WHY_HOLES;
	place_dark(b, reach, far, spec[a].dark_lo - thin, spec[a].dark_hi - thin);
	prune_dark(b, dark_panels[a] * (2 + team) / 3 + 8 * holes);
	if (!place_barriers(b, reach)) return LIB_WHY_BARRIER;
	place_bonus(b, reach, spec[a].bonus);
	place_items(b, spec[a].items);
	place_loot(b, spec[a].loot);
	if (b->rules & LIB_RULE_TOLLS) place_traps(b, spec[a].traps);
	if ((b->rules & LIB_RULE_TOLLS) && !b->traps) return LIB_WHY_TRAPS;
	if (b->rules & LIB_RULE_REACH) place_roamers(b, reach, far, roam, spec[a].kinds);
	if (!fair(b, reach)) return LIB_WHY_FAIR;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) b->dark += lib_dark_kind(b->kind[y][x]);
	return LIB_WHY_NONE;
}

bool lib_board_make(LibBoard *b, uint32_t seed, int act, int team) {
	uint8_t rejected[16] = { 0 };
	for (int t = 0; t < MAKE_TRIES; ++t) {
		rs = (seed ^ 0x4C494245u) + (uint32_t)t * 0x9E3779B9u;   /* "LIBE" */
		if (!rs) rs = 1;
		for (int k = 0; k < 4; ++k) rnext();
		uint32_t plan_seed = rnext();
		int why = make_once(b, act, team);
		if (why == LIB_WHY_NONE) why = lib_plan(b, plan_seed);
		b->why = why;
		b->tries = t + 1;
		memcpy(b->rejected, rejected, sizeof rejected);
		if (why == LIB_WHY_NONE) return true;
		++b->rejected[why];
		rejected[why] = b->rejected[why];
	}
	return false;
}
