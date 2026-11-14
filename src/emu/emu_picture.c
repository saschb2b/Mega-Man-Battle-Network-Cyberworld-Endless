/* The core's picture on the canvas (emu.h's emu_picture_draw): BN6's frame
 * as emu_video has it, the town's hour and a liberation's dark panels on
 * its layers, then the GBA's screen (lcd_frame.h: mGBA's Color Correction
 * and Interframe Blending, which a frontend runs on the core's frame), at
 * the canvas's (x, y). scene_emu.c's draw puts the rest over it. */
#include "emu.h"

#include <SDL.h>

#include "director.h"
#include "lcd_frame.h"
#include "online_visit.h"
#include "platform.h"
#ifdef __3DS__
#include "present_3ds.h"
#endif
#include "stereo_scene.h"
#include "voxel_scene.h"

/* The town's hour on its picture (docs/world/home/time-of-day.md): each colour
 * channel of the map's layers and of the sprites over it times the hour's
 * (in 256ths: afternoon a touch warm, evening orange, night a dim blue).
 * mGBA's top byte names a pixel's layer: a background's has 0x08 and its
 * index at bit 4 (BG0, the chat's text, kept), a sprite's none; with a chat
 * box open the sprites in its rows (its frame and face) keep theirs too. */
#define TINT_BOX_TOP 96
static uint32_t tinted(uint32_t c, int hour, bool box_row) {
	static const uint16_t by[4][3] = { { 256, 256, 256 }, { 262, 246, 222 }, { 270, 196, 150 }, { 104, 124, 184 } };
	uint32_t flags = c >> 24;
	if ((flags & 0x08) ? (flags >> 4 & 3) == 0 : box_row) return c;
	uint32_t r = (c & 0xFF) * by[hour][0] >> 8, g = (c >> 8 & 0xFF) * by[hour][1] >> 8, b = (c >> 16 & 0xFF) * by[hour][2] >> 8;
	return (c & 0xFF000000u) | (b > 255 ? 255 : b) << 16 | (g > 255 ? 255 : g) << 8 | (r > 255 ? 255 : r);
}

/* The town's hour on colours of the world, as on its picture's (voxel
 * mode's, which draws the map itself) */
void emu_picture_tint_world(uint32_t *px, int n) {
	int hour = online_visit_away() ? 0 : director_town_tint(&(bool){ false });
	for (int i = 0; hour && i < n; ++i) px[i] = tinted(px[i] | 0x18000000u, hour, false) | 0xFF000000u;
}

static SDL_Texture *tex;

#ifdef __3DS__
/* The picture (mGBA's colours, 0x??BBGGRR) into the 3DS's canvas, RGBA8888,
 * its (0, 0) at the canvas's (x, y), what falls off it left out: past
 * SDL's queue of draws, flushed first so the scene's black stays under it */
static void picture_into(SDL_Surface *canvas, const uint32_t *px, int x, int y) {
	SDL_RenderFlush(P.renderer);
	for (int j = 0; j < EMU_H; ++j) {
		int cy = y + j, from = x < 0 ? -x : 0, to = canvas->w - x < EMU_W ? canvas->w - x : EMU_W;
		if (cy < 0 || cy >= canvas->h) continue;
		uint32_t *row = (uint32_t *)((uint8_t *)canvas->pixels + cy * canvas->pitch) + x;
		for (int i = from; i < to; ++i) {
			uint32_t c = px[j * EMU_W + i];
			row[i] = (c & 0xFFu) << 24 | (c & 0xFF00u) << 8 | (c >> 8 & 0xFF00u) | 0xFFu;
		}
	}
}
#endif

/* (a part of the picture timed for the frame log, and named while it runs: a read of the game in it that waits is told by it) */
#define PART(name, call) do { \
		uint64_t t0_ = frame_part_begin(); \
		const char *was_ = emu_draw_part; \
		emu_draw_part = name; \
		call; \
		emu_draw_part = was_; \
		frame_part_end(name, t0_); \
	} while (0)

void emu_picture_draw(int x, int y) {
	/* (on the 3DS in the canvas's order, so the copy onto it converts
	 * nothing: mGBA's red and blue swapped here, where a computer's GPU
	 * converts them) */
#ifdef __3DS__
	const Uint32 format = SDL_PIXELFORMAT_ARGB8888;
#else
	const Uint32 format = SDL_PIXELFORMAT_ABGR8888;
#endif
	if (!tex) {
		tex = SDL_CreateTexture(P.renderer, format, SDL_TEXTUREACCESS_STREAMING, EMU_W, EMU_H);
		SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
	}
	/* mGBA keeps layer flags in the top byte; GL renderers read it as alpha */
	static uint32_t px[EMU_W * EMU_H];
	const uint32_t *v = emu_video();
	bool box = false;
	int hour = online_visit_away() ? 0 : director_town_tint(&box);
	PART("picture: tint", for (int i = 0; i < EMU_W * EMU_H; ++i) px[i] = hour ? tinted(v[i], hour, box && i / EMU_W >= TINT_BOX_TOP) : v[i]);
	/* (a liberation's dark panels, docs/game/liberation/) */
	PART("picture: board", director_tint_board(px));
	/* (a Glitch layer's, docs/game/glitch-layer.md) */
	PART("picture: glitch", director_glitch_frame(px));
	/* (each picture the core made blended once, as a frontend blends each
	 * frame it is given; the game paused, the last one's frame again) */
	static uint32_t seen;
	uint32_t made = emu_pictures();
	PART("picture: lcd", lcd_frame(px, EMU_W * EMU_H, made != seen));
	seen = made;
#ifndef __3DS__
	/* (voxel mode, voxel_scene.h: the world drawn under the canvas, which
	 * keeps the HUD's pixels and is clear at the rest; not the 3DS's) */
	static uint8_t hud[EMU_W * EMU_H];
	bool voxel = voxel_scene_frame(px, hud);
#else
	/* (straight into the canvas's memory: through a texture and SDL's copy
	 * onto the canvas, a copy each way, it took 1.7 ms of the 3DS's frame) */
	SDL_Surface *canvas = present3ds_canvas();
	if (canvas) {
		PART("picture: 3d", stereo_scene_frame(px));
		PART("picture: onto the canvas", picture_into(canvas, px, x, y));
		return;
	}
#endif
	for (int i = 0; i < EMU_W * EMU_H; ++i) {
		uint32_t c = px[i];
#ifdef __3DS__
		px[i] = 0xFF000000u | (c & 0xFF00u) | (c & 0xFFu) << 16 | (c >> 16 & 0xFFu);
#else
		px[i] = voxel && !hud[i] ? 0 : c | 0xFF000000u;
#endif
	}
	/* (the 3DS's 3D depth, stereo_scene.h: the frame's rows in depth at its present) */
	PART("picture: 3d", stereo_scene_frame(px));
	SDL_Rect dst = { x, y, EMU_W, EMU_H };
	PART("picture: onto the canvas", SDL_UpdateTexture(tex, NULL, px, EMU_W * 4); SDL_RenderCopy(P.renderer, tex, NULL, &dst));
}
