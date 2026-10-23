/* The Arena's two PET screens (arena_card.h), in BN6's PET screens' look
 * (pet_look.h): the band with the screen's name and its note at the
 * right, the body a window's navy field, values on slots, names in gold. */
#include "arena_card.h"

#include <stdio.h>

#include "arena_text.h"
#include "audio.h"
#include "gfx.h"
#include "pet_look.h"
#include "pixfont.h"
#include "platform.h"
#include "second_frame.h"

#define W      240
#define H      160
#define HEAD   18
#define BODY_Y (HEAD + 4)
#define INK    rgba(0, 49, 74, 255)   /* a shade under pixel letters */
#define PLATE_W 100
#define PLATE_H 70
#define ROW_H   14
#define COL0    150   /* the card's columns' centres: mine, his */
#define COL1    204

/* the screen, its band with `place` as its note, the window whose field the card fills */
static void frame(int x, int y, const char *place) {
	look_screen(x, y, W, H, arena_title);
	look_band_note(x, y, W, place, LOOK_PAGE);
	/* (under the note's shade, as BN6's windows sit under its hints) */
	look_window(x + 5, y + LOOK_BAND_H + 4, W - 10, H - LOOK_BAND_H - 4 - LOOK_RAIL - 1, 0, false);
}

/* a label in gold and its value in the engine's small letters, white, on one line w wide */
static void small_row(int x, int y, int w, const char *label, const char *value) {
	pixfont_draw(x, y, label, PET_GOLD, INK, PIXFONT_LEFT, 1);
	int lw = pixfont_width(label, 1) + 6;
	char lines[2][96];
	int n = pixfont_wrap(value, w - lw, 1, lines, 2);
	for (int i = 0; i < n; ++i) pixfont_draw(x + lw, y + i * PIXFONT_LINE, lines[i], PET_WHITE, INK, PIXFONT_LEFT, 1);
}

/* a side's wait at the plate's foot: a bar for my way in, else its word */
static void side_wait(int x, int y, ArenaSideWait wait, int progress, uint32_t t) {
	second_slot(x, y, PLATE_W, 14);
	if (wait == ARENA_SIDE_READY) { text_draw(x + PLATE_W / 2, y + 1, arena_ready, PET_GOLD, TEXT_CENTER); return; }
	if (progress >= 0) {
		int bw = PLATE_W - 8, done = bw * progress / 100;
		fill_rect(x + 4, y + 5, bw, 4, PET_EXIT);
		fill_rect(x + 4, y + 5, done, 4, PET_CYAN_HI);
		return;
	}
	/* (his side: the word, blinking as BN6's waits blink) */
	if (t / 30 % 2 == 0) text_draw(x + PLATE_W / 2, y + 1, wait == ARENA_SIDE_COMING ? arena_coming : arena_linking, PET_CYAN_HI, TEXT_CENTER);
}

/* a player's plate: his name on a slot, his MegaMan as carried (once known) */
static void plate(int x, int y, const char *name, const OnlineLoadout *l, bool known) {
	second_slot(x, y, PLATE_W, PLATE_H);
	fill_rect(x + 1, y + 1, PLATE_W - 2, 14, PET_EXIT);
	text_draw(x + PLATE_W / 2, y + 2, name, PET_WHITE, TEXT_CENTER);
	if (!known) return;
	char hp[16];
	/* (a Team NetBattle's winner goes on with what he has left: his HP of his max) */
	if (l && l->hp && l->hp < l->max_hp) snprintf(hp, sizeof hp, "%u/%u", (unsigned)l->hp, (unsigned)l->max_hp);
	else snprintf(hp, sizeof hp, "%u", l ? (unsigned)l->max_hp : 100u);
	text_draw(x + 5, y + 18, arena_hp, PET_GOLD, TEXT_LEFT);
	text_draw(x + PLATE_W - 5, y + 18, hp, PET_WHITE, TEXT_RIGHT);
	small_row(x + 5, y + 34, PLATE_W - 8, arena_cross, arena_crosses_named(l ? l->crosses : 0));
	small_row(x + 5, y + 34 + 2 * PIXFONT_LINE - 4, PLATE_W - 8, arena_soul, arena_souls_named(l ? l->souls : 0));
}

#define SMALL_H 30   /* a 2v2's plate: the name, the HP */

/* a 2v2's plate: his name on a slot, his HP once his MegaMan has come; mine lit */
static void small_plate(int x, int y, const char *name, const OnlineLoadout *l, bool known, bool mine) {
	second_slot(x, y, PLATE_W, SMALL_H);
	fill_rect(x + 1, y + 1, PLATE_W - 2, 14, mine ? PET_GOLD : PET_EXIT);
	text_draw(x + PLATE_W / 2, y + 2, name, mine ? INK : PET_WHITE, TEXT_CENTER);
	if (!known) return;
	char hp[16];
	snprintf(hp, sizeof hp, "%u", l ? (unsigned)l->max_hp : 100u);
	text_draw(x + 5, y + 15, arena_hp, PET_GOLD, TEXT_LEFT);
	text_draw(x + PLATE_W - 5, y + 15, hp, PET_WHITE, TEXT_RIGHT);
}

/* the way in of two MegaMen a side: my team's plates on the left, theirs on the right */
static void way_four(int x, int y, const ArenaWayView *v) {
	int py = y + BODY_Y + 6;
	for (int k = 0; k < 4; ++k) {
		int px = x + 12 + (k / 2) * (PLATE_W + 16), ky = py + (k % 2) * (SMALL_H + 34);
		small_plate(px, ky, v->names[k], v->loadouts[k], v->knowns[k], k == v->me);
		side_wait(px, ky + SMALL_H + 3, v->waits[k], k == v->me && v->waits[k] != ARENA_SIDE_READY ? v->progress : -1, v->t);
	}
	text_draw(x + W / 2, py + SMALL_H + 10, arena_vs, PET_GOLD, TEXT_CENTER);
}

void arena_card_way(const ArenaWayView *v) {
	int x = P.core_x, y = P.core_y;
	frame(x, y, v->four ? arena_side_by_side : v->team ? arena_team : arena_practice);
	if (v->four) { way_four(x, y, v); return; }
	int py = y + BODY_Y + 4;
	for (int side = 0; side < 2; ++side) {
		int px = x + 12 + side * (PLATE_W + 16);
		plate(px, py, v->name[side], v->loadout[side], v->known[side]);
		side_wait(px, py + PLATE_H + 4, v->wait[side], side == 0 && v->wait[0] != ARENA_SIDE_READY ? v->progress : -1, v->t);
	}
	text_draw(x + W / 2, py + PLATE_H / 2 - 6, arena_vs, PET_GOLD, TEXT_CENTER);
	if (!v->set) return;
	int sy = py + PLATE_H + 24;
	second_row(x + W / 2 - 60, sy, 120, arena_set, arena_score(v->won, v->lost));
}

/* a row on a slot, ROW_H tall: `label` in gold, then a value under each column (`v1` NULL: `v0` at the right) */
static void cell_row(int x, int y, const char *label, const char *v0, const char *v1) {
	second_slot(x + 12, y, W - 24, ROW_H);
	text_draw(x + 16, y + 1, label, PET_GOLD, TEXT_LEFT);
	if (!v1) { text_draw(x + W - 16, y + 1, v0, PET_WHITE, TEXT_RIGHT); return; }
	text_draw(x + COL0, y + 1, v0, PET_WHITE, TEXT_CENTER);
	text_draw(x + COL1, y + 1, v1, PET_WHITE, TEXT_CENTER);
}

/* the rows of a battle read: HP left and chips used, mine and his, then the time and the set */
static void rows(int x, int y, const ArenaCardView *v) {
	text_draw(x + COL0, y, arena_you, PET_CYAN_HI, TEXT_CENTER);
	text_draw(x + COL1, y, v->name, PET_CYAN_HI, TEXT_CENTER);
	char cell[2][16];
	snprintf(cell[0], sizeof cell[0], "%u", v->hp[0]);
	snprintf(cell[1], sizeof cell[1], "%u", v->hp[1]);
	cell_row(x, y + 12, arena_hp_left, cell[0], cell[1]);
	snprintf(cell[0], sizeof cell[0], "%u", v->used[0]);
	snprintf(cell[1], sizeof cell[1], "%u", v->used[1]);
	cell_row(x, y + 13 + ROW_H, arena_chips_used, cell[0], cell[1]);
	unsigned s = v->frames / 60;
	snprintf(cell[0], sizeof cell[0], "%u:%02u", s / 60, s % 60);
	/* (the time on the left half, the set's score on the right) */
	int ty = y + 14 + 2 * ROW_H;
	cell_row(x, ty, arena_time, v->set ? "" : cell[0], NULL);
	if (!v->set) return;
	text_draw(x + 16 + text_width(arena_time) + 8, ty + 1, cell[0], PET_WHITE, TEXT_LEFT);
	text_draw(x + COL0 - 10, ty + 1, arena_set, PET_GOLD, TEXT_LEFT);
	text_draw(x + W - 16, ty + 1, arena_score(v->won, v->lost), PET_WHITE, TEXT_RIGHT);
}

/* the rematch asked: the question, Yes and No (the cursor's lit), and his answer as it comes */
static void rematch(int x, int y, const ArenaCardView *v) {
	text_draw(x + 13, y, arena_again_asked(v->name), PET_WHITE, TEXT_LEFT);
	for (int k = 0; k < 2; ++k) {
		bool lit = v->ask == ARENA_ASK_OPEN ? v->yes == (k == 0) : v->ask == ARENA_ASK_SENT && k == 0;
		int bx = x + W - 13 - 2 * 36 + k * 36;
		if (lit) fill_rect(bx, y - 1, 32, 14, PET_GOLD);
		else second_slot(bx, y - 1, 32, 14);
		text_draw(bx + 16, y, k ? arena_no : arena_yes, lit ? INK : PET_DIM, TEXT_CENTER);
	}
	const char *line = v->theirs == ARENA_THEIRS_YES ? arena_again_wanted(v->name) :
		v->theirs == ARENA_THEIRS_GONE ? arena_again_gone(v->name) :
		v->ask == ARENA_ASK_SENT ? arena_again_waiting(v->name) : NULL;
	if (line && (v->theirs != ARENA_THEIRS_UNSAID || v->t / 30 % 2 == 0))
		text_draw(x + 13, y + 14, line, v->theirs == ARENA_THEIRS_YES ? PET_GOLD : PET_CYAN_HI, TEXT_LEFT);
}

/* a team bout's foot: the MegaMen standing and who is next, or the match's end */
static void team_foot(int x, int y, const ArenaCardView *v) {
	/* (side by side: the team's result alone) */
	if (v->four) {
		second_slot(x + 12, y + 11, W - 24, 16);
		if (v->team_result) text_draw(x + W / 2, y + 13, arena_team_end(v->team_result), v->team_result == 1 ? PET_GOLD : PET_CYAN_HI, TEXT_CENTER);
		return;
	}
	second_slot(x + 12, y - 1, W - 24, 28);
	text_draw(x + 16, y, arena_standing, PET_GOLD, TEXT_LEFT);
	text_draw(x + W - 16, y, arena_score(v->standing[0], v->standing[1]), PET_WHITE, TEXT_RIGHT);
	if (v->team_result) text_draw(x + W / 2, y + 13, arena_team_end(v->team_result), v->team_result == 1 ? PET_GOLD : PET_CYAN_HI, TEXT_CENTER);
	else if (v->next[0] && v->next[1]) text_draw(x + W / 2, y + 13, arena_team_next(v->next[0], v->next[1]), PET_CYAN_HI, TEXT_CENTER);
}

/* the rows of a 2v2 read: each MegaMan's HP left, my team's on the left, theirs on the right, then the time */
static void rows_four(int x, int y, const ArenaCardView *v) {
	for (int k = 0; k < 2; ++k) {
		int ry = y + k * (ROW_H + 1);
		second_slot(x + 12, ry, W - 24, ROW_H);
		for (int team = 0; team < 2; ++team) {
			int cx = x + 16 + team * (W / 2 - 8);
			char hp[8];
			snprintf(hp, sizeof hp, "%u", v->hp4[2 * team + k]);
			text_draw(cx, ry + 1, v->names[2 * team + k], team ? PET_CYAN_HI : PET_WHITE, TEXT_LEFT);
			text_draw(cx + W / 2 - 22, ry + 1, hp, PET_WHITE, TEXT_RIGHT);
		}
	}
	unsigned s = v->frames / 60;
	char t[16];
	snprintf(t, sizeof t, "%u:%02u", s / 60, s % 60);
	cell_row(x, y + 2 * (ROW_H + 1) + 1, arena_time, t, NULL);
}

void arena_card_result(const ArenaCardView *v) {
	int x = P.core_x, y = P.core_y;
	frame(x, y, v->four ? arena_side_by_side : v->team ? arena_team : arena_practice);
	static const char *const head[6] = { arena_ended, arena_won, arena_lost, arena_ended, arena_broke, arena_stopped };
	const char *h = head[v->result <= 5 ? v->result : 0];
	int by = y + BODY_Y + 2;
	text_draw_scaled(x + 13, by, h, v->result == 1 ? PET_GOLD : PET_WHITE, TEXT_LEFT, 2);
	/* (a 2v2's two names in the small letters, clear of the head) */
	if (v->four) pixfont_draw(x + W - 13, by + 12, arena_vs_name(v->name), PET_CYAN_HI, INK, PIXFONT_RIGHT, 1);
	else text_draw(x + W - 13, by + 9, arena_vs_name(v->name), PET_CYAN_HI, TEXT_RIGHT);
	if (v->why) second_wrapped(v->why, x + 13, by + 30, W - 26, 4, PET_WHITE);
	else if (v->tally && v->four) rows_four(x, by + 26, v);
	else if (v->tally) rows(x, by + 26, v);
	if (v->team) team_foot(x, y + H - 50, v);
	else if (v->ask != ARENA_ASK_NONE || v->theirs != ARENA_THEIRS_UNSAID) rematch(x, y + H - 48, v);
	const char *keys = v->ask == ARENA_ASK_OPEN ? arena_keys_ask : v->square ? arena_keys_back : arena_back;
	if (v->ask != ARENA_ASK_SENT) pixfont_draw(x + W / 2, y + H - 20, keys, LOOK_EDGE_HI, INK, PIXFONT_CENTER, 1);
}

void arena_card_fade(int alpha, bool white) {
	if (alpha <= 0) return;
	int a = alpha > 255 ? 255 : alpha;
	fill_rect(P.core_x, P.core_y, W, H, white ? rgba(247, 255, 247, a) : rgba(0, 0, 0, a));
}

void arena_card_cue(ArenaCue cue) {
	static const Sfx sfx[] = { SFX_DESC_OPEN, SFX_DESC_CLOSE, SFX_CHIP_CURSOR, SFX_CHIP_SELECT, SFX_CANCEL, SFX_ITEM, SFX_ENCOUNTER, SFX_REVEAL };
	if ((unsigned)cue < sizeof sfx / sizeof *sfx) audio_sfx(sfx[cue]);
}
