#include "test_stereo.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "gba_layers.h"
#include "stereo.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static uint8_t vram[0x18000], oam[0x400], io[0x60], pal[0x400];

static void put16(uint8_t *p, unsigned at, unsigned v) { p[at] = (uint8_t)v; p[at + 1] = (uint8_t)(v >> 8); }

/* BG1 (the map) full of tile 1 at priority 2; sprite 0 a 16x16 of tile 2
 * at (100, 60) of priority 1, standing; sprite 1 an 8x8 in the HP's
 * corner; BG0 (the HUD) one tile at the screen's (0, 152) */
static void video_make(void) {
	memset(vram, 0, sizeof vram);
	memset(io, 0, sizeof io);
	for (unsigned n = 0; n < 128; ++n) put16(oam, n * 8, 0x0200);
	put16(io, 0x00, 0x1340);   /* mode 0, BG0, BG1 and the sprites, their tiles in a row */
	put16(io, 0x08, 0x0A00);   /* BG0: priority 0, its tiles at 0, its map at 0x5000 */
	put16(io, 0x0A, 0x0802);   /* BG1: priority 2, its map at 0x4000 */
	memset(vram + 32, 0x11, 32);
	for (unsigned t = 0; t < 32 * 32; ++t) put16(vram, 0x4000 + t * 2, 1);
	put16(vram, 0x5000 + (19 * 32 + 0) * 2, 1);
	memset(vram + 0x10000 + 2 * 32, 0x33, 32 * 4);
	put16(oam, 0, 60);
	put16(oam, 2, 100 | 1 << 14);
	put16(oam, 4, 2 | 1 << 10);
	put16(oam, 8, 2);
	put16(oam, 10, 4);
	put16(oam, 12, 2 | 1 << 10);
}

static void test_sort(void) {
	static GbaLayers l;
	static StereoFrame f;
	video_make();
	GbaVideo v = { vram, oam, io, pal };
	CHECK(gba_layers_sprites(&v, &l), "stereo: the sprites read");
	stereo_sort(&v, &l, 0, &f);
	CHECK(f.nbodies == 1 && f.body[0].x0 == 100 && f.body[0].y0 == 60 && f.body[0].x1 == 115 && f.body[0].y1 == 75,
		"stereo: the sprite one body, its box: %d bodies, (%d, %d)-(%d, %d)", f.nbodies, f.body[0].x0, f.body[0].y0, f.body[0].x1, f.body[0].y1);
	CHECK(f.what[70 * STEREO_W + 105] == STEREO_BODY, "stereo: the body's pixel");
	CHECK(f.what[3 * STEREO_W + 5] == STEREO_FLAT, "stereo: the HP's corner flat");
	CHECK(f.what[155 * STEREO_W + 3] == STEREO_FLAT, "stereo: BG0's pixel flat");
	CHECK(f.what[100 * STEREO_W + 50] == STEREO_WORLD, "stereo: the map's pixel the world's");
	/* (behind a background of a lower priority's number, or blended, a sprite is the world's) */
	put16(io, 0x0A, 0x0800);
	gba_layers_sprites(&v, &l);
	stereo_sort(&v, &l, 0, &f);
	CHECK(f.nbodies == 0 && f.what[70 * STEREO_W + 105] == STEREO_WORLD, "stereo: a sprite behind BG1 of priority 0 is the world's");
	put16(io, 0x0A, 0x0802);
	put16(oam, 0, 60 | 1 << 10);
	gba_layers_sprites(&v, &l);
	stereo_sort(&v, &l, 0, &f);
	CHECK(f.nbodies == 0 && f.what[70 * STEREO_W + 105] == STEREO_WORLD, "stereo: a blended sprite (a shadow) is the world's");
	/* (with a chat box open, a sprite in its rows is the HUD's) */
	put16(oam, 0, 100);
	gba_layers_sprites(&v, &l);
	stereo_sort(&v, &l, 96, &f);
	CHECK(f.nbodies == 0 && f.what[105 * STEREO_W + 105] == STEREO_FLAT, "stereo: a sprite in an open chat box's rows is flat");
}

static void test_depth(void) {
	CHECK(stereo_depth(STEREO_H - 1) == 0.0f && stereo_depth(0) == 1.0f, "stereo: the last row at the screen, the first the deepest");
	CHECK(stereo_shift(-1, 6.0f, 0) == -3.0f && stereo_shift(1, 6.0f, 0) == 3.0f, "stereo: the eyes parted by 6 at the deepest, each its way");
	CHECK(stereo_shift(1, 6.0f, STEREO_H - 1) == 0.0f, "stereo: none at the last row");
}

/* A picture of the test's own: the world a colour by its row and column, a body at
 * (100, 60)-(115, 75), the engine's drawing over the canvas at (200, 20) */
static void test_layers(void) {
	static StereoFrame f;
	static uint32_t picture[STEREO_N], canvas[STEREO_N], world[STEREO_N], bodies[STEREO_N], flat[STEREO_N], eye[2][STEREO_N];
	memset(&f, 0, sizeof f);
	for (int i = 0; i < STEREO_N; ++i) picture[i] = canvas[i] = 0xFF000000u | (uint32_t)(i / STEREO_W) | (uint32_t)(i % STEREO_W) << 8;
	f.nbodies = 1;
	f.body[0] = (StereoBox){ 100, 60, 115, 75 };
	for (int y = 60; y <= 75; ++y)
		for (int x = 100; x <= 115; ++x) {
			f.what[y * STEREO_W + x] = STEREO_BODY;
			picture[y * STEREO_W + x] = canvas[y * STEREO_W + x] = 0xFF0000FFu;
		}
	canvas[20 * STEREO_W + 200] = 0xFFFFFFFFu;
	StereoLayers l = { world, bodies, flat, STEREO_W, 0xFF000000u, 0, 0, { 0 } };
	stereo_layers(&f, canvas, STEREO_W, picture, &l);
	CHECK(bodies[70 * STEREO_W + 105] == 0xFF0000FFu && world[70 * STEREO_W + 50] == picture[70 * STEREO_W + 50], "stereo: the body in its layer, the world in its own");
	CHECK(world[70 * STEREO_W + 105] == picture[70 * STEREO_W + 99] && world[70 * STEREO_W + 110] == picture[70 * STEREO_W + 116],
		"stereo: the world behind the body takes the colours beside it: %08x %08x", (unsigned)world[70 * STEREO_W + 105], (unsigned)world[70 * STEREO_W + 110]);
	CHECK(flat[20 * STEREO_W + 200] == 0xFFFFFFFFu && !(bodies[20 * STEREO_W + 200] & l.alpha), "stereo: the engine's drawing over the game flat");
	CHECK(l.row_world[20] && l.row_world[159], "stereo: each row has the world in it");
	stereo_compose(&l, &f, -1, 6.0f, 1, eye[0]);
	stereo_compose(&l, &f, 1, 6.0f, 1, eye[1]);
	CHECK(eye[0][20 * STEREO_W + 200] == 0xFFFFFFFFu && eye[1][20 * STEREO_W + 200] == 0xFFFFFFFFu, "stereo: the flat the same in both eyes");
	/* (the world's top row three pixels apart each way; the body moved by its feet's depth, the same in both eyes but for the way) */
	CHECK(eye[0][0 * STEREO_W + 10] == world[13] && eye[1][0 * STEREO_W + 10] == world[7], "stereo: the top row moved 3 each way: %08x %08x",
		(unsigned)eye[0][10], (unsigned)eye[1][10]);
	int at = (int)floorf(stereo_shift(1, 6.0f, 75) + 0.5f);
	CHECK(eye[1][70 * STEREO_W + 100 + at] == 0xFF0000FFu && eye[0][70 * STEREO_W + 100 - at] == 0xFF0000FFu, "stereo: the body at its feet's depth, %d each way", at);
}

int test_stereo(void) {
	failures = 0;
	test_sort();
	test_depth();
	test_layers();
	return failures;
}
