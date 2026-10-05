/* second_folder.h. BN6's editor shows the chip under the cursor on the top
 * screen, its picture, code, element and text; this shows the whole: the
 * folder's thirty chips as their icons, each framed by its rank as BN5 DS
 * frames them (grey, blue Mega, red Giga, purple Dark) and marked where it
 * is the Regular chip or a TagChip, the cursor's among them; then the
 * folder's makeup: its codes (the Custom screen deals by code), its
 * elements, its Megas and Gigas against the folder's limits; then the
 * pack. */
#include "second_folder.h"

#include <stdio.h>

#include "second_art.h"
#include "second_frame.h"
#include "second_state.h"

#define CELL_W  20   /* a chip's cell: its icon framed, its code under it */
#define CELL_H  30
#define COLS    15
#define LINE_H  16   /* a line of the makeup */

static SDL_Color rank_colour(int rank) {
	static const SDL_Color c[4] = { { 165, 189, 214, 255 }, { 66, 140, 239, 255 }, { 231, 66, 57, 255 }, { 165, 82, 222, 255 } };
	return c[rank];
}

/* A label in the PET's gold, the x after it */
static int label(int x, int y, const char *s) {
	text_draw(x, y, s, PET_GOLD, TEXT_LEFT);
	return x + text_width(s) + 6;
}

/* a mark in a cell's corner: the Regular chip's red, a TagChip's green */
static void corner(int x, int y, SDL_Color c) {
	fill_rect(x, y, 5, 5, PET_SLOT_EDGE);
	fill_rect(x + 1, y + 1, 3, 3, c);
}

/* Chip entry `e` in the cell at (x, y): its icon in its rank's frame, its
 * code under it; the cursor's in a gold frame */
static void cell(int x, int y, uint16_t e, int count, bool cursor, int mark) {
	int chip = e & 0x1FF, code = e >> 9;
	if (cursor) {
		fill_rect(x - 1, y - 1, 20, 20, PET_GOLD);
		fill_rect(x, y, 18, 18, PET_NAVY);
	}
	if (!chip) {
		fill_rect(x + 1, y + 1, 16, 16, PET_SLOT);
		return;
	}
	SDL_Color rc = rank_colour(second_chip_rank(chip));
	SDL_Rect frame[4] = { { x, y, 18, 1 }, { x, y + 17, 18, 1 }, { x, y + 1, 1, 16 }, { x + 17, y + 1, 1, 16 } };
	fill_rects(frame, 4, rc);
	second_chip_icon(chip, x + 1, y + 1);
	if (mark == 1) corner(x + 13, y, rgba(231, 66, 57, 255));
	if (mark == 2) corner(x + 13, y, rgba(74, 231, 115, 255));
	char s[8];
	if (count > 1) snprintf(s, sizeof s, "%c%d", code >= 26 ? '*' : 'A' + code, count);
	else snprintf(s, sizeof s, "%c", code >= 26 ? '*' : 'A' + code);
	text_draw(x + 9, y + 17, s, cursor ? PET_GOLD : PET_WHITE, TEXT_CENTER);
}

/* the folder's thirty chips, two rows of fifteen from (x, y) */
static void folder_grid(int x, int y) {
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		int mark = i == S2.reg ? 1 : i == S2.tag[0] || i == S2.tag[1] ? 2 : 0;
		cell(x + (i % COLS) * CELL_W + 1, y + (i / COLS) * CELL_H, S2.folder[i], 1, i == S2.entry, mark);
	}
}

/* its codes, the most held first: "A4 B4 *3" */
static void codes_line(int x, int y, int right) {
	int count[27] = { 0 };
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i)
		if (S2.folder[i] & 0x1FF) ++count[S2.folder[i] >> 9 < 27 ? S2.folder[i] >> 9 : 26];
	x = label(x, y, "Codes");
	for (int left = 1; left;) {
		int best = -1;
		for (int c = 0; c < 27; ++c)
			if (count[c] && (best < 0 || count[c] > count[best])) best = c;
		if (best < 0) break;
		char s[8];
		snprintf(s, sizeof s, "%c%d", best == 26 ? '*' : 'A' + best, count[best]);
		if (x + text_width(s) > right) break;
		text_draw(x, y, s, PET_WHITE, TEXT_LEFT);
		x += text_width(s) + 7;
		count[best] = 0;
	}
}

/* its elements, each icon with its count */
static void elements_line(int x, int y, int right) {
	int count[11] = { 0 };
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		int el = (S2.folder[i] & 0x1FF) ? second_chip_element(S2.folder[i] & 0x1FF) : -1;
		if (el >= 0) ++count[el];
	}
	for (int el = 0; el < 11; ++el) {
		if (!count[el]) continue;
		char s[8];
		snprintf(s, sizeof s, "%d", count[el]);
		if (x + 18 + text_width(s) > right) break;
		second_element_icon(el, x, y - 2);
		text_draw(x + 18, y, s, PET_WHITE, TEXT_LEFT);
		x += 18 + text_width(s) + 8;
	}
}

/* its Megas and Gigas against the folder's limits */
static void ranks_line(int x, int y) {
	int mega = 0, giga = 0;
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		int r = (S2.folder[i] & 0x1FF) ? second_chip_rank(S2.folder[i] & 0x1FF) : -1;
		mega += r == RANK_MEGA;
		giga += r == RANK_GIGA;
	}
	char s[16];
	x = label(x, y, "Mega");
	snprintf(s, sizeof s, "%d/%d", mega, S2.mega_level);
	text_draw(x, y, s, PET_WHITE, TEXT_LEFT);
	x = label(x + text_width(s) + 14, y, "Giga");
	snprintf(s, sizeof s, "%d/%d", giga, S2.giga_level);
	text_draw(x, y, s, PET_WHITE, TEXT_LEFT);
}

/* the pack: its chips as the folder's, their copies beside their codes,
 * as many as fit around the cursor's */
static void pack_row(int x, int y, int right) {
	int fit = (right - x) / CELL_W, first = S2.pack_entry - fit / 2;
	if (first > S2.npack - fit) first = S2.npack - fit;
	if (first < 0) first = 0;
	for (int i = first; i < S2.npack && i < first + fit; ++i, x += CELL_W) cell(x + 1, y, S2.pack[i], S2.pack_count[i], i == S2.pack_entry, 0);
}

void second_folder_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + (b.w - COLS * CELL_W) / 2, right = b.x + b.w - 4;
	int y = b.y + 3;
	/* (the side the editor is on: its name lit) */
	text_draw(x + 1, y, "Folder", S2.pack_side ? PET_CYAN_HI : PET_GOLD, TEXT_LEFT);
	y += LINE_H;
	folder_grid(x, y);
	y += 2 * CELL_H + 2;
	fill_rect(b.x + 4, y, b.w - 8, 1, PET_SLOT_EDGE);
	codes_line(x + 1, y + 3, right);
	elements_line(x + 1, y + 3 + LINE_H + 2, right);
	ranks_line(x + 1, y + 3 + 2 * LINE_H + 4);
	y += 3 * LINE_H + 10;
	fill_rect(b.x + 4, y, b.w - 8, 1, PET_SLOT_EDGE);
	char s[24];
	snprintf(s, sizeof s, S2.npack ? "Pack" : "Pack: empty");
	text_draw(x + 1, y + 3, s, S2.pack_side ? PET_GOLD : PET_CYAN_HI, TEXT_LEFT);
	pack_row(x, y + 3 + LINE_H, right);
}
