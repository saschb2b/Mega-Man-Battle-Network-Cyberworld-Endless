/* present_3ds.h. SDL's own 3DS present copied and turned every pixel of
 * the screen on the CPU, 13 ms of a frame; here the GPU's copy engine
 * moves the canvas into a texture and the GPU draws it (citro2d). */
#include "present_3ds.h"

#ifdef __3DS__
#include <string.h>

#include <citro2d.h>

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

void present3ds_frame(bool fill) {
	/* (none once the system asks the game to close: the HOME Menu has the
	 * GPU then, and waiting on it hung the console, a frame drawn after
	 * the close) */
	if (!surface || aptShouldClose()) return;
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
	}
	GPU_TEXTURE_FILTER_PARAM f = fill ? GPU_LINEAR : GPU_NEAREST;
	C3D_TexSetFilter(&tex, f, f);
	/* (the GPU swaps the picture in at the screen's next refresh; waiting
	 * for that refresh here cost a frame slower than one refresh a second) */
	C3D_FrameBegin(0);
	C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
	C2D_SceneBegin(top);
	float s = fill ? (float)GSP_SCREEN_WIDTH / (float)ch : 1.0f;
	C2D_Image img = { &tex, &sub };
	C2D_DrawImageAt(img, ((float)GSP_SCREEN_HEIGHT_TOP - cw * s) / 2, ((float)GSP_SCREEN_WIDTH - ch * s) / 2, 0.5f, NULL, s, s);
	/* the bottom screen: its picture, else black, both of its buffers once */
	if (bot_on) {
		C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));
		C2D_SceneBegin(bottom);
		C2D_Image b = { &bot_tex, &bot_sub };
		C2D_DrawImageAt(b, 0, 0, 0.5f, NULL, 1.0f, 1.0f);
	} else if (cleared < 2) {
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
	C2D_Fini();
	C3D_Fini();
	if (pixels) linearFree(pixels);
	pixels = NULL;
}
#endif
