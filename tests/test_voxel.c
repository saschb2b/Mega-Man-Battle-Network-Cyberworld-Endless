#include "test_voxel.h"

#include <stdio.h>
#include <string.h>

#include "gba_layers.h"
#include "gba_world.h"
#include "voxel.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static uint8_t vram[0x18000], oam[0x400], io[0x60], pal[0x400];

static void put16(uint8_t *p, unsigned at, unsigned v) { p[at] = (uint8_t)v; p[at + 1] = (uint8_t)(v >> 8); }

/* Video memory of one background tile and one sprite: BG1's tile 1 (every
 * pixel colour 1) at its screen map's first entry, sprite 0 an 8x8 of
 * tile 2 (colour 3) at (4, 4) */
static void video_make(void) {
	memset(vram, 0, sizeof vram);
	memset(io, 0, sizeof io);
	memset(pal, 0, sizeof pal);
	for (unsigned n = 0; n < 128; ++n) put16(oam, n * 8, 0x0200);   /* (each sprite off) */
	put16(io, 0x00, 0x1240);   /* mode 0, BG1 and the sprites, their tiles in a row */
	put16(io, 0x0A, 0x0803);   /* BG1: priority 3, its tiles at 0, its map at 0x4000 */
	memset(vram + 32, 0x11, 32);
	put16(vram, 0x4000, 1);
	memset(vram + 0x10000 + 2 * 32, 0x33, 32);
	put16(oam, 0, 4);
	put16(oam, 2, 4);
	put16(oam, 4, 2 | 1 << 10);
}

static void test_layers(void) {
	static GbaLayers l;
	video_make();
	CHECK(gba_layers(&(GbaVideo){ vram, oam, io, pal }, &l), "layers: mode 0 is read");
	CHECK(l.top[0] == 1 && l.under[0] == 1, "layers: BG1's tile shows at (0, 0): %d", l.top[0]);
	CHECK(l.top[4 * 240 + 4] == GBA_LAYER_OBJ && l.obj[4 * 240 + 4] == 1 && l.under[4 * 240 + 4] == 1, "layers: the sprite over BG1 at (4, 4)");
	CHECK(l.top[20 * 240 + 20] == GBA_LAYER_NONE && !l.obj[20 * 240 + 20], "layers: nothing at (20, 20)");
	CHECK(l.x[0] == 4 && l.y[0] == 4 && l.w[0] == 8 && l.h[0] == 8 && !l.w[1], "layers: the sprite's box, and none for one that is off");
	/* (a blended sprite is marked; a sprite under the background of a lower priority's number is not on top) */
	put16(oam, 0, 4 | 1 << 10);
	put16(oam, 4, 2 | 3 << 10);
	put16(io, 0x0A, 0x0802);
	gba_layers(&(GbaVideo){ vram, oam, io, pal }, &l);
	CHECK(l.sheer[4 * 240 + 4] == 1 && l.top[4 * 240 + 4] == 1, "layers: a blended sprite of priority 3 under BG1 of priority 2");
	/* (one of priority 3, not blended, lies on the floor: its colour kept, no sprite on top) */
	put16(oam, 0, 4);
	put16(pal, 0x200 + 3 * 2, 0x03E0);
	gba_layers(&(GbaVideo){ vram, oam, io, pal }, &l);
	CHECK(l.lying[4 * 240 + 4] == 0xFF00FF00u && !l.obj[4 * 240 + 4] && l.top[4 * 240 + 4] == 1, "layers: a sprite of priority 3 lies on the floor, green: %08x",
		(unsigned)l.lying[4 * 240 + 4]);
	/* (BG3 before the map's backgrounds by its priority is the HUD's; behind or level with them the sky) */
	put16(io, 0x00, 0x1A40);
	put16(io, 0x0E, 0x0001);
	CHECK(gba_bg3_front(io), "layers: BG3 of priority 1 before BG1 of priority 2");
	put16(io, 0x0E, 0x0002);
	CHECK(!gba_bg3_front(io), "layers: BG3 level with BG1 is the sky");
	put16(io, 0x00, 0x1241);
	CHECK(!gba_layers(&(GbaVideo){ vram, oam, io, pal }, &l), "layers: mode 1 is refused");
}

static void test_world(void) {
	static uint16_t tiles[2][40 * 30];
	static uint32_t map[64 * 64], sky[64 * 64];
	video_make();
	memset(tiles, 0, sizeof tiles);
	tiles[0][5 * 40 + 5] = 1 | 2 << 12;   /* tile 1 in palette 2 at the map's tile (5, 5) */
	put16(pal, 0, 0x7C00);                /* the backdrop blue */
	put16(pal, (2 * 16 + 1) * 2, 0x7FFF); /* palette 2's colour 1 white */
	GbaWorld g = { vram, pal, io, { tiles[0], tiles[1] }, 40, 30, 0, 0 };
	gba_world(&g, 0, 0, 64, 64, map, sky);
	CHECK(map[44 * 64 + 44] == 0xFFFFFFFFu, "world: the tile's pixel white at (44, 44): %08x", (unsigned)map[44 * 64 + 44]);
	CHECK(!map[0] && sky[0] == 0xFFFF0000u, "world: none at (0, 0), the backdrop blue behind: %08x", (unsigned)sky[0]);
	/* (the frame 8 pixels into the map: the tile 8 to the left) */
	g.ox = 8;
	gba_world(&g, 0, 0, 64, 64, map, sky);
	CHECK(map[44 * 64 + 36] && !map[44 * 64 + 44], "world: the map's place moves its picture");
	/* (BG1 faded to black: BLDCNT's mode 3 on it, BLDY 16) */
	put16(io, 0x50, 0x00C2);
	put16(io, 0x54, 16);
	gba_world(&g, 0, 0, 64, 64, map, sky);
	CHECK(map[44 * 64 + 36] == 0xFF000000u && sky[0] == 0xFFFF0000u, "world: a fade on BG1 alone");
}

/* A scene of one platform: floor at columns 100-139, ground rows 60-99,
 * ten rows of side under it, a sprite standing on it at columns 118-122, rows
 * 50-69, and BG0 at the frame's corner */
static VoxScene scene;
static VoxGround ground;
static GbaLayers layers;
static uint32_t frame[VOX_N], lit_map[VOX_WN], lit_sprite[VOX_N], out[VOX_N];

static int window(int x, int y) { return (y + VOX_UP) * VOX_WW + x + VOX_LEFT; }

static void scene_make(void) {
	memset(&scene, 0, sizeof scene);
	memset(&ground, 0, sizeof ground);
	memset(&layers, 0, sizeof layers);
	memset(layers.top, GBA_LAYER_NONE, sizeof layers.top);
	memset(layers.under, GBA_LAYER_NONE, sizeof layers.under);
	for (int y = 60; y < 110; ++y)
		for (int x = 100; x < 140; ++x) {
			scene.map[window(x, y)] = 0xFF808080u;
			if (y < 100) ground.floor[(y + VOX_UP + VOX_MARGIN) * VOX_GW + x + VOX_LEFT + VOX_MARGIN] = 1;
		}
	for (int y = 50; y < 70; ++y)
		for (int x = 118; x <= 122; ++x) {
			layers.top[y * VOX_W + x] = GBA_LAYER_OBJ;
			layers.obj[y * VOX_W + x] = 1;
			frame[y * VOX_W + x] = 0x00204060u;
		}
	layers.x[0] = 118; layers.y[0] = 50; layers.w[0] = 8; layers.h[0] = 32;
	layers.top[0] = 0;
	layers.hud[0] = 1;
}

static void test_lift(void) {
	scene_make();
	voxel_lift(&layers, 0, frame, &ground, &scene);
	CHECK(scene.kind[window(110, 65)] == VOX_TOP && scene.gy[window(110, 65)] == 65, "lift: a top on its own ground row");
	CHECK(scene.kind[window(110, 105)] == VOX_WALL && scene.gy[window(110, 105)] == 100, "lift: a side hangs from the edge above: row %d", scene.gy[window(110, 105)]);
	CHECK(scene.kind[window(50, 65)] == VOX_SKY, "lift: the sky where the map has nothing");
	CHECK(scene.nbodies == 1 && scene.bodies[1].gy == 70 && scene.bodies[1].x0 == 118 && scene.bodies[1].y0 == 50, "lift: one body on its feet: %d at row %d", scene.nbodies,
		scene.bodies[1].gy);
	CHECK(scene.sprite[60 * VOX_W + 120] == 0xFF204060u && scene.body[60 * VOX_W + 120] == 1, "lift: the sprite's colour from the frame");
	CHECK(scene.hud[0] == 1 && !scene.hud[60 * VOX_W + 120], "lift: BG0 is the HUD's, a sprite on the floor is not");
	/* (a pad lying on the floor, and past its edge: the floor's colour there, floor where there was none) */
	layers.lying[65 * VOX_W + 110] = layers.lying[65 * VOX_W + 90] = 0xFF00FF00u;
	voxel_lift(&layers, 0, frame, &ground, &scene);
	CHECK(scene.map[window(110, 65)] == 0xFF00FF00u && scene.kind[window(110, 65)] == VOX_TOP && scene.nbodies == 1, "lift: what lies on the floor is the floor's");
	CHECK(scene.map[window(90, 65)] == 0xFF00FF00u && scene.kind[window(90, 65)] == VOX_TOP && scene.gy[window(90, 65)] == 65, "lift: ... and floor where the map had none");
	layers.lying[65 * VOX_W + 110] = layers.lying[65 * VOX_W + 90] = 0;
	scene.map[window(110, 65)] = 0xFF808080u;
	scene.map[window(90, 65)] = 0;
	/* (art beside the floor, under no edge: the scenery's, flat; a long run under an edge too; and in the HP's corner a sprite is the HUD's) */
	for (int x = 150; x < 160; ++x) scene.map[window(x, 65)] = 0xFF808080u;
	for (int y = 110; y < 150; ++y) scene.map[window(100, y)] = 0xFF808080u;
	layers.x[0] = 8; layers.y[0] = 0; layers.w[0] = 8; layers.h[0] = 16;
	voxel_lift(&layers, 0, frame, &ground, &scene);
	CHECK(scene.kind[window(155, 65)] == VOX_TOP && scene.gy[window(155, 65)] == 65, "lift: art beside the floor lies flat");
	CHECK(scene.kind[window(100, 105)] == VOX_TOP && scene.kind[window(104, 105)] == VOX_TOP && scene.kind[window(120, 105)] == VOX_WALL,
		"lift: a long run under an edge is more floor, a short one beside it too, one apart from it a side");
	CHECK(scene.hud[60 * VOX_W + 120] == 1 && scene.nbodies == 0, "lift: a sprite all in the HP's corner is the HUD's");
	/* (a stair: a ramp 20 rows high where the floor was, at columns 100-119; the pixel 10 rows up its ground row shows it 10 high) */
	for (int y = 60; y < 100; ++y)
		for (int x = 100; x < 120; ++x) {
			int at = (y + VOX_UP + VOX_MARGIN) * VOX_GW + x + VOX_LEFT + VOX_MARGIN;
			ground.floor[at] = 0;
			ground.ramp[at] = (uint8_t)(1 + (99 - y) / 2);
		}
	ground.z[1] = 20;
	voxel_lift(&layers, 0, frame, &ground, &scene);
	CHECK(scene.kind[window(110, 70)] == VOX_TOP && scene.gy[window(110, 70)] - 70 >= 9 && scene.gy[window(110, 70)] - 70 <= 11, "lift: a ramp's pixel stands as high as it shows: %d",
		scene.gy[window(110, 70)] - 70);
	CHECK(scene.kind[window(130, 70)] == VOX_TOP && scene.gy[window(130, 70)] == 70, "lift: ... and the floor beside it lies on the ground");
}

static unsigned red(uint32_t c) { return c & 0xFF; }

static void test_light_and_draw(void) {
	scene_make();
	voxel_lift(&layers, 0, frame, &ground, &scene);
	VoxLook look = voxel_look_default();
	voxel_light(&scene, &ground, &look, lit_map, lit_sprite);
	/* (the sun from the front left: the shadow falls behind the body, to its right) */
	CHECK(red(lit_map[window(123, 66)]) < red(lit_map[window(104, 66)]), "light: the floor behind the body is in its shadow: %u against %u",
		red(lit_map[window(123, 66)]), red(lit_map[window(104, 66)]));
	CHECK(red(lit_map[window(104, 105)]) < red(lit_map[window(104, 66)]), "light: a side is darker than a top");
	CHECK(lit_sprite[60 * VOX_W + 120] && !lit_sprite[0], "light: the sprite lit, nothing where it is not");
	/* (BN6's own camera, the look's soft layers off: every face where the frame has its pixel) */
	look.debug = 1;
	voxel_light(&scene, &ground, &look, lit_map, lit_sprite);
	VoxCamera cam = { 30.0f, 1.0f, 0.0f, 120.0f, 70.0f, 0.0f };
	CHECK(voxel_draw(&scene, lit_map, lit_sprite, &look, &cam, 1, out), "draw: a picture at 1x");
	CHECK(out[65 * VOX_W + 110] == lit_map[window(110, 65)] && out[105 * VOX_W + 110] == lit_map[window(110, 105)], "draw: at BN6's camera a top and a side stay in place");
	CHECK(out[60 * VOX_W + 120] == lit_sprite[60 * VOX_W + 120], "draw: ... and the sprite before the floor");
	/* (a lower camera: the floor's rows closer, the side longer) */
	cam.pitch = 20.0f;
	voxel_draw(&scene, lit_map, lit_sprite, &look, &cam, 1, out);
	int tops = 0, sides = 0;
	for (int y = 0; y < VOX_H; ++y) {
		tops += out[y * VOX_W + 105] == lit_map[window(105, 65)];
		sides += out[y * VOX_W + 105] == lit_map[window(105, 105)];
	}
	CHECK(tops > 20 && tops < 32 && sides >= 1, "draw: at 20 degrees %d rows of top (40 at 30, 27 by the angle), the side still shown (%d)", tops, sides);
}

int test_voxel(void) {
	failures = 0;
	test_layers();
	test_world();
	test_lift();
	test_light_and_draw();
	return failures;
}
