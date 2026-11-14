#include "stereo_scene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "display.h"
#include "emu.h"
#include "gba_layers.h"
#include "platform.h"
#include "stereo.h"
#include "voxel_scene.h"
#ifdef __3DS__
#include "present_3ds.h"
#endif

/* The eyes' parting at the deepest with the slider at its top, in the
 * frame's pixels: 9 of the 3DS's screen at its 1.5x */
#define SEP_MOST 6.0f

/* (made at the first frame in 3D: none of it where the 3D is off) */
static struct Stereo {
	uint8_t vram[0x18000], oam[0x400], io[0x60], pal[0x400];
	GbaLayers layers;
	StereoFrame frame;
	uint32_t picture[STEREO_N];   /* the game's picture as the canvas has it */
	int x, y, hud_rows;
	bool kept;                    /* the video memory of the frame drawn kept */
	uint64_t live;                /* P.frame + 1 of the frame sorted */
} *st;

/* CYBERWORLD_STEREO_SHOT's frame and file (-1: none) */
static long shot_at = -2;
static char shot_path[512];

static bool shot_wanted(void) {
	if (shot_at == -2) {
		const char *e = getenv("CYBERWORLD_STEREO_SHOT");
		const char *colon = e ? strchr(e, ':') : NULL;
		shot_at = colon ? atol(e) : -1;
		if (colon) snprintf(shot_path, sizeof shot_path, "%s", colon + 1);
	}
	return shot_at >= 0 && (long)P.frame <= shot_at;
}

/* The 3D slider: the 3DS's, or all the way up for the shot */
static float slider(void) {
#ifdef __3DS__
	if (present3ds_slider() > 0.0f) return present3ds_slider();
#endif
	return shot_wanted() ? 1.0f : 0.0f;
}

/* (after each frame the core ends: its video memory, and what the frame is of, kept for the drawing) */
static void keep(void) {
	if (!st) return;
	st->kept = false;
	int kind = slider() > 0.0f ? voxel_scene_kind() : VOXEL_SCENE_NONE;
	if (kind == VOXEL_SCENE_NONE) return;
	st->hud_rows = voxel_scene_hud_rows(kind);
	emu_read_bytes(0x06000000u, st->vram, sizeof st->vram);
	emu_read_bytes(0x07000000u, st->oam, sizeof st->oam);
	emu_read_bytes(0x04000000u, st->io, sizeof st->io);
	emu_read_bytes(0x05000000u, st->pal, sizeof st->pal);
	st->kept = true;
}

/* The canvas's colour of the picture's pixel: the 3DS's canvas is RGBA8888
 * (its picture ARGB), elsewhere the picture's own (ABGR) */
static uint32_t as_canvas(uint32_t c) {
#ifdef __3DS__
	return c << 8 | 0xFFu;
#else
	return c;
#endif
}

static const uint32_t ALPHA =
#ifdef __3DS__
	0x000000FFu;
#else
	0xFF000000u;
#endif

/* The two eyes side by side, at 2x */
static void shot_save(void) {
	enum { SCALE = 2, W = STEREO_W * SCALE, H = STEREO_H * SCALE };
	uint32_t *layer = malloc(sizeof *layer * STEREO_N * 3), *eye = malloc(sizeof *eye * W * H);
	SDL_Surface *out = SDL_CreateRGBSurfaceWithFormat(0, W * 2, H, 32, SDL_PIXELFORMAT_ABGR8888);
	if (layer && eye && out) {
		StereoLayers l = { layer, layer + STEREO_N, layer + STEREO_N * 2, STEREO_W, ALPHA, st->x, st->y, { 0 } };
		stereo_layers(&st->frame, st->picture, STEREO_W, st->picture, &l);
		for (int side = 0; side < 2; ++side) {
			stereo_compose(&l, &st->frame, side ? 1 : -1, SEP_MOST, SCALE, eye);
			for (int y = 0; y < H; ++y) memcpy((uint8_t *)out->pixels + y * out->pitch + side * W * 4, &eye[y * W], W * 4);
		}
		printf("stereo: %s, %d bodies\n", SDL_SaveBMP(out, shot_path) ? SDL_GetError() : shot_path, st->frame.nbodies);
	}
	if (out) SDL_FreeSurface(out);
	free(eye);
	free(layer);
}

#ifdef __3DS__
/* The 3DS's present asks at each frame: the layers from the canvas, each
 * row's and body's shift (the bodies deepest first) */
static bool make(const uint32_t *canvas, int pitch, Present3dsStereo *out) {
	if (!st || st->live != P.frame + 1) return false;
	int lp = 0;
	StereoLayers l = { present3ds_layer(0, &lp), present3ds_layer(1, &lp), present3ds_layer(2, &lp), 0, ALPHA, st->x, st->y, { 0 } };
	if (!l.world || !l.bodies || !l.flat) return false;
	l.pitch = lp;
	stereo_layers(&st->frame, canvas, pitch, st->picture, &l);
	float sep = SEP_MOST * slider();
	for (int y = 0; y < STEREO_H; ++y) out->row_shift[y] = l.row_world[y] ? stereo_shift(1, sep, y) : 0.0f;
	bool done[STEREO_BODIES] = { false };
	out->nbodies = 0;
	for (int k = 0; k < st->frame.nbodies && k < PRESENT3DS_BODIES; ++k) {
		int next = -1;
		for (int b = 0; b < st->frame.nbodies; ++b)
			if (!done[b] && (next < 0 || st->frame.body[b].y1 < st->frame.body[next].y1)) next = b;
		done[next] = true;
		const StereoBox *b = &st->frame.body[next];
		int x0 = b->x0 + st->x, y0 = b->y0 + st->y, x1 = b->x1 + st->x, y1 = b->y1 + st->y;
		if (x0 < 0) x0 = 0;
		if (y0 < 0) y0 = 0;
		if (x1 >= STEREO_W) x1 = STEREO_W - 1;
		if (y1 >= STEREO_H) y1 = STEREO_H - 1;
		if (x0 > x1 || y0 > y1) continue;
		int16_t *box = out->box[out->nbodies];
		box[0] = (int16_t)x0, box[1] = (int16_t)y0, box[2] = (int16_t)x1, box[3] = (int16_t)y1;
		out->box_shift[out->nbodies++] = stereo_shift(1, sep, b->y1 + st->y);
	}
	return true;
}
#endif

void stereo_scene_frame(const uint32_t *px, int x, int y) {
	if (slider() <= 0.0f) {
		if (st) st->live = 0;
		return;
	}
	if (!st) {
		/* (the first frame's memory went by unkept: the next is the first sorted) */
		st = calloc(1, sizeof *st);
		if (!st) return;
		emu_after_frame(keep);
#ifdef __3DS__
		present3ds_stereo_from(make);
#endif
		return;
	}
	GbaVideo v = { st->vram, st->oam, st->io, st->pal };
	if (!st->kept || !gba_layers_sprites(&v, &st->layers)) {
		st->live = 0;
		return;
	}
	stereo_sort(&v, &st->layers, st->hud_rows, &st->frame);
	for (int i = 0; i < STEREO_N; ++i) st->picture[i] = as_canvas(px[i]);
	st->x = x;
	st->y = y;
	st->live = P.frame + 1;
	if (shot_wanted() && (long)P.frame == shot_at) shot_save();
}
