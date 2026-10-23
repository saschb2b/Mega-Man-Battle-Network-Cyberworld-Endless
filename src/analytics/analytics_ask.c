/* analytics_ask.h. The panel's layout from the picture's corner: the
 * heading, the question, what is sent and what never is (wrapped as it
 * opens), Yes and No, and under them where it is changed and the keys or
 * a phone's hint. */
#include "analytics_ask.h"

#include <stdio.h>
#include <string.h>

#include "analytics.h"
#include "analytics_text.h"
#include "audio.h"
#include "controls.h"
#include "gfx.h"
#include "minifont.h"
#include "pet_panel.h"
#include "platform.h"
#include "touch.h"

#define ASK_WAIT 30          /* frames before it takes a press */
#define ASK_TEXT_W (CORE_W - 28)
#define ASK_WIN_Y 29
#define ASK_BODY_Y (ASK_WIN_Y + LOOK_FRAME + 2)
#define ASK_LINES 6
#define ASK_CHOICE_Y 118
#define ASK_CHOICE_DX 40     /* Yes and No either side of the middle */
#define ASK_LINE 96

static struct {
	bool open, first, yes;
	int t;
	char lines[ASK_LINES][ASK_LINE];
	int nlines;
} K;

/* `s` in lines `w` pixels wide at most, broken at spaces, after the `n`
 * already there; how many there are then */
static int wrap(const char *s, int w, int n) {
	while (*s && n < ASK_LINES) {
		while (*s == ' ') ++s;
		size_t len = strlen(s), cut = 0;
		for (size_t k = 1; k <= len && k < ASK_LINE; ++k) {
			if (s[k] != ' ' && s[k]) continue;
			char t[ASK_LINE];
			snprintf(t, sizeof t, "%.*s", (int)k, s);
			if (text_width(t) > w) break;
			cut = k;
		}
		if (!cut) cut = len < ASK_LINE - 1 ? len : ASK_LINE - 1;   /* (a word wider than the line) */
		snprintf(K.lines[n++], ASK_LINE, "%.*s", (int)cut, s);
		s += cut;
	}
	return n;
}

static void open_panel(bool first) {
	memset(&K, 0, sizeof K);
	K.open = true;
	K.first = first;
	K.yes = analytics_consent() == ANALYTICS_ON;
	K.nlines = wrap(analytics_word(AW_NOT), ASK_TEXT_W, wrap(analytics_word(AW_WHAT), ASK_TEXT_W, 0));
	/* (the finger that opened it presses nothing more) */
	touch_release();
}

void analytics_ask_offer(void) {
	if (analytics_unasked()) open_panel(true);
}

bool analytics_ask_open(void) {
	if (!analytics_here()) return false;
	open_panel(false);
	return true;
}

bool analytics_ask_here(void) { return analytics_here(); }

bool analytics_ask_shown(void) { return K.open; }

static void answer(bool yes) {
	analytics_answer(yes);
	K.open = false;
	audio_sfx(SFX_SELECT);
}

void analytics_ask_update(void) {
	if (!K.open || ++K.t < ASK_WAIT) return;
	if (P.menu_pressed & BTN_B) {
		if (K.first) answer(false);
		else { K.open = false; audio_sfx(SFX_CANCEL); }
		return;
	}
	if (P.menu_repeat & (BTN_LEFT | BTN_RIGHT | BTN_UP | BTN_DOWN)) {
		K.yes = !K.yes;
		audio_sfx(SFX_CURSOR);
	}
	if (P.menu_pressed & (BTN_A | BTN_START)) answer(K.yes);
}

bool analytics_ask_back(void) {
	if (!K.open) return false;
	if (K.first) answer(false);
	else {
		K.open = false;
		audio_sfx(SFX_CANCEL);
	}
	return true;
}

void analytics_ask_tap(int x, int y) {
	if (!K.open || K.t < ASK_WAIT || y < ASK_CHOICE_Y - 4 || y >= ASK_CHOICE_Y + TEXT_H + 2) return;
	int cx = CORE_W / 2;
	if (x >= cx - ASK_CHOICE_DX - 28 && x < cx - 8) answer(true);
	else if (x >= cx + 8 && x < cx + ASK_CHOICE_DX + 28) answer(false);
}

/* Yes and No, the chosen one gold with the PET's arrow before it */
static void choices_draw(int cx, int y) {
	const char *w[2] = { analytics_word(AW_YES), analytics_word(AW_NO) };
	for (int i = 0; i < 2; ++i) {
		bool on = K.yes == (i == 0);
		int x = cx + (i ? ASK_CHOICE_DX : -ASK_CHOICE_DX);
		text_draw(x, y, w[i], on ? LOOK_GOLD : LOOK_TEXT, TEXT_CENTER);
		if (on && K.t >= ASK_WAIT) look_cursor(x - text_width(w[i]) / 2 - 12, y + 2);
	}
}

/* Under them: where it is changed later, then the keys, or a phone's tap */
static void hints_draw(int cx, int y) {
	char s[96], a[24];
	pet_panel_note(cx, y, analytics_later(), LOOK_PAGE);
	if (touch_shown()) {
		pet_panel_note(cx, y + 9, analytics_word(AW_TAP), LOOK_PAGE);
		return;
	}
	/* (controls_word keeps a keyboard's word in one buffer: the first copied) */
	snprintf(a, sizeof a, "%s", controls_word(BTN_A));
	analytics_keys(a, controls_word(BTN_B), K.first, s, sizeof s);
	pet_panel_note(cx, y + 9, s, LOOK_PAGE);
}

void analytics_ask_draw(void) {
	if (!K.open) return;
	int x0 = P.core_x, y0 = P.core_y, cx = x0 + CORE_W / 2;
	pet_panel_draw(x0, y0, analytics_word(AW_TITLE));
	text_draw(cx, y0 + 16, analytics_word(AW_QUESTION), LOOK_PAGE, TEXT_CENTER);
	/* (the body in the bold letters: the lists' thin ones are wider, and would take a line more than the window has) */
	look_window(x0 + 7, y0 + ASK_WIN_Y, CORE_W - 14, 2 * LOOK_FRAME + 4 + K.nlines * 12, 0, false);
	for (int i = 0; i < K.nlines; ++i) text_draw(cx, y0 + ASK_BODY_Y + i * 12, K.lines[i], LOOK_TEXT, TEXT_CENTER);
	choices_draw(cx, y0 + ASK_CHOICE_Y);
	hints_draw(cx, y0 + 138);
}
