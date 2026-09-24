/* Raised rooms: a dead-end room reached by a straight corridor from the side
 * a stair can climb from is lifted one level, and the last two corridor
 * panels before it become a stair, two panels wide.
 *
 * The map draws a room raised by one panel's height where a flat room one
 * panel up the screen would be (grid x - 1, y - 1), so that place and its
 * neighbours must be free of other floor. Decisions use their own random
 * numbers, so the rest of the layer does not change with them. */
#include <stdlib.h>
#include <string.h>

#include "net.h"

#define RAISE_CHANCE 70   /* % of the rooms that could be raised */

static uint32_t rnd;
static int roll(int n) { rnd = rnd * 1103515245u + 12345u; return (int)((rnd >> 16) % (uint32_t)n); }

static bool floor_at(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH; }

static bool in_room(const Room *r, int x, int y) { return x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h; }

/* The single way into room r, as the first cell outside it on side `side`
 * (0 right: +x, 1 bottom: +y), or false when r has another way in. */
static bool single_entry(const Room *r, int side, int *ex, int *ey) {
	int n = 0;
	for (int y = r->y - 1; y <= r->y + r->h; ++y)
		for (int x = r->x - 1; x <= r->x + r->w; ++x) {
			if (in_room(r, x, y) || !floor_at(x, y)) continue;
			bool corner = (x < r->x || x >= r->x + r->w) && (y < r->y || y >= r->y + r->h);
			if (corner) continue;
			bool on_side = side == 0 ? x == r->x + r->w : y == r->y + r->h;
			if (!on_side) return false;
			if (!n++) { *ex = x; *ey = y; }
		}
	return n >= 1 && n <= 2;
}

/* The stair block (2 x 2 cells, top-left at sx, sy) and a landing beyond it,
 * along the corridor: along +x for side 0, +y for side 1. */
static bool fits_stair(const Room *r, int side, int ex, int ey, int *sx, int *sy) {
	if (side == 0) {
		*sx = ex;
		*sy = ey + 1 < r->y + r->h ? ey : ey - 1;
		if (*sy < r->y) return false;
		/* the corridor runs on past the stair */
		return floor_at(ex + 1, ey) && floor_at(ex + 2, ey);
	}
	*sy = ey;
	*sx = ex + 1 < r->x + r->w ? ex : ex - 1;
	if (*sx < r->x) return false;
	return floor_at(ex, ey + 1) && floor_at(ex, ey + 2);
}

static bool in_block(int x, int y, int sx, int sy, int w, int h) { return x >= sx && x < sx + w && y >= sy && y < sy + h; }

/* Nothing else within one cell of where the raised room is drawn. */
static bool clear_on_screen(const Room *r, int shift, int sx, int sy, int side) {
	int bw = side == 0 ? 3 : 2, bh = side == 0 ? 2 : 3;   /* stair and landing */
	for (int y = r->y - shift - 1; y <= r->y + r->h - shift; ++y)
		for (int x = r->x - shift - 1; x <= r->x + r->w - shift; ++x) {
			if (!floor_at(x, y) || in_room(r, x, y) || in_block(x, y, sx, sy, bw, bh)) continue;
			return false;
		}
	return true;
}

void layer_raise_rooms(uint32_t seed, unsigned dirs, int rise) {
	rnd = seed ^ 0x5EEDu;
	int shift = rise / 32;
	if (!dirs || shift < 1) return;
	for (int i = 1; i < layer.nrooms && layer.nstairs < MAX_STAIRS; ++i) {
		if (i == layer.exit_room) continue;
		const Room *r = &layer.rooms[i];
		for (int side = 0; side < 2; ++side) {
			/* side 0 climbs towards -x, side 1 towards -y */
			int dir = side == 0 ? STAIR_UP_NX : STAIR_UP_NY;
			int ex = 0, ey = 0, sx, sy;
			if (!(dirs & (1u << dir)) || !single_entry(r, side, &ex, &ey) || !fits_stair(r, side, ex, ey, &sx, &sy)) continue;
			if (!clear_on_screen(r, shift, sx, sy, side) || roll(100) >= RAISE_CHANCE) continue;
			for (int y = r->y; y < r->y + r->h; ++y)
				for (int x = r->x; x < r->x + r->w; ++x) layer.level[y][x] = 1;
			/* a stair two panels wide, and its landing */
			for (int dy = 0; dy < (side ? 3 : 2); ++dy)
				for (int dx = 0; dx < (side ? 2 : 3); ++dx) layer.cell[sy + dy][sx + dx] = C_PATH;
			layer.stair[layer.nstairs].x = sx;
			layer.stair[layer.nstairs].y = sy;
			layer.stair[layer.nstairs++].dir = dir;
			layer.rise = rise;
			break;
		}
	}
}
