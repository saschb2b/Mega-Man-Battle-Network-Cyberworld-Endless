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

static SDL_Texture *tex;

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
	for (int i = 0; i < EMU_W * EMU_H; ++i) px[i] = hour ? tinted(v[i], hour, box && i / EMU_W >= TINT_BOX_TOP) : v[i];
	/* (a liberation's dark panels, docs/game/liberation/) */
	director_tint_board(px);
	/* (a Glitch layer's, docs/game/glitch-layer.md) */
	director_glitch_frame(px);
	/* (each picture the core made blended once, as a frontend blends each
	 * frame it is given; the game paused, the last one's frame again) */
	static uint32_t seen;
	uint32_t made = emu_pictures();
	lcd_frame(px, EMU_W * EMU_H, made != seen);
	seen = made;
#ifndef __3DS__
	/* (voxel mode, voxel_scene.h: the world drawn under the canvas, which
	 * keeps the HUD's pixels and is clear at the rest; not the 3DS's) */
	static uint8_t hud[EMU_W * EMU_H];
	bool voxel = voxel_scene_frame(px, hud);
#endif
	for (int i = 0; i < EMU_W * EMU_H; ++i) {
		uint32_t c = px[i];
#ifdef __3DS__
		px[i] = 0xFF000000u | (c & 0xFF00u) | (c & 0xFFu) << 16 | (c >> 16 & 0xFFu);
#else
		px[i] = voxel && !hud[i] ? 0 : c | 0xFF000000u;
#endif
	}
	SDL_UpdateTexture(tex, NULL, px, EMU_W * 4);
	SDL_Rect dst = { x, y, EMU_W, EMU_H };
	SDL_RenderCopy(P.renderer, tex, NULL, &dst);
}
