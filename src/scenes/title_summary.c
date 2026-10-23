/* title_summary.h */
#include "title_summary.h"

#include <stdio.h>
#include <string.h>

#include "game.h"
#include "gfx.h"
#include "meta.h"
#include "minifont.h"
#include "platform.h"
#include "run.h"
#include "save.h"
#include "text.h"
#include "title_lines.h"

#define SUMMARY_FACE_X 30   /* Lan's face on it */
#define SUMMARY_FACE_Y 66

/* a helped run's helpers, small over its summary's title (docs/META.md:
 * the summary names a helped run): "HELP: HP+ ALL *" */
static void helpers_named(int cx, int y) {
	if (!(run.helpers & ((1 << HELPERS) - 1))) return;
	char s[48] = "Help:";
	for (int h = 0; h < HELPERS; ++h)
		if (run.helpers >> h & 1) {
			size_t m = strlen(s);
			snprintf(s + m, sizeof s - m, " %s", meta_helper(h)->name);
		}
	minifont_draw_centered(cx, y, s, rgba(150, 160, 190, 255), 1);
}

/* The finished run's summary over its area: how it ended, how far, the
 * Library, the best, Dad's and Lan's words, what it opened or the
 * closest goal, and a helped run's helpers */
void title_summary_draw(int x0, int y0) {
	SDL_Color gold = rgba(255, 230, 90, 255), sky = rgba(170, 200, 255, 255);
	int x = x0 + CORE_W / 2;
	helpers_named(x, y0 + 2);
	if (title_won) text_draw(x, y0 + 8, "The Nest has fallen!", gold, TEXT_CENTER);
	else text_draw(x, y0 + 8, "MegaMan was deleted", rgba(255, 120, 120, 255), TEXT_CENTER);
	if (title_cause[0]) text_draw(x, y0 + 22, title_cause, sky, TEXT_CENTER);
	/* Lan at his PET, and how far they got */
	Sprite *lan = sprite_get(SPR_MUGSHOT, 0x00);
	if (lan) sprite_draw_frame(lan, 0, 0, x0 + SUMMARY_FACE_X, y0 + SUMMARY_FACE_Y, false, 0, 0);
	int lx = x0 + 70, rx = x0 + CORE_W - 14;
	text_draw(lx, y0 + 40, "Reached", WHITE, TEXT_LEFT);
	text_drawf(rx, y0 + 40, WHITE, TEXT_RIGHT, "Layer %d", run_reached());
	text_draw(lx, y0 + 52, "Viruses deleted", WHITE, TEXT_LEFT);
	text_drawf(rx, y0 + 52, WHITE, TEXT_RIGHT, "%d", run.viruses_deleted);
	text_draw(lx, y0 + 64, "Navis deleted", WHITE, TEXT_LEFT);
	text_drawf(rx, y0 + 64, WHITE, TEXT_RIGHT, "%d", run.bosses_beaten);
	/* the Library and what the run added (docs/META.md); a dive's none, its week's best below */
	text_draw(lx, y0 + 76, "Library", WHITE, TEXT_LEFT);
	int lib = meta_library_count(-1), added = title_dive ? 0 : meta_library_new();
	if (added) text_drawf(rx, y0 + 76, WHITE, TEXT_RIGHT, "%d (+%d)", lib, added);
	else text_drawf(rx, y0 + 76, WHITE, TEXT_RIGHT, "%d", lib);
	if (title_new_best && !title_dive) text_draw(lx, y0 + 89, "New best!", gold, TEXT_LEFT);
	else {
		text_draw(lx, y0 + 89, title_dive ? "This week" : "Best", gold, TEXT_LEFT);
		text_drawf(rx, y0 + 89, gold, TEXT_RIGHT, "Layer %d", title_dive ? title_dive : profile.best_depth);
	}
	/* what the run opened for the next (docs/META.md): three lines
	 * where it opened three, the lines above drawn closer (a won run
	 * opened the endless net, a threat, a mark and a Cross start, and
	 * the summary named two) */
	const char *open[3];
	int n = meta_unlocked(open, 3), up = n >= 3 ? 2 : 0;
	/* Dad's backup, as his call promised, and Lan's word; a won run
	 * jacks out */
	if (title_won) {
		text_draw(x, y0 + 102 - up, summary_words(SUMMARY_WON_HOW), sky, TEXT_CENTER);
		text_draw(x, y0 + 114 - 2 * up, summary_words(SUMMARY_WON), WHITE, TEXT_CENTER);
	} else {
		/* (a line the game's width holds: "brought MegaMan home." ran off
		 * both sides where the screen shows no more than the game's 240
		 * pixels, a 480 x 320 handheld's or a 640 x 480 filled, issue #36) */
		text_draw(x, y0 + 102 - up, summary_words(SUMMARY_LOST_HOW), sky, TEXT_CENTER);
		/* (a first fight with the guardian who deleted him, named on
		 * the line above: what it gave, his battle data, which the
		 * next briefing reads, before a new best, which "New best!"
		 * says above; "We've got BlastMan's battle data now!" ran off
		 * the screen's sides) */
		int said = title_learned[0] ? SUMMARY_LEARNED : title_new_best ? SUMMARY_BEST : run_reached() <= 2 ? SUMMARY_ROUGH : SUMMARY_FURTHER;
		text_draw(x, y0 + 114 - 2 * up, summary_words(said), WHITE, TEXT_CENTER);
	}
	/* ... else the closest goal */
	int y = y0 + (n >= 3 ? 124 : 129);
	for (int i = 0; i < n; ++i, y += 12) text_drawf(x, y, gold, TEXT_CENTER, "Unlocked: %s", open[i]);
	const char *next = title_dive ? NULL : meta_next_goal();
	if (next && n < 2) text_draw(x, y, next, sky, TEXT_CENTER);
}
