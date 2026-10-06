/* second_card.h. A chip as BN5 DS frames its pictures, larger (in the
 * Library with the codes it comes in, which BN6's Library leaves out); a
 * NaviCust program as BN6's board draws its blocks. */
#include "second_card.h"

#include <stdio.h>

#include "data.h"
#include "second_art.h"
#include "second_frame.h"

#define ART_SCALE 2
#define ART_W (56 * ART_SCALE)   /* a card's picture */
#define ART_H (48 * ART_SCALE)

/* Chip `chip` (its record `ci`) as a card from (x, y), w wide, `codes` in
 * gold beside its element: the code of a folder's entry, or those it
 * comes in */
static void card(int chip, const ChipInfo *ci, const char *codes, int x, int y, int w) {
	fill_rect(x, y, ART_W + 4, SECOND_CARD_H, PET_SLOT_EDGE);
	second_chip_art(chip, x + 2, y + 2, ART_SCALE);
	int tx = x + ART_W + 12;
	text_draw(tx, y + 2, ci->name, PET_WHITE, TEXT_LEFT);
	text_draw_scaled(tx, y + 18, codes, PET_GOLD, TEXT_LEFT, 2);
	/* (one code's element where it always stood, several codes' after them) */
	second_element_icon(second_chip_element(chip), tx + (codes[0] && codes[1] ? 2 * text_width(codes) + 8 : 24), y + 22);
	if (ci->power > 0) {
		char s[16];
		snprintf(s, sizeof s, "%d", ci->power);
		text_draw_scaled(x + w - 4, y + 18, s, PET_WHITE, TEXT_RIGHT, 2);
	}
	char desc[160];
	chip_desc(chip, desc, sizeof desc);
	second_wrapped(desc, tx, y + 52, x + w - tx - 4, 3, PET_WHITE);
}

void second_chip_card(uint16_t e, int x, int y, int w) {
	ChipInfo ci;
	char code[2] = { (char)((e >> 9) >= 26 ? '*' : 'A' + (e >> 9)), 0 };
	chip_info(e & 0x1FF, &ci);
	card(e & 0x1FF, &ci, code, x, y, w);
}

void second_library_card(int chip, int x, int y, int w) {
	if (chip) {
		ChipInfo ci;
		chip_info(chip, &ci);
		card(chip, &ci, ci.codes, x, y, w);
		return;
	}
	/* (a number never seen: BN6's own "??", its card blank) */
	fill_rect(x, y, ART_W + 4, SECOND_CARD_H, PET_SLOT_EDGE);
	fill_rect(x + 2, y + 2, ART_W, ART_H, PET_SLOT);
	text_draw(x + ART_W + 12, y + 2, "??", PET_WHITE, TEXT_LEFT);
}

/* BN6's program colours 1-6 as its blocks draw them, a face and its
 * shade: measured from the NaviCustomizer's preview of a program of each
 * colour (white, yellow, pink, red, blue, green) */
static const SDL_Color face[7] = { { 0, 0, 0, 255 }, { 255, 255, 255, 255 }, { 239, 247, 0, 255 },
	{ 255, 148, 231, 255 }, { 231, 0, 0, 255 }, { 16, 99, 255, 255 }, { 0, 239, 16, 255 } };
static const SDL_Color shade[7] = { { 0, 0, 0, 255 }, { 206, 206, 214, 255 }, { 214, 198, 0, 255 },
	{ 214, 99, 165, 255 }, { 189, 0, 0, 255 }, { 0, 57, 214, 255 }, { 0, 189, 8, 255 } };

void second_program_shape(const NaviShape *s, int x, int y) {
	int c = s->color >= 1 && s->color <= 6 ? s->color : 0;
	fill_rect(x, y, SECOND_SHAPE_W, SECOND_SHAPE_W, PET_SLOT_EDGE);
	for (int r = 0; r < 7; ++r)
		for (int k = 0; k < 7; ++k) {
			int bx = x + 2 + k * SECOND_SHAPE_CELL, by = y + 2 + r * SECOND_SHAPE_CELL;
			if (!s->cell[r][k] || !c) {
				fill_rect(bx, by, SECOND_SHAPE_CELL - 1, SECOND_SHAPE_CELL - 1, PET_SLOT);
				continue;
			}
			fill_rect(bx, by, SECOND_SHAPE_CELL, SECOND_SHAPE_CELL, shade[c]);
			fill_rect(bx, by, SECOND_SHAPE_CELL - 1, SECOND_SHAPE_CELL - 1, face[c]);
		}
}


void second_color_swatch(int color, int x, int y, int size) {
	int c = color >= 1 && color <= 6 ? color : 0;
	fill_rect(x, y, size + 1, size + 1, shade[c]);
	fill_rect(x, y, size, size, face[c]);
}
