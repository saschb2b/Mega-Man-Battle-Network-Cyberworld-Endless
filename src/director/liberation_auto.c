/* The test autopilot on a liberation's board (liberation.h, emu/autopilot.c):
 * a scripted run to the Darkloid. Each phase it liberates the dark panel
 * that brings it nearest the next target (the nearest DarkHole standing,
 * then the Darkloid; a barrier's key where a barrier walls them off), from the clean panel nearest it beside that one,
 * walking there as a player walks MegaMan (the pad, BN6's walk), turning
 * to face it and pressing A; fights a guardian or the Darkloid it faces;
 * passes below two fifths of its HP. Its presses a frame apart. */
#include "liberation.h"

#include <string.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "emu.h"
#include "netmap.h"
#include "liberation_state.h"

static const int dirs[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
static const uint32_t pads[4] = { KEY_DOWN | KEY_RIGHT, KEY_DOWN | KEY_LEFT, KEY_UP | KEY_LEFT, KEY_UP | KEY_RIGHT };

/* The panels' cost to the target: the dark panels a walk from each
 * liberates on its way there (lib_cost_map: barriers shut; the keys of
 * the barriers before the sealed one where nothing else is reached) */
static int16_t cost[MAP_H][MAP_W];

static void cost_from_target(void) {
	lib_cost_map(&LB.b, cost, false);
	if (cost[LB.my][LB.mx] >= 0x7F00) lib_cost_map(&LB.b, cost, true);
}

/* Where to stand and which way to face: the clean panel the walk reaches
 * (from MegaMan's, over clean panels) beside the best panel to take;
 * the first step's way there into *step (-1 there already) */
static bool plan(int *face, int *step) {
	static int16_t walk[MAP_H][MAP_W], first[MAP_H][MAP_W];
	static uint16_t q[MAP_W * MAP_H];
	memset(walk, 0xFF, sizeof walk);
	int head = 0, tail = 0, best = 1 << 30, bx = -1, by = -1, bf = 0;
	walk[LB.my][LB.mx] = 0;
	first[LB.my][LB.mx] = -1;
	q[tail++] = (uint16_t)(LB.my * MAP_W + LB.mx);
	while (head < tail) {
		int c = q[head++], x = c % MAP_W, y = c / MAP_W;
		for (int d = 0; d < 4; ++d) {
			int nx = x + dirs[d][0], ny = y + dirs[d][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H) continue;
			int u = lib_unit_at(nx, ny);
			bool unit = u >= 0, take = (u > 0) || (u == 0 && !lib_holes_left(&LB.b)) || (u < 0 && lib_liberable(&LB.b, nx, ny));
			int score = take ? cost[ny][nx] * 64 + walk[y][x] : 1 << 30;
			if (score < best) { best = score; bx = x; by = y; bf = d; }
			if (walk[ny][nx] >= 0 || !lib_walkable(&LB.b, nx, ny) || unit) continue;
			walk[ny][nx] = (int16_t)(walk[y][x] + 1);
			first[ny][nx] = first[y][x] < 0 ? (int16_t)d : first[y][x];
			q[tail++] = (uint16_t)(ny * MAP_W + nx);
		}
	}
	if (bx < 0) return false;
	*face = bf;
	*step = first[by][bx];
	return true;
}

/* The pad's way toward world (x, y) from MegaMan's place (as the
 * autopilot's own walk picks it), 0 within `near` units */
static uint32_t toward(int x, int y, int near) {
	static const struct { int x, y; uint32_t k; } way[8] = {
		{ 7, -7, KEY_UP }, { 10, 0, KEY_UP | KEY_RIGHT }, { 7, 7, KEY_RIGHT }, { 0, 10, KEY_DOWN | KEY_RIGHT },
		{ -7, 7, KEY_DOWN }, { -10, 0, KEY_DOWN | KEY_LEFT }, { -7, -7, KEY_LEFT }, { 0, -10, KEY_UP | KEY_LEFT },
	};
	int dx = x - bn6_player_x(), dy = y - bn6_player_y();
	if (dx * dx + dy * dy <= near * near) return 0;
	long best = -1000000;
	uint32_t k = 0;
	for (int i = 0; i < 8; ++i) {
		long d = (long)way[i].x * dx + (long)way[i].y * dy;
		if (d > best) { best = d; k = way[i].k; }
	}
	return k;
}

uint32_t lib_auto_keys(int frame) {
	if (LB.step == LIB_STEP_MENU) {
		if (frame & 1) return 0;   /* (a press, then a frame up) */
		int want = LB.can[LIB_CMD_LIBERATE] && emu_read16(BN6_NAVI_HP) * 5 > emu_read16(BN6_NAVI_MAX_HP) * 2 ? LIB_CMD_LIBERATE : LIB_CMD_PASS;
		return LB.menu == want ? KEY_A : KEY_DOWN;
	}
	if (LB.step != LIB_STEP_MOVE) return 0;
	cost_from_target();
	int face, step;
	if (!plan(&face, &step)) return 0;
	int wx, wy;
	/* (on to the next panel of the way, else to the middle of the stand) */
	netmap_world(LB.mx + (step >= 0 ? dirs[step][0] : 0), LB.my + (step >= 0 ? dirs[step][1] : 0), &wx, &wy);
	uint32_t k = toward(wx, wy, step >= 0 ? 2 : 6);
	if (k) return k;
	/* (there: a nudge toward the panel to face it, then A) */
	if (LB.tx != LB.mx + dirs[face][0] || LB.ty != LB.my + dirs[face][1]) return pads[face];
	return frame & 1 ? 0 : KEY_A;
}
