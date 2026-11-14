#include "voxel_scene.h"

#include <stdlib.h>
#include <string.h>
#if defined(__linux__) || defined(__APPLE__)
#include <sched.h>
#endif

#include <SDL.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "bn6_map.h"
#include "director.h"
#include "display.h"
#include "emu.h"
#include "frame_log.h"
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
	/* each of the 32768 colours as the world shows them: made again when
	 * the screen's correction or the town's hour changes */
	uint32_t lut[0x8000];
	int lut_lcd, lut_hour;
	bool lut_ok;
	/* the floor kept between frames: the layer's it is of, the camera it was made at */
	bool ground_ok;
	unsigned ground_serial;
	int ground_u, ground_v;
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

static void on_threads(VoxBand band, int rows, void *ctx);

/* The floor over the window's lattice, each level's panels: its columns
 * x0 to x1 of the rows a band is given */
typedef struct { int cam_u, cam_v, x0, x1; } Cells;

static void ground_rows(int from, int to, void *ctx) {
	const Cells *c = ctx;
	for (int j = from; j < to; ++j)
		for (int i = c->x0; i < c->x1; ++i) {
			int u = i - VOX_LEFT - VOX_MARGIN - CAM_X + c->cam_u, v = j - VOX_UP - VOX_MARGIN - CAM_Y + c->cam_v;
			/* (a stair is its ramp, floor of neither level) */
			int ramp = V.ground.ramps ? netmap_stair_height(u - 2 * v, u + 2 * v) : -1;
			unsigned bits = ramp < 0 && netmap_floor_at(u - 2 * v, u + 2 * v, 0) ? 1u : 0u;
			if (ramp < 0 && V.ground.z[1] > 0 && netmap_floor_at(u - 2 * v, u + 2 * v, 1)) bits |= 2u;
			V.ground.floor[j * VOX_GW + i] = (uint8_t)bits;
			V.ground.ramp[j * VOX_GW + i] = (uint8_t)(ramp + 1);
		}
}

/* A lattice moved under a camera moved by (du, dv): each cell takes the
 * one du columns and dv rows on, what comes in from beyond left as it was */
static void shifted(uint8_t *a, int du, int dv) {
	if (dv > 0) memmove(a, a + dv * VOX_GW, (size_t)((VOX_GH - dv) * VOX_GW));
	else if (dv < 0) memmove(a - dv * VOX_GW, a, (size_t)((VOX_GH + dv) * VOX_GW));
	for (int j = 0; du && j < VOX_GH; ++j) {
		uint8_t *row = a + j * VOX_GW;
		if (du > 0) memmove(row, row + du, (size_t)(VOX_GW - du));
		else memmove(row - du, row, (size_t)(VOX_GW + du));
	}
}

/* The layer's floor under the camera: made whole on a new layer (a few
 * hundred thousand cells), else moved with the camera and only the cells
 * it brings in asked for (a walk's frame, a few rows and columns) */
static void ground_at(int cam_u, int cam_v) {
	unsigned serial = netmap_serial();
	int rise = netmap_rise(), du = cam_u - V.ground_u, dv = cam_v - V.ground_v;
	bool keep = V.ground_ok && serial == V.ground_serial && rise == V.ground.z[1] && abs(du) < VOX_GW && abs(dv) < VOX_GH;
	V.ground_ok = true;
	V.ground_serial = serial;
	V.ground_u = cam_u;
	V.ground_v = cam_v;
	if (!keep) {
		V.ground.z[0] = 0;
		V.ground.z[1] = rise;
		V.ground.ramps = netmap_stairs() > 0;
		on_threads(ground_rows, VOX_GH, &(Cells){ cam_u, cam_v, 0, VOX_GW });
		return;
	}
	if (!du && !dv) return;
	shifted(V.ground.floor, du, dv);
	shifted(V.ground.ramp, du, dv);
	int y0 = dv > 0 ? VOX_GH - dv : 0, y1 = dv > 0 ? VOX_GH : -dv;
	ground_rows(y0, y1, &(Cells){ cam_u, cam_v, 0, VOX_GW });
	/* (the columns it brings in, in the rows not made already) */
	Cells cols = { cam_u, cam_v, du > 0 ? VOX_GW - du : 0, du > 0 ? VOX_GW : -du };
	if (du) {
		ground_rows(dv > 0 ? 0 : y1, dv > 0 ? y0 : VOX_GH, &cols);
	}
}

/* The frame's layers, a band of its rows */
static void layer_rows(int from, int to, void *ctx) { gba_layers_rows(ctx, &V.layers, from, to); }

/* The map's picture over the window */
static void world_rows(int from, int to, void *ctx) {
	VoxSpan sp = V.scene.span;
	int at = (sp.y0 + from) * VOX_WW + sp.x0;
	gba_world(ctx, &(GbaWindow){ sp.x0 - VOX_LEFT, sp.y0 + from - VOX_UP, sp.x1 - sp.x0, to - from, VOX_WW, V.scene.map + at, V.scene.sky + at });
}

/* ---- the draw's bands on threads ---- */

/* (a worker a core but this one, seven at most) */
#define WORKERS_MOST 7
/* How long a worker waits for the next band awake before it sleeps: a
 * frame's bands come a fraction of a millisecond apart, and woken from
 * sleep each time a worker came late to most of them (four threads did
 * the frame in 3 ms, one in 4) */
#define AWAKE_MS     0.5
/* ... and spins without giving its core away (then it yields it to any
 * thread that waits for one: with more threads than cores, a worker that
 * kept its core held up the one whose band was late) */
#define SPIN_MS      0.02
#if defined(__linux__) || defined(__APPLE__)
#define YIELD() sched_yield()
#else
#define YIELD() SDL_Delay(0)
#endif
#if defined(__i386__) || defined(__x86_64__)
#define PAUSE() __builtin_ia32_pause()
#elif defined(__aarch64__) || (defined(__arm__) && defined(__ARM_ARCH) && __ARM_ARCH >= 7)
#define PAUSE() __asm__ __volatile__("yield")
#else
#define PAUSE() ((void)0)
#endif
/* The round's work: `band` over rows 0 to `rows`, taken a few rows at a
 * time by whoever is free (a band a thread, the floor's rows lay in two
 * of four and a slow core held up the rest) */
static struct { VoxBand band; int rows, chunk; void *ctx; } job;
static struct { SDL_sem *go; SDL_atomic_t asleep; } worker[WORKERS_MOST];
static SDL_atomic_t round_of, busy, taken;   /* the rounds handed out so far; the workers still at this one; its rows taken */
static int workers = -1;

/* Waiting a moment: spun the first SPIN_MS since `from`, then each look
 * after the core yielded */
static void wait_a_moment(uint64_t from, int spins) {
	if ((spins & 63) || SDL_GetPerformanceCounter() - from < (uint64_t)((double)SDL_GetPerformanceFrequency() * SPIN_MS / 1000.0)) PAUSE();
	else YIELD();
}

/* The next round's number, waited for awake a while, then asleep */
static int next_round(int w, int seen) {
	uint64_t from = SDL_GetPerformanceCounter(), until = from + (uint64_t)((double)SDL_GetPerformanceFrequency() * AWAKE_MS / 1000.0);
	for (int spins = 0; SDL_AtomicGet(&round_of) == seen; ++spins) {
		wait_a_moment(from, spins);
		if ((spins & 63) || SDL_GetPerformanceCounter() < until) continue;
		/* (asleep, then the round looked at again: one handed out between is not missed, and a post left over only wakes it to look) */
		SDL_AtomicSet(&worker[w].asleep, 1);
		while (SDL_AtomicGet(&round_of) == seen) SDL_SemWait(worker[w].go);
		SDL_AtomicSet(&worker[w].asleep, 0);
	}
	return SDL_AtomicGet(&round_of);
}

/* The round's rows, a chunk at a time, till none are left */
static void take_rows(void) {
	for (int from; (from = SDL_AtomicAdd(&taken, job.chunk)) < job.rows;)
		job.band(from, from + job.chunk < job.rows ? from + job.chunk : job.rows, job.ctx);
}

static int work(void *p) {
	int w = (int)(intptr_t)p, seen = 0;
	for (;;) {
		seen = next_round(w, seen);
		take_rows();
		SDL_AtomicAdd(&busy, -1);
	}
	return 0;
}

/* (none where a system has no threads) */
static void workers_start(void) {
	int want = SDL_GetCPUCount() - 1;
	workers = 0;
	for (int w = 0; w < want && w < WORKERS_MOST; ++w) {
		worker[w].go = SDL_CreateSemaphore(0);
		SDL_Thread *t = worker[w].go ? SDL_CreateThread(work, "voxel", (void *)(intptr_t)w) : NULL;
		if (!t) break;
		SDL_DetachThread(t);
		++workers;
	}
}

/* `band` over rows 0 to `rows`, on the workers and here */
static void on_threads(VoxBand band, int rows, void *ctx) {
	if (workers < 0) workers_start();
	if (!workers) { band(0, rows, ctx); return; }
	int chunk = rows / ((workers + 1) * 4);
	job.band = band;
	job.rows = rows;
	job.chunk = chunk < 1 ? 1 : chunk;
	job.ctx = ctx;
	SDL_AtomicSet(&taken, 0);
	SDL_AtomicSet(&busy, workers);
	SDL_AtomicAdd(&round_of, 1);
	for (int w = 0; w < workers; ++w)
		if (SDL_AtomicGet(&worker[w].asleep)) SDL_SemPost(worker[w].go);
	take_rows();
	uint64_t from = SDL_GetPerformanceCounter();
	for (int spins = 0; SDL_AtomicGet(&busy) > 0; ++spins) wait_a_moment(from, spins);
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

/* The table of every colour as the world shows it, the town's hour on
 * those off a layer: made once for each correction and hour (made a frame
 * at a time from the window's pixels, it was the window's every one) */
static void lut_for(int hour) {
	int lcd = (int)lcd_colour();
	if (V.lut_ok && V.lut_lcd == lcd && V.lut_hour == hour) return;
	for (unsigned c = 0; c < 0x8000u; ++c) V.lut[c] = gba_shown(c);
	world_colours(V.lut, 0x8000);
	if (hour) emu_picture_tint_world(V.lut, 0x8000);
	V.lut_ok = true;
	V.lut_lcd = lcd;
	V.lut_hour = hour;
}

/* A colour as mGBA shows it (gba_shown's) as the table shows it */
static uint32_t by_lut(uint32_t c) { return V.lut[(c >> 3 & 31u) | (c >> 11 & 31u) << 5 | (c >> 19 & 31u) << 10]; }

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
	GbaVideo video = { V.vram, V.oam, V.io, V.pal };
	if (!gba_layers_sprites(&video, &V.layers) || !out_sized()) return false;
	on_threads(layer_rows, VOX_H, &video);
	if (kind == SCENE_BATTLE || !world_read(&world, cam_u, cam_v)) {
		if (kind == SCENE_LAYER) return false;
		world = (GbaWorld){ V.vram, V.pal, V.io, { NULL, NULL }, 0, 0, 0, 0, NULL };
	}
	lut_for(kind == SCENE_FLAT && !online_visit_away() ? director_town_tint(&(bool){ false }) : 0);
	world.lut = V.lut;
	on_threads(world_rows, V.scene.span.y1 - V.scene.span.y0, &world);
	for (int i = 0; i < VOX_N; ++i)
		if (V.layers.lying[i]) V.layers.lying[i] = by_lut(V.layers.lying[i]);
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
	VoxCamera cam = { 30.0f, 1.0f, 0.0f, CAM_X, CAM_Y, 0.0f };
	if (on_layer) cam = (VoxCamera){ display_voxel_pitch(), display_voxel_zoom(), display_voxel_depth(), (float)(CAM_X + px + py - cam_u), (float)(CAM_Y + (py - px) / 2 - cam_v),
		(float)bn6_player_z() };
	/* (what the camera can show: the sides hang no deeper than their run's
	 * longest and the slack about it, the raised floor no higher than its rise) */
	V.scene.span = voxel_span(&cam, 40, on_layer ? netmap_rise() + 4 : 4);
	if (!scene_read(kind, cam_u, cam_v)) return false;
	if (on_layer) ground_at(cam_u, cam_v);
	else if (V.ground_ok || V.ground.z[1]) {
		memset(&V.ground, 0, sizeof V.ground);
		V.ground_ok = false;
	}
	voxel_lift(&V.layers, emu_read8(BN6_CHATBOX) ? CHAT_TOP : kind == SCENE_BATTLE ? -BATTLE_TOP : 0, shown, &V.ground, &V.scene);
	VoxLook look = voxel_look_default();
	look.debug = display_voxel() == DISPLAY_VOXEL_DEBUG;
	/* (a battle is read whole: nothing of it out of focus) */
	if (kind == SCENE_BATTLE) look.focus = 0;
	voxel_light(&V.scene, &V.ground, &look, V.lit_map, V.lit_sprite);
	if (!voxel_draw(&V.scene, V.lit_map, V.lit_sprite, &look, &cam, V.scale, V.out)) return false;
	/* (a system the frame takes too long on draws it smaller: a scale less
	 * after a second of frames over the budget, and no more again) */
	uint64_t took = SDL_GetPerformanceCounter() - t0;
	double ms = (double)took * 1000.0 / (double)SDL_GetPerformanceFrequency();
	if (platform_frame_log) frame_log_depth(took, V.scale);
	V.slow = ms > BUDGET_MS ? V.slow + 1 : 0;
	if (V.slow >= 60 && V.scale > 1) { V.scale_most = V.scale - 1; V.slow = 0; }
	V.live = P.frame + 1;
	memcpy(hud, V.scene.hud, VOX_N);
	return true;
}
