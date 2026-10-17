#include "online_trade_panel.h"

#include "data.h"
#include "emu.h"
#include "gfx.h"
#include "online_trade_text.h"
#include "pixfont.h"
#include "platform.h"
#include "second_art.h"
#include "second_frame.h"

static void label(int x, int y, const char *text, SDL_Color color) {
	pixfont_draw(P.core_x + x, P.core_y + y, text, color, PET_EXIT, PIXFONT_LEFT, 1);
}

static void chip_card(uint16_t entry, int x, const char *heading) {
	int chip = entry & 511;
	ChipInfo info;
	chip_info(chip, &info);
	fill_rect(P.core_x + x, P.core_y + 32, 110, 83, PET_SLOT_EDGE);
	fill_rect(P.core_x + x + 1, P.core_y + 33, 108, 81, PET_SLOT);
	label(x + 5, 35, heading, PET_CYAN_HI);
	second_chip_art(chip, P.core_x + x + 5, P.core_y + 47, 1);
	label(x + 72, 47, trade_panel_code, PET_WHITE);
	char code[2] = { (char)((entry >> 9) == 26 ? '*' : 'A' + (entry >> 9)), 0 };
	text_draw_scaled(P.core_x + x + 83, P.core_y + 58, code, PET_GOLD, TEXT_CENTER, 2);
	second_element_icon(second_chip_element(chip), P.core_x + x + 76, P.core_y + 83);
	label(x + 5, 103, info.name, PET_WHITE);
}

void online_trade_panel_draw(const OnlineTradePanel *panel) {
	if (!panel || (unsigned)panel->phase >= TRADE_PANEL_PHASES) return;
	fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, PET_EXIT);
	fill_rect(P.core_x, P.core_y, EMU_W, 15, PET_GREEN);
	text_draw(P.core_x + 6, P.core_y + 1, trade_panel_title, PET_WHITE, TEXT_LEFT);
	text_draw(P.core_x + 234, P.core_y + 1, panel->peer, PET_WHITE, TEXT_RIGHT);
	chip_card(panel->give, 6, trade_panel_give);
	chip_card(panel->receive, 124, trade_panel_receive);
	label(8, 121, trade_panel_status[panel->phase][0], PET_GOLD);
	label(8, 132, trade_panel_status[panel->phase][1], PET_WHITE);
	fill_rect(P.core_x, P.core_y + 145, EMU_W, 15, PET_NAVY);
	label(8, 149, trade_panel_help[panel->phase], PET_WHITE);
}
