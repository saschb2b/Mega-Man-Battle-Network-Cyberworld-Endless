#include "voxel_scene.h"

#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "bn6_map.h"
#include "director.h"
#include "display.h"
#include "emu.h"
#include "gba_world.h"
#include "lcd_frame.h"
#include "liberation.h"
#include "online_visit.h"
#include "netmap.h"
#include "platform.h"
#include "voxel.h"

/* The map's scroll (docs/rom/net-maps.md, BN6's map drawing): BG1's, the
 * world's x + y and (y - x) / 2 at the picture's middle */
#define GBA_BG1HOFS  0x04000014u
#define GBA_BG1VOFS  0x04000016u
#define CAM_X        120
#define CAM_Y        80
#define SCALE_WHOLE  4
/* What a frame of it may take before the picture is made smaller */
#define BUDGET_MS    11.0
/* The rows whose sprites are the HUD's: an open chat box's from this row down, a battle's gauge and numbers above this one */
#define CHAT_TOP     96
#define BATTLE_TOP   40
/* The frame in tiles, a tile more where the scroll shows part of one */
#define TILES_X      31
#define TILES_Y      21
#define TILES_MOST   0xA000   /* the tile map's entries, both layers: what its buffer holds (BN6_MAP_TILEMAP to BN6_MAP_COORDS) */

/* (made at the first frame it draws: a few megabytes no system pays for that leaves it off) */
static struct Vox {
	VoxGround ground;
	VoxScene scene;
	GbaLayers layers;
	uint8_t vram[0x18000], oam[0x400], io[0x60], pal[0x400], raw[TILES_MOST * 2];
	uint16_t tiles[TILES_MOST];
	uint32_t lit_map[VOX_WN], lit_sprite[VOX_N];
	uint32_t *out;
	int scale, out_scale;
	/* the tile map's place: its pixel at the frame's (0, 0) less the camera's, found by matching */
	int place_x, place_y, placed_tw, placed_th;
	SDL_Texture *tex;
	SDL_Renderer *on;
	int tex_scale, lost;
	uint64_t live;        /* P.frame + 1 of the frame the picture is of */
	int slow, scale_most; /* frames in a row over the budget; the scale it then keeps to (0 none) */
} *vox;
#define V (*vox)

/* The scroll `v` as the camera's place: the one nearest MegaMan's of
 * those a 256-pixel background shows alike (a map's scroll is its
 * camera's plus a multiple of 256 that differs from area to area) */
static int unwrap(int v, int near) {
	int d = ((v - near) % 256 + 256 + 128) % 256 - 128;
	return near + d;
}

static int floordiv8(int v) { return v >= 0 ? v >> 3 : -((7 - v) >> 3); }

/* ---- the whole tile map, and where the frame lies on it ---- */

/* How many of the frame's tiles BG1's screen map and the tile map at
 * (ox, oy) disagree on, -1 where most of the frame lies off the map */
static int misfit(const GbaWorld *g, int ox, int oy, int most) {
	unsigned cnt = (unsigned)V.io[0x0A] | (unsigned)V.io[0x0B] << 8, screen = (cnt >> 8 & 31u) * 0x800u;
	unsigned hofs = ((unsigned)V.io[0x14] | (unsigned)V.io[0x15] << 8) & 0x1FFu, vofs = ((unsigned)V.io[0x16] | (unsigned)V.io[0x17] << 8) & 0x1FFu;
	int bad = 0, in = 0;
	for (int j = 0; j < TILES_Y; ++j)
		for (int i = 0; i < TILES_X; ++i) {
			int tx = floordiv8(i * 8 + ox), ty = floordiv8(j * 8 + oy);
			if (tx < 0 || ty < 0 || tx >= g->tw || ty >= g->th) continue;
			unsigned at = screen + ((((unsigned)(j * 8) + vofs) >> 3 & 31u) * 32u + (((unsigned)(i * 8) + hofs) >> 3 & 31u)) * 2u;
			++in;
			if (g->map[0][ty * g->tw + tx] != ((unsigned)V.vram[at] | (unsigned)V.vram[at + 1] << 8) && ++bad > most) return bad;
		}
	return in * 2 < TILES_X * TILES_Y ? -1 : bad;
}

/* The tile map read and placed under the frame; false where there is
 * none or the frame's tiles are found nowhere on it */
static bool world_read(GbaWorld *g, int cam_u, int cam_v) {
	int tw = emu_read8(BN6_MAP_TILEMAP_W), th = emu_read8(BN6_MAP_TILEMAP_H);
	if (tw < TILES_X || th < TILES_Y || tw * th * 2 > TILES_MOST) return false;
	emu_read_bytes(BN6_MAP_TILEMAP + BN6_MAP_TILEMAP_HEAD, V.raw, (uint32_t)(tw * th * 4));
	for (int i = 0; i < tw * th * 2; ++i) V.tiles[i] = (uint16_t)(V.raw[i * 2] | V.raw[i * 2 + 1] << 8);
	*g = (GbaWorld){ V.vram, V.pal, V.io, { V.tiles, V.tiles + tw * th }, tw, th, 0, 0 };
	g->ox = cam_u + V.place_x;
	g->oy = cam_v + V.place_y;
	if (V.placed_tw == tw && V.placed_th == th && misfit(g, g->ox, g->oy, 0) == 0) return true;
	/* (anywhere the scroll's own tiles can lie: BG1's scroll plus whole tiles) */
	int hofs = emu_read16(GBA_BG1HOFS) & 0x1FF, vofs = emu_read16(GBA_BG1VOFS) & 0x1FF;
	for (int ky = -TILES_Y - 64; ky <= th; ++ky)
		for (int kx = -TILES_X - 64; kx <= tw; ++kx) {
			if (misfit(g, hofs + kx * 8, vofs + ky * 8, 0) != 0) continue;
			g->ox = hofs + kx * 8;
			g->oy = vofs + ky * 8;
			V.place_x = g->ox - cam_u;
			V.place_y = g->oy - cam_v;
			V.placed_tw = tw;
			V.placed_th = th;
			return true;
		}
	return false;
}

/* The floor over the window's lattice, each level's panels */
typedef struct { int cam_u, cam_v; } Camera;

static void ground_rows(int from, int to, void *ctx) {
	const Camera *c = ctx;
	for (int j = from; j < to; ++j)
		for (int i = 0; i < VOX_GW; ++i) {
			int u = i - VOX_LEFT - VOX_MARGIN - CAM_X + c->cam_u, v = j - VOX_UP - VOX_MARGIN - CAM_Y + c->cam_v;
			/* (a stair is its ramp, floor of neither level) */
			int ramp = V.ground.z[1] > 0 ? netmap_stair_height(u - 2 * v, u + 2 * v) : -1;
			unsigned bits = ramp < 0 && netmap_floor_at(u - 2 * v, u + 2 * v, 0) ? 1u : 0u;
			if (ramp < 0 && V.ground.z[1] > 0 && netmap_floor_at(u - 2 * v, u + 2 * v, 1)) bits |= 2u;
			V.ground.floor[j * VOX_GW + i] = (uint8_t)bits;
			V.ground.ramp[j * VOX_GW + i] = (uint8_t)(ramp + 1);
		}
}

/* The map's picture over the window */
static void world_rows(int from, int to, void *ctx) {
	gba_world(ctx, -VOX_LEFT, -VOX_UP + from, VOX_WW, to - from, V.scene.map + from * VOX_WW, V.scene.sky + from * VOX_WW);
}

/* ---- the draw's bands on threads ---- */

#define WORKERS_MOST 3
typedef struct { VoxBand band; int from, to; void *ctx; } Band;
static struct { SDL_sem *go; Band job; } worker[WORKERS_MOST];
static SDL_sem *done;
static int workers = -1;

static int work(void *p) {
	int w = (int)(intptr_t)p;
	for (;;) {
		SDL_SemWait(worker[w].go);
		worker[w].job.band(worker[w].job.from, worker[w].job.to, worker[w].job.ctx);
		SDL_SemPost(done);
	}
	return 0;
}

/* (a worker a core but this one, three at most; none where a system has no threads) */
static void workers_start(void) {
	int want = SDL_GetCPUCount() - 1;
	workers = 0;
	done = SDL_CreateSemaphore(0);
	for (int w = 0; done && w < want && w < WORKERS_MOST; ++w) {
		worker[w].go = SDL_CreateSemaphore(0);
		SDL_Thread *t = worker[w].go ? SDL_CreateThread(work, "voxel", (void *)(intptr_t)w) : NULL;
		if (!t) break;
		SDL_DetachThread(t);
		++workers;
	}
}

/* `band` over rows 0 to `rows`: a band a worker, the last one here */
static void on_threads(VoxBand band, int rows, void *ctx) {
	if (workers < 0) workers_start();
	int n = workers + 1;
	for (int w = 0; w < workers; ++w) {
		worker[w].job = (Band){ band, rows * w / n, rows * (w + 1) / n, ctx };
		SDL_SemPost(worker[w].go);
	}
	band(rows * workers / n, rows, ctx);
	for (int w = 0; w < workers; ++w) SDL_SemWait(done);
}

/* The world's colours as the picture's are shown (the GBA's screen's
 * colour correction, docs/engine/lcd.md), which the frame's sprites
 * already are: lcd_frame's wants red at bit 16 */
static void world_colours(uint32_t *px, int n) {
	if (lcd_colour() == LCD_COLOUR_OFF) return;
	for (int pass = 0; pass < 2; ++pass) {
		for (int i = 0; i < n; ++i) px[i] = (px[i] & 0xFF00FF00u) | (px[i] & 0xFFu) << 16 | (px[i] >> 16 & 0xFFu);
		if (!pass) lcd_correct_pixels(px, n);
	}
}

/* ---- the picture, and its place under the canvas ---- */

static bool under(SDL_Renderer *r, const SDL_Rect *canvas) {
	if (!vox || !V.out || V.live != P.frame + 1) return false;
	SDL_Rect dst = { canvas->x + P.core_x * canvas->w / P.w, canvas->y + P.core_y * canvas->h / P.h, VOX_W * canvas->w / P.w, VOX_H * canvas->h / P.h };
	if (!V.tex || V.on != r || V.tex_scale != V.scale || V.lost != P.textures_lost) {
		if (V.tex && V.on == r && V.lost == P.textures_lost) SDL_DestroyTexture(V.tex);
		V.tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, VOX_W * V.scale, VOX_H * V.scale);
		V.on = r;
		V.tex_scale = V.scale;
		V.lost = P.textures_lost;
		if (!V.tex) return false;
		SDL_SetTextureBlendMode(V.tex, SDL_BLENDMODE_NONE);
	}
	SDL_SetTextureScaleMode(V.tex, dst.w % (VOX_W * V.scale) == 0 ? SDL_ScaleModeNearest : SDL_ScaleModeLinear);
	SDL_UpdateTexture(V.tex, NULL, V.out, VOX_W * V.scale * 4);
	SDL_RenderCopy(r, V.tex, NULL, &dst);
	return true;
}

static bool out_sized(void) {
	/* (past 4x, half the screen's scale: each voxel two of its pixels a
	 * side, a quarter of the work) */
	int s = P.fill > 0 ? (int)(P.fill + 0.999f) : P.scale;
	if (s > SCALE_WHOLE) s = (s + 1) / 2;
	if (V.scale_most && s > V.scale_most) s = V.scale_most;
	V.scale = s < 1 ? 1 : s > SCALE_WHOLE ? SCALE_WHOLE : s;
	if (V.out && V.out_scale == V.scale) return true;
	free(V.out);
	V.out = malloc((size_t)VOX_N * (size_t)(V.scale * V.scale) * sizeof *V.out);
	V.out_scale = V.scale;
	return V.out != NULL;
}

/* What the frame is of: a layer (its own floor known), another map (the
 * town, Lan's HP: no floor of ours, all of it flat under BN6's own
 * camera), a battle (the same, its field from the screen maps), or
 * nothing voxel mode draws (a menu, a battle beginning, a liberation's
 * board, whose panels' tints are the picture's, a visit to another's
 * Square) */
enum { SCENE_NONE, SCENE_LAYER, SCENE_FLAT, SCENE_BATTLE };

static int scene_now(void) {
	if (emu_read8(emu_read32(BN6_TOOLKIT)) != BN6_MODE_GAME || online_visit_away() || online_visit_held()) return SCENE_NONE;
	int sub = emu_read8(BN6_GAMESTATE);
	if (sub == BN6_SUB_BATTLE) return SCENE_BATTLE;
	if (sub != BN6_SUB_MAP) return SCENE_NONE;
	if (!director_on_map() || director_in_town() || director_in_hp()) return SCENE_FLAT;
	return lib_on() ? SCENE_NONE : SCENE_LAYER;
}

/* The video memory read, the frame's layers and the world's picture
 * (the map's whole tile map where it is found, else the frame alone);
 * false where the frame cannot be read */
static bool scene_read(int kind, int cam_u, int cam_v) {
	emu_read_bytes(0x06000000u, V.vram, sizeof V.vram);
	emu_read_bytes(0x07000000u, V.oam, sizeof V.oam);
	emu_read_bytes(0x04000000u, V.io, sizeof V.io);
	emu_read_bytes(0x05000000u, V.pal, sizeof V.pal);
	GbaWorld world;
	if (!gba_layers(&(GbaVideo){ V.vram, V.oam, V.io, V.pal }, &V.layers) || !out_sized()) return false;
	if (kind == SCENE_BATTLE || !world_read(&world, cam_u, cam_v)) {
		if (kind == SCENE_LAYER) return false;
		world = (GbaWorld){ V.vram, V.pal, V.io, { NULL, NULL }, 0, 0, 0, 0 };
	}
	on_threads(world_rows, VOX_WH, &world);
	world_colours(V.scene.map, VOX_WN);
	world_colours(V.scene.sky, VOX_WN);
	for (int i = 0; i < VOX_N; ++i)
		if (V.layers.lying[i]) world_colours(&V.layers.lying[i], 1);
	if (kind != SCENE_FLAT) return true;
	emu_picture_tint_world(V.scene.map, VOX_WN);
	emu_picture_tint_world(V.scene.sky, VOX_WN);
	emu_picture_tint_world(V.layers.lying, VOX_N);
	return true;
}

bool voxel_scene_frame(const uint32_t *shown, uint8_t *hud) {
	int kind = display_voxel() ? scene_now() : SCENE_NONE;
	if (kind == SCENE_NONE) return false;
	if (!vox) {
		vox = calloc(1, sizeof *vox);
		if (!vox) return false;
		platform_set_under(under);
		voxel_bands(on_threads);
	}
	uint64_t t0 = SDL_GetPerformanceCounter();
	int px = bn6_player_x(), py = bn6_player_y(), hofs = emu_read16(GBA_BG1HOFS) & 0x1FF, vofs = emu_read16(GBA_BG1VOFS) & 0x1FF;
	bool on_layer = kind == SCENE_LAYER;
	/* (off a layer the scroll itself: only the tile map's place hangs on it) */
	int cam_u = on_layer ? unwrap(hofs - 256, px + py) : hofs, cam_v = on_layer ? unwrap(vofs + 256, (py - px) / 2) : vofs;
	if (!scene_read(kind, cam_u, cam_v)) return false;
	memset(&V.ground, 0, sizeof V.ground);
	V.ground.z[1] = on_layer ? netmap_rise() : 0;
	if (on_layer) on_threads(ground_rows, VOX_GH, &(Camera){ cam_u, cam_v });
	voxel_lift(&V.layers, emu_read8(BN6_CHATBOX) ? CHAT_TOP : kind == SCENE_BATTLE ? -BATTLE_TOP : 0, shown, &V.ground, &V.scene);
	VoxLook look = voxel_look_default();
	look.debug = display_voxel() == DISPLAY_VOXEL_DEBUG;
	/* (a battle is read whole: nothing of it out of focus) */
	if (kind == SCENE_BATTLE) look.focus = 0;
	voxel_light(&V.scene, &V.ground, &look, V.lit_map, V.lit_sprite);
	VoxCamera cam = { 30.0f, 1.0f, 0.0f, CAM_X, CAM_Y, 0.0f };
	if (on_layer) cam = (VoxCamera){ display_voxel_pitch(), display_voxel_zoom(), display_voxel_depth(), (float)(CAM_X + px + py - cam_u), (float)(CAM_Y + (py - px) / 2 - cam_v),
		(float)bn6_player_z() };
	if (!voxel_draw(&V.scene, V.lit_map, V.lit_sprite, &look, &cam, V.scale, V.out)) return false;
	/* (a system the frame takes too long on draws it smaller: a scale less
	 * after a second of frames over the budget, and no more again) */
	double ms = (double)(SDL_GetPerformanceCounter() - t0) * 1000.0 / (double)SDL_GetPerformanceFrequency();
	V.slow = ms > BUDGET_MS ? V.slow + 1 : 0;
	if (V.slow >= 60 && V.scale > 1) { V.scale_most = V.scale - 1; V.slow = 0; }
	V.live = P.frame + 1;
	memcpy(hud, V.scene.hud, VOX_N);
	return true;
}
