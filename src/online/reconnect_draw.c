/* reconnect_draw.h. The design (the game-design skill, interface: guidance
 * and feedback, transitions): a wait over the frozen world rather than a
 * screen of its own, so the player sees where he will be put back; the time
 * left only past a few seconds, as a load's bar is; the way out (B) always
 * shown, so being held is never helplessness; and the return said in a
 * moment, a band, not a dialog, as a player on a bad link may see it often. */
#include "reconnect_draw.h"

#include "gfx.h"
#include "minifont.h"
#include "netpass_draw.h"
#include "reconnect_text.h"
#include "second_frame.h"

#define W 240
#define H 160
#define HEAD_H 18
#define PANEL_W 200
#define PANEL_H 92
#define PANEL_X ((W - PANEL_W) / 2)
#define PANEL_Y 34
#define INK      rgba(0, 49, 74, 255)
#define SHADE    rgba(0, 18, 32, 150)       /* the Square under the wait, dimmed */
#define BAR_AFTER_MS 5000                    /* the time left shown past this */

/* the heading: BN6's green band, the title and its stripes, `right` on a slot */
static void band(int x, int y, int w, const char *right, SDL_Color c) {
	fill_rect(x, y, w, HEAD_H, PET_GREEN);
	fill_rect(x, y, w, 1, PET_LINE);
	fill_rect(x, y + HEAD_H - 2, w, 2, PET_DARK);
	text_draw(x + 7, y + 3, reconnect_text_title, PET_WHITE, TEXT_LEFT);
	second_stripes(x + 11 + text_width(reconnect_text_title), y + 3);
	int sw = text_width(right) + 12, sx = x + w - 4 - sw;
	second_slot(sx, y + 2, sw, HEAD_H - 5);
	text_draw(sx + 6, y + 2, right, c, TEXT_LEFT);
}

/* a signal's four bars from (x, y), lit one more each quarter second and
 * dark again: the PET searching */
static void signal(int x, int y, uint32_t t) {
	int lit = (int)(t / 250 % 5);
	for (int b = 0; b < 4; ++b) {
		int h = 5 + b * 4, bx = x + b * 6;
		fill_rect(bx, y + 18 - h, 4, h, b < lit ? PET_CYAN_HI : PET_SLOT_EDGE);
	}
}

/* a key's cap with `cap` in it, then `what` */
static void key(int x, int y, const char *cap, const char *what) {
	int cw = minifont_width(cap, 1) + 5;
	netpass_panel(x, y, cw, 9, PET_CYAN_HI, PET_CYAN);
	minifont_draw(x + 3, y + 2, cap, INK, 1);
	minifont_draw(x + cw + 3, y + 2, what, PET_WHITE, 1);
}

/* the time left to try: a bar that runs down, and its seconds */
static void time_left(int x, int y, int w, const ReconnectView *v) {
	uint32_t left = v->elapsed < v->window ? v->window - v->elapsed : 0;
	const char *s = reconnect_text_left((left + 999) / 1000);
	int sw = minifont_width(s, 1), bw = w - sw - 6;
	fill_rect(x, y, bw, 5, PET_SLOT_EDGE);
	fill_rect(x + 1, y + 1, bw - 2, 3, PET_EXIT);
	fill_rect(x + 1, y + 1, (int)((uint64_t)(bw - 2) * left / (v->window ? v->window : 1)), 3, PET_CYAN);
	minifont_draw(x + bw + 6, y, s, PET_CYAN_HI, 1);
}

void reconnect_draw(int x, int y, const ReconnectView *v) {
	if (v->back) {
		band(x, y, W, reconnect_text_back, PET_GOLD);
		return;
	}
	fill_rect(x, y, W, H, SHADE);
	int px = x + PANEL_X, py = y + PANEL_Y;
	netpass_panel(px - 1, py - 1, PANEL_W + 2, PANEL_H + 2, PET_EXIT, PET_CYAN);
	netpass_ground(px, py, PANEL_W, PANEL_H);
	band(px, py, PANEL_W, reconnect_text_lost, PET_CYAN_HI);
	signal(px + 10, py + HEAD_H + 8, v->t);
	text_draw(px + 42, py + HEAD_H + 7, reconnect_text_trying((int)(v->t / 400 % 4)), PET_WHITE, TEXT_LEFT);
	minifont_draw(px + 42, py + HEAD_H + 23, reconnect_text_kept, PET_DIM, 1);
	if (v->elapsed >= BAR_AFTER_MS) time_left(px + 10, py + HEAD_H + 38, PANEL_W - 20, v);
	key(px + 10, py + PANEL_H - 15, "B", reconnect_text_key_home);
}
