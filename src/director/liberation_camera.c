/* The camera over a liberation's board as BN5's moves it (liberation.h;
 * docs/game/liberation/bn5/look-and-motion.md): to the unit about to
 * act before it acts and the banners, and back to MegaMan. BN6's own
 * camera does it: while the board holds it, its mode is 0, so it no longer
 * copies MegaMan's place, and its place is the one written here each frame;
 * let go, it follows him again as it did. MegaMan walks as BN6 walks him
 * throughout: the camera moves only while he stands. */
#include "liberation.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "debug.h"
#include "director_state.h"
#include "emu.h"
#include "liberation_state.h"
#include "netmap.h"

#define PAN_MAX  20   /* pixels a frame at most, BN5's (its pans measured on its BG1 scroll: about 20, then a quarter of */
#define PAN_PART 4    /* ... what is left each frame) */

static struct {
	bool held;
	uint8_t mode;          /* the camera's own, put back as it is let go */
	uint32_t follow;
	int32_t x, y;          /* its place, 16.16 world */
} C;

static uint32_t camera(void) { return emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CAMERA); }

static bool hold(void) {
	uint32_t cam = camera();
	if (!cam || !on_map()) return false;
	if (!C.held) {
		C.mode = emu_read8(cam + BN6_CAMERA_MODE);
		C.follow = emu_read32(cam + BN6_CAMERA_FOLLOW);
		C.x = (int32_t)emu_read32(cam + BN6_CAMERA_X);
		C.y = (int32_t)emu_read32(cam + BN6_CAMERA_Y);
		C.held = true;
		emu_write8(cam + BN6_CAMERA_MODE, 0);
		if (emu_debug_on()) fprintf(stderr, "liberation camera: held (mode %d, following %08x; MegaMan's place %08x)\n", C.mode, C.follow, BN6_PLAYER_X);
	}
	return true;
}

static int isqrt(int64_t v) {
	int64_t r = 0;
	while ((r + 1) * (r + 1) <= v) ++r;
	return (int)r;
}

/* One frame of the pan to world (wx, wy): true where it is there */
static bool step_to(int wx, int wy) {
	int64_t dx = ((int64_t)wx << 16) - C.x, dy = ((int64_t)wy << 16) - C.y;
	/* (on the screen a world step is dx + dy across and (dy - dx) / 2 down) */
	int64_t sx = (dx + dy) >> 16, sy = ((dy - dx) / 2) >> 16;
	int len = isqrt(sx * sx + sy * sy);
	if (len <= 1) { C.x = wx << 16; C.y = wy << 16; }
	else {
		int step = len / PAN_PART;
		if (step > PAN_MAX) step = PAN_MAX;
		if (step < 1) step = 1;
		C.x += (int32_t)(dx * step / len);
		C.y += (int32_t)(dy * step / len);
	}
	uint32_t cam = camera();
	emu_write32(cam + BN6_CAMERA_X, (uint32_t)C.x);
	emu_write32(cam + BN6_CAMERA_Y, (uint32_t)C.y);
	return len <= 1;
}

bool lib_camera_at(int wx, int wy) {
	if (!lib_look_on() || !hold()) return true;
	return step_to(wx, wy);
}

bool lib_camera_home(void) {
	if (!C.held) return true;
	if (!on_map()) { C.held = false; return true; }
	if (!step_to(bn6_player_x(), bn6_player_y())) return false;
	uint32_t cam = camera();
	emu_write32(cam + BN6_CAMERA_FOLLOW, C.follow);
	emu_write8(cam + BN6_CAMERA_MODE, C.mode);
	C.held = false;
	return true;
}

void lib_camera_nudge(int sx, int sy) {
	if (!lib_look_on() || !hold() || (!sx && !sy)) return;
	/* (a step on the screen, sx across and sy down, as one on the world:
	 * sx = dx + dy, sy = (dy - dx) / 2) */
	C.x += (sx - 2 * sy) * 0x8000;
	C.y += (sx + 2 * sy) * 0x8000;
	/* (kept over the board, as BN5's map bounds its camera) */
	int x0 = 1 << 30, y0 = 1 << 30, x1 = -(1 << 30), y1 = -(1 << 30);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (LB.b.kind[y][x] == LIB_VOID) continue;
			int wx, wy;
			netmap_world(x, y, &wx, &wy);
			if (wx < x0) x0 = wx;
			if (wx > x1) x1 = wx;
			if (wy < y0) y0 = wy;
			if (wy > y1) y1 = wy;
		}
	if (x0 <= x1) {
		if (C.x < x0 * 0x10000) C.x = x0 * 0x10000;
		if (C.x > x1 * 0x10000) C.x = x1 * 0x10000;
		if (C.y < y0 * 0x10000) C.y = y0 * 0x10000;
		if (C.y > y1 * 0x10000) C.y = y1 * 0x10000;
	}
	uint32_t cam = camera();
	emu_write32(cam + BN6_CAMERA_X, (uint32_t)C.x);
	emu_write32(cam + BN6_CAMERA_Y, (uint32_t)C.y);
}

bool lib_camera_place(int *wx, int *wy) {
	if (!C.held) return false;
	*wx = C.x >> 16;
	*wy = C.y >> 16;
	return true;
}

void lib_camera_mend(void) {
	/* (a CONTINUE: a state saved while the board held the camera left it
	 * still; BN6's map follows MegaMan's place, BN6_PLAYER_X's three words) */
	uint32_t cam = camera();
	C.held = false;
	if (cam && emu_read8(cam + BN6_CAMERA_MODE) == 0) {
		emu_write32(cam + BN6_CAMERA_FOLLOW, BN6_PLAYER_X);
		emu_write8(cam + BN6_CAMERA_MODE, 1);
	}
}
