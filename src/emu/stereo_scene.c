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
 * frame's pixels: 15 of the 3DS's screen at its 1.5x (9 first: the owner
 * found the slider's part in it too small to see) */
#define SEP_MOST 10.0f

/* (made at the first frame in 3D: none of it where the 3D is off) */
static struct Stereo {
	uint8_t vram[0x10000], oam[0x400], io[0x60];   /* the backgrounds' part of VRAM */
	StereoFlat flat;
	bool kept;          /* the frame drawn's flat parts read */
	uint64_t live;      /* P.frame + 1 of the frame drawn in 3D */
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

/* (after each frame the core ends, before the next begins: what of it is
 * flat, from its sprites' table and its HUD backgrounds' maps, where the
 * drawing's own reads would wait for the next frame) */
static void keep(void) {
	if (!st) return;
	st->kept = false;
	int kind = slider() > 0.0f ? voxel_scene_kind() : VOXEL_SCENE_NONE;
	if (kind == VOXEL_SCENE_NONE) return;
	emu_read_bytes(0x06000000u, st->vram, sizeof st->vram);
	emu_read_bytes(0x07000000u, st->oam, sizeof st->oam);
	emu_read_bytes(0x04000000u, st->io, sizeof st->io);
	stereo_flat(&(GbaVideo){ st->vram, st->oam, st->io, NULL }, voxel_scene_hud_rows(kind), &st->flat);
	st->kept = true;
}

/* The two eyes side by side, at 2x */
static void shot_save(const uint32_t *px) {
	enum { SCALE = 2, W = STEREO_W * SCALE, H = STEREO_H * SCALE };
	uint32_t *eye = malloc(sizeof *eye * W * H);
	SDL_Surface *out = SDL_CreateRGBSurfaceWithFormat(0, W * 2, H, 32, SDL_PIXELFORMAT_ABGR8888);
	if (eye && out) {
		for (int side = 0; side < 2; ++side) {
			stereo_compose(px, &st->flat, side ? 1 : -1, SEP_MOST, SCALE, eye);
			for (int y = 0; y < H; ++y) memcpy((uint8_t *)out->pixels + y * out->pitch + side * W * 4, &eye[y * W], W * 4);
		}
		printf("stereo: %s, %d of the HUD's sprites flat\n", SDL_SaveBMP(out, shot_path) ? SDL_GetError() : shot_path, st->flat.nrects);
	}
	if (out) SDL_FreeSurface(out);
	free(eye);
}

#ifdef __3DS__
/* The 3DS's present asks at each frame: each row's spans and shift */
static bool make(Present3dsStereo *out) {
	if (!st || st->live != P.frame + 1) return false;
	memset(out, 0, sizeof *out);
	float sep = SEP_MOST * slider();
	for (int y = 0; y < STEREO_H; ++y) {
		StereoRow r;
		stereo_row(&st->flat, y, &r);
		out->row[y].n = r.n;
		for (int i = 0; i < r.n; ++i) {
			out->row[y].x[i] = r.x[i];
			out->row[y].flat[i] = r.flat[i];
		}
		out->shift[y] = stereo_shift(1, sep, y);
	}
	return true;
}
#endif

void stereo_scene_frame(const uint32_t *px) {
	if (slider() <= 0.0f) {
		if (st) st->live = 0;
		return;
	}
	if (!st) {
		/* (the first frame went by unread: the next is the first in 3D) */
		st = calloc(1, sizeof *st);
		if (!st) return;
		emu_after_frame(keep);
#ifdef __3DS__
		present3ds_stereo_from(make);
#endif
		return;
	}
	st->live = st->kept ? P.frame + 1 : 0;
	if (st->kept && shot_wanted() && (long)P.frame == shot_at) shot_save(px);
}
