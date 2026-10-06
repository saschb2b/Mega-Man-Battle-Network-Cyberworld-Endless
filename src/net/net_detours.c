/* A layer's detours (docs/LEVEL_DESIGN.md): the branches off the way
 * measured, what each ends in, blue data where a long one ends, and the
 * empty ones given a reason to walk them. */
#include "net_detours.h"

#include <string.h>

#include "net_gen.h"

/* ---- Detours (docs/LEVEL_DESIGN.md, Set pieces) ----
 * How far each floor cell lies off the way, in panels walked (0 on it, -1
 * where no walk reaches), and the way's cell its walk leaves from: the
 * cells of a spur, a room off the way and what lies past them share it, a
 * branch. The originals' blue data lie at a branch's far end, and what a
 * detour holds follows its length. */
int16_t ng_detour[MAP_H][MAP_W];
static int16_t branch[MAP_H][MAP_W];
#define DETOUR_EMPTY 5   /* (a detour this long is never left empty: fill_empty_detours) */

/* Whether (x, y) lies on the open floor of the layer's signature, out of
 * every other room: a plaza's corners and a ring road's far side, which the
 * way crosses or runs round, are seen from it and no ends (issue #98); the
 * pad in a ring's middle, a room of its own, is one. */
static bool in_room(int r, int x, int y) {
	const Room *m = &layer.rooms[r];
	return x >= m->x && y >= m->y && x < m->x + m->w && y < m->y + m->h;
}

bool layer_detour_open(int x, int y) {
	if (layer.sig_room < 0 || !in_room(layer.sig_room, x, y)) return false;
	for (int i = 0; i < layer.nrooms; ++i)
		if (i != layer.sig_room && in_room(i, x, y)) return false;
	return true;
}

/* (the guardian's arena is no detour: the way ends at him, in its middle) */
bool ng_in_arena(int x, int y) {
	if (layer.arena < 0) return false;
	const Room *a = &layer.rooms[layer.arena];
	return x >= a->x && x < a->x + a->w && y >= a->y && y < a->y + a->h;
}

void ng_measure_detours(void) {
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	static const int d4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	int h = 0, t = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			ng_detour[y][x] = branch[y][x] = -1;
			if (ng_way_band[y][x] != 2 || layer.cell[y][x] != C_PATH || ng_in_arena(x, y)) continue;
			ng_detour[y][x] = 0;
			branch[y][x] = (int16_t)(y * MAP_W + x);
			qx[t] = (int16_t)x; qy[t++] = (int16_t)y;
		}
	while (h < t) {
		int x = qx[h], y = qy[h++];
		for (int k = 0; k < 4; ++k) {
			int nx = x + d4[k][0], ny = y + d4[k][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || layer.cell[ny][nx] != C_PATH || ng_detour[ny][nx] >= 0 || ng_in_arena(nx, ny)) continue;
			ng_detour[ny][nx] = (int16_t)(ng_detour[y][x] + 1);
			branch[ny][nx] = branch[y][x];
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
}

int layer_detour(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H ? ng_detour[y][x] : -1; }
int layer_branch(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H ? branch[y][x] : -1; }

/* A pad's middle where (x, y) lies on a pad: the originals set a pad's data
 * there; else (x, y). */
void ng_pad_middle(int *x, int *y) {
	for (int i = 0; i < layer.nrooms; ++i) {
		const Room *r = &layer.rooms[i];
		if (r->kind != ROOM_PAD || *x < r->x || *x >= r->x + r->w || *y < r->y || *y >= r->y + r->h) continue;
		int mx = r->x + r->w / 2, my = r->y + r->h / 2;
		if (ng_cell_free(mx, my) && !ng_cuts_way(mx, my)) { *x = mx; *y = my; }
		return;
	}
}

bool ng_detour_end(int min, uint8_t *taken, DetourEnd *out) {
	enum { MAX_D = 160 };
	static int16_t cells[MAP_W * MAP_H];
	int count[MAX_D + 1] = { 0 }, start[MAX_D + 1], total = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (ng_detour[y][x] >= min) { ++count[ng_detour[y][x] < MAX_D ? ng_detour[y][x] : MAX_D]; ++total; }
	/* (by distance, farthest first; in a distance, in the grid's order) */
	for (int d = MAX_D, at = 0; d >= 0; --d) { start[d] = at; at += count[d]; }
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (ng_detour[y][x] >= min) cells[start[ng_detour[y][x] < MAX_D ? ng_detour[y][x] : MAX_D]++] = (int16_t)(y * MAP_W + x);
	for (int i = 0; i < total; ++i) {
		int x = cells[i] % MAP_W, y = cells[i] / MAP_W, b = branch[y][x];
		if (taken[b] || !ng_cell_free(x, y) || ng_behind_gap(x, y) || ng_near_talker(x, y) || ng_cuts_way(x, y)) continue;
		taken[b] = 1;
		out->d = ng_detour[y][x];
		ng_pad_middle(&x, &y);
		out->x = x; out->y = y;
		return true;
	}
	return false;
}

/* The branch (x, y) lies on, where it is off the way; -1 on the way or off
 * the floor. */
static int off_way_branch(int x, int y) { return layer_detour(x, y) > 0 ? layer_branch(x, y) : -1; }

/* A green data lying loose: on the way, or on a detour shorter than
 * DETOUR_EMPTY (`far`: each branch's longest), and no set piece's; the
 * nearest the way; -1 none. */
static int loose_green(const int16_t *far) {
	int best = -1, best_d = 1 << 30;
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		if (o->type != OBJ_MYSTERY || o->param != 0 || o->prize) continue;
		int x = (int)o->x, y = (int)o->y, d = layer_detour(x, y), b = off_way_branch(x, y);
		if (d < 0 || (b >= 0 && far[b] >= DETOUR_EMPTY) || d >= best_d) continue;
		best = i;
		best_d = d;
	}
	return best;
}

/* A branch's cells and how wide it runs: wide[b] where two by two of its
 * panels are floor (a band that reads as the way, not a spur); open[b]
 * where its far end lies on the signature's open floor (no end) */
static void branch_shape(int16_t *far, uint8_t *wide, uint8_t *open) {
	int b;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if ((b = off_way_branch(x, y)) < 0) continue;
			if (ng_detour[y][x] > far[b]) { far[b] = ng_detour[y][x]; open[b] = layer_detour_open(x, y); }
			if (off_way_branch(x + 1, y) == b && off_way_branch(x, y + 1) == b && off_way_branch(x + 1, y + 1) == b) wide[b] = 1;
		}
}

/* Once all stands: a detour DETOUR_EMPTY panels or more off the way that
 * holds nothing, no object and no set piece's stand, takes a loose green
 * data at its end, the farthest first (a playtester walked a two-wide band
 * to an empty end, session 59; half the layers held one, 321 of 1589
 * such detours); moved, not added. A wide one still empty takes a new
 * one where the map has room: a band reads as the way, a spur to nothing
 * as BN6's own. The signature's open floor is no end (issue #98: a ring
 * road's far side, a plaza's corners, seen from the way). */
void ng_fill_empty_detours(void) {
	static uint8_t holds[MAP_W * MAP_H], wide[MAP_W * MAP_H], open[MAP_W * MAP_H];
	static int16_t far[MAP_W * MAP_H], keep_detour[MAP_H][MAP_W], keep_branch[MAP_H][MAP_W];
	memset(holds, 0, sizeof holds);
	memset(far, 0, sizeof far);
	memset(wide, 0, sizeof wide);
	memset(open, 0, sizeof open);
	/* (the layer's detours as its data were placed stay its own: an arrow
	 * lane, carved since, is measured from them) */
	memcpy(keep_detour, ng_detour, sizeof ng_detour);
	memcpy(keep_branch, branch, sizeof branch);
	ng_measure_detours();
	branch_shape(far, wide, open);
	int b;
	for (b = 0; b < MAP_W * MAP_H; ++b) holds[b] = open[b];
	for (int i = 0; i < layer.nobj; ++i)
		if ((b = off_way_branch((int)layer.obj[i].x, (int)layer.obj[i].y)) >= 0) holds[b] = 1;
	for (int i = 0; i < layer.ngaps; ++i) if ((b = off_way_branch(layer.gap[i].x, layer.gap[i].y)) >= 0) holds[b] = 1;
	for (int i = 0; i < layer.nblocks; ++i) if ((b = off_way_branch(layer.block[i].x, layer.block[i].y)) >= 0) holds[b] = 1;
	for (int i = 0; i < layer.npaths; ++i) if ((b = off_way_branch(layer.path[i].x, layer.path[i].y)) >= 0) holds[b] = 1;
	for (int i = 0; i < layer.nlanes; ++i) if ((b = off_way_branch(layer.lane[i].x, layer.lane[i].y)) >= 0) holds[b] = 1;
	for (int i = 0; i < 2 * (layer.nteleports > 0); ++i)
		if ((b = off_way_branch(layer.teleport_x[i], layer.teleport_y[i])) >= 0) holds[b] = 1;
	DetourEnd end;
	int g;
	while ((g = loose_green(far)) >= 0 && ng_detour_end(DETOUR_EMPTY, holds, &end)) {
		layer.obj[g].x = (float)end.x + 0.5f;
		layer.obj[g].y = (float)end.y + 0.5f;
	}
	for (b = 0; b < MAP_W * MAP_H; ++b) holds[b] |= !wide[b];
	NetObj *o;
	while (ng_npcs_for(1) && ng_detour_end(DETOUR_EMPTY, holds, &end) && (o = ng_add_obj(OBJ_MYSTERY, end.x, end.y))) o->param = 0;
	memcpy(ng_detour, keep_detour, sizeof ng_detour);
	memcpy(branch, keep_branch, sizeof branch);
}
