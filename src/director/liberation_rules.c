/* A liberation board's rules (liberation_board.h): its panels' (which are
 * dark, walked, liberated; the DarkHoles left, a barrier opened, the
 * Darkloid's heal) and its team's (which Navi stands where, each one's
 * command and the panels it takes, KnightMan's shield). */
#include "liberation_board.h"

#include <stdlib.h>

#include "bn5_liberation.h"

static const int dir4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
static const int dir8[8][2] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };

static bool on_board(const LibBoard *b, int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && b->kind[y][x] != LIB_VOID; }

/* A unit still on the board stands on panel (x, y) */
static bool unit_standing(const LibBoard *b, int x, int y) {
	for (int u = 0; u < b->nunits; ++u)
		if (!b->unit[u].gone && b->unit[u].x == x && b->unit[u].y == y) return true;
	return false;
}

/* ---- the panels ---- */

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

int lib_darkloid_heal(const LibBoard *b, int hp, int max) {
	if (hp >= max) return hp;
	hp += max / 10 * lib_dark_around(b, b->boss_x, b->boss_y);
	return hp > max ? max : hp;
}

int lib_open_barrier(LibBoard *b, int n) {
	int opened = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (b->kind[y][x] == LIB_BAR && b->item[y][x] == n) { b->kind[y][x] = LIB_CLEAN; b->item[y][x] = 0; ++opened; }
	return opened;
}

/* ---- the team ---- */

int lib_mate_at(const LibBoard *b, int x, int y) {
	for (int k = 0; k < b->team.n; ++k)
		if (!b->mate[k].out && b->mate[k].x == x && b->mate[k].y == y) return k;
	return -1;
}

int lib_command_of(int navi) {
	switch (navi) {
	case MATE_MEGAMAN: return LIB_DO_LONGSWRD;
	case MATE_COLONEL: return LIB_DO_SCRENDIV;
	case MATE_TOMAHAWKMAN: return LIB_DO_TOMAHAWK;
	case MATE_NUMBERMAN: return LIB_DO_NUMBER;
	case MATE_SHADOWMAN: return LIB_DO_SNEAK;
	case MATE_TOADMAN: return LIB_DO_MELODY;
	/* (the Cross Navis: Team ProtoMan's, by element, docs/LIBERATION.md) */
	case MATE_HEATMAN: return LIB_DO_NAPALM;
	case MATE_ELECMAN: return LIB_DO_MAGNET;
	case MATE_SLASHMAN: return LIB_DO_WIDESWRD;
	case MATE_ERASEMAN: return LIB_DO_SEARCH;
	case MATE_CHARGEMAN: return LIB_DO_BOMB;
	default: return LIB_DO_NONE;   /* (KnightMan: his StoneBody, always on) */
	}
}

bool lib_command_battles(int command) {
	return command == LIB_DO_LONGSWRD || command == LIB_DO_SCRENDIV || command == LIB_DO_TOMAHAWK || command == LIB_DO_NAPALM ||
	       command == LIB_DO_WIDESWRD || command == LIB_DO_SEARCH;
}

bool lib_flown(const LibBoard *b, int x, int y) {
	if (!on_board(b, x, y)) return false;
	int k = b->kind[y][x];
	return k == LIB_DARK || k == LIB_ITEM || k == LIB_TRAP;
}

/* The shapes of the commands, from the panel faced (docs/LIBERATION.md,
 * The Navis: BN5's shapes table BN5_LIB_SHAPES, read for LongSwrd,
 * ScrenDiv, TomahawkSwing and the Cross Navis' five; the guides' for the
 * rest): LongSwrd two in a line; ScrenDiv and WideSwrd three across;
 * TomahawkSwing and NumberCheck two deep and three wide; SneakAttack one
 * or two ahead; LifeMelody's five ahead; Napalm four ahead and both sides
 * of the third; PanelSearch the item and trap panels of the dark line from
 * the one faced; BombLiberation the plain dark panel the flier hovers over
 * (x, y his own); MagnetBarrier none, the whole team's */
int lib_command_panels(const LibBoard *b, int command, int x, int y, int dir, int xs[6], int ys[6]) {
	static const struct { int8_t deep, wide; } shape[] = {
		[LIB_DO_NONE] = { 1, 0 }, [LIB_DO_LONGSWRD] = { 2, 0 }, [LIB_DO_SCRENDIV] = { 1, 1 }, [LIB_DO_TOMAHAWK] = { 2, 1 },
		[LIB_DO_NUMBER] = { 2, 1 }, [LIB_DO_SNEAK] = { 2, 0 }, [LIB_DO_MELODY] = { 5, 0 }, [LIB_DO_NAPALM] = { 4, 0 },
		[LIB_DO_WIDESWRD] = { 1, 1 }, [LIB_DO_MAGNET] = { 0, 0 }, [LIB_DO_SEARCH] = { 0, 0 }, [LIB_DO_BOMB] = { 1, 0 },
	};
	if (command < 0 || command >= LIB_DOS || dir < 0 || dir > 3) return 0;
	/* (BombLiberation: a plain dark panel alone, BN5_LIB_BOMB_KINDS) */
	if (command == LIB_DO_BOMB) {
		if (!on_board(b, x, y) || b->kind[y][x] != LIB_DARK) return 0;
		xs[0] = x; ys[0] = y;
		return 1;
	}
	/* (PanelSearch: the line from the faced panel on while dark or an
	 * item's, no unit on it, BN5_LIB_SEARCH_MAX at most, cut to its item
	 * and trap panels, BN5_LIB_SEARCH_KEEP) */
	if (command == LIB_DO_SEARCH) {
		int n = 0;
		for (int d = 0; d < BN5_LIB_SEARCH_MAX && n < 6; ++d) {
			int tx = x + d * dir4[dir][0], ty = y + d * dir4[dir][1];
			if (!lib_flown(b, tx, ty) || unit_standing(b, tx, ty)) break;
			if (b->kind[ty][tx] != LIB_DARK) { xs[n] = tx; ys[n] = ty; ++n; }
		}
		return n;
	}
	int n = 0, px = -dir4[dir][1], py = dir4[dir][0];
	for (int d = 0; d < shape[command].deep; ++d)
		for (int w = -shape[command].wide; w <= shape[command].wide; ++w) {
			int tx = x + d * dir4[dir][0] + w * px, ty = y + d * dir4[dir][1] + w * py;
			if (n < 6 && on_board(b, tx, ty)) { xs[n] = tx; ys[n] = ty; ++n; }
		}
	/* (Napalm's flames spread at the third panel ahead) */
	for (int w = -1; command == LIB_DO_NAPALM && w <= 1; w += 2) {
		int tx = x + 2 * dir4[dir][0] + w * px, ty = y + 2 * dir4[dir][1] + w * py;
		if (n < 6 && on_board(b, tx, ty)) { xs[n] = tx; ys[n] = ty; ++n; }
	}
	return n;
}

bool lib_mate_shielded(const LibBoard *b, int k) {
	if (k < 0 || k >= b->team.n || b->mate[k].out) return false;
	for (int j = 0; j < b->team.n; ++j)
		if (b->team.navi[j] == MATE_KNIGHTMAN && !b->mate[j].out && abs(b->mate[j].x - b->mate[k].x) + abs(b->mate[j].y - b->mate[k].y) <= 1) return true;
	return false;
}
