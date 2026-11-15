#include "test_stereo.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "gba_layers.h"
#include "stereo.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static uint8_t vram[0x18000], oam[0x400], io[0x60];

static void put16(uint8_t *p, unsigned at, unsigned v) { p[at] = (uint8_t)v; p[at + 1] = (uint8_t)(v >> 8); }

/* BG1 (the map) full of tile 1; sprite 0 a 16x16 at (100, 60), standing;
 * sprite 1 an 8x8 at (4, 2), in the HP's corner; BG0 (the HUD) one tile at
 * the screen's (0, 152) */
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
	put16(oam, 0, 60);
	put16(oam, 2, 100 | 1 << 14);
	put16(oam, 4, 2 | 1 << 10);
	put16(oam, 8, 2);
	put16(oam, 10, 4);
	put16(oam, 12, 2 | 1 << 10);
}

static void test_flat(void) {
	static StereoFlat f;
	video_make();
	GbaVideo v = { vram, oam, io, NULL };
	stereo_flat(&v, 0, &f);
	CHECK(f.nrects == 1 && f.rect[0].x0 == 4 && f.rect[0].y0 == 2 && f.rect[0].x1 == 12 && f.rect[0].y1 == 10, "stereo: the HP's corner one flat box, the sprite standing none: %d",
		f.nrects);
	CHECK(f.cell[19][0] && !f.cell[19][1] && !f.cell[7][12], "stereo: BG0's tile one flat cell");
	/* (in a battle the top rows' sprites are the HUD's; with a chat box open, its rows') */
	put16(oam, 0, 10);
	stereo_flat(&v, -40, &f);
	CHECK(f.nrects == 2, "stereo: a sprite in a battle's top rows is flat: %d", f.nrects);
	put16(oam, 0, 100);
	stereo_flat(&v, 96, &f);
	CHECK(f.nrects == 2 && f.rect[0].y0 == 100, "stereo: a sprite in an open chat box's rows is flat");
	/* (BG3 in front of the map, a battle's Custom screen, flat where it has a pixel) */
	put16(io, 0x00, 0x1B40);
	put16(io, 0x0E, 0x0C01);   /* BG3: priority 1, its map at 0x6000 */
	put16(vram, 0x6000 + (5 * 32 + 3) * 2, 1);
	stereo_flat(&v, 0, &f);
	CHECK(f.cell[5][3] && !f.cell[5][4], "stereo: BG3 in front flat where it has a tile");
}

static void test_rows(void) {
	static StereoFlat f;
	video_make();
	stereo_flat(&(GbaVideo){ vram, oam, io, NULL }, 0, &f);
	StereoRow r;
	stereo_row(&f, 5, &r);
	CHECK(r.n == 3 && r.x[0] == 0 && !r.flat[0] && r.x[1] == 4 && r.flat[1] && r.x[2] == 12 && !r.flat[2], "stereo: row 5 the world, the HP's box, the world: %d spans", r.n);
	stereo_row(&f, 155, &r);
	CHECK(r.n == 2 && r.flat[0] && r.x[1] == 8 && !r.flat[1], "stereo: row 155 BG0's cell, then the world");
	stereo_row(&f, 80, &r);
	CHECK(r.n == 1 && !r.flat[0], "stereo: row 80 the world's whole");
	/* (past the spans a row may have, the rest flat) */
	memset(&f, 0, sizeof f);
	for (int c = 0; c < 30; c += 2) f.cell[0][c] = 1;
	stereo_row(&f, 0, &r);
	CHECK(r.n == STEREO_SPANS && r.flat[STEREO_SPANS - 1], "stereo: a row of many spans ends flat");
}

static void test_depth(void) {
	CHECK(stereo_depth(STEREO_H - 1) == 0.0f && stereo_depth(0) == 1.0f, "stereo: the last row 0, the first 1");
	bool behind = true;
	for (int y = 0; y < STEREO_H; ++y) behind = behind && stereo_sky_depth(y) >= stereo_map_depth(y) + 0.2f;
	CHECK(stereo_map_depth(STEREO_H - 1) == 0.0f && stereo_map_depth(0) > stereo_map_depth(80) && behind, "stereo: the map tilted back from the screen, the background behind it at every row");
	CHECK(stereo_shift(-1, 10.0f, stereo_sky_depth(0)) == -5.0f && stereo_shift(1, 10.0f, stereo_sky_depth(STEREO_H - 1)) == 2.5f, "stereo: the background tilted, 5 each way at the top, 2.5 at the foot");
	StereoBox b = { 100, 60, 116, 76 };
	CHECK(stereo_body_depth(&b) == stereo_map_depth(75), "stereo: a body at the map's depth at its feet");
}

static void test_sprites(void) {
	static uint8_t mask[STEREO_N];
	StereoBodies bodies;
	video_make();
	GbaVideo v = { vram, oam, io, NULL };
	/* (sprite 2 an 8x8 touching sprite 0's right side; sprite 3 one of priority 3, lying; sprite 4 a blended one) */
	memset(vram + 0x10000 + 2 * 32, 0x33, 32 * 4);
	put16(oam, 16, 64);
	put16(oam, 18, 116);
	put16(oam, 20, 2 | 1 << 10);
	put16(oam, 24, 120);
	put16(oam, 26, 20);
	put16(oam, 28, 2 | 3 << 10);
	put16(oam, 32, 120 | 1 << 10);
	put16(oam, 34, 60);
	put16(oam, 36, 2 | 1 << 10);
	stereo_sprites(&v, 0, mask, &bodies);
	CHECK(mask[70 * STEREO_W + 105] && mask[66 * STEREO_W + 118], "stereo: the standing sprites' pixels marked");
	CHECK(!mask[3 * STEREO_W + 5], "stereo: the HP's corner not a body");
	CHECK(!mask[122 * STEREO_W + 22] && !mask[122 * STEREO_W + 62], "stereo: a lying sprite and a blended one not bodies");
	CHECK(bodies.n == 1 && bodies.box[0].x0 == 100 && bodies.box[0].y0 == 60 && bodies.box[0].x1 == 124 && bodies.box[0].y1 == 76,
		"stereo: two touching sprites one body: %d, (%d, %d)-(%d, %d)", bodies.n, bodies.box[0].x0, bodies.box[0].y0, bodies.box[0].x1, bodies.box[0].y1);
}

/* A picture of the test's own: a colour by row and column; the background
 * left of column 60, a body at (100, 60)-(116, 76), the map the rest; the
 * HP's box flat */
static void test_compose(void) {
	static StereoFlat f;
	static StereoBodies bodies;
	static uint32_t picture[STEREO_N], eye[2][STEREO_N];
	static uint8_t part[STEREO_N];
	memset(&f, 0, sizeof f);
	f.nrects = 1;
	f.rect[0].x0 = 4, f.rect[0].y0 = 2, f.rect[0].x1 = 12, f.rect[0].y1 = 10;
	bodies.n = 1;
	bodies.box[0] = (StereoBox){ 100, 60, 116, 76 };
	for (int i = 0; i < STEREO_N; ++i) {
		int x = i % STEREO_W, y = i / STEREO_W;
		picture[i] = 0xFF000000u | (uint32_t)y | (uint32_t)x << 8;
		part[i] = x < 60 ? STEREO_SKY : x >= 100 && x < 116 && y >= 60 && y < 76 ? STEREO_SPRITE : STEREO_MAP;
	}
	StereoPicture p = { picture, part, &f, &bodies, true };
	stereo_compose(&p, -1, 10.0f, 1, eye[0]);
	stereo_compose(&p, 1, 10.0f, 1, eye[1]);
	CHECK(eye[1][0 * STEREO_W + 25] == picture[0 * STEREO_W + 20] && eye[0][0 * STEREO_W + 15] == picture[0 * STEREO_W + 20], "stereo: the background's top 5 each way");
	CHECK(eye[1][159 * STEREO_W + 23] == picture[159 * STEREO_W + 20] && eye[0][159 * STEREO_W + 18] == picture[159 * STEREO_W + 20], "stereo: the background's foot 2.5 each way");
	int at = (int)floorf(150.0f + stereo_shift(1, 10.0f, stereo_map_depth(80)) + 0.5f);
	CHECK(eye[1][80 * STEREO_W + at] == picture[80 * STEREO_W + 150], "stereo: the map at its row's depth");
	int body = (int)floorf(105.0f + stereo_shift(1, 10.0f, stereo_body_depth(&bodies.box[0])) + 0.5f);
	CHECK(eye[1][62 * STEREO_W + body] == picture[62 * STEREO_W + 105] && eye[1][74 * STEREO_W + body] == picture[74 * STEREO_W + 105],
		"stereo: the body upright, its head moved as its feet");
	CHECK(eye[0][5 * STEREO_W + 8] == picture[5 * STEREO_W + 8] && eye[1][5 * STEREO_W + 8] == picture[5 * STEREO_W + 8], "stereo: the HP's box the same in both eyes");
}

int test_stereo(void) {
	failures = 0;
	test_flat();
	test_rows();
	test_depth();
	test_sprites();
	test_compose();
	return failures;
}
