/* map_label.h. Moved out of online_labels.c (the shared dive's name tags,
 * issue #712). */
#include "map_label.h"
#include "pet_look.h"

#include "bn6.h"
#include "bn6_fields.h"
#include "emu.h"
#include "gfx.h"
#include "pixfont.h"
#include "platform.h"

#define HEAD 31   /* a Navi's head over his feet, in the picture's rows */
#define TOP 18    /* the picture's top rows a label keeps off */
#define FOOT 20   /* ... and its foot's */

bool map_label_camera(int camera[3]) {
	uint32_t at = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CAMERA);
	if (at < BN6_EWRAM || at > BN6_EWRAM_END - 80) return false;
	camera[0] = (int32_t)emu_read32(at + BN6_CAMERA_RENDER_X) >> 16;
	camera[1] = (int32_t)emu_read32(at + BN6_CAMERA_RENDER_Y) >> 16;
	camera[2] = (int32_t)emu_read32(at + BN6_CAMERA_RENDER_Z) >> 16;
	return true;
}

void map_label_floor(int x, int y, int z, const int camera[3], int *px, int *py) {
	/* BN6's overworld projection (bn6f sub_300631C), including the
	 * displayed camera rather than assuming MegaMan stays centered. */
	*px = 120 + x - camera[0] + y - camera[1];
	*py = 82 + (y - camera[1] - x + camera[0]) / 2 - z + camera[2];
}

void map_label_at(int x, int y, int z, const int camera[3], int *px, int *py) {
	map_label_floor(x, y, z, camera, px, py);
	*py -= HEAD;
}

bool map_label_shown(int px, int py) { return px >= 0 && px < EMU_W && py >= TOP && py <= EMU_H - FOOT; }

int map_label_x(int px, const char *text) {
	int w = pixfont_width(text, 1);
	if (px < w / 2 + 2) px = w / 2 + 2;
	if (px > EMU_W - w / 2 - 2) px = EMU_W - w / 2 - 2;
	return px;
}

int map_label_plate(int px, int py, const char *text, SDL_Color colour) {
	int w = pixfont_width(text, 1), x = map_label_x(px, text);
	fill_rect(P.core_x + x - w / 2 - 2, P.core_y + py - 1, w + 4, MAP_LABEL_ROW - 1, rgba(9, 24, 42, 210));
	pixfont_draw(P.core_x + x, P.core_y + py, text, colour, BLACK, PIXFONT_CENTER, 1);
	return x;
}

void map_label_cursor(int px, int py) {
	/* (the PET's own cursor, pointing at him from his left) */
	look_cursor(P.core_x + px - 14, P.core_y + py);
}
