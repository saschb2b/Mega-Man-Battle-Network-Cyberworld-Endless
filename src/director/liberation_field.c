/* A liberation battle's field (liberation_field.h).
 *
 * BN5 weighs the dark round the panel a Navi liberates by where it stands
 * from him (0x08051D9E): the panel beyond it 20, its two sides 10, the two
 * diagonals beside him 2, the rest nothing; the sum's level (0, to 9, to
 * 19, to 30, above) picks the battle from the mission's lists, and every
 * battle of a level has one field (0x0811488C's +6): MegaMan's four
 * columns at 0, three at 1, two at 2, and from 3 Surrounded, his foes in
 * columns 1-2 and 5-6 and he at column 3 between them. BN6 reads the same
 * byte and keeps BN5's Surrounded whole: the columns' owners, the foes on
 * the left turned to face him, and MegaMan's turn round on L or R in place
 * of the Custom screen's request (bn6_panels.h). So the run's battle for
 * the panel keeps its foes and is set out on BN5's field. */
#include "liberation_field.h"

#include <stdlib.h>

#include "bn5_liberation.h"
#include "bn6_panels.h"
#include "liberation_state.h"
#include "visitor_families.h"

static const int dirs[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

/* the kinds BN5 counts (BN5_LIB_DARK_COUNTS: dark, an item's or a trap's,
 * a DarkHole; not the Darkloid's ring) */
static int dark(int x, int y) {
	if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return 0;
	int k = LB.b.kind[y][x];
	return k == LIB_DARK || k == LIB_ITEM || k == LIB_TRAP || k == LIB_HOLE;
}

int lib_field_level(int x, int y, int face) {
	int dx = dirs[face & 3][0], dy = dirs[face & 3][1], px = -dy, py = dx;
	int w = 20 * dark(x + dx, y + dy) + 10 * (dark(x + px, y + py) + dark(x - px, y - py)) +
		2 * (dark(x - dx + px, y - dy + py) + dark(x - dx - px, y - dy - py));
	return w <= 0 ? 0 : w <= 9 ? 1 : w <= 19 ? 2 : w <= 30 ? 3 : 4;
}

int lib_field_sides(int level, bool darkloid) {
	static const int by_level[LIB_LEVELS] = { BN6_SIDES_WIDE, BN6_SIDES_PLAIN, BN6_SIDES_NARROW, BN6_SIDES_SURROUNDED,
		BN6_SIDES_SURROUNDED };
	if (level < 0) level = 0;
	if (level >= LIB_LEVELS) level = LIB_LEVELS - 1;
	if (darkloid && by_level[level] == BN6_SIDES_SURROUNDED) return BN6_SIDES_NARROW;
	return by_level[level];
}

/* ---- setting the battle out ---- */

static bool used[3][6];

/* a panel one can stand on: not a hole, nor off the field's own panels */
static bool open_at(const Encounter *e, int col, int row) {
	if (col < 0 || col > 5 || row < 0 || row > 2 || used[row][col]) return false;
	int p = visitor_panel(e, col, row);
	return p < 0 || p >= 2;
}

/* the nearest open panel to (col, row) among columns lo-hi, its row first */
static bool take(const Encounter *e, int lo, int hi, int *col, int *row) {
	for (int d = 0; d < 6; ++d)
		for (int r = 0; r < 3; ++r)
			for (int c = lo; c <= hi; ++c)
				if (abs(c - *col) + abs(r - *row) == d && open_at(e, c, r)) {
					*col = c;
					*row = r;
					used[r][c] = true;
					return true;
				}
	return false;
}

/* his foes' columns (from 0) on the pattern: the right-hand pair, and where
 * Surrounded the left-hand one too */
static void foe_columns(int sides, bool left, int *lo, int *hi) {
	*lo = sides == BN6_SIDES_NARROW ? 2 : 3;
	*hi = 5;
	if (sides == BN6_SIDES_WIDE || sides == BN6_SIDES_SURROUNDED) *lo = 4;
	if (left) { *lo = 0; *hi = 1; }
}

static void set_megaman(Encounter *e, int sides) {
	int at = e->player ? e->player : 0x22;
	if (sides == BN6_SIDES_SURROUNDED) at = BN5_LIB_SURROUNDED_AT;
	int col = (at & 15) - 1, row = (at >> 4) - 1, lo = 0, hi = sides == BN6_SIDES_NARROW ? 1 : sides == BN6_SIDES_WIDE ? 3 : 2;
	if (sides == BN6_SIDES_SURROUNDED) lo = 2, hi = 3;
	if (col > hi) col = hi;
	if (take(e, lo, hi, &col, &row)) e->player = (row + 1) << 4 | (col + 1);
}

void lib_field_apply(Encounter *e, int sides) {
	e->sides = sides == BN6_SIDES_PLAIN ? 0 : sides;
	if (!e->sides) return;
	for (int r = 0; r < 3; ++r)
		for (int c = 0; c < 6; ++c) used[r][c] = false;
	set_megaman(e, sides);
	int kept = 0;
	for (int i = 0; i < e->nfoes; ++i) {
		/* (Surrounded: every other foe to the left, as BN5's records
		 * share them out, the right-hand columns mirrored) */
		bool left = sides == BN6_SIDES_SURROUNDED && i % 2;
		int lo, hi, col = e->foes[i].col, row = e->foes[i].row;
		foe_columns(sides, false, &lo, &hi);
		if (col < lo) col = lo;
		if (left) {
			col = 5 - col;
			foe_columns(sides, true, &lo, &hi);
		}
		bool placed = take(e, lo, hi, &col, &row);
		if (!placed && left) {
			foe_columns(sides, false, &lo, &hi);
			placed = take(e, lo, hi, &col, &row);
		}
		if (!placed) continue;
		e->foes[kept] = e->foes[i];
		e->foes[kept].col = col;
		e->foes[kept++].row = row;
	}
	e->nfoes = kept;
	/* (the field's objects stay where they stood, out of the way) */
	int objs = 0;
	for (int i = 0; i < e->nobj; ++i) {
		int col = (e->obj[i].panel & 15) - 1, row = (e->obj[i].panel >> 4) - 1;
		if (col < 0 || col > 5 || row < 0 || row > 2 || used[row][col]) continue;
		used[row][col] = true;
		e->obj[objs++] = e->obj[i];
	}
	e->nobj = objs;
}
