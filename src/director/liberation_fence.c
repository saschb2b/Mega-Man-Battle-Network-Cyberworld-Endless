/* A liberation's board fenced (liberation.h): MegaMan walks the board as
 * BN6 walks him, and a dark panel holds him back as a wall would, as BN5's
 * board holds its Navis back (docs/LIBERATION.md: BN5 raises its border
 * cells' heights). Each panel that is not walked (dark, an item's, a
 * DarkHole, the Darkloid's ring and his panel) gets walls of BN6's own on
 * its cells that border another panel's floor, each wall's value 0x80 + the
 * panel's number: BN6 skips such a wall while event 0x1640 + that number
 * is set (Rush's gaps' walls, docs/ROM_DATA.md). The walls are written with
 * the layer's own; liberated, a panel's event is set each frame (entering a
 * map clears them) and its walls let MegaMan through. */
#include "liberation.h"

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "director_state.h"
#include "flags.h"
#include "liberation_state.h"
#include "netmap.h"

#define FENCE_EVENT   0x1640   /* BN6's map events: a wall of value 0x80 + n is off while 0x1640 + n is set */
#define FENCE_WALLS   4096     /* nm_extra's room for a section's cells */

static bool blocking(int k) { return k == LIB_DARK || k == LIB_ITEM || k == LIB_HOLE || k == LIB_RING || k == LIB_BOSS; }

/* The board panel a wall cell (8 world units) lies in, by its middle */
static bool cell_panel(int cx, int cy, int *x, int *y) {
	return netmap_panel(cx * 8 + 4, cy * 8 + 4, x, y) && *x >= 0 && *y >= 0 && *x < MAP_W && *y < MAP_H;
}

/* Floor of the layer outside panel (px, py) */
static bool floor_else(int cx, int cy, int px, int py) {
	int x, y;
	return netmap_floor_cell(cx, cy, 0) && cell_panel(cx, cy, &x, &y) && (x != px || y != py);
}

/* Cell (cx, cy) of panel (px, py), where it meets another panel's floor:
 * its wall as coords.c's walls() would set it there (an edge naming the
 * floor's side, a corner where floor lies round it) */
static int fence_type(int cx, int cy, int px, int py) {
	static const int dir[8][3] = {
		{ -1, 0, 1 }, { 1, 0, 2 }, { 0, -1, 3 }, { 0, 1, 4 },
		{ -1, 1, 7 }, { -1, -1, 5 }, { 1, -1, 6 }, { 1, 1, 8 },
	};
	int m = 0;
	for (int k = 0; k < 4; ++k) if (floor_else(cx + dir[k][0], cy + dir[k][1], px, py)) m |= 1 << k;
	if ((m & 3) && (m & 3) != 3 && (m & 12) && (m & 12) != 12) {
		static const int corner[2][2] = { { 5, 7 }, { 6, 8 } };
		return corner[(m & 2) != 0][(m & 8) != 0];
	}
	for (int k = 0; k < 8; ++k)
		if (floor_else(cx + dir[k][0], cy + dir[k][1], px, py)) return dir[k][2];
	return 0;
}

/* The board's panels the map left without floor at their middles (its
 * legal.c drops a few to draw the rest): off the board, where nothing of
 * the board's stands on them */
static void off_board(void) {
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			int k = LB.b.kind[y][x], wx, wy;
			if (k != LIB_CLEAN && k != LIB_DARK && k != LIB_ITEM && k != LIB_BONUS) continue;
			netmap_world(x, y, &wx, &wy);
			if (netmap_floor_cell(wx >> 3, wy >> 3, 0) || lib_unit_at(x, y) >= 0) continue;
			if (emu_debug_on()) fprintf(stderr, "liberation: panel %d,%d off the map's floor\n", x, y);
			LB.b.kind[y][x] = LIB_VOID;
		}
}

void lib_fence_build(void) {
	memset(LB.fence, 0xFF, sizeof LB.fence);
	if (!LB.on) return;
	off_board();
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (blocking(LB.b.kind[y][x]) && n < LIB_FENCES) LB.fence[y][x] = (uint8_t)n++;
	static CoordCell walls[FENCE_WALLS];
	int nw = 0;
	for (int cy = -126; cy < 126; ++cy)
		for (int cx = -126; cx < 126; ++cx) {
			int px, py;
			if (!netmap_floor_cell(cx, cy, 0) || !cell_panel(cx, cy, &px, &py) || LB.fence[py][px] == 0xFF) continue;
			int t = fence_type(cx, cy, px, py);
			if (t && nw < FENCE_WALLS)
				walls[nw++] = (CoordCell){ (int16_t)(cx * 8), (int16_t)(cy * 8), 0, (uint8_t)(0x80 | LB.fence[py][px]), 8, (uint8_t)t };
		}
	netmap_add_walls(walls, nw);

	if (emu_debug_on()) fprintf(stderr, "liberation: %d panels fenced, %d walls\n", n, nw);
}

void lib_fence_update(void) {
	/* (the board won: every wall off, the way to the exit open) */
	bool open = LB.step == LIB_STEP_PRIZE || LB.step == LIB_STEP_DONE;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			int f = LB.fence[y][x];
			if (f == 0xFF) continue;
			/* (walked: its walls off; dark again, Bladia's: on) */
			if (!open && (blocking(LB.b.kind[y][x]) || lib_unit_at(x, y) >= 0)) flag_clear(FENCE_EVENT + f);
			else flag_set(FENCE_EVENT + f);
		}
}
