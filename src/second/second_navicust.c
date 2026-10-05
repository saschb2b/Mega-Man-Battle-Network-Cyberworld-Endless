/* second_navicust.h. BN6's NaviCustomizer shows the board, the list, a
 * small preview of the program under the cursor and a line of its text;
 * what a bug would be it tells only at RUN. Here the program the cursor
 * is on, in the list, on the board or held over it, is a card: its shape
 * in its colour, its name, what kind of part it is and where that may go,
 * whether it fits the board's free cells and whether L and R turn it, and
 * what it does in MegaMan's words. Under it, the board as it stands: what
 * RUN would bring (docs/NAVICUST.md's rules, the bug's cause MegaMan
 * names after RUN, a step sooner) and the rules in a line. */
#include "second_navicust.h"

#include "second_card.h"
#include "second_frame.h"
#include "second_state.h"
#include "second_text.h"


/* The program's card: its shape, its name twice as large, its kind and
 * where it goes, its place and fit, its turn; what it does under them */
static void card(int x, int y, int w) {
	second_program_shape(&S2.nc_shape, x, y);
	int tx = x + SECOND_SHAPE_W + 10;
	text_draw_scaled(tx, y, S2.nc_name, PET_GOLD, TEXT_LEFT, 2);
	char about[200];
	second_program_lines(&S2, about, sizeof about);
	int under = second_wrapped(about, tx, y + 26, x + w - tx, 4, PET_WHITE);
	second_wrapped(second_program_does(S2.nc_variant / 4), x, (under > y + SECOND_SHAPE_W ? under : y + SECOND_SHAPE_W) + 4, w, 2, PET_CYAN_HI);
}

/* RUN's verdict as the board stands, and the rules, at the bottom of `b` */
static void verdict(SDL_Rect b) {
	int x = b.x + 6, w = b.w - 12, y = b.y + b.h - 6 - 5 * SECOND_LINE_H;
	fill_rect(b.x + 4, y - 6, b.w - 8, 1, PET_SLOT_EDGE);
	bool bug = *S2.nc_bug;
	text_draw(x, y, second_run_line(bug), bug ? PET_GOLD : PET_WHITE, TEXT_LEFT);
	if (bug) second_wrapped(S2.nc_bug, x, y + SECOND_LINE_H, w, 2, PET_WHITE);
	second_wrapped(second_board_rules(), x, y + 3 * SECOND_LINE_H + 4, w, 2, PET_DIM);
}

/* On RUN: the programs on the board, two to a line, each by its colour */
static void board_list(int x, int y, int w) {
	for (int i = 0; i < S2.nc_nboard; ++i) {
		int cx = x + (i % 2) * (w / 2), cy = y + (i / 2) * (SECOND_LINE_H + 3);
		second_color_swatch(S2.nc_board[i].color, cx, cy + 1, SECOND_SHAPE_CELL);
		text_draw(cx + SECOND_SHAPE_CELL + 6, cy, S2.nc_board[i].name, PET_WHITE, TEXT_LEFT);
	}
}

void second_navicust_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + 6, w = b.w - 12;
	if (S2.nc_variant) card(x, b.y + 6, w);
	else if (S2.nc_on == NC_RUN) {
		text_draw_scaled(x, b.y + 6, "RUN", PET_GOLD, TEXT_LEFT, 2);
		board_list(x, b.y + 36, w);
	}
	verdict(b);
}
