/* second_pet.h. BN6's STATUS shows MegaMan's HP and his buster; its
 * Library the chips the game's PET holds. Beside them, what only the run
 * and the profile know: the run's record and where MegaMan's HP comes
 * from, the programs on his board, the Cross the run brought; the
 * profile's Library class by class against what a run can hold, and what
 * this run added to it. */
#include "second_pet.h"

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

void second_library_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + 6, y = b.y + 6, w = b.w - 12;
	SecondRow r[4];
	int n = second_library(r, 4);
	for (int i = 0; i < n; ++i) second_row(x, y + i * SECOND_ROW_H, w, r[i].name, r[i].value);
	second_wrapped(second_library_line(), x, y + n * SECOND_ROW_H + 8, w, 3, PET_CYAN_HI);
}
