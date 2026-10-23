/* reconnect_draw.h. The design (the game-design skill, interface: guidance
 * and feedback, transitions): a wait over the frozen world rather than a
 * screen of its own, so the player sees where he will be put back; the time
 * left only past a few seconds, as a load's bar is; the way out (B) always
 * shown, the one key's hint at the band's end as E-Mail's START:SORT, so
 * being held is never helplessness; and the return said in a moment, a
 * band, not a dialog, as a player on a bad link may see it often. */
#include "reconnect_draw.h"

#include "gfx.h"
#include "minifont.h"
#include "pet_look.h"
#include "reconnect_text.h"

#define W 240
#define H 160
#define PANEL_W 200
#define PANEL_H 80
#define PANEL_X ((W - PANEL_W) / 2)
#define PANEL_Y 34
#define SHADE    rgba(0, 18, 32, 150)       /* the Square under the wait, dimmed */
#define BAR_AFTER_MS 5000                    /* the time left shown past this */

/* the heading: BN6's band, its title and `right` as its note */
static void band(int x, int y, int w, const char *right, SDL_Color c) {
	look_band(x, y, w, reconnect_text_title);
	look_band_note(x, y, w, right, c);
}

/* a signal's four bars from (x, y), lit one more each quarter second and
 * dark again: the PET searching */
static void searching(int x, int y, uint32_t t) {
	int lit = (int)(t / 250 % 5);
	for (int b = 0; b < 4; ++b) {
		int h = 5 + b * 4, bx = x + b * 6;
		fill_rect(bx, y + 18 - h, 4, h, b < lit ? LOOK_EDGE_HI : LOOK_BAND);
	}
}

/* the time left to try: a bar that runs down, and its seconds */
static void time_left(int x, int y, int w, const ReconnectView *v) {
	uint32_t left = v->elapsed < v->window ? v->window - v->elapsed : 0;
	const char *s = reconnect_text_left((left + 999) / 1000);
	int sw = minifont_width(s, 1), bw = w - sw - 6;
	fill_rect(x, y, bw, 5, LOOK_SHADE);
	fill_rect(x + 1, y + 1, bw - 2, 3, LOOK_NAVY_LO);
	fill_rect(x + 1, y + 1, (int)((uint64_t)(bw - 2) * left / (v->window ? v->window : 1)), 3, LOOK_EDGE);
	minifont_draw(x + bw + 6, y, s, LOOK_EDGE_HI, 1);
}

void reconnect_draw(int x, int y, const ReconnectView *v) {
	if (v->back) {
		band(x, y, W, reconnect_text_back, LOOK_GOLD);
		fill_rect(x + 7, y + LOOK_BAND_H, W - 14, 1, LOOK_GREEN_LO);
		return;
	}
	fill_rect(x, y, W, H, SHADE);
	int px = x + PANEL_X, py = y + PANEL_Y;
	look_screen(px, py, PANEL_W, PANEL_H, reconnect_text_title);
	look_band_hint(px, py, PANEL_W, "B", reconnect_text_key_home);
	/* (in SubChip's flat panel: BN6 keeps its window for a list) */
	SDL_Rect f = look_panel(px + 7, py + 18, PANEL_W - 14, 52);
	f = (SDL_Rect){ f.x + 2, f.y + 3, f.w - 4, f.h - 6 };
	searching(f.x + 4, f.y + 2, v->t);
	look_text(f.x + 34, f.y + 1, reconnect_text_trying((int)(v->t / 400 % 4)), LOOK_TEXT, LOOK_BAND, TEXT_LEFT);
	minifont_draw(f.x + 34, f.y + 15, reconnect_text_kept, LOOK_DIM, 1);
	if (v->elapsed >= BAR_AFTER_MS) time_left(f.x + 4, f.y + 29, f.w - 8, v);
}
