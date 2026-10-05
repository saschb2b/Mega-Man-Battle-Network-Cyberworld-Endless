/* second_home.h. As BN5 DS's and Operate Shooting Star's PETs rest on
 * their Navi: MegaMan's face from the ROM (drawn as the game runs, never
 * kept in a file), twice as large in a slot; in the town the run's next
 * step and its setup beside it, on the title the profile's record and
 * whether BN5's ROM was found. */
#include "second_home.h"

#include <stdio.h>

#include "rom.h"
#include "second.h"
#include "second_frame.h"
#include "second_state.h"
#include "second_text.h"
#include "text.h"

#define FACE_W 40   /* a mugshot, its origin at its centre (chatbox.h) */
#define FACE_H 48
#define SLOT_W SECOND_FACE_W

void second_face(int x, int y) {
	second_slot(x, y, SLOT_W, FACE_H * 2 + 6);
	Sprite *s = sprite_get(SPR_MUGSHOT, FACE_MEGAMAN);
	if (s) sprite_draw_into(s, 0, 0, x + 3 + FACE_W, y + 3 + FACE_H, 0, 2);
}

/* a heading in gold and its lines in white under it: the y under them */
static int item(const char *head, const char *text, int x, int y, int w) {
	text_draw(x, y, head, PET_GOLD, TEXT_LEFT);
	return second_wrapped(text, x, y + SECOND_LINE_H, w, 3, PET_WHITE) + 6;
}

void second_home_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + 6, y = b.y + 6, tx = x + SLOT_W + 10, w = b.x + b.w - 6 - tx;
	second_face(x, y);
	text_draw_scaled(tx, y, "MegaMan", PET_GOLD, TEXT_LEFT, 2);
	y = item("Next", S2.home_next, tx, y + 30, w);
	item("The run", S2.home_setup, tx, y, w);
}

bool second_title_draw(int w, int h) {
	second_frame_forget();
	SDL_Rect b = second_frame_rest(w, h, "PET", "Home");
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + 6, y = b.y + 6, tx = x + SLOT_W + 10, rw = b.x + b.w - 6 - tx;
	second_face(x, y);
	text_draw_scaled(tx, y, "MegaMan", PET_GOLD, TEXT_LEFT, 2);
	SecondRow r[5];
	int n = second_record(r, 5);
	for (int i = 0; i < n; ++i) second_row(tx, y + 30 + i * SECOND_ROW_H, rw, r[i].name, r[i].value);
	bool bn5 = false;
	for (int i = 0; i < XROM_COUNT; ++i) bn5 |= XR[i].data != NULL;
	second_wrapped(second_bn5_line(bn5), x, b.y + b.h - 6 - 2 * SECOND_LINE_H, b.w - 12, 2, bn5 ? PET_CYAN_HI : PET_DIM);
	return true;
}

bool second_title_changed(void) { return false; }
