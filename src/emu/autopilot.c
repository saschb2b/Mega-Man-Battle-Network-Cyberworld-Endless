/* The autopilot finds a path of floor panels from MegaMan's panel to the
 * exit (breadth first on the layer grid) and holds the pad toward the next
 * panel's centre. In battles and text it taps A and B. Testing only. */
#include "autopilot.h"

#include <stdio.h>
#include <stdlib.h>

#include "bn6.h"
#include "director.h"
#include "emu.h"
#include "net.h"
#include "netmap.h"

bool autopilot_on(void) { return getenv("CYBERWORLD_AUTOPILOT") != NULL; }

static int next_panel(int sx, int sy, int tx, int ty, int *nx, int *ny) {
	static int16_t prev[MAP_H][MAP_W];
	static int16_t q[MAP_W * MAP_H];
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) prev[y][x] = -1;
	int h = 0, t = 0;
	q[t++] = (int16_t)(sy * MAP_W + sx);
	prev[sy][sx] = (int16_t)(sy * MAP_W + sx);
	while (h < t) {
		int c = q[h++], x = c % MAP_W, y = c / MAP_W;
		if (x == tx && y == ty) break;
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int ax = x + d[k][0], ay = y + d[k][1];
			if (ax < 0 || ay < 0 || ax >= MAP_W || ay >= MAP_H || prev[ay][ax] >= 0 || layer.cell[ay][ax] != C_PATH) continue;
			prev[ay][ax] = (int16_t)c;
			q[t++] = (int16_t)(ay * MAP_W + ax);
		}
	}
	if (prev[ty][tx] < 0) return 0;
	int c = ty * MAP_W + tx;
	while (prev[c / MAP_W][c % MAP_W] != sy * MAP_W + sx && c != sy * MAP_W + sx) c = prev[c / MAP_W][c % MAP_W];
	*nx = c % MAP_W;
	*ny = c / MAP_W;
	return 1;
}

uint32_t autopilot_keys(void) {
	static uint32_t frame;
	++frame;
	int mode = emu_read8(emu_read32(BN6_TOOLKIT));
	if (mode != BN6_MODE_GAME || emu_read8(BN6_GAMESTATE) != BN6_SUB_MAP) {
		/* battles and screens: a 480-frame rhythm of picking chips, OK and the buster */
		uint32_t t = frame % 480;
		if (t < 48) return (t % 12) < 4 ? KEY_A : 0;          /* pick chips */
		if (t < 60) return t < 52 ? KEY_START : 0;            /* to OK */
		if (t < 72) return t < 64 ? KEY_A : 0;                /* send */
		if (t % 40 < 6) return KEY_A;                          /* use a chip / advance text */
		return (t % 12) < 4 ? KEY_B : 0;                       /* buster */
	}
	if (emu_read8(BN6_CHATBOX)) return (frame / 4) & 1 ? KEY_A : 0;
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	int cx, cy, ex, ey, nx, ny;
	if (!netmap_panel(px, py, &cx, &cy) || !director_exit_panel(&ex, &ey)) return 0;
	if (cx < 0 || cy < 0 || cx >= MAP_W || cy >= MAP_H || !next_panel(cx, cy, ex, ey, &nx, &ny)) { nx = ex; ny = ey; }
	int wx, wy;
	netmap_world(nx, ny, &wx, &wy);
	/* the pad direction whose world motion best follows the path (UP moves
	 * +X -Y, RIGHT +X +Y, DOWN -X +Y, LEFT -X -Y; diagonals one axis) */
	static const struct { int x, y; uint32_t k; } dirs[8] = {
		{ 7, -7, KEY_UP }, { 10, 0, KEY_UP | KEY_RIGHT }, { 7, 7, KEY_RIGHT }, { 0, 10, KEY_DOWN | KEY_RIGHT },
		{ -7, 7, KEY_DOWN }, { -10, 0, KEY_DOWN | KEY_LEFT }, { -7, -7, KEY_LEFT }, { 0, -10, KEY_UP | KEY_LEFT },
	};
	int dx = wx - px, dy = wy - py;
	if (abs(dx) + abs(dy) < 3) return 0;
	uint32_t k = 0;
	long best = -1000000;
	for (int i = 0; i < 8; ++i) {
		long d = (long)dirs[i].x * dx + (long)dirs[i].y * dy;
		if (d > best) { best = d; k = dirs[i].k; }
	}
	return k;
}
