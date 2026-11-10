/* The differential's NaviCust items (xdiff_ncp.h). */
#include "xdiff_ncp.h"

#include <stdio.h>

#include "bn5.h"
#include "bn6.h"
#include "emu.h"
#include "gamecall.h"
#include "guest.h"

static bool in_ewram(uint32_t a) { return a >= BN6_EWRAM && a < BN6_EWRAM_END; }

/* (the guest's bytes: a halfword's half written) */
static void guest_wr8(uint32_t a, uint8_t v) {
	uint16_t h = guest_read16(a & ~1u);
	guest_write16(a & ~1u, (uint16_t)(a & 1 ? (h & 0xFF) | v << 8 : (h & 0xFF00) | v));
}

/* ncp6=: the program on BN6's own NaviCust board, alone, and BN6's own compile run (BN6_NAVICUST_RELOAD): its entry in the
 * placed list, its shape's cells on the grid from its record (the core's copy: program 48's is the engine's, xncp.h) */
bool xd_ncp_subject(int v, int col, int row) {
	if (!v) return true;
	static const uint32_t record_refs[] = BN6_NCP_RECORD_REFS;
	uint32_t rec = emu_read32(record_refs[0]) + (uint32_t)v * BN6_NCP_RECORD, shape = emu_read32(rec + BN6_NCP_RECORD_SHAPE);
	uint8_t grid[BN6_NCP_SHAPE] = { 0 }, placed[BN6_NAVICUST_SLOTS * 8] = { (uint8_t)v, (uint8_t)(v >> 8) };
	placed[BN6_NCP_PLACED_COLUMN] = (uint8_t)col;
	placed[BN6_NCP_PLACED_ROW] = (uint8_t)row;
	for (int y = 0; y < 7; ++y)
		for (int x = 0; x < 7; ++x) {
			int gx = col - 3 + x, gy = row - 3 + y;
			if (emu_read8(shape + (uint32_t)(y * 7 + x)) && gx >= 0 && gx < 7 && gy >= 0 && gy < 7) grid[gy * 7 + gx] = 1;
		}
	emu_write(BN6_NAVICUST_PLACED, placed, sizeof placed);
	emu_write(BN6_NAVICUST_GRID, grid, sizeof grid);
	gamecall_install();   /* (the run's own two hooks, which the differential's core has none of until a spec asks) */
	bool ok = game_call(BN6_NAVICUST_RELOAD, 0, 0);
	printf("xdiff: BN6's NaviCust: variant %#x at column %d, row %d, compiled %s: its extra turns %d\n", v, col, row,
		ok ? "by BN6" : "never", (int8_t)emu_read8(BN6_NAVI_STATS + BN6_NAVI_TURN_EXTRA));
	return ok;
}

/* ncp5=: the same on BN5's own board, by BN5's own compile (BN5_NAVICUST_COMPILE), on its map before its battle is asked */
bool xd_ncp_reference(int v, int col, int row) {
	if (!v) return true;
	uint32_t grid = guest_read32(BN5_TOOLKIT + BN5_TOOLKIT_NC_GRID), placed = guest_read32(BN5_TOOLKIT + BN5_TOOLKIT_NC_PLACED);
	uint32_t shape = guest_read32(BN5_NCP_RECORDS + (uint32_t)v * BN6_NCP_RECORD + BN6_NCP_RECORD_SHAPE);   /* (a record as BN6's) */
	if (!in_ewram(grid) || !in_ewram(placed)) return false;
	for (uint32_t i = 0; i < BN5_NC_PLACED_MAX * 8; ++i) guest_wr8(placed + i, 0);
	for (uint32_t i = 0; i < BN5_NC_SIDE * BN5_NC_SIDE; ++i) guest_wr8(grid + i, 0);
	guest_wr8(placed, (uint8_t)v);
	guest_wr8(placed + BN5_NC_PLACED_COLUMN, (uint8_t)col);
	guest_wr8(placed + BN5_NC_PLACED_ROW, (uint8_t)row);
	for (int y = 0; y < BN5_NC_SIDE; ++y)
		for (int x = 0; x < BN5_NC_SIDE; ++x) {
			int gx = col - 2 + x, gy = row - 2 + y;
			if (guest_read8(shape + (uint32_t)(y * BN5_NC_SIDE + x)) && gx >= 0 && gx < BN5_NC_SIDE && gy >= 0 && gy < BN5_NC_SIDE)
				guest_wr8(grid + (uint32_t)(gy * BN5_NC_SIDE + gx), 1);
		}
	bool ok = guest_call(BN5_NAVICUST_COMPILE);
	printf("xdiff: BN5's NaviCust: variant %#x at column %d, row %d, compiled %s: its extra turns %d\n", v, col, row,
		ok ? "by BN5" : "never", (int8_t)guest_read8(BN5_NAVI_TURN_EXTRA));
	return ok;
}
