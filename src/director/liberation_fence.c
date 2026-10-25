/* A liberation's board fenced (liberation.h): MegaMan walks the board as
 * BN6 walks him, and a panel he may not walk holds him back as a wall
 * would, as BN5's board holds its Navis back (docs/LIBERATION.md: BN5
 * raises its border cells' heights, per panel, as the board changes).
 * Every panel that is not walked when the board is made (dark, an item's,
 * a DarkHole, the Darkloid's ring and his panel) gets walls of BN6's own on
 * its cells that border another panel's floor, written with the layer's
 * own. Their value names one of two of BN6's map events (a wall whose value
 * has bit 7 set is skipped while event 0x1640 + its low bits is set; Rush's
 * gaps' walls, docs/ROM_DATA.md): FENCE_SHUT, never set, so the wall
 * stands, or FENCE_OPEN, set each frame (entering a map clears it), so it
 * is skipped. As panels are liberated, or darkened again, each panel's
 * walls are switched between the two in BN6's coordinate data in RAM
 * (BN6_MAP_COORDS, read in place), so no board has more fences than BN6's
 * events allow, however many panels it holds. */
#include "liberation.h"

#include <stdio.h>
#include <string.h>

#include "bn6_map.h"
#include "debug.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "liberation_state.h"
#include "liberation_team.h"
#include "netmap.h"

#define FENCE_EVENT   0x1640   /* BN6's map events: a wall of value 0x80 + n is off while 0x1640 + n is set */
/* (0x80 + 0-7 are the Link Navi obstacles' walls, blockers.c; 0x7E-0x7F
 * Rush's gaps', coords.c) */
#define FENCE_SHUT    0x60     /* never set: the wall stands */
#define FENCE_OPEN    0x61     /* set each frame: the wall is skipped */
#define FENCE_WALLS   4096     /* nm_extra's room for a section's cells */
#define COORD_MAX     16384    /* coords.c's walls a section at most */

static bool blocking(int k) { return k == LIB_DARK || k == LIB_ITEM || k == LIB_HOLE || k == LIB_RING || k == LIB_BOSS || k == LIB_BAR || k == LIB_TRAP || k == LIB_BONUS; }

/* The fence's walls as written, and where BN6's coordinate data holds each
 * once the map is entered (found: the walls' section's count and every
 * fence wall's entry, its key checked before each write) */
static struct {
	int n;                         /* walls written */
	uint16_t key[FENCE_WALLS];     /* each wall's cell, as BN6 keys it */
	uint8_t px[FENCE_WALLS], py[FENCE_WALLS];
	uint32_t count;                /* the section's count where they were found, 0 not yet */
	uint16_t at[FENCE_WALLS];      /* each one's entry in the section */
} F;

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

/* (the board is made on the map's floor as built, lib_layer_made: every
 * panel of it has floor) */
void lib_fence_build(void) {
	memset(LB.fence, 0, sizeof LB.fence);
	F.n = 0;
	F.count = 0;
	if (!LB.on) return;
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (blocking(LB.b.kind[y][x])) LB.fence[y][x] = 1, ++n;
	static CoordCell walls[FENCE_WALLS];
	for (int cy = -126; cy < 126; ++cy)
		for (int cx = -126; cx < 126; ++cx) {
			int px, py;
			if (!netmap_floor_cell(cx, cy, 0) || !cell_panel(cx, cy, &px, &py) || !LB.fence[py][px]) continue;
			int t = fence_type(cx, cy, px, py);
			if (!t || F.n >= FENCE_WALLS) continue;
			walls[F.n] = (CoordCell){ (int16_t)(cx * 8), (int16_t)(cy * 8), 0, 0x80 | FENCE_SHUT, 8, (uint8_t)t };
			F.key[F.n] = (uint16_t)((cy + 127) * BN6_COORD_KEY_SPAN + cx + 127);
			F.px[F.n] = (uint8_t)px, F.py[F.n] = (uint8_t)py;
			++F.n;
		}
	netmap_add_walls(walls, F.n);

	if (emu_debug_on()) fprintf(stderr, "liberation: %d panels fenced, %d walls\n", n, F.n);
}

/* The section's entry of the wall on cell `key` with a fence's value, from
 * `from` on (its keys ascend), -1 none */
static int entry_of(uint32_t count, uint16_t key, uint32_t from) {
	uint32_t shapes = BN6_COORD_ENTRIES + count * BN6_COORD_ENTRY;
	for (uint32_t i = from; i < count; ++i) {
		uint16_t k = emu_read16(BN6_COORD_ENTRIES + i * BN6_COORD_ENTRY);
		if (k > key) break;
		uint8_t v = emu_read8(shapes + i * BN6_COORD_ENTRY + BN6_COORD_VALUE);
		if (k == key && (v == (0x80 | FENCE_SHUT) || v == (0x80 | FENCE_OPEN))) return (int)i;
	}
	return -1;
}

/* Every fence wall's entry in the map's coordinate data, once it is ours
 * (all found, in the order written: keys ascending) */
static bool fence_find(void) {
	uint32_t count = emu_read32(BN6_MAP_COORDS);
	if (count < (uint32_t)F.n || count > COORD_MAX) return false;
	/* (written by cell rows, y then x: their keys ascend as the section's) */
	uint32_t from = 0;
	for (int i = 0; i < F.n; ++i) {
		int e = entry_of(count, F.key[i], from);
		if (e < 0) return false;
		F.at[i] = (uint16_t)e;
		from = (uint32_t)e + 1;
	}
	F.count = count;
	if (emu_debug_on()) fprintf(stderr, "liberation: the fence's %d walls found among %u\n", F.n, (unsigned)count);
	return true;
}

/* Whether the map's coordinate data is still the board's, as found */
static bool fence_there(void) {
	if (!F.count || emu_read32(BN6_MAP_COORDS) != F.count) return false;
	return emu_read16(BN6_COORD_ENTRIES + F.at[0] * BN6_COORD_ENTRY) == F.key[0] && emu_read16(BN6_COORD_ENTRIES + F.at[F.n - 1] * BN6_COORD_ENTRY) == F.key[F.n - 1];
}

void lib_fence_update(void) {
	if (!LB.on || !F.n) return;
	flag_set(FENCE_EVENT + FENCE_OPEN);
	flag_clear(FENCE_EVENT + FENCE_SHUT);
	/* (on the board's map only: elsewhere the buffer holds another's) */
	if (!on_map() || (!fence_there() && !fence_find())) return;
	/* (the board won: every wall off, the way to the exit open) */
	bool open = LB.step == LIB_STEP_PRIZE || LB.step == LIB_STEP_DONE || (LB.step == LIB_STEP_TALK && LB.after == LIB_STEP_DONE);
	uint32_t shapes = BN6_COORD_ENTRIES + F.count * BN6_COORD_ENTRY;
	bool flier = (LB.step == LIB_STEP_MOVE || LB.step == LIB_STEP_MENU) && lib_mate_command(lib_in_play()) == LIB_DO_BOMB;
	for (int i = 0; i < F.n; ++i) {
		int x = F.px[i], y = F.py[i];
		/* (walked: its walls off; dark again, Bladia's: on; a dark one off
		 * for BombLiberation's flier in play, who hovers over it) */
		bool shut = !open && ((blocking(LB.b.kind[y][x]) && !(flier && lib_flown(&LB.b, x, y))) || lib_unit_at(x, y) >= 0);
		uint8_t want = 0x80 | (shut ? FENCE_SHUT : FENCE_OPEN);
		uint32_t at = shapes + F.at[i] * BN6_COORD_ENTRY + BN6_COORD_VALUE;
		if (emu_read8(at) != want && emu_read16(BN6_COORD_ENTRIES + F.at[i] * BN6_COORD_ENTRY) == F.key[i]) emu_write8(at, want);
	}
}
