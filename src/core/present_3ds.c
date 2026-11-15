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
/* the 3D screen's right eye, and what says each frame's spans (docs/engine/stereo.md) */
static C3D_RenderTarget *right_eye;
static bool (*make_spans)(Present3dsStereo *out);

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

void present3ds_stereo_from(bool (*make)(Present3dsStereo *out)) { make_spans = make; }

float present3ds_slider(void) { return display_stereo() && right_eye ? osGet3DSliderState() : 0.0f; }

/* Where the canvas's picture lies on the screen and at what scale */
typedef struct { float x0, y0, sx, s; } Place;

/* The canvas's columns a to b of its rows y to y + rows, at the canvas's
 * column `at`, `w` of its columns wide */
static void span_draw(const Place *p, int a, int b, int y, int rows, float at, float w) {
	Tex3DS_SubTexture t = { (u16)(b - a), (u16)rows, (float)a / TEX, 1.0f - (float)y / TEX, (float)b / TEX, 1.0f - (float)(y + rows) / TEX };
	C2D_Image img = { &tex, &t };
	C2D_DrawImageAt(img, p->x0 + at * p->sx, p->y0 + (float)y * p->s, 0.5f, NULL, p->sx * w / (float)(b - a), p->s);
}

/* Whether a row is the world's whole */
static bool all_world(const Present3dsRow *r) { return r->n <= 1 && (!r->n || !r->flat[0]); }

/* A run of rows alike in their spans, over the sheared world: the gap
 * each world span leaves beside a flat one filled with its edge's column
 * drawn `wide` (under the same shear, it ends where the span begins), or,
 * not `sheared`, the flat spans where they are */
static void rows_draw(const Present3dsRow *r, const Place *p, int y, int rows, float wide, bool sheared) {
	for (int i = 0; i < r->n; ++i) {
		int a = r->x[i], b = i + 1 < r->n ? r->x[i + 1] : cw;
		if (b <= a) continue;
		if (!sheared && r->flat[i]) span_draw(p, a, b, y, rows, (float)a, (float)(b - a));
		if (!sheared || r->flat[i]) continue;
		if (wide > 0 && i > 0) span_draw(p, a, a + 1, y, rows, (float)a - wide, wide);
		if (wide < 0 && i + 1 < r->n) span_draw(p, b - 1, b, y, rows, (float)b, -wide);
	}
}

/* One eye's picture, `eye` -1 the left, +1 the right: the world as one
 * quad sheared by its rows' depth (a row's shift falls evenly to none at
 * the last), its gaps beside the flat spans filled, then the flat spans.
 * Drawn a few rows at a time, a battle's took twice the GPU's calls a
 * frame has room for. */
static void eye_draw(const Present3dsStereo *st, int eye, const Place *p) {
	float deepest = (float)eye * st->shift[0], slope = -deepest * p->sx / (159.0f * p->s);
	C2D_ViewReset();
	C2D_ViewTranslate(deepest * p->sx - slope * p->y0, 0.0f);
	C2D_ViewShear(slope, 0.0f);
	C2D_DrawImageAt((C2D_Image){ &tex, &sub }, p->x0, p->y0, 0.5f, NULL, p->sx, p->s);
	for (int pass = 0; pass < 2; ++pass) {
		if (pass) C2D_ViewReset();
		for (int y = 0, rows; y < ch && y < 160; y += rows) {
			rows = 1;
			while (y + rows < ch && y + rows < 160 && !memcmp(&st->row[y + rows], &st->row[y], sizeof st->row[y])) ++rows;
			if (!all_world(&st->row[y])) rows_draw(&st->row[y], p, y, rows, deepest + (deepest > 0 ? 1.0f : deepest < 0 ? -1.0f : 0.0f), !pass);
		}
	}
}

/* The screen in 3D while Options' 3D depth is on (the slider at 0 shows
 * the left eye alone), and this frame's spans where the slider is up:
 * false where the frame is to be flat */
static bool spans_ready(Present3dsStereo *st, bool *three_d) {
	*three_d = display_stereo() && right_eye;
	if (*three_d != gfxIs3D()) gfxSet3D(*three_d);
	return *three_d && osGet3DSliderState() > 0.0f && make_spans && make_spans(st);
}

void present3ds_frame(bool fill, bool stretch, bool sync) {
	/* (none once the system asks the game to close: the HOME Menu has the
	 * GPU then, and waiting on it hung the console, a frame drawn after
	 * the close) */
	if (!surface || aptShouldClose()) return;
	static Present3dsStereo st;
	bool three_d, layered = spans_ready(&st, &three_d);
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
	Place place = { ((float)GSP_SCREEN_HEIGHT_TOP - cw * sx) / 2, ((float)GSP_SCREEN_WIDTH - ch * s) / 2, sx, s };
	C2D_Image img = { &tex, &sub };
	/* (each eye: the left's on the screen's own target, the right's on its other in 3D) */
	for (int eye = -1; eye <= (three_d ? 1 : -1); eye += 2) {
		C3D_RenderTarget *t = eye < 0 ? top : right_eye;
		C2D_TargetClear(t, C2D_Color32(0, 0, 0, 255));
		C2D_SceneBegin(t);
		if (layered) eye_draw(&st, eye, &place);
		else C2D_DrawImageAt(img, place.x0, place.y0, 0.5f, NULL, sx, s);
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
	C2D_Fini();
	C3D_Fini();
	if (pixels) linearFree(pixels);
	pixels = NULL;
}
#endif
