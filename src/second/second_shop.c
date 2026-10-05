/* second_shop.h. BN6's shops list their stock with prices and, on R, a
 * line of text; its traders say what they take. Here the entry under the
 * shop's cursor is a card (a chip's picture and text, a program's shape
 * and what it does, an item's name) with what the run holds of it: the
 * copies in the folder and the Pack, a program's on the board. A trader
 * says what it takes and gives, and how many chips the Pack holds. */
#include "second_shop.h"

#include "bn6.h"
#include "second_card.h"
#include "second_frame.h"
#include "second_state.h"
#include "second_text.h"

/* a program's card: its shape, name, kind and what it does */
static int program(int x, int y, int w) {
	second_program_shape(&S2.sh_shape, x, y);
	int tx = x + SECOND_SHAPE_W + 10;
	text_draw_scaled(tx, y, S2.sh_name, PET_GOLD, TEXT_LEFT, 2);
	text_draw(tx, y + 28, S2.sh_shape.kind == NAVI_PLUS ? "Plus part" : "Program part", PET_WHITE, TEXT_LEFT);
	second_wrapped(second_program_does(S2.sh_id / 4), tx, y + 28 + SECOND_LINE_H, x + w - tx, 2, PET_CYAN_HI);
	return y + SECOND_SHAPE_W;
}

void second_shop_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + 6, y = b.y + 6, w = b.w - 12;
	if (!S2.sh_kind) return;
	if (S2.sh_kind == BN6_SHOP_KIND_CHIP) {
		second_chip_card((uint16_t)(S2.sh_id | S2.sh_code << 9), x, y, w);
		y += SECOND_CARD_H;
	} else if (S2.sh_kind == BN6_SHOP_KIND_PROGRAM) y = program(x, y, w);
	else {
		text_draw_scaled(x, y, S2.sh_name, PET_GOLD, TEXT_LEFT, 2);
		y = second_wrapped(second_item_does(S2.sh_id), x, y + 30, w, 2, PET_CYAN_HI);
	}
	char held[80];
	second_held_lines(&S2, held, sizeof held);
	second_wrapped(held, x, y + 8, w, 2, PET_WHITE);
}

void second_trader_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + 6, y = b.y + 6;
	if (S2.trader < 0) return;
	text_draw_scaled(x, y, second_trader_name(S2.trader), PET_GOLD, TEXT_LEFT, 2);
	char lines[120];
	second_trader_lines(S2.trader, S2.pack_chips, lines, sizeof lines);
	second_wrapped(lines, x, y + 32, b.w - 12, 4, PET_WHITE);
}
