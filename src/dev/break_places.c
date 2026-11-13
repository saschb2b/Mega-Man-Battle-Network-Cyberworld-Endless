/* Where a layer's breaks lie: each break in the last layer's picture
 * (netmap_last_breaks) by the kind of place its tile lies on, read from
 * the layer's cells as they stand after the legalizer (net.h). A room's
 * corner, a walkway's fork and the panel where the two meet are places an
 * area's maps may have no panel for; the count says which of them cost a
 * layer most. */
#include "break_places.h"

#include <string.h>

#include "net.h"
#include "netmap.h"

const char *const break_place_names[BP_PLACES] = { "room inside", "room edge", "room corner", "room notch", "walkway at room", "walkway run",
	"walkway bend", "walkway fork", "walkway end", "pad", "raised" };

static bool floor_at(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] != C_VOID; }

/* A walkway's: floor in no 2 x 2 block of floor. */
static bool walk_at(int x, int y) {
	if (!floor_at(x, y)) return false;
	for (int dy = -1; dy <= 0; ++dy)
		for (int dx = -1; dx <= 0; ++dx)
			if (floor_at(x + dx, y + dy) && floor_at(x + dx + 1, y + dy) && floor_at(x + dx, y + dy + 1) && floor_at(x + dx + 1, y + dy + 1)) return false;
	return true;
}

static bool pad_at(int x, int y) {
	for (int r = 0; r < layer.nrooms; ++r) {
		const Room *m = &layer.rooms[r];
		if (m->kind == ROOM_PAD && x >= m->x && y >= m->y && x < m->x + m->w && y < m->y + m->h) return true;
	}
	return false;
}

static bool stair_at(int x, int y) {
	for (int i = 0; i < layer.nstairs; ++i)
		if (x >= layer.stair[i].x && x < layer.stair[i].x + 2 && y >= layer.stair[i].y && y < layer.stair[i].y + 2) return true;
	return false;
}

static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

/* The place of floor cell (x, y). */
static int place_of(int x, int y) {
	if (layer.level[y][x] || stair_at(x, y)) return BP_RAISED;
	if (pad_at(x, y)) return BP_PAD;
	bool walk = walk_at(x, y), side[4];
	int n = 0;
	for (int d = 0; d < 4; ++d) {
		side[d] = floor_at(x + d4[d][0], y + d4[d][1]);
		n += side[d];
		/* (a walkway's panel beside a room's, or a room's beside a walkway's) */
		if (side[d] && walk_at(x + d4[d][0], y + d4[d][1]) != walk) return BP_JOIN;
	}
	if (walk) return n <= 1 ? BP_WALK_END : n >= 3 ? BP_WALK_FORK : side[0] == side[2] ? BP_WALK_RUN : BP_WALK_BEND;
	if (n == 4) {
		for (int dy = -1; dy <= 1; dy += 2)
			for (int dx = -1; dx <= 1; dx += 2)
				if (!floor_at(x + dx, y + dy)) return BP_ROOM_NOTCH;
		return BP_ROOM_IN;
	}
	return n == 3 || side[0] == side[2] ? BP_ROOM_EDGE : BP_ROOM_CORNER;
}

/* The floor cell tile (tx, ty) counts for: the one its middle lies in, or
 * off the floor the first one beside it; false none. */
static bool cell_of(int tw, int th, int tx, int ty, int *x, int *y) {
	int u = tx * 8 + 4 - tw * 4, v = 2 * (ty * 8 + 4 - th * 4);
	if (!netmap_panel((u - v) / 2, (u + v) / 2, x, y)) return false;
	if (floor_at(*x, *y)) return true;
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
			if (floor_at(*x + dx, *y + dy)) { *x += dx; *y += dy; return true; }
	return false;
}

void break_places(BreakPlaces *out) {
	int tw, th;
	const uint8_t *breaks = netmap_last_breaks();
	memset(out, 0, sizeof *out);
	netmap_last_tiles(&tw, &th);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (floor_at(x, y)) out->cells[place_of(x, y)]++;
	for (int i = 0; breaks && i < tw * th; ++i) {
		int x, y, n = (breaks[i] & 1) + (breaks[i] >> 1 & 1);
		if (n && cell_of(tw, th, i % tw, i / tw, &x, &y)) out->breaks[place_of(x, y)] += n;
	}
}
