/* The autopilot finds a path of floor panels from MegaMan's panel to the
 * exit (breadth first on the layer grid) and holds the pad toward the next
 * panel's centre. In battles and text it taps A and B. Testing only. */
#include "autopilot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "emu.h"
#include "net.h"
#include "netmap.h"

bool autopilot_on(void) { return getenv("CYBERWORLD_AUTOPILOT") != NULL; }

/* a solid object (a talker) stands on the panel */
static bool blocked(int x, int y) {
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].solid && (int)layer.obj[i].x == x && (int)layer.obj[i].y == y) return true;
	return false;
}

static int search(int sx, int sy, int tx, int ty, bool avoid, int *nx, int *ny) {
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
			if (ax < 0 || ay < 0 || ax >= MAP_W || ay >= MAP_H || prev[ay][ax] >= 0 || layer.cell[ay][ax] != C_PATH || (avoid && blocked(ax, ay) && (ax != tx || ay != ty))) continue;
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

/* The next panel towards (tx, ty): around talkers when possible, else past
 * them (MegaMan fits beside one on a panel). */
static int next_panel(int sx, int sy, int tx, int ty, int *nx, int *ny) {
	return search(sx, sy, tx, ty, true, nx, ny) || search(sx, sy, tx, ty, false, nx, ny);
}

/* CYBERWORLD_AUTOPILOT=weak: enemies keep 1 HP, so every battle is won and
 * what follows a win (a guardian's reward, the exit opening) can be tested. */
#define T1_OBJECTS BN6_T1_OBJECTS
#define T1_SIZE    BN6_T1_SIZE
#define T1_COUNT   BN6_T1_COUNT

/* A battle object that is in play: flag bit 0 of its header, HP left. */
static bool alive(uint32_t o) { return (emu_read8(o) & 1) && emu_read16(o + 0x24) > 0; }

/* Up or down to MegaMan's row towards the nearest enemy's, 0 when aligned
 * (PanelY at +0x13, Alliance at +0x16). */
static uint32_t toward_enemy_row(void) {
	int mine = -1, theirs = -1;
	for (uint32_t i = 0; i < T1_COUNT; ++i) {
		uint32_t o = T1_OBJECTS + i * T1_SIZE;
		if (!alive(o)) continue;
		int row = emu_read8(o + 0x13);
		if (emu_read8(o + 0x16) == 0) { if (mine < 0) mine = row; }
		else if (theirs < 0 || abs(row - mine) < abs(theirs - mine)) theirs = row;
	}
	if (mine < 0 || theirs < 0 || mine == theirs) return 0;
	return theirs < mine ? KEY_UP : KEY_DOWN;
}

/* Enemies at 1 HP, MegaMan topped up when low (HP +0x24, MaxHP +0x26, Alliance
 * +0x16), and enemies it has not hit after WEAK_FRAMES at none: what
 * follows a battle gets tested even where the autopilot fights badly
 * (planes out of its row, StarFish under water, Nightmares). */
#define WEAK_FRAMES 1200

static uint32_t battle_frames;   /* off the map in a row */

static void weaken_enemies(bool finish) {
	for (uint32_t i = 0; i < T1_COUNT; ++i) {
		uint32_t o = T1_OBJECTS + i * T1_SIZE;
		if (!(emu_read8(o) & 1)) continue;
		int alliance = emu_read8(o + 0x16), hp = emu_read16(o + 0x24), max = emu_read16(o + 0x26);
		uint8_t v[2] = { 1, 0 };
		if (alliance == 0 && hp > 0 && hp < 60) {
			int full = max > hp ? max : 100;
			v[0] = (uint8_t)full; v[1] = (uint8_t)(full >> 8);
			emu_write(o + 0x24, v, 2);
		}
		if (alliance == 1 && finish) { v[0] = 0; emu_write(o + 0x24, v, 2); }
		else if (alliance == 1 && hp > 1) emu_write(o + 0x24, v, 2);
	}
}

uint32_t autopilot_keys(void) {
	static uint32_t frame;
	++frame;
	int mode = emu_read8(emu_read32(BN6_TOOLKIT));
	if (mode != BN6_MODE_GAME || emu_read8(BN6_GAMESTATE) != BN6_SUB_MAP) {
		const char *how = getenv("CYBERWORLD_AUTOPILOT");
		if (how && !strcmp(how, "weak")) weaken_enemies(++battle_frames > WEAK_FRAMES);
		/* battles and screens: a 480-frame rhythm of picking chips, OK and the buster */
		uint32_t t = frame % 480;
		if (t < 48) return (t % 12) < 4 ? KEY_A : 0;          /* pick chips */
		if (t < 60) return t < 52 ? KEY_START : 0;            /* to OK */
		if (t < 72) return t < 64 ? KEY_A : 0;                /* send */
		if (t % 40 < 6) return KEY_A;                          /* use a chip / advance text */
		uint32_t row = toward_enemy_row();                     /* line up, then the buster */
		if (row) return (t % 8) < 2 ? row : 0;
		return (t % 12) < 4 ? KEY_B : 0;
	}
	battle_frames = 0;
	if (emu_read8(BN6_CHATBOX)) return (frame / 4) & 1 ? KEY_A : 0;
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	int cx, cy, ex, ey, nx, ny;
	bool talk;
	if (!netmap_panel(px, py, &cx, &cy) || !director_goal_panel(&ex, &ey, &talk)) return 0;
	if (cx < 0 || cy < 0 || cx >= MAP_W || cy >= MAP_H || !next_panel(cx, cy, ex, ey, &nx, &ny)) { nx = ex; ny = ey; }
	int wx, wy;
	netmap_world(nx, ny, &wx, &wy);
	/* the pad direction whose world motion best follows the path (UP moves
	 * +X -Y, RIGHT +X +Y, DOWN -X +Y, LEFT -X -Y; diagonals one axis) */
	static const struct { int x, y; uint32_t k; } dirs[8] = {
		{ 7, -7, KEY_UP }, { 10, 0, KEY_UP | KEY_RIGHT }, { 7, 7, KEY_RIGHT }, { 0, 10, KEY_DOWN | KEY_RIGHT },
		{ -7, 7, KEY_DOWN }, { -10, 0, KEY_DOWN | KEY_LEFT }, { -7, -7, KEY_LEFT }, { 0, -10, KEY_UP | KEY_LEFT },
	};
	/* beside the guardian: face him and talk (A also answers Yes) */
	if (talk) {
		int gx, gy;
		netmap_world(ex, ey, &gx, &gy);
		if (abs(gx - px) + abs(gy - py) < 40) {
			if (frame % 8 < 2) return KEY_A;
			wx = gx; wy = gy;   /* walk into him: MegaMan faces him */
		}
	}
	/* stuck on something (a Mystery Data, an NPC, a corner): take it, then
	 * try each direction in turn */
	static int last_x, last_y, still, unstick, tries;
	still = px == last_x && py == last_y ? still + 1 : 0;
	last_x = px; last_y = py;
	if (still > 45) { still = 0; unstick = 30; ++tries; }
	if (unstick > 0) {
		--unstick;
		if (unstick > 24) return unstick & 1 ? KEY_A : 0;
		return dirs[tries % 8].k;
	}
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
