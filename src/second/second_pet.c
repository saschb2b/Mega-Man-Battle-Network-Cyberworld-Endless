/* second_pet.h. BN6's STATUS shows MegaMan's HP and his buster; its
 * Library the chips the game's PET holds, the one under the cursor as a
 * small card. Beside them, what only the run and the profile know: the
 * run's record and where MegaMan's HP comes from, the programs on his
 * board, the Cross the run brought; the Library's chip as a card the size
 * of the Custom screen's (issue #83), with the codes it comes in, which
 * BN6's leaves out, and its copies in the folder and the pack; the
 * profile's Library entry by entry (the run's own Library's pages,
 * src/library/) against what a run can hand out, and what this run added
 * to it. */
#include "second_pet.h"

#include <string.h>

#include "forms.h"
#include "forms_text.h"
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
	SecondRow r[5];
	y = rows_two(r, second_library(r, 5), x, y, half) + 6;
	char lines[4][SECOND_WRAP];
	/* (what the Library is to a run, where the whole of it fits) */
	int k = second_wrap(second_library_line(), w, lines, 4);
	if (y + k * SECOND_LINE_H <= b.y + b.h - 4) second_wrapped(second_library_line(), x, y, w, k, PET_CYAN_HI);
}

bool second_forms_up(void) { return forms_shown(); }

void second_read_forms(void) {
	const FormsRun *r = forms_run();
	if (!r) return;
	S2.forms = *r;
	forms_cursor(&S2.forms_cross, &S2.forms_soul);
}

/* the five as CROSSSELECT will list them, in its order, each with its weakness, from (x, y): the y under them */
static int five_rows(int x, int y, int w) {
	text_draw(x, y, forms_word(FT_FIVE), PET_GOLD, TEXT_LEFT);
	y += SECOND_LINE_H + 2;
	for (int i = 0; i < S2.forms.n_crosses; ++i)
		if (S2.forms.active >> S2.forms.crosses[i].cross & 1) {
			second_row(x, y, w, S2.forms.crosses[i].name, S2.forms.crosses[i].weak ? S2.forms.crosses[i].weak : "");
			y += SECOND_ROW_H;
		}
	return y;
}

/* The forms screen's: the Cross under its cursor (its weakness, what it
 * does more) and CROSSSELECT's five as they stand; or the Soul (its kind,
 * the chip it takes) */
void second_forms_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + 6, y = b.y + 6, w = b.w - 12;
	char s[160];
	if (S2.forms_soul >= 0 && S2.forms_soul < S2.forms.n_souls) {
		const FormsSoul *so = &S2.forms.souls[S2.forms_soul];
		text_draw(x, y, so->name, PET_GOLD, TEXT_LEFT);
		second_row(x, y + SECOND_LINE_H + 4, w, forms_word(FT_KIND), so->kind);
		forms_soul_words(s, sizeof s, so->name, so->kind);
		second_wrapped(strchr(s, '|') ? strchr(s, '|') + 1 : s, x, y + SECOND_LINE_H + 4 + SECOND_ROW_H + 4, w, 3, PET_WHITE);
		return;
	}
	if (S2.forms_cross >= 0 && S2.forms_cross < S2.forms.n_crosses) {
		const FormsCross *c = &S2.forms.crosses[S2.forms_cross];
		text_draw(x, y, c->name, PET_GOLD, TEXT_LEFT);
		y += SECOND_LINE_H + 4;
		second_row(x, y, w, forms_word(FT_WEAK), c->weak ? c->weak : "");
		y += SECOND_ROW_H + 2;
		if (c->strong) y = second_wrapped(c->strong, x, y, w, 2, PET_WHITE) + 6;
		y += 6;
	}
	if (S2.forms.n_crosses) five_rows(x, y, w);
	else second_wrapped(forms_word(FT_NONE), x, y, w, 2, PET_WHITE);
}
