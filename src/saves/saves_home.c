/* saves_home.h. What a home line does is saves_activate's; here only how
 * the cursor walks, and what is asked before. */
#include "saves_home.h"

#include "audio.h"
#include "pick.h"
#include "platform.h"
#include "rom.h"
#include "saves_actions.h"
#include "saves_state.h"

static int asking = -1;
static bool yes;

int saves_home_asking(void) { return asking; }
bool saves_home_yes(void) { return yes; }

/* (the lines that change saves or what is kept elsewhere) */
static bool asks(int action) {
	return action == SAVES_UNDO || action == SAVES_TURN_ON || action == SAVES_UNLINK || action == SAVES_DELETE;
}

static void sound(Sfx s) {
	if (R.data) audio_sfx(s);
}

void saves_home_choose(int action) {
	SV.page_focus[SV.page] = action;
	if (!asks(action)) {
		saves_activate(action);
		return;
	}
	asking = action;
	yes = false;
	SV.said[0] = 0;
	sound(SFX_SELECT);
}

void saves_home_answer(bool chosen) {
	int action = asking;
	asking = -1;
	if (chosen) saves_activate(action);
	else sound(SFX_CANCEL);
}

bool saves_home_back(void) {
	if (asking < 0) return false;
	saves_home_answer(false);
	return true;
}

/* (the page's lines, Up and Down round) */
static void walk(int step) {
	int lines[SAVES_ACTIONS], n = saves_actions(lines), at = 0;
	if (!n) return;
	for (int i = 0; i < n; ++i) if (lines[i] == SV.focus) at = i;
	SV.focus = lines[(at + step + n) % n];
	SV.page_focus[SV.page] = SV.focus;
	sound(SFX_CURSOR);
}

/* (the cursor on a line the page has: the cloud's lines change as it
 * links and unlinks under it) */
static void keep_focus(void) {
	int lines[SAVES_ACTIONS], n = saves_actions(lines);
	for (int i = 0; i < n; ++i) if (lines[i] == SV.focus) return;
	SV.focus = n ? lines[0] : SAVES_DONE;
	for (int i = 0; i < n; ++i) if (lines[i] == SV.page_focus[SV.page]) SV.focus = lines[i];
}

void saves_home_keys(void (*turn)(int step)) {
	keep_focus();
	if (asking >= 0) {
		if (btn_pressed(BTN_LEFT | BTN_RIGHT | BTN_UP | BTN_DOWN)) {
			yes = !yes;
			sound(SFX_CURSOR);
		}
		if (btn_pressed(BTN_A)) saves_home_answer(yes);
		return;
	}
	if (btn_pressed(BTN_L) || btn_pressed(BTN_R)) { turn(btn_pressed(BTN_L) ? -1 : 1); return; }
	if (SV.undo && btn_pressed(BTN_START)) { saves_home_choose(SAVES_UNDO); return; }
	if (btn_repeat(BTN_UP)) walk(-1);
	if (btn_repeat(BTN_DOWN)) walk(1);
	/* (a value line: Left and Right turn it, as Options' do) */
	if (SV.focus == SAVES_AUTO && btn_pressed(BTN_LEFT | BTN_RIGHT)) saves_activate(SAVES_AUTO);
	if (btn_pressed(BTN_A)) saves_home_choose(SV.focus);
}
