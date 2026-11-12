#include "voxel_scene.h"

#include <stdio.h>
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
/* The frame in tiles, a tile more where the scroll shows part of one */
#define TILES_X      31
#define TILES_Y      21
#define TILES_MOST   0xA000   /* the tile map's entries, both layers: what its buffer holds (BN6_MAP_TILEMAP to BN6_MAP_COORDS) */

static struct {
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
	bool hooked;
} V;

static int unwrap(int v, int near) {
	int d = ((v - near) % 512 + 512 + 256) % 512 - 256;
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
static void ground_read(int cam_u, int cam_v) {
	V.ground.z[0] = 0;
	V.ground.z[1] = netmap_rise();
	for (int j = 0; j < VOX_GH; ++j)
		for (int i = 0; i < VOX_GW; ++i) {
			int u = i - VOX_LEFT - VOX_MARGIN - CAM_X + cam_u, v = j - VOX_UP - VOX_MARGIN - CAM_Y + cam_v;
			unsigned bits = netmap_floor_at(u - 2 * v, u + 2 * v, 0) ? 1u : 0u;
			if (V.ground.z[1] > 0 && netmap_floor_at(u - 2 * v, u + 2 * v, 1)) bits |= 2u;
			V.ground.floor[j * VOX_GW + i] = (uint8_t)bits;
		}
}

/* ---- the picture, and its place under the canvas ---- */

static bool under(SDL_Renderer *r, const SDL_Rect *canvas) {
	if (!V.out || V.live != P.frame + 1) return false;
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
	V.scale = s < 1 ? 1 : s > SCALE_WHOLE ? SCALE_WHOLE : s;
	if (V.out && V.out_scale == V.scale) return true;
	free(V.out);
	V.out = malloc((size_t)VOX_N * (size_t)(V.scale * V.scale) * sizeof *V.out);
	V.out_scale = V.scale;
	return V.out != NULL;
}

bool voxel_scene_frame(const uint32_t *shown, uint8_t *hud) {
	if (!display_voxel() || !director_on_map() || director_in_town() || director_in_hp()) return false;
	if (!V.hooked) { platform_set_under(under); V.hooked = true; }
	uint64_t t[8]; int tn = 0;
#define TICK t[tn++] = SDL_GetPerformanceCounter()
	TICK;
	int px = bn6_player_x(), py = bn6_player_y();
	int cam_u = unwrap((emu_read16(GBA_BG1HOFS) & 0x1FF) - 256, px + py);
	int cam_v = unwrap((emu_read16(GBA_BG1VOFS) & 0x1FF) + 256, (py - px) / 2);
	emu_read_bytes(0x06000000u, V.vram, sizeof V.vram);
	emu_read_bytes(0x07000000u, V.oam, sizeof V.oam);
	emu_read_bytes(0x04000000u, V.io, sizeof V.io);
	emu_read_bytes(0x05000000u, V.pal, sizeof V.pal);
	TICK;
	GbaWorld world;
	if (!gba_layers(&(GbaVideo){ V.vram, V.oam, V.io }, &V.layers) || !world_read(&world, cam_u, cam_v) || !out_sized()) return false;
	TICK;
	gba_world(&world, -VOX_LEFT, -VOX_UP, VOX_WW, VOX_WH, V.scene.map, V.scene.sky);
	TICK;
	ground_read(cam_u, cam_v);
	TICK;
	voxel_lift(&V.layers, emu_read8(BN6_CHATBOX) != 0, shown, &V.ground, &V.scene);
	TICK;
	VoxLook look = voxel_look_default();
	look.debug = display_voxel() == DISPLAY_VOXEL_DEBUG;
	voxel_light(&V.scene, &V.ground, &look, V.lit_map, V.lit_sprite);
	TICK;
	VoxCamera cam = { display_voxel_pitch(), display_voxel_zoom(), display_voxel_depth(), (float)(CAM_X + px + py - cam_u), (float)(CAM_Y + (py - px) / 2 - cam_v),
		(float)bn6_player_z() };
	if (!voxel_draw(&V.scene, V.lit_map, V.lit_sprite, &look, &cam, V.scale, V.out)) return false;
	TICK;
	if (getenv("VOXTIME") && P.frame % 60 == 0) { fprintf(stderr, "voxtime"); for (int i = 1; i < tn; ++i) fprintf(stderr, " %.1f", (double)(t[i] - t[i - 1]) * 1000.0 / (double)SDL_GetPerformanceFrequency()); fprintf(stderr, " ms (read, layers+place, world, ground, lift, light, draw)\n"); }
	V.live = P.frame + 1;
	memcpy(hud, V.scene.hud, VOX_N);
	return true;
}
