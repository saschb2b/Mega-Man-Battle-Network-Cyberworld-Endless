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
	uint8_t io[0x60];
	StereoFlat flat, made_flat;   /* the frame's flat parts, and those the present's spans were last made from */
	float made_sep;
	uint8_t sprites[STEREO_N];    /* the standing sprites' pixels */
	StereoBodies bodies;
	bool sky;           /* the moving background behind (a layer's, a battle's: in town BG3 may be the map's) */
	bool kept;          /* the frame drawn's parts read */
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
	/* (VRAM's backgrounds and OAM read where the core keeps them: copied, 65 KB a frame cost the 3DS a millisecond) */
	const uint8_t *vram = emu_memory(0x06000000u, 0x10000), *oam = emu_memory(0x07000000u, 0x400);
	if (!vram || !oam) return;
	emu_read_bytes(0x04000000u, st->io, sizeof st->io);
	GbaVideo video = { vram, oam, st->io, NULL };
	int hud_rows = voxel_scene_hud_rows(kind);
	stereo_flat(&video, hud_rows, &st->flat);
	if (!st->flat.whole) stereo_sprites(&video, hud_rows, st->sprites, &st->bodies);
	st->sky = kind == VOXEL_SCENE_LAYER || kind == VOXEL_SCENE_BATTLE;
	st->kept = true;
}

/* A pixel's part (stereo.h's STEREO_MAP, ...) by the standing sprites and
 * mGBA's tag on the core's picture (`tagged`, emu_video's) */
static uint8_t part_of(uint32_t tagged, int i) {
	if (st->sprites[i]) return STEREO_SPRITE;
	return st->sky && stereo_sky_tag(tagged) ? STEREO_SKY : STEREO_MAP;
}

/* The two eyes side by side, at 2x */
static void shot_save(const uint32_t *px) {
	enum { SCALE = 2, W = STEREO_W * SCALE, H = STEREO_H * SCALE };
	uint32_t *eye = malloc(sizeof *eye * W * H);
	uint8_t *part = malloc(STEREO_N);
	SDL_Surface *out = SDL_CreateRGBSurfaceWithFormat(0, W * 2, H, 32, SDL_PIXELFORMAT_ABGR8888);
	if (eye && part && out) {
		const uint32_t *tags = emu_video();
		for (int i = 0; i < STEREO_N; ++i) part[i] = part_of(tags[i], i);
		StereoPicture pic = { px, part, &st->flat, &st->bodies, st->sky };
		for (int side = 0; side < 2; ++side) {
			stereo_compose(&pic, side ? 1 : -1, SEP_MOST, SCALE, eye);
			for (int y = 0; y < H; ++y) memcpy((uint8_t *)out->pixels + y * out->pitch + side * W * 4, &eye[y * W], W * 4);
		}
		printf("stereo: %s, %d of the HUD's sprites flat\n", SDL_SaveBMP(out, shot_path) ? SDL_GetError() : shot_path, st->flat.nrects);
	}
	if (out) SDL_FreeSurface(out);
	free(part);
	free(eye);
}

#ifdef __3DS__
/* The 3DS's present asks at each frame: each row's spans and shift */
static bool make(Present3dsStereo *out) {
	/* (a screen of menus flat: the picture as it is in both eyes) */
	if (!st || st->live != P.frame + 1 || st->flat.whole) return false;
	float sep = SEP_MOST * slider();
	out->map_first = stereo_shift(1, sep, stereo_map_depth(0));
	out->map_last = stereo_shift(1, sep, stereo_map_depth(STEREO_H - 1));
	out->sky_first = stereo_shift(1, sep, stereo_sky_depth(0));
	out->sky_last = stereo_shift(1, sep, stereo_sky_depth(STEREO_H - 1));
	out->sky_on = st->sky;
	out->nbodies = st->bodies.n < PRESENT3DS_BODIES ? st->bodies.n : PRESENT3DS_BODIES;
	for (int b = 0; b < out->nbodies; ++b) {
		const StereoBox *box = &st->bodies.box[b];
		out->box[b][0] = box->x0, out->box[b][1] = box->y0, out->box[b][2] = box->x1, out->box[b][3] = box->y1;
		out->box_shift[b] = stereo_shift(1, sep, stereo_body_depth(box));
	}
	/* (the spans the present keeps, where the flat parts are as they were: made anew, up to 1.7 ms a frame) */
	if (!memcmp(&st->flat, &st->made_flat, sizeof st->flat) && st->made_sep > 0) return true;
	st->made_flat = st->flat;
	st->made_sep = sep;
	memset(out->row, 0, sizeof out->row);
	for (int y = 0; y < STEREO_H; ++y) {
		StereoRow r;
		stereo_row(&st->flat, y, &r);
		out->row[y].n = r.n;
		for (int i = 0; i < r.n; ++i) {
			out->row[y].x[i] = r.x[i];
			out->row[y].flat[i] = r.flat[i];
		}
	}
	return true;
}

const uint8_t *stereo_scene_sprites(void) { return st && st->kept && !st->flat.whole && slider() > 0.0f ? st->sprites : NULL; }

bool stereo_scene_sky(void) { return st && st->sky; }
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
