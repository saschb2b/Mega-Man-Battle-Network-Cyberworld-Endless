/* title_summary.h. Two PET screens, as BN6's battle's RESULT gives its
 * record and then what it got: drawn as Options is (options_draw.c), BN6's
 * screen frame with the screen's name on its band, the run's rows in the
 * window, MegaMan in E-Mail's NAVIGATOR box; then the records, what the run
 * opened marked NEW as the JACK-IN SETUP marks it, and the closest goal in
 * KeyItem's information box. Drawing changes nothing. */
#include "title_summary.h"

#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "game.h"
#include "meta.h"
#include "pet_panel.h"
#include "run.h"
#include "save.h"
#include "text.h"
#include "title_lines.h"
#include "title_summary_text.h"

#define SUMMARY_MIN 60   /* frames before the first screen turns (a press meant for the battle) */
#define RECORDS_MIN 30   /* ... before the second closes: its fade, and a beat to read it */

/* where it draws, from the picture's corner. The run: how it ended in
 * SubChip's flat panel, the rows in the window, MegaMan's box to the
 * screen's foot. The records: the rows in the window, the goal under it */
#define ROW_H 12
#define ROW_IN 9          /* a row's label and value in from the field's sides */
#define END_Y 18
#define END_H 28
#define TALLY_Y 47
#define TALLY_H 47        /* three rows */
#define NAV_Y 96
#define REC_Y 17
#define REC_H 86          /* six rows: the Library, the best and what opened */
#define OPENED_ROWS 4
#define NOTE_Y 104
#define NOTE_H 50
#define NOTE_LINES 3

static struct {
	bool records;   /* the second screen */
	int t;          /* frames since the screen opened */
} U;

void title_summary_open(void) {
	memset(&U, 0, sizeof U);
}

bool title_summary_update(void) {
	++U.t;
	if (U.t <= (U.records ? RECORDS_MIN : SUMMARY_MIN) || (!btn_pressed(BTN_A) && !btn_pressed(BTN_START))) return false;
	if (U.records) return true;
	U.records = true;
	U.t = 0;
	audio_sfx(SFX_SELECT);
	return false;
}

/* ---- the run ---- */

/* a helped run's helpers on the band's right, by name where they fit
 * (docs/game/meta/: the summary names a helped run) */
static void helpers_draw(int x0, int y0) {
	if (!(run.helpers & ((1 << HELPERS) - 1))) return;
	char names[64] = "", s[80];
	int on = 0;
	for (int h = 0; h < HELPERS; ++h)
		if (run.helpers >> h & 1) {
			size_t m = strlen(names);
			snprintf(names + m, sizeof names - m, "%s%s", m ? " " : "", meta_helper(h)->name);
			++on;
		}
	summary_helpers(s, sizeof s, names);
	if (look_caps_width(s) > CORE_W - 24) summary_helpers_count(s, sizeof s, on);
	look_band_note(x0, y0, CORE_W, s, LOOK_PAGE);
}

/* how it ended, and where or by whom, in the flat panel: words that say
 * what happened, as SubChip's describe */
static void ending_draw(int x0, int y0) {
	SDL_Rect r = look_panel(x0 + 7, y0 + END_Y, CORE_W - 14, END_H);
	look_text(r.x + 3, r.y, summary_text(title_won ? SM_FALLEN : SM_DELETED), LOOK_TEXT, LOOK_PANEL_SHADE, TEXT_LEFT);
	const char *where = look_text_width(title_cause) <= r.w - 6 || !title_cause_short[0] ? title_cause : title_cause_short;
	look_text(r.x + 3, r.y + ROW_H, where, LOOK_EDGE_HI, LOOK_PANEL_SHADE, TEXT_LEFT);
}

/* a row of the window's field `f`: its label at the left, its value at the right */
static void row_draw(SDL_Rect f, int i, const char *label, const char *value) {
	int y = f.y + 1 + i * ROW_H;
	look_text(f.x + ROW_IN, y, label, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
	look_text(f.x + f.w - ROW_IN, y, value, LOOK_TEXT, LOOK_SHADE, TEXT_RIGHT);
}

static void run_draw(int x0, int y0) {
	SDL_Rect f = look_window(x0 + 7, y0 + TALLY_Y, CORE_W - 14, TALLY_H, 0, false);
	char v[24];
	row_draw(f, 0, summary_text(SM_REACHED), summary_layer(v, sizeof v, run_reached()));
	snprintf(v, sizeof v, "%d", run.viruses_deleted);
	row_draw(f, 1, summary_text(SM_VIRUSES), v);
	snprintf(v, sizeof v, "%d", run.bosses_beaten);
	row_draw(f, 2, summary_text(SM_NAVIS), v);
}

/* MegaMan's word, as E-Mail's NAVIGATOR box has him: how he came home,
 * and on a new line his cheer, or what the loss gave, a new best, a rough
 * start, or on (a first fight with the guardian who deleted him, named in
 * the panel: his battle data, which the next briefing reads) */
static void navigator_draw(int x0, int y0) {
	SDL_Rect pg = look_navigator(x0 + 7, y0 + NAV_Y, CORE_W - 14, summary_text(SM_NAVIGATOR), FACE_MEGAMAN);
	int first = title_won ? SUMMARY_WON : SUMMARY_LOST_HOW;
	int then = title_won ? SUMMARY_WON_HOW
		: title_learned[0] ? SUMMARY_LEARNED : title_new_best ? SUMMARY_BEST : run_reached() <= 2 ? SUMMARY_ROUGH : SUMMARY_FURTHER;
	char say[96];
	snprintf(say, sizeof say, "%s|%s", summary_words(first), summary_words(then));
	look_wrapped(say, pg.x + 4, pg.y + 4, pg.w - 8, 3, LOOK_INK, LOOK_INK_SHADE);
}

/* ---- the records ---- */

/* the Library and what the run added (a dive's none), the best or this
 * week's, NEW beside it where the run went deeper than any before */
static void records_rows(SDL_Rect f) {
	char v[24];
	int added = title_dive ? 0 : meta_library_new();
	row_draw(f, 0, summary_text(SM_LIBRARY), summary_library(v, sizeof v, meta_library_count(-1), added));
	const char *best = summary_text(title_dive ? SM_WEEK : SM_BEST);
	int layer = title_dive ? title_dive : run_reached() > profile.best_depth ? run_reached() : profile.best_depth;
	row_draw(f, 1, best, summary_layer(v, sizeof v, layer));
	if (title_new_best && !title_dive) look_new(f.x + ROW_IN + look_text_width(best) + 5, f.y + 1 + ROW_H + 3, summary_text(SM_NEW));
}

/* what the run opened for the next (docs/game/meta/), each NEW as the setup
 * marks it, and under them in the information box the closest goal */
static void opened_draw(int x0, int y0, SDL_Rect f) {
	const char *open[OPENED_ROWS];
	int n = meta_unlocked(open, OPENED_ROWS);
	for (int i = 0; i < n; ++i) {
		int y = f.y + 1 + (2 + i) * ROW_H;
		const char *mark = summary_text(SM_NEW);
		look_new(f.x + ROW_IN, y + 3, mark);
		look_text(f.x + ROW_IN + look_caps_width(mark) + 6, y, open[i], LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
	}
	const char *next = title_dive ? NULL : meta_next_goal();
	if (!next) return;
	SDL_Rect r = look_info(x0 + 7, y0 + NOTE_Y, CORE_W - 14, NOTE_H, summary_text(SM_INFORMATION));
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	look_wrapped(next, r.x + 6, r.y + 1, r.w - 12, NOTE_LINES, LOOK_INK, LOOK_INK_SHADE);
}

void title_summary_draw(int x0, int y0) {
	if (!U.records) {
		pet_panel_draw(x0, y0, summary_text(SM_RESULT));
		helpers_draw(x0, y0);
		ending_draw(x0, y0);
		run_draw(x0, y0);
		navigator_draw(x0, y0);
	} else {
		pet_panel_draw(x0, y0, summary_text(SM_RECORDS));
		SDL_Rect f = look_window(x0 + 7, y0 + REC_Y, CORE_W - 14, REC_H, 0, false);
		records_rows(f);
		opened_draw(x0, y0, f);
	}
	look_fade_in(x0, y0, CORE_W, CORE_H, U.t);
}
