/* The test autopilot on a liberation's board (liberation.h, emu/autopilot.c):
 * a scripted run to the Darkloid. Each phase it liberates the dark panel
 * that brings it nearest the next target (the nearest DarkHole standing,
 * then the Darkloid), from the clean panel nearest it beside that one;
 * fights a guardian or the Darkloid it faces; passes below two fifths of
 * its HP. Its presses a frame apart, as a player's. */
#include "liberation.h"

#include <string.h>

#include "bn6.h"
#include "emu.h"
#include "liberation_state.h"

static const int dirs[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
static const uint32_t pads[4] = { KEY_DOWN | KEY_RIGHT, KEY_DOWN | KEY_LEFT, KEY_UP | KEY_LEFT, KEY_UP | KEY_RIGHT };

/* The panels' cost to the target: the dark panels a walk from each
 * liberates on its way there (0-1) */
static int16_t cost[MAP_H][MAP_W];

static bool dark_kind(int k) { return k == LIB_DARK || k == LIB_ITEM || k == LIB_RING || k == LIB_HOLE; }

static void cost_from_target(void) {
	static uint16_t dq[MAP_W * MAP_H * 2];
	memset(cost, 0x7F, sizeof cost);
	int head = MAP_W * MAP_H, tail = head;
	bool holes = lib_holes_left(&LB.b) > 0;
	for (int h = 0; h < LB.b.nholes; ++h) {
		int x = LB.b.hole_x[h], y = LB.b.hole_y[h];
		if (holes && LB.b.kind[y][x] == LIB_HOLE) { cost[y][x] = 0; dq[tail++] = (uint16_t)(y * MAP_W + x); }
	}
	if (!holes) { cost[LB.b.boss_y][LB.b.boss_x] = 0; dq[tail++] = (uint16_t)(LB.b.boss_y * MAP_W + LB.b.boss_x); }
	while (head < tail) {
		int c = dq[head++], x = c % MAP_W, y = c / MAP_W;
		for (int d = 0; d < 4; ++d) {
			int nx = x + dirs[d][0], ny = y + dirs[d][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || LB.b.kind[ny][nx] == LIB_VOID) continue;
			int w = dark_kind(LB.b.kind[y][x]) ? 1 : 0, nc = cost[y][x] + w;
			if (nc >= cost[ny][nx]) continue;
			cost[ny][nx] = (int16_t)nc;
			if (w) dq[tail++] = (uint16_t)(ny * MAP_W + nx);
			else dq[--head] = (uint16_t)(ny * MAP_W + nx);
		}
	}
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

uint32_t lib_auto_keys(int frame) {
	if (frame & 1) return 0;   /* (a press, then a frame up) */
	if (LB.step == LIB_STEP_MENU) {
		int want = LB.can[LIB_CMD_LIBERATE] && emu_read16(BN6_NAVI_HP) * 5 > emu_read16(BN6_NAVI_MAX_HP) * 2 ? LIB_CMD_LIBERATE : LIB_CMD_PASS;
		return LB.menu == want ? KEY_A : KEY_DOWN;
	}
	if (LB.step != LIB_STEP_MOVE || LB.walk_x >= 0) return 0;
	cost_from_target();
	int face, step;
	if (!plan(&face, &step)) return KEY_A;
	if (step >= 0) return pads[step];
	if (LB.face != face) return pads[face];
	return KEY_A;
}
