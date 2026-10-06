/* second_pet.h. BN6's STATUS shows MegaMan's HP and his buster; its
 * Library the chips the game's PET holds, the one under the cursor as a
 * small card. Beside them, what only the run and the profile know: the
 * run's record and where MegaMan's HP comes from, the programs on his
 * board, the Cross the run brought; the Library's chip as a card the size
 * of the Custom screen's (issue #83), with the codes it comes in, which
 * BN6's leaves out, and its copies in the folder and the pack; the
 * profile's Library class by class against what a run can hold, and what
 * this run added to it. */
#include "second_pet.h"

#include "second_card.h"
#include "second_frame.h"
#include "second_home.h"
#include "second_state.h"
#include "second_text.h"

void second_status_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + 6, y = b.y + 6, tx = x + SECOND_FACE_W + 10, w = b.x + b.w - 6 - tx;
	second_face(x, y);
	SecondRow r[6];
	int n = second_status(r, 6, S2.st_max_hp, S2.st_base_hp);
	for (int i = 0; i < n; ++i) second_row(tx, y + i * SECOND_ROW_H, w, r[i].name, r[i].value);
	int py = y + (n * SECOND_ROW_H > 104 ? n * SECOND_ROW_H : 104) + 6;
	text_draw(x, py, "On the board", PET_GOLD, TEXT_LEFT);
	second_wrapped(*S2.st_programs ? S2.st_programs : "No programs", x, py + SECOND_LINE_H, b.w - 12, 3, PET_WHITE);
}

/* Rows two a line from (x, y), each `half` wide: the y under them */
static int rows_two(const SecondRow *r, int n, int x, int y, int half) {
	for (int i = 0; i < n; ++i) second_row(x + i % 2 * (half + 4), y + i / 2 * SECOND_ROW_H, half, r[i].name, r[i].value);
	return y + (n + 1) / 2 * SECOND_ROW_H;
}

void second_library_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	if (!S2.pet_ready) return;
	int x = b.x + 6, y = b.y + 6, w = b.w - 12, half = (w - 4) / 2;
	/* (the card and the chip's copies stand still as the cursor moves on
	 * and off a number never seen) */
	if (S2.lib_chip || S2.lib_unseen) {
		second_library_card(S2.lib_chip, x, y, w);
		SecondRow h[2];
		int n = S2.lib_chip ? second_copies(h, 2, S2.lib_folder, S2.lib_pack) : 0;
		y = rows_two(h, n, x, y + SECOND_CARD_H + 6, half);
		if (!n) y += SECOND_ROW_H;
		y += 4;
	}
	SecondRow r[4];
	y = rows_two(r, second_library(r, 4), x, y, half) + 6;
	/* (what the Library is to a run, where the whole of it fits) */
	char lines[4][SECOND_WRAP];
	int k = second_wrap(second_library_line(), w, lines, 4);
	if (y + k * SECOND_LINE_H <= b.y + b.h - 4) second_wrapped(second_library_line(), x, y, w, k, PET_CYAN_HI);
}
