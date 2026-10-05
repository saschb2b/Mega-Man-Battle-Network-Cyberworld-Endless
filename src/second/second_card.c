/* second_card.h. A chip as BN5 DS frames its pictures, larger; a NaviCust
 * program as BN6's board draws its blocks. */
#include "second_card.h"

#include <stdio.h>

#include "data.h"
#include "second_art.h"
#include "second_frame.h"

#define ART_SCALE 2

void second_chip_card(uint16_t e, int x, int y, int w) {
	int chip = e & 0x1FF;
	ChipInfo ci;
	chip_info(chip, &ci);
	fill_rect(x, y, 56 * ART_SCALE + 4, SECOND_CARD_H, PET_SLOT_EDGE);
	second_chip_art(chip, x + 2, y + 2, ART_SCALE);
	int tx = x + 56 * ART_SCALE + 12;
	text_draw(tx, y + 2, ci.name, PET_WHITE, TEXT_LEFT);
	char s[16];
	snprintf(s, sizeof s, "%c", (e >> 9) >= 26 ? '*' : 'A' + (e >> 9));
	text_draw_scaled(tx, y + 18, s, PET_GOLD, TEXT_LEFT, 2);
	second_element_icon(second_chip_element(chip), tx + 24, y + 22);
	if (ci.power > 0) {
		snprintf(s, sizeof s, "%d", ci.power);
		text_draw_scaled(x + w - 4, y + 18, s, PET_WHITE, TEXT_RIGHT, 2);
	}
	char desc[160];
	chip_desc(chip, desc, sizeof desc);
	second_wrapped(desc, tx, y + 52, x + w - tx - 4, 3, PET_WHITE);
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
