/* The Arena's two PET screens (arena_card.h). After BN6's PET screens and
 * the project's PET panels (second_frame.c, launcher_draw.c): a dark
 * green band with the screen's name and BN6's three stripes, a slot at
 * its right, the body framed in the PET's cyan on navy, values on dark
 * slots, names in gold. */
#include "arena_card.h"

#include <stdio.h>

#include "arena_text.h"
#include "audio.h"
#include "gfx.h"
#include "pixfont.h"
#include "platform.h"
#include "second_frame.h"

#define W      240
#define H      160
#define HEAD   18
#define BODY_Y (HEAD + 4)
#define INK    rgba(0, 49, 74, 255)   /* a shade under pixel letters */
#define PLATE_W 104
#define PLATE_H 70
#define ROW_H   14
#define COL0    150   /* the card's columns' centres: mine, his */
#define COL1    204

/* the band, the green round the body and the body's cyan edge on navy,
 * `place` on a slot at the band's right */
static void frame(int x, int y, const char *place) {
	fill_rect(x, y, W, H, PET_GREEN);
	fill_rect(x, y, W, HEAD, PET_DARK);
	fill_rect(x, y + HEAD, W, 1, PET_LINE);
	text_draw(x + 6, y + 3, arena_title, PET_WHITE, TEXT_LEFT);
	second_stripes(x + 12 + text_width(arena_title), y + 3);
	int pw = text_width(place) + 12, px = x + W - 4 - pw;
	second_slot(px, y + 2, pw, HEAD - 4);
	text_draw(px + 6, y + 3, place, PET_CYAN_HI, TEXT_LEFT);
	SDL_Rect b = { x + 4, y + BODY_Y, W - 8, H - BODY_Y - 4 };
	SDL_Rect edge[4] = { { b.x - 2, b.y - 2, b.w + 4, 2 }, { b.x - 2, b.y + b.h, b.w + 4, 2 }, { b.x - 2, b.y, 2, b.h }, { b.x + b.w, b.y, 2, b.h } };
	fill_rects(edge, 4, PET_CYAN);
	fill_rect(b.x - 1, b.y - 1, b.w + 2, 1, PET_CYAN_HI);
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
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
	snprintf(hp, sizeof hp, "%u", l ? (unsigned)l->max_hp : 100u);
	text_draw(x + 5, y + 18, arena_hp, PET_GOLD, TEXT_LEFT);
	text_draw(x + PLATE_W - 5, y + 18, hp, PET_WHITE, TEXT_RIGHT);
	small_row(x + 5, y + 34, PLATE_W - 8, arena_cross, arena_crosses_named(l ? l->crosses : 0));
	small_row(x + 5, y + 34 + 2 * PIXFONT_LINE - 4, PLATE_W - 8, arena_soul, arena_souls_named(l ? l->souls : 0));
}

void arena_card_way(const ArenaWayView *v) {
	int x = P.core_x, y = P.core_y;
	frame(x, y, arena_practice);
	int py = y + BODY_Y + 4;
	for (int side = 0; side < 2; ++side) {
		int px = x + 8 + side * (PLATE_W + 20);
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
	second_slot(x + 8, y, W - 16, ROW_H);
	text_draw(x + 13, y + 1, label, PET_GOLD, TEXT_LEFT);
	if (!v1) { text_draw(x + W - 13, y + 1, v0, PET_WHITE, TEXT_RIGHT); return; }
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
	text_draw(x + 13 + text_width(arena_time) + 8, ty + 1, cell[0], PET_WHITE, TEXT_LEFT);
	text_draw(x + COL0 - 10, ty + 1, arena_set, PET_GOLD, TEXT_LEFT);
	text_draw(x + W - 13, ty + 1, arena_score(v->won, v->lost), PET_WHITE, TEXT_RIGHT);
}

/* the rematch asked: the question, Yes and No (the cursor's lit), and his answer as it comes */
static void rematch(int x, int y, const ArenaCardView *v) {
	text_draw(x + 8, y, arena_again_asked(v->name), PET_WHITE, TEXT_LEFT);
	for (int k = 0; k < 2; ++k) {
		bool lit = v->ask == ARENA_ASK_OPEN ? v->yes == (k == 0) : v->ask == ARENA_ASK_SENT && k == 0;
		int bx = x + W - 8 - 2 * 36 + k * 36;
		if (lit) fill_rect(bx, y - 1, 32, 14, PET_GOLD);
		else second_slot(bx, y - 1, 32, 14);
		text_draw(bx + 16, y, k ? arena_no : arena_yes, lit ? INK : PET_DIM, TEXT_CENTER);
	}
	const char *line = v->theirs == ARENA_THEIRS_YES ? arena_again_wanted(v->name) :
		v->theirs == ARENA_THEIRS_GONE ? arena_again_gone(v->name) :
		v->ask == ARENA_ASK_SENT ? arena_again_waiting(v->name) : NULL;
	if (line && (v->theirs != ARENA_THEIRS_UNSAID || v->t / 30 % 2 == 0))
		text_draw(x + 8, y + 14, line, v->theirs == ARENA_THEIRS_YES ? PET_GOLD : PET_CYAN_HI, TEXT_LEFT);
}

void arena_card_result(const ArenaCardView *v) {
	int x = P.core_x, y = P.core_y;
	frame(x, y, arena_practice);
	static const char *const head[6] = { arena_ended, arena_won, arena_lost, arena_ended, arena_broke, arena_stopped };
	const char *h = head[v->result <= 5 ? v->result : 0];
	int by = y + BODY_Y + 2;
	text_draw_scaled(x + 10, by, h, v->result == 1 ? PET_GOLD : PET_WHITE, TEXT_LEFT, 2);
	text_draw(x + W - 10, by + 9, arena_vs_name(v->name), PET_CYAN_HI, TEXT_RIGHT);
	if (v->why) second_wrapped(v->why, x + 10, by + 30, W - 20, 4, PET_WHITE);
	else if (v->tally) rows(x, by + 26, v);
	if (v->ask != ARENA_ASK_NONE || v->theirs != ARENA_THEIRS_UNSAID) rematch(x, y + H - 44, v);
	const char *keys = v->ask == ARENA_ASK_OPEN ? arena_keys_ask : v->square ? arena_keys_back : arena_back;
	if (v->ask != ARENA_ASK_SENT) pixfont_draw(x + W / 2, y + H - 14, keys, PET_DIM, INK, PIXFONT_CENTER, 1);
}

void arena_card_fade(int alpha, bool white) {
	if (alpha <= 0) return;
	int a = alpha > 255 ? 255 : alpha;
	fill_rect(P.core_x, P.core_y, W, H, white ? rgba(247, 255, 247, a) : rgba(0, 0, 0, a));
}

void arena_card_cue(ArenaCue cue) {
	static const Sfx sfx[] = { SFX_DESC_OPEN, SFX_DESC_CLOSE, SFX_CHIP_CURSOR, SFX_CHIP_SELECT, SFX_CANCEL, SFX_ITEM, SFX_ENCOUNTER };
	if ((unsigned)cue < sizeof sfx / sizeof *sfx) audio_sfx(sfx[cue]);
}
