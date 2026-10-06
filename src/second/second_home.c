/* second_home.h. As BN5 DS's and Operate Shooting Star's PETs rest on
 * their Navi: MegaMan's face from the ROM (drawn as the game runs, never
 * kept in a file), twice as large in a slot; in the town the run's next
 * step and its setup beside it, on the title the profile's record and
 * whether BN5's ROM was found. While BN6's PET menu is open on the top
 * screen, the home lies dimmed under ACCESSING (issue #83). */
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

void second_mugshot(int face, int x, int y) {
	second_slot(x, y, SLOT_W, SECOND_FACE_H);
	Sprite *s = sprite_get(SPR_MUGSHOT, face);
	if (s) sprite_draw_into(s, 0, 0, x + 3 + FACE_W, y + 3 + FACE_H, 0, 2);
}

void second_face(int x, int y) { second_mugshot(FACE_MEGAMAN, x, y); }

/* a heading in gold and its lines in white under it, as many as fit above
 * `bottom` (none: left out): the y under them */
static int item(const char *head, const char *text, int x, int y, int w, int bottom) {
	int fit = (bottom - y - SECOND_LINE_H) / SECOND_LINE_H;
	if (!text[0] || fit < 1) return y;
	text_draw(x, y, head, PET_GOLD, TEXT_LEFT);
	return second_wrapped(text, x, y + SECOND_LINE_H, w, fit < 3 ? fit : 3, PET_WHITE) + 6;
}

/* The PET's menu open on the top screen: the home behind glass under a
 * band, ACCESSING, as BN5 DS dims its field while its PET menu is open;
 * the band in the frame's own colours, its word in white and BN6's three
 * stripes after it, still (no blink, issue #72) */
static void accessing(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, rgba(0, 20, 28, 168));
	const char *word = second_accessing();
	int h = 2 * TEXT_H + 8, y = b.y + (b.h - h) / 2, tw = 2 * text_width(word);
	int x = b.x + (b.w - tw - 12 - SECOND_STRIPES_W) / 2;
	fill_rect(b.x, y, b.w, h, PET_DARK);
	fill_rect(b.x, y, b.w, 1, PET_LINE);
	fill_rect(b.x, y + h - 1, b.w, 1, PET_LINE);
	text_draw_scaled(x, y + 4, word, PET_WHITE, TEXT_LEFT, 2);
	second_stripes(x + tw + 12, y + (h - SECOND_STRIPES_H) / 2);
}

void second_home_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	int x = b.x + 6, y = b.y + 6, tx = x + SLOT_W + 10, w = b.x + b.w - 6 - tx, bottom = b.y + b.h - 4;
	second_face(x, y);
	text_draw_scaled(tx, y, "MegaMan", PET_GOLD, TEXT_LEFT, 2);
	/* (at home the visit's hour beside his name) */
	if (S2.home_hour[0]) text_draw(b.x + b.w - 6, y + 4, S2.home_hour, PET_CYAN_HI, TEXT_RIGHT);
	/* (the most wanted first, as the panel's height lets them: the 3DS's
	 * is shorter than an Android display's) */
	y = item("Next", S2.home_next, tx, y + 30, w, bottom);
	y = item("Request", S2.home_job, tx, y, w, bottom);
	/* (the rest under the face, the panel's whole width) */
	int below = b.y + 6 + FACE_H * 2 + 12;
	if (y < below) y = below;
	y = item("Ways", S2.home_ways, x, y, b.w - 12, bottom);
	y = item("The Net's clock", S2.home_clock, x, y, b.w - 12, bottom);
	item("The run", S2.home_setup, x, y, b.w - 12, bottom);
	if (S2.context == SECOND_PET) accessing(b);
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
