/* present_3ds.h. SDL's own 3DS present copied and turned every pixel of
 * the screen on the CPU, 13 ms of a frame; here the GPU's copy engine
 * moves the canvas into a texture and the GPU draws it (citro2d). */
#include "present_3ds.h"

#ifdef __3DS__
#include <string.h>

#include <citro2d.h>

#include "display.h"

#define TEX 256   /* the texture's side, the canvas's rows 256 pixels apart */

static C3D_RenderTarget *top, *bottom;
static C3D_Tex tex;
static Tex3DS_SubTexture sub;
static u32 *pixels;   /* linear memory, which the GPU's copy engine reads */
static SDL_Surface *surface;
static int cw, ch, cleared;
/* the bottom screen's picture (issue #9): 320 x 240 in a 512 x 256 texture */
#define BOT_W 512
#define BOT_H 256
static C3D_Tex bot_tex;
static Tex3DS_SubTexture bot_sub;
static u32 *bot_pixels;
static bool bot_ready, bot_on, bot_new;
static int bot_draws;   /* frames left to draw it in, both of the screen's buffers */
/* the 3D screen's right eye, and the three layers (docs/engine/stereo.md) */
static C3D_RenderTarget *right_eye;
static C3D_Tex layer_tex[3];
static u32 *layer_px[3];
static bool layers_ready, layers_tried;
static Tex3DS_SubTexture row_sub[160], box_sub[PRESENT3DS_BODIES];
static bool (*make_layers)(const uint32_t *canvas, int pitch, Present3dsStereo *out);

SDL_Surface *present3ds_init(int w, int h) {
	if (w > TEX || h > TEX) return NULL;
	cw = w;
	ch = h;
	pixels = linearAlloc(TEX * TEX * 4);
	if (!pixels) return NULL;
	memset(pixels, 0, TEX * TEX * 4);
	if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE) || !C2D_Init(C2D_DEFAULT_MAX_OBJECTS)) return NULL;
	C2D_Prepare();
	/* (SDL made the screens RGBA8, where citro2d's own targets write RGB8) */
	u32 out = GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |
		GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) |
		GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO);
	top = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_TOP, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	bottom = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_BOTTOM, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	if (!top || !bottom) return NULL;
	C3D_RenderTargetSetOutput(top, GFX_TOP, GFX_LEFT, out);
	C3D_RenderTargetSetOutput(bottom, GFX_BOTTOM, GFX_LEFT, out);
	/* (the right eye's: its own target, shown where the screen is in 3D) */
	right_eye = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_TOP, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	if (right_eye) C3D_RenderTargetSetOutput(right_eye, GFX_TOP, GFX_RIGHT, out);
	if (!C3D_TexInit(&tex, TEX, TEX, GPU_RGBA8)) return NULL;
	C3D_TexSetWrap(&tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
	/* (the canvas's first row is the texture's top as the copy leaves it:
	 * turned over by the copy, it drew upside down in the screen's foot) */
	sub = (Tex3DS_SubTexture){ (u16)w, (u16)h, 0.0f, 1.0f, (float)w / TEX, 1.0f - (float)h / TEX };
	surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels, w, h, 32, TEX * 4, SDL_PIXELFORMAT_RGBA8888);
	return surface;
}

void *present3ds_bottom(int *pitch) {
	if (!bot_ready) {
		bot_ready = true;   /* (tried once) */
		bot_pixels = linearAlloc(BOT_W * BOT_H * 4);
		if (!bot_pixels || !C3D_TexInit(&bot_tex, BOT_W, BOT_H, GPU_RGBA8)) {
			if (bot_pixels) linearFree(bot_pixels);
			bot_pixels = NULL;
			return NULL;
		}
		memset(bot_pixels, 0, BOT_W * BOT_H * 4);
		C3D_TexSetWrap(&bot_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
		C3D_TexSetFilter(&bot_tex, GPU_NEAREST, GPU_NEAREST);
		bot_sub = (Tex3DS_SubTexture){ GSP_SCREEN_HEIGHT_BOTTOM, GSP_SCREEN_WIDTH, 0.0f, 1.0f,
			(float)GSP_SCREEN_HEIGHT_BOTTOM / BOT_W, 1.0f - (float)GSP_SCREEN_WIDTH / BOT_H };
	}
	*pitch = BOT_W * 4;
	return bot_pixels;
}

void present3ds_bottom_show(bool on) {
	/* (black again: both of its buffers cleared once more) */
	if (bot_on && !on) cleared = 0;
	bot_on = on && bot_pixels;
	bot_new |= bot_on;
}

/* (made at the first frame in 3D: three textures no player pays for with it off) */
static bool layers_make(void) {
	if (layers_tried) return layers_ready;
	layers_tried = true;
	for (int n = 0; n < 3; ++n) {
		layer_px[n] = linearAlloc(TEX * TEX * 4);
		if (!layer_px[n] || !C3D_TexInit(&layer_tex[n], TEX, TEX, GPU_RGBA8)) return false;
		memset(layer_px[n], 0, TEX * TEX * 4);
		C3D_TexSetWrap(&layer_tex[n], GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
	}
	for (int y = 0; y < 160; ++y)
		row_sub[y] = (Tex3DS_SubTexture){ (u16)cw, 1, 0.0f, 1.0f - (float)y / TEX, (float)cw / TEX, 1.0f - (float)(y + 1) / TEX };
	layers_ready = true;
	return true;
}

uint32_t *present3ds_layer(int n, int *pitch) {
	*pitch = TEX;
	return n >= 0 && n < 3 && layers_make() ? (uint32_t *)layer_px[n] : NULL;
}

void present3ds_stereo_from(bool (*make)(const uint32_t *canvas, int pitch, Present3dsStereo *out)) { make_layers = make; }

float present3ds_slider(void) { return display_stereo() && right_eye ? osGet3DSliderState() : 0.0f; }

static void layer_copy(int n) {
	GSPGPU_FlushDataCache(layer_px[n], TEX * TEX * 4);
	C3D_SyncDisplayTransfer(layer_px[n], GX_BUFFER_DIM(TEX, TEX), (u32 *)layer_tex[n].data, GX_BUFFER_DIM(TEX, TEX),
		GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) |
		GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) |
		GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));
}

/* One eye's picture from the layers at (x0, y0), `eye` -1 the left, +1 the right */
static void layers_draw(const Present3dsStereo *st, int eye, float x0, float y0, float sx, float s) {
	for (int y = 0; y < ch && y < 160; ++y) {
		C2D_Image row = { &layer_tex[0], &row_sub[y] };
		C2D_DrawImageAt(row, x0 + (float)eye * st->row_shift[y] * sx, y0 + (float)y * s, 0.5f, NULL, sx, s);
	}
	for (int b = 0; b < st->nbodies; ++b) {
		const int16_t *box = st->box[b];
		box_sub[b] = (Tex3DS_SubTexture){ (u16)(box[2] - box[0] + 1), (u16)(box[3] - box[1] + 1), (float)box[0] / TEX, 1.0f - (float)box[1] / TEX,
			(float)(box[2] + 1) / TEX, 1.0f - (float)(box[3] + 1) / TEX };
		C2D_Image body = { &layer_tex[1], &box_sub[b] };
		C2D_DrawImageAt(body, x0 + ((float)box[0] + (float)eye * st->box_shift[b]) * sx, y0 + (float)box[1] * s, 0.5f, NULL, sx, s);
	}
	C2D_Image flat = { &layer_tex[2], &sub };
	C2D_DrawImageAt(flat, x0, y0, 0.5f, NULL, sx, s);
}

/* The screen in 3D while Options' 3D depth is on (the slider at 0 shows
 * the left eye alone), and this frame's layers made and copied where the
 * slider is up: false where the frame is to be flat */
static bool layers_ready_for(bool fill, Present3dsStereo *st, bool *three_d) {
	*three_d = display_stereo() && right_eye;
	if (*three_d != gfxIs3D()) gfxSet3D(*three_d);
	if (!*three_d || osGet3DSliderState() <= 0.0f || !make_layers || !layers_make() || !make_layers((const uint32_t *)pixels, TEX, st)) return false;
	for (int n = 0; n < 3; ++n) {
		C3D_TexSetFilter(&layer_tex[n], fill ? GPU_LINEAR : GPU_NEAREST, fill ? GPU_LINEAR : GPU_NEAREST);
		layer_copy(n);
	}
	return true;
}

void present3ds_frame(bool fill, bool stretch, bool sync) {
	/* (none once the system asks the game to close: the HOME Menu has the
	 * GPU then, and waiting on it hung the console, a frame drawn after
	 * the close) */
	if (!surface || aptShouldClose()) return;
	static Present3dsStereo st;
	bool three_d, layered = layers_ready_for(fill, &st, &three_d);
	GSPGPU_FlushDataCache(pixels, TEX * TEX * 4);
	C3D_SyncDisplayTransfer(pixels, GX_BUFFER_DIM(TEX, TEX), (u32 *)tex.data, GX_BUFFER_DIM(TEX, TEX),
		GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) |
		GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) |
		GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));
	/* (the bottom screen's picture the same way, as it changes) */
	if (bot_on && bot_new) {
		GSPGPU_FlushDataCache(bot_pixels, BOT_W * BOT_H * 4);
		C3D_SyncDisplayTransfer(bot_pixels, GX_BUFFER_DIM(BOT_W, BOT_H), (u32 *)bot_tex.data, GX_BUFFER_DIM(BOT_W, BOT_H),
			GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) |
			GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) |
			GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));
		bot_new = false;
		bot_draws = 2;
	}
	GPU_TEXTURE_FILTER_PARAM f = fill ? GPU_LINEAR : GPU_NEAREST;
	C3D_TexSetFilter(&tex, f, f);
	/* (the GPU swaps the picture in at the screen's next refresh. With the
	 * GBA core on the main core, waiting for that refresh here cost a frame
	 * slower than one refresh a second; with the core on its own, since its
	 * wait for VBlank is halted, a frame takes less than a refresh, and the
	 * wait paces the frames evenly, where the main loop's clock alone had
	 * five to nine a second come early or late, issue #35) */
	C3D_FrameBegin(sync ? C3D_FRAME_SYNCDRAW : 0);
	float s = fill ? (float)GSP_SCREEN_WIDTH / (float)ch : 1.0f, sx = stretch ? (float)GSP_SCREEN_HEIGHT_TOP / (float)cw : s;
	float x0 = ((float)GSP_SCREEN_HEIGHT_TOP - cw * sx) / 2, y0 = ((float)GSP_SCREEN_WIDTH - ch * s) / 2;
	C2D_Image img = { &tex, &sub };
	/* (each eye: the left's on the screen's own target, the right's on its other in 3D) */
	for (int eye = -1; eye <= (three_d ? 1 : -1); eye += 2) {
		C3D_RenderTarget *t = eye < 0 ? top : right_eye;
		C2D_TargetClear(t, C2D_Color32(0, 0, 0, 255));
		C2D_SceneBegin(t);
		if (layered) layers_draw(&st, eye, x0, y0, sx, s);
		else C2D_DrawImageAt(img, x0, y0, 0.5f, NULL, sx, s);
	}
	/* the bottom screen: a new picture, else black, in both of its buffers
	 * once (a screen not drawn keeps its picture; drawn every frame, it
	 * cost the GPU's time each one) */
	if (bot_on && bot_draws > 0) {
		C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));
		C2D_SceneBegin(bottom);
		C2D_Image b = { &bot_tex, &bot_sub };
		C2D_DrawImageAt(b, 0, 0, 0.5f, NULL, 1.0f, 1.0f);
		--bot_draws;
	} else if (!bot_on && cleared < 2) {
		C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));
		C2D_SceneBegin(bottom);
		++cleared;
	}
	C3D_FrameEnd(0);
}

void present3ds_exit(void) {
	if (surface) SDL_FreeSurface(surface);
	surface = NULL;
	if (bot_pixels) { C3D_TexDelete(&bot_tex); linearFree(bot_pixels); }
	bot_pixels = NULL;
	for (int n = 0; n < 3; ++n) {
		if (layer_px[n] && layers_ready) C3D_TexDelete(&layer_tex[n]);
		if (layer_px[n]) linearFree(layer_px[n]);
		layer_px[n] = NULL;
	}
	C2D_Fini();
	C3D_Fini();
	if (pixels) linearFree(pixels);
	pixels = NULL;
}
#endif
