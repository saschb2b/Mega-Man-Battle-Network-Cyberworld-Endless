/* options_scene.h. The scene's picture runs through a blending history of
 * its own (lcd_frame_in), so it blends apart from the paused game's, and
 * lcd3x and smooth motion reach it as they reach the whole screen. */
#include "options_scene.h"

#include "display.h"
#include "emu.h"
#include "options_state.h"
#include "platform.h"
#include "showcase.h"

static struct {
	SDL_Texture *tex;
	uint32_t px[EMU_W * EMU_H];
	LcdHistory *history;
	uint32_t seen;       /* the pictures made when the last was blended */
	bool grabbed;        /* the paused game's picture kept this frame (P.fx_copy) */
} V;

/* the scene for a line: flicker for Frame blending, steady movement for
 * Smooth motion, colour for the rest */
static ShowScene scene_of(int kind) {
	return kind == LINE_BLEND ? SCENE_FLICKER : kind == LINE_SMOOTH ? SCENE_MOTION : SCENE_COLOUR;
}

void options_scene_update(void) {
	if (!O.open) {
		showcase_close();
		lcd_history_free(V.history);
		V.history = NULL;
		return;
	}
	if (O.page != PAGE_DISPLAY || !showcase_here()) return;
	showcase_want(scene_of(options_line_kind()));
	showcase_frame();
}

void options_scene_grab(void) {
	V.grabbed = false;
	if (!P.fx_copy || O.page != PAGE_DISPLAY) return;
	SDL_SetRenderTarget(P.renderer, P.fx_copy);
	SDL_SetTextureBlendMode(P.canvas, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(P.renderer, P.canvas, NULL, NULL);
	SDL_SetTextureBlendMode(P.canvas, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(P.renderer, P.canvas);
	V.grabbed = true;
}

/* the scene's picture through the display's settings, into the texture:
 * false while it has none */
static bool scene_picture(void) {
	uint32_t made;
	const uint32_t *pic = showcase_picture(&made);
	if (!pic) return false;
	/* (the core's R in its low byte, as emu_video's: ABGR8888 to a texture) */
	if (!V.tex) {
		V.tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, EMU_W, EMU_H);
		if (!V.tex) return false;
		SDL_SetTextureBlendMode(V.tex, SDL_BLENDMODE_NONE);
	}
	if (!V.history) V.history = lcd_history_new();
	for (int i = 0; i < EMU_W * EMU_H; ++i) V.px[i] = pic[i] | 0xFF000000u;
	lcd_frame_in(V.history, V.px, EMU_W * EMU_H, made != V.seen);
	V.seen = made;
	SDL_UpdateTexture(V.tex, NULL, V.px, EMU_W * 4);
	return true;
}

void options_scene_draw(SDL_Rect f) {
	int fx, fy;
	showcase_focus(&fx, &fy);
	SDL_Rect src = { fx - f.w / 2, fy - f.h / 2, f.w, f.h };
	if (scene_picture()) SDL_RenderCopy(P.renderer, V.tex, &src, &f);
	else if (V.grabbed) {
		/* (the paused game's own, from its middle) */
		SDL_Rect mid = { P.core_x + (CORE_W - f.w) / 2, P.core_y + (CORE_H - f.h) / 2, f.w, f.h };
		SDL_SetTextureBlendMode(P.fx_copy, SDL_BLENDMODE_NONE);
		SDL_RenderCopy(P.renderer, P.fx_copy, &mid, &f);
	}
}
