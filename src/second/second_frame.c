/* second_frame.h. After BN6's own PET screens: a dark green header band
 * with a light edge, the screen's name in white on its left and the place
 * on a dark slot at its right; a strip of dark slots for HP, Zenny and
 * BugFrags, as BN5 DS's PET carries them in its header; the body framed in
 * the PET's cyan. */
#include "second_frame.h"

#include <stdio.h>
#include <string.h>

#include "second_state.h"

#define HEAD_H  20   /* the header band */
#define STRIP_Y 24   /* the strip of slots */
#define STRIP_H 16
#define BODY_Y  44   /* the body, to the bottom */

void second_slot(int x, int y, int w, int h) {
	fill_rect(x, y, w, h, PET_SLOT_EDGE);
	fill_rect(x + 1, y + 1, w - 2, h - 2, PET_SLOT);
}

void second_stripes(int x, int y) {
	for (int k = 0; k < 3; ++k)
		for (int r = 0; r < SECOND_STRIPES_H; ++r) fill_rect(x + k * 6 + (SECOND_STRIPES_H - 1 - r) / 2, y + r, 2, 1, PET_LINE);
}

void second_row(int x, int y, int w, const char *name, const char *value) {
	second_slot(x, y, w, SECOND_ROW_H - 2);
	text_draw(x + 5, y + 2, name, PET_GOLD, TEXT_LEFT);
	text_draw(x + w - 5, y + 2, value, PET_WHITE, TEXT_RIGHT);
}

/* the place on its slot: `area` (the layer's, the town's name) and
 * `layer` (its number), either "" */
static void place(int w, const char *area, const char *layer) {
	int lw = text_width(layer), aw = text_width(area), gap = lw && aw ? 8 : 0;
	int x = w - 4 - (lw + gap + aw) - 12;
	second_slot(x, 2, w - 4 - x, HEAD_H - 4);
	text_draw(x + 6, 4, area, PET_CYAN_HI, TEXT_LEFT);
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
		second_slot(x, STRIP_Y, sw, STRIP_H);
		text_draw(x + 5, STRIP_Y + 2, name[i], PET_GOLD, TEXT_LEFT);
		text_draw(x + sw - 5, STRIP_Y + 2, v[i], PET_WHITE, TEXT_RIGHT);
	}
}

/* The band with `title`, the green round the body from `top` down and
 * the body's cyan edge: the body */
static SDL_Rect frame(int w, int h, const char *title, int slide, int top) {
	/* (each pixel filled once: the panel fills the body, on a 3DS's time) */
	fill_rect(0, 0, w, HEAD_H, PET_DARK);
	fill_rect(0, HEAD_H, w, 1, PET_LINE);
	fill_rect(0, HEAD_H + 1, w, top - 3 - HEAD_H, PET_GREEN);
	fill_rect(0, top - 2, 2, h - top + 2, PET_GREEN);
	fill_rect(w - 2, top - 2, 2, h - top + 2, PET_GREEN);
	fill_rect(2, h - 2, w - 4, 2, PET_GREEN);
	text_draw(8 - slide, 3, title, PET_WHITE, TEXT_LEFT);
	second_stripes(8 - slide + text_width(title) + 10, 4);
	/* the body's cyan edge, two pixels, lit along its top */
	SDL_Rect body = { 4, top, w - 8, h - top - 4 };
	SDL_Rect edge[4] = { { 2, top - 2, w - 4, 2 }, { 2, h - 4, w - 4, 2 }, { 2, top, 2, body.h }, { w - 4, top, 2, body.h } };
	fill_rects(edge, 4, PET_CYAN);
	fill_rect(3, top - 1, w - 6, 1, PET_CYAN_HI);
	return body;
}

SDL_Rect second_frame_body(int w, int h) { return (SDL_Rect){ 4, BODY_Y, w - 8, h - BODY_Y - 4 }; }

SDL_Rect second_frame(int w, int h, const char *title, int slide) {
	SDL_Rect body = frame(w, h, title, slide, BODY_Y);
	char layer[16] = "";
	if (!S2.town) snprintf(layer, sizeof layer, "Layer %d", S2.depth);
	place(w, S2.area, layer);
	strip(w);
	return body;
}

SDL_Rect second_frame_rest(int w, int h, const char *title, const char *where) {
	SDL_Rect body = frame(w, h, title, 0, HEAD_H + 6);
	place(w, where, "");
	return body;
}

int second_wrap(const char *text, int w, char lines[][SECOND_WRAP], int most) {
	int n = 0;
	char line[SECOND_WRAP] = "";
	for (const char *p = text; *p && n < most;) {
		if (*p == '|') {
			snprintf(lines[n++], SECOND_WRAP, "%s", line);
			*line = 0;
			++p;
			continue;
		}
		while (*p == ' ') ++p;
		size_t k = strcspn(p, " |");
		if (!k) continue;
		char next[SECOND_WRAP * 2];
		snprintf(next, sizeof next, "%s%s%.*s", line, *line ? " " : "", (int)(k < SECOND_WRAP ? k : SECOND_WRAP - 1), p);
		p += k;
		if (*line && text_width(next) > w) {
			snprintf(lines[n++], SECOND_WRAP, "%s", line);
			snprintf(line, sizeof line, "%.*s", (int)(k < SECOND_WRAP ? k : SECOND_WRAP - 1), p - k);
		} else snprintf(line, sizeof line, "%.*s", SECOND_WRAP - 1, next);
	}
	if (*line && n < most) snprintf(lines[n++], SECOND_WRAP, "%s", line);
	return n;
}

int second_wrapped(const char *text, int x, int y, int w, int most, SDL_Color c) {
	char lines[8][SECOND_WRAP];
	int n = second_wrap(text, w, lines, most < 8 ? most : 8);
	for (int i = 0; i < n; ++i) text_draw(x, y + i * SECOND_LINE_H, lines[i], c, TEXT_LEFT);
	return y + n * SECOND_LINE_H;
}
