/* second_frame.h. After BN6's own PET screens: a dark green header band
 * with a light edge, the screen's name in white on its left and the place
 * on a dark slot at its right; a strip of dark slots for HP, Zenny and
 * BugFrags, as BN5 DS's PET carries them in its header; the body framed in
 * the PET's cyan. */
#include "second_frame.h"

#include <stdio.h>

#include "second_state.h"

#define HEAD_H  20   /* the header band */
#define STRIP_Y 24   /* the strip of slots */
#define STRIP_H 16
#define BODY_Y  44   /* the body, to the bottom */

/* A dark slot, BN6's for a value */
static void slot(int x, int y, int w, int h) {
	fill_rect(x, y, w, h, PET_SLOT_EDGE);
	fill_rect(x + 1, y + 1, w - 2, h - 2, PET_SLOT);
}

/* the place: the layer's area and its number, or the town's name */
static void place(int w) {
	char layer[16] = "";
	if (!S2.town) snprintf(layer, sizeof layer, "Layer %d", S2.depth);
	int lw = text_width(layer), aw = text_width(S2.area), gap = lw && aw ? 8 : 0;
	int x = w - 4 - (lw + gap + aw) - 12;
	slot(x, 2, w - 4 - x, HEAD_H - 4);
	text_draw(x + 6, 4, S2.area, PET_CYAN_HI, TEXT_LEFT);
	text_draw(w - 10, 4, layer, PET_WHITE, TEXT_RIGHT);
}

/* the strip: a slot each for HP, Zenny and BugFrags, its name in gold on
 * its left and its value in white on its right */
static void strip(int w) {
	char v[3][16];
	snprintf(v[0], sizeof v[0], "%d/%d", S2.hp, S2.max_hp);
	snprintf(v[1], sizeof v[1], "%uz", S2.zenny);
	snprintf(v[2], sizeof v[2], "%u", S2.bugfrags);
	static const char *const name[3] = { "HP", "Zenny", "BugFrags" };
	int sw = (w - 16) / 3;
	for (int i = 0; i < 3; ++i) {
		int x = 4 + i * (sw + 4);
		slot(x, STRIP_Y, sw, STRIP_H);
		text_draw(x + 5, STRIP_Y + 2, name[i], PET_GOLD, TEXT_LEFT);
		text_draw(x + sw - 5, STRIP_Y + 2, v[i], PET_WHITE, TEXT_RIGHT);
	}
}

SDL_Rect second_frame(int w, int h, const char *title, int slide) {
	/* (each pixel filled once: the panel fills the body, on a 3DS's time) */
	fill_rect(0, 0, w, HEAD_H, PET_DARK);
	fill_rect(0, HEAD_H, w, 1, PET_LINE);
	fill_rect(0, HEAD_H + 1, w, BODY_Y - 3 - HEAD_H, PET_GREEN);
	fill_rect(0, BODY_Y - 2, 2, h - BODY_Y + 2, PET_GREEN);
	fill_rect(w - 2, BODY_Y - 2, 2, h - BODY_Y + 2, PET_GREEN);
	fill_rect(2, h - 2, w - 4, 2, PET_GREEN);
	text_draw(8 - slide, 3, title, PET_WHITE, TEXT_LEFT);
	/* (BN6's three light stripes after a screen's name, slanting up) */
	int sx = 8 - slide + text_width(title) + 10;
	for (int k = 0; k < 3; ++k)
		for (int y = 0; y < 12; ++y) fill_rect(sx + k * 6 + (11 - y) / 2, 4 + y, 2, 1, PET_LINE);
	place(w);
	strip(w);
	/* the body's cyan edge, two pixels, lit along its top */
	SDL_Rect body = { 4, BODY_Y, w - 8, h - BODY_Y - 4 };
	SDL_Rect edge[4] = { { 2, BODY_Y - 2, w - 4, 2 }, { 2, h - 4, w - 4, 2 }, { 2, BODY_Y, 2, body.h }, { w - 4, BODY_Y, 2, body.h } };
	fill_rects(edge, 4, PET_CYAN);
	fill_rect(3, BODY_Y - 1, w - 6, 1, PET_CYAN_HI);
	return body;
}
