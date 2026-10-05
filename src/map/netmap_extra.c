/* The set pieces' cells on a generated map (netmap.c): Rush's gaps and
 * the invisible paths' panels, the obstacles' and cubes' edges, the arrow
 * lanes, and the stairs' extra cells. */
#include "netmap_extra.h"

#include <string.h>

#include "net_shapes.h"
#include "netmap_learn.h"
#include "netmap_paste.h"

static void add_extra(int s, CoordCell c);

/* ---- Rush's gaps (issue #14) ----
 * For the walls a gap's panels are a walkway, walled along its sides and
 * open at its mouths, where coords.c sets the floor's edges again with a
 * flag that Rush lying there switches off (Central Area 2's gap has its
 * walls so); its tiles stay void. */
static struct { int A, B, g; } gap_panels[MAX_GAPS * 3];
static int ngap_panels;
static bool gaps_shut;

static int gap_panel(int A, int B) {
	for (int i = 0; i < ngap_panels; ++i)
		if (gap_panels[i].A == A && gap_panels[i].B == B) return gap_panels[i].g;
	return -1;
}

void netmap_gaps_shut(bool shut) { gaps_shut = shut; }

bool netmap_gap_any(void) { return ngap_panels > 0; }

int netmap_gap_at(int cx, int cy) {
	if (!ngap_panels) return 0;
	return gap_panel(nm_floordiv(cx * 8 + 4 - nm_place.ex, 32), nm_floordiv(cy * 8 + 4 - nm_place.ey, 32)) + 1;
}

/* Whether panel (A, B) is an invisible path's (issue #46): void as drawn,
 * floor to the walls, never shut. */
static bool path_panel(int A, int B) {
	for (int i = 0; i < layer.npaths; ++i)
		for (int k = 1; k <= layer.path[i].len; ++k) {
			int pA, pB;
			nm_grid_to_panel(layer.path[i].x + dir_dx[layer.path[i].dir] * k, layer.path[i].y + dir_dy[layer.path[i].dir] * k, &pA, &pB);
			if (pA == A && pB == B) return true;
		}
	return false;
}

static bool floor_level(int A, int B, int level) {
	int k = nm_kind_at(A, B);
	if (!level && !gaps_shut && ngap_panels && gap_panel(A, B) >= 0) return true;
	if (!level && layer.npaths && path_panel(A, B)) return true;
	return k == K_STAIR || k == (level ? K_RAISED : K_FLOOR);
}

void netmap_block_edges(const NetBlock *b, int *dir, int *edge, int *side) {
	int wx, wy, rx, ry;
	netmap_world(b->x, b->y, &wx, &wy);
	netmap_world(b->x - dir_dx[b->dir], b->y - dir_dy[b->dir], &rx, &ry);
	int ux = (wx - rx) / 32, uy = (wy - ry) / 32;
	if (ux) { *dir = ux > 0 ? 0 : 2; *edge = rx + 16 * ux; *side = wy - 16; }
	else { *dir = uy > 0 ? 1 : 3; *edge = ry + 16 * uy; *side = wx - 16; }
}

/* The obstacles' walls and checks (issue #42), as BN6's own stand in a
 * walkway's mouth: a line of five wall cells across it, a cell into the
 * walkway, flagged 0x80 + its slot (off once it opens, wall flag 0x1640 +
 * slot); and its check cells, value 0xF0 + slot, four along from the edge
 * and nine across, where the engine's A probe (24 ahead, 17 on a diagonal)
 * lands from the floor before it. */
void nm_blocks_place(void) {
	for (int k = 0; k < layer.nblocks; ++k) {
		int dir, e, s;
		netmap_block_edges(&layer.block[k], &dir, &e, &s);
		bool along_x = !(dir & 1);
		int sign = dir < 2 ? 1 : -1, at = e + (sign > 0 ? 4 : -12);
		for (int i = 0; i < 5; ++i) {
			int a = at, c = s - 4 + 8 * i;
			add_extra(0, (CoordCell){ (int16_t)(along_x ? a : c), (int16_t)(along_x ? c : a), 0, (uint8_t)(0x80 + k), 8, (uint8_t)(1 + dir % 2 * 2 + (dir >= 2)) });
		}
		for (int j = 0; j < 4; ++j)
			for (int i = 0; i < 9; ++i) {
				int a = at + sign * 8 * j, c = s - 20 + 8 * i;
				add_extra(3, (CoordCell){ (int16_t)(along_x ? a : c), (int16_t)(along_x ? c : a), 0, (uint8_t)(0xF0 + k), 8, 0x11 });
			}
	}
}

/* The gaps' panels, and each one's trigger strip: the lane's floor cells
 * across the gap's first panel, where the engine's probe lands, 24 ahead
 * of MegaMan, when he presses A at the edge (BN6's own strip is the
 * mouth's row, its probe 8 ahead), taking the gap's near record (0x30 +
 * 2g). */
void nm_gaps_place(void) {
	ngap_panels = 0;
	gaps_shut = false;
	for (int g = 0; g < layer.ngaps; ++g) {
		const NetGap *p = &layer.gap[g];
		int A0, B0, A1, B1;
		nm_grid_to_panel(p->x, p->y, &A0, &B0);
		for (int k = 1; k <= p->len && ngap_panels < MAX_GAPS * 3; ++k) {
			int A, B;
			nm_grid_to_panel(p->x + dir_dx[p->dir] * k, p->y + dir_dy[p->dir] * k, &A, &B);
			gap_panels[ngap_panels++] = (__typeof__(gap_panels[0])){ A, B, g };
		}
		nm_grid_to_panel(p->x + dir_dx[p->dir], p->y + dir_dy[p->dir], &A1, &B1);
		int ux = A1 - A0, uy = B1 - B0;
		int ex = nm_place.ex + 16 + 32 * A0 + 16 * ux, ey = nm_place.ey + 16 + 32 * B0 + 16 * uy;   /* the near mouth */
		for (int cy = (nm_place.ey + 32 * B1) >> 3; cy < (nm_place.ey + 32 * B1 + 32) >> 3; ++cy)
			for (int cx = (nm_place.ex + 32 * A1) >> 3; cx < (nm_place.ex + 32 * A1 + 32) >> 3; ++cx) {
				int depth = (cx * 8 + 4 - ex) * ux + (cy * 8 + 4 - ey) * uy;
				if (depth > 0 && depth < 32 && netmap_floor_cell(cx, cy, 0))
					add_extra(3, (CoordCell){ (int16_t)(cx * 8), (int16_t)(cy * 8), 0, (uint8_t)(0x30 + 2 * g), 8, 0x11 });
			}
	}
}

/* The arrow lanes' trigger cells (section 3, issue #43), as BN6's ring a
 * lane a panel wide: across its first panel's near edge the start cells
 * (0x48 + BN6's way), across its last panel's far edge the end cells (0x4C
 * + the way), three to an edge between its corners, each centred on the
 * edge; the ride runs from one to the other. Their shape is the way's
 * (+X 0x13, +Y 0x15, -X 0x12, -Y 0x14), as every original's. */
void nm_lanes_place(void) {
	static const uint8_t shape[4] = { 0x13, 0x15, 0x12, 0x14 };
	for (int i = 0; i < layer.nlanes; ++i) {
		const NetLane *l = &layer.lane[i];
		int d = (l->dir + 1) & 3, ux = d == 0 ? 1 : d == 2 ? -1 : 0, uy = d == 1 ? 1 : d == 3 ? -1 : 0;
		for (int end = 0; end < 2; ++end) {
			int k = end ? l->len : 1, A, B;
			nm_grid_to_panel(l->x + dir_dx[l->dir] * k, l->y + dir_dy[l->dir] * k, &A, &B);
			int X0 = nm_place.ex + 32 * A, Y0 = nm_place.ey + 32 * B;
			/* (the near edge against the way, the far one along it) */
			int sign = end ? 1 : -1, X = ux ? X0 + 16 + 16 * ux * sign : 0, Y = uy ? Y0 + 16 + 16 * uy * sign : 0;
			for (int c = 0; c < 3; ++c)
				add_extra(3, (CoordCell){ (int16_t)(ux ? X - 4 : X0 + 4 + 8 * c), (int16_t)(uy ? Y - 4 : Y0 + 4 + 8 * c), 0,
					(uint8_t)((end ? 0x4C : 0x48) + d), 8, shape[d] });
		}
	}
}

/* the layer's floor at `level` in world panels: its own level and stairs */
bool netmap_floor_cell(int cx, int cy, int level) {
	/* the cell centre, relative to the panel edges */
	int X = cx * 8 + 4 - nm_place.ex, Y = cy * 8 + 4 - nm_place.ey;
	int A = nm_floordiv(X, 32), B = nm_floordiv(Y, 32);
	bool onx = (X & 31) == 0, ony = (Y & 31) == 0;
	if (!floor_level(A, B, level)) return false;
	if (onx && !floor_level(A - 1, B, level)) return false;
	if (ony && !floor_level(A, B - 1, level)) return false;
	if (onx && ony && !floor_level(A - 1, B - 1, level)) return false;
	return true;
}

bool netmap_stair_cell(int cx, int cy) {
	int A = nm_floordiv(cx * 8 + 4 - nm_place.ex, 32), B = nm_floordiv(cy * 8 + 4 - nm_place.ey, 32);
	return nm_kind_at(A, B) == K_STAIR;
}

/* Coordinate cells beyond the walls: raised floor heights and the stairs'
 * own ramps, walls and layer priorities, placed at their blocks. */
CoordExtra nm_extra;
static CoordCell extra_cells[4][4096];
static CoordCell over_cells[1024];   /* walls in place of the floor's own (props') */

static void add_extra(int s, CoordCell c) {
	if (nm_extra.n[s] < 4096) extra_cells[s][nm_extra.n[s]++] = c;
}

/* The row past each stair's top a step, as the originals' is: a cell from
 * the ramp's foot as high as the rise (z 0, height 32), where the raised
 * floor's own cells start at the rise (z 32, height 0). The game takes a
 * floor cell only within its heights, and coming up the ramp MegaMan is
 * under the rise until he has crossed that row: with the raised floor's
 * cells there, he fell to the ground and walked on under the room (issue
 * #22). */
static void stair_steps(const Learned *L) {
	for (int i = 0; i < nm_cur->nstairs; ++i) {
		const StairTemplate *t = &L->stairs[nm_cur->stairs[i].dir];
		if (!t->ok) continue;
		int X0, Y0;
		nm_stair_origin(&nm_cur->stairs[i], &X0, &Y0);
		for (int k = 0; k < t->nramp; ++k) {
			const CoordCell *r = &t->ramp[k];
			if ((r->type != 0x13 && r->type != 0x14) || r->height != t->rise) continue;
			/* (0x13 climbs towards +X, 0x14 towards -Y) */
			CoordCell step = { (int16_t)(r->x + X0 + (r->type == 0x13 ? 8 : 0)), (int16_t)(r->y + Y0 - (r->type == 0x14 ? 8 : 0)), r->z, 0, (uint8_t)t->rise, 0x11 };
			bool found = false;
			for (int c = 0; c < nm_extra.n[1]; ++c) {
				CoordCell *e = &extra_cells[1][c];
				if (e->type != 0x11 || e->x >> 3 != step.x >> 3 || e->y >> 3 != step.y >> 3) continue;
				*e = step;
				found = true;
			}
			if (!found) add_extra(1, step);
		}
	}
}

void nm_build_extra(const Learned *L) {
	memset(&nm_extra, 0, sizeof nm_extra);
	for (int s = 0; s < 4; ++s) nm_extra.cells[s] = extra_cells[s];
	/* the props' own walls: a counter's ring, whose back row the walls of
	 * its walled-off aisle would duplicate */
	nm_extra.over = over_cells;
	nm_props_place(L);
	for (int i = 0; i < MAX_PROPS; ++i) {
		if (!nm_prop_at[i].ok) continue;
		for (int k = 0; k < nm_prop_at[i].st->nwalls && nm_extra.nover < 1024; ++k) {
			CoordCell c = nm_prop_at[i].st->walls[k];
			c.x = (int16_t)(c.x + nm_prop_at[i].X);
			c.y = (int16_t)(c.y + nm_prop_at[i].Y);
			over_cells[nm_extra.nover++] = c;
		}
		/* (and its layer priorities, as a stair's are laid) */
		for (int k = 0; k < nm_prop_at[i].st->nprio; ++k) {
			CoordCell c = nm_prop_at[i].st->prio[k];
			c.x = (int16_t)(c.x + nm_prop_at[i].X);
			c.y = (int16_t)(c.y + nm_prop_at[i].Y);
			add_extra(2, c);
		}
	}
	for (int y = 0; y < nm_cur->gh; ++y)
		for (int x = 0; x < nm_cur->gw; ++x) {
			if (nm_kind(x, y) != K_RAISED) continue;
			int A, B;
			nm_grid_to_panel(x, y, &A, &B);
			int X0 = nm_place.ex + 32 * A, Y0 = nm_place.ey + 32 * B;
			/* the 8 x 8 cells whose centres lie on the panel */
			for (int cy = nm_floordiv(Y0 - 4 + 7, 8); cy * 8 + 4 < Y0 + 32; ++cy)
				for (int cx = nm_floordiv(X0 - 4 + 7, 8); cx * 8 + 4 < X0 + 32; ++cx) {
					CoordCell c = { (int16_t)(cx * 8), (int16_t)(cy * 8), (int8_t)nm_cur->rise, 0, 0, 0x11 };
					add_extra(1, c);
				}
		}
	for (int i = 0; i < nm_cur->nstairs; ++i) {
		const StairTemplate *t = &L->stairs[nm_cur->stairs[i].dir];
		if (!t->ok) continue;
		int X0, Y0;
		nm_stair_origin(&nm_cur->stairs[i], &X0, &Y0);
		const CoordCell *src[3] = { t->walls, t->ramp, t->prio };
		int n[3] = { t->nwalls, t->nramp, t->nprio };
		for (int s = 0; s < 3; ++s)
			for (int k = 0; k < n[s]; ++k) {
				CoordCell c = src[s][k];
				c.x = (int16_t)(c.x + X0);
				c.y = (int16_t)(c.y + Y0);
				add_extra(s, c);
			}
	}
	stair_steps(L);
}
