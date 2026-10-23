/* A liberation's board from the dark layer's floor (liberation_board.h):
 * BN5's kinds placed by BN5's rules, as tools/liberation.py's `generate`
 * places them on its own floors (docs/LIBERATION.md, The generator): the
 * arrival's start zone kept clean, the Darkloid's 3x3 on the far side,
 * DarkHoles in rings of dark panels each with its guardian, dark panels
 * thickening with the walk from the start and gathered by one majority
 * pass, BonusPanels at dead ends, items in the dark's corners first. The
 * prototype's barriers and sealed routes, its chips' and zenny's items and
 * its planner's par come later (stage 1 builds the board for MegaMan alone).
 * Its own random numbers, the game's left alone. */
#include "liberation_board.h"

#include <stdlib.h>
#include <string.h>

/* Per act from the second (0-based 1-5), BN5's mission it stands for
 * (docs/LIBERATION.md, What a mission is made of): its team, DarkHoles,
 * roaming guardians, the guardians' kinds, the dark share's range
 * (percent), item panels and BonusPanels. A smaller team than BN5's meets
 * a DarkHole less for every two Navis short, and dark five points thinner
 * for each (MegaMan alone, stage 1, liberated act 2's 197 panels in 18
 * phases by the par's yardstick with two DarkHoles, 12 with one). */
static const struct { int team, holes, roam, kinds, dark_lo, dark_hi, items, bonus; } spec[6] = {
	{ 2, 2, 0, 1, 40, 55, 5, 2 },    /* (act 1 has none: as act 2's, for a test's) */
	{ 2, 2, 0, 1, 40, 55, 5, 2 },
	{ 3, 3, 1, 2, 45, 58, 6, 3 },
	{ 4, 4, 2, 3, 52, 66, 8, 3 },
	{ 5, 5, 2, 3, 58, 68, 10, 3 },
	{ 5, 5, 3, 3, 55, 68, 9, 3 },
};
#define MAKE_TRIES 8

static const int dir4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
static const int dir8[8][2] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };

/* ---- its random numbers ---- */
static uint32_t rs;
static uint32_t rnext(void) { rs ^= rs << 13; rs ^= rs >> 17; rs ^= rs << 5; return rs; }
static int rrange(int n) { return n <= 1 ? 0 : (int)(rnext() % (uint32_t)n); }
static int rpercent(void) { return rrange(100); }

static bool on_board(const LibBoard *b, int x, int y) {
	return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && b->kind[y][x] != LIB_VOID;
}

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
	return k == LIB_DARK || k == LIB_ITEM || k == LIB_HOLE || (k == LIB_RING && !lib_holes_left(b));
}

int lib_dark_around(const LibBoard *b, int x, int y) {
	int n = 0;
	for (int d = 0; d < 8; ++d) {
		int nx = x + dir8[d][0], ny = y + dir8[d][1];
		if (!on_board(b, nx, ny)) continue;
		int k = b->kind[ny][nx];
		n += k == LIB_DARK || k == LIB_ITEM || k == LIB_HOLE || k == LIB_RING;
	}
	return n;
}

/* Walks from (sx, sy) over the board, every panel: into dist (-1 unreached);
 * the farthest */
static int walk(const LibBoard *b, int sx, int sy, int16_t dist[MAP_H][MAP_W]) {
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
			dist[ny][nx] = (int16_t)(dist[y][x] + 1);
			q[tail++] = (uint16_t)(ny * MAP_W + nx);
		}
	}
	return far;
}

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

static int manhattan(int ax, int ay, int bx, int by) { return (ax > bx ? ax - bx : bx - ax) + (ay > by ? ay - by : by - ay); }

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

/* BonusPanels at dead ends (BN5's "?" at the ends of its stubs) */
static void place_bonus(LibBoard *b, int16_t reach[MAP_H][MAP_W], int want) {
	for (int got = 0, tries = 0; got < want && tries < 400; ++tries) {
		int x = rrange(MAP_W), y = rrange(MAP_H), k = b->kind[y][x];
		if ((k != LIB_CLEAN && k != LIB_DARK) || reach[y][x] <= 2 || neighbours(b, x, y) != 1) continue;
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
				if (b->kind[y][x] == LIB_DARK && neighbours(b, x, y) == few && rpercent() < 60) darks[n++] = (uint16_t)(y * MAP_W + x);
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

/* Roaming guardians on dark panels past two fifths of the walk */
static void place_roamers(LibBoard *b, int16_t reach[MAP_H][MAP_W], int far, int want, int kinds) {
	for (int got = 0, tries = 0; got < want && b->nunits < LIB_UNITS && tries < 400; ++tries) {
		int x = rrange(MAP_W), y = rrange(MAP_H);
		if (b->kind[y][x] != LIB_DARK || reach[y][x] * 10 <= far * 4) continue;
		bool taken = false;
		for (int u = 0; u < b->nunits && !taken; ++u) taken = b->unit[u].x == x && b->unit[u].y == y;
		if (taken) continue;
		b->unit[b->nunits++] = (LibUnit){ (uint8_t)(UNIT_BRUTE + rrange(kinds)), (uint8_t)x, (uint8_t)y, -1, false };
		++got;
	}
}

/* BN5's guarantees (docs/LIBERATION.md, Guarantees): something to
 * liberate in the first phase, no guardian within three of the start */
static bool fair(const LibBoard *b, int16_t reach[MAP_H][MAP_W]) {
	for (int u = 1; u < b->nunits; ++u)
		if (reach[b->unit[u].y][b->unit[u].x] <= 3) return false;
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

/* The dark panels a walk from (sx, sy) liberates to reach the nearest of
 * the targets still standing (a 0-1 walk: a dark panel 1, a clean one 0);
 * into (*tx, *ty) which */
static int cost_to(const LibBoard *b, int sx, int sy, const bool target[MAP_H][MAP_W], int *tx, int *ty) {
	static int16_t cost[MAP_H][MAP_W];
	static uint16_t dq[MAP_W * MAP_H * 2];
	memset(cost, 0x7F, sizeof cost);
	int head = MAP_W * MAP_H, tail = head;
	cost[sy][sx] = 0;
	dq[tail++] = (uint16_t)(sy * MAP_W + sx);
	while (head < tail) {
		int c = dq[head++], x = c % MAP_W, y = c / MAP_W;
		if (target[y][x]) { *tx = x; *ty = y; return cost[y][x]; }
		for (int d = 0; d < 4; ++d) {
			int nx = x + dir4[d][0], ny = y + dir4[d][1];
			if (!on_board(b, nx, ny)) continue;
			int k = b->kind[ny][nx], w = k == LIB_CLEAN || k == LIB_BONUS ? 0 : 1, nc = cost[y][x] + w;
			if (nc >= cost[ny][nx]) continue;
			cost[ny][nx] = (int16_t)nc;
			if (w) dq[tail++] = (uint16_t)(ny * MAP_W + nx);
			else dq[--head] = (uint16_t)(ny * MAP_W + nx);
		}
	}
	return -1;
}

/* The par (stage 1): the dark panels on the way from the start to each
 * DarkHole in turn, nearest first, then to the Darkloid; a lone MegaMan
 * liberates about two a phase (one, and the eight round it a one-turn win
 * in four or so), and fights each hole's guardian and hole and the
 * Darkloid a phase each, and rests once. The planner's par
 * (tools/liberation.py) is for teams, a later stage's. */
#define PANELS_A_PHASE 2
int lib_par(const LibBoard *b) {
	static bool target[MAP_H][MAP_W];
	static LibBoard scratch;
	scratch = *b;
	int x = b->start_x, y = b->start_y, dark = 0;
	for (int left = b->nholes; left >= 0; --left) {
		memset(target, 0, sizeof target);
		if (left) for (int h = 0; h < b->nholes; ++h) target[scratch.hole_y[h]][scratch.hole_x[h]] = scratch.kind[scratch.hole_y[h]][scratch.hole_x[h]] == LIB_HOLE;
		else target[b->boss_y][b->boss_x] = true;
		int tx = x, ty = y, c = cost_to(&scratch, x, y, target, &tx, &ty);
		if (c < 0) break;
		dark += c;
		scratch.kind[ty][tx] = LIB_CLEAN;
		x = tx; y = ty;
	}
	return (dark + PANELS_A_PHASE - 1) / PANELS_A_PHASE + 2 * b->nholes + 2;
}

static int arrival(int *x, int *y) {
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_WARP_IN) { *x = (int)layer.obj[i].x; *y = (int)layer.obj[i].y; return 1; }
	return 0;
}

static bool make_once(LibBoard *b, int act, int team) {
	int a = act < 0 ? 0 : act > 5 ? 5 : act, short_by = spec[a].team - team < 0 ? 0 : spec[a].team - team;
	int holes = spec[a].holes - (short_by + 1) / 2, thin = 5 * short_by;
	if (holes < 1) holes = 1;
	memset(b, 0, sizeof *b);
	b->act = act;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			b->kind[y][x] = layer.cell[y][x] == C_PATH && !layer.level[y][x] ? LIB_CLEAN : LIB_VOID;
			b->panels += b->kind[y][x] != LIB_VOID;
		}
	if (!arrival(&b->start_x, &b->start_y) || !on_board(b, b->start_x, b->start_y)) return false;
	static int16_t reach[MAP_H][MAP_W];
	int far = walk(b, b->start_x, b->start_y, reach);
	/* (only what the start reaches is the board) */
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (reach[y][x] < 0 && b->kind[y][x] != LIB_VOID) { b->kind[y][x] = LIB_VOID; --b->panels; }
	if (!place_darkloid(b, reach, far)) return false;
	if (!place_holes(b, reach, far, holes, spec[a].kinds)) return false;
	place_dark(b, reach, far, spec[a].dark_lo - thin, spec[a].dark_hi - thin);
	place_bonus(b, reach, spec[a].bonus);
	place_items(b, spec[a].items);
	place_roamers(b, reach, far, spec[a].roam, spec[a].kinds);
	if (!fair(b, reach)) return false;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			int k = b->kind[y][x];
			b->dark += k == LIB_DARK || k == LIB_ITEM || k == LIB_HOLE || k == LIB_RING;
		}
	b->par = lib_par(b);
	return true;
}

bool lib_board_make(LibBoard *b, uint32_t seed, int act, int team) {
	for (int t = 0; t < MAKE_TRIES; ++t) {
		rs = (seed ^ 0x4C494245u) + (uint32_t)t * 0x9E3779B9u;   /* "LIBE" */
		if (!rs) rs = 1;
		for (int k = 0; k < 4; ++k) rnext();
		if (make_once(b, act, team)) { b->tries = t + 1; return true; }
	}
	return false;
}
