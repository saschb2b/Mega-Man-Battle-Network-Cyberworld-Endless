/* saves_layout.h. Buttons two a row where the canvas is wide, one a row
 * where it is narrow (a phone held upright), the last of an odd count
 * the row's width; under the panels at home, at the foot of the others. */
#include "saves_layout.h"

#include <limits.h>

#include "pet_look.h"

#define ROW 18     /* a button's height and its gap */
#define NARROW 230
#define CODE_H 34  /* SEND's code in letters twice the size, and a line under it */

/* RECEIVE's keys: the ten digits a row (two of five where narrow) over
 * ERASE and OK; `actions` the digits first */
static void receive_keys(SavesLayout *l, int width, int height, const int *actions, int n, bool narrow) {
	int inner = width - 12, per_row = narrow ? 5 : 10, gap = 2;
	int kw = (inner - (per_row - 1) * gap) / per_row;
	int digits = n > 10 ? 10 : n, rows = (digits + per_row - 1) / per_row;
	int top = height - (rows + 1) * ROW - 4;
	for (int i = 0; i < digits; ++i) {
		SDL_Rect r = { 6 + (i % per_row) * (kw + gap), top + (i / per_row) * ROW, kw, ROW - 2 };
		l->buttons[l->count++] = (SavesButton){ r, actions[i] };
	}
	int bw = (inner - 4) / 2;
	for (int i = digits; i < n && l->count < SAVES_ACTIONS; ++i) {
		SDL_Rect r = { 6 + (i - digits) % 2 * (bw + 4), top + rows * ROW, bw, ROW - 2 };
		l->buttons[l->count++] = (SavesButton){ r, actions[i] };
	}
	l->line = (SDL_Rect){ 10, 26, width - 20, top - 30 };
}

SavesLayout saves_layout(int width, int height, SavesView view, const int *actions, int n) {
	SavesLayout l = { 0 };
	bool narrow = width < NARROW;
	int cols = narrow ? 1 : 2, rows = (n + cols - 1) / cols, inner = width - 12;
	l.back = (SDL_Rect){ 0, 0, 76, 20 };   /* (the band's left: its arrow and the name, BACK's tap) */
	int top = 24;
	if (view == SAVES_RECEIVE_CODE) {
		receive_keys(&l, width, height, actions, n, narrow);
		return l;
	}
	if (view == SAVES_HOME || view == SAVES_SEND_CODE) {
		l.here = (SDL_Rect){ 6, top, inner, 48 };
		l.line = (SDL_Rect){ 8, top + 51, width - 16, 9 };
		top += view == SAVES_HOME ? 64 : 52;
	} else if (view == SAVES_COMPARE) {
		int ph = 58;   /* (a title and four rows) */
		if (narrow) {
			l.here = (SDL_Rect){ 6, top, inner, ph };
			l.file = (SDL_Rect){ 6, top + ph + 3, inner, ph };
		} else {
			l.here = (SDL_Rect){ 6, top, (inner - 4) / 2, ph };
			l.file = (SDL_Rect){ 10 + (inner - 4) / 2, top, (inner - 4) / 2, ph };
		}
		top = l.file.y + ph + 4;
	}
	int buttons_y = view == SAVES_HOME ? top : height - rows * ROW - 4;
	/* (SEND's code under its panel, its words to the foot's button) */
	if (view == SAVES_SEND_CODE) l.code = (SDL_Rect){ 10, top, width - 20, CODE_H };
	if (view == SAVES_SEND_CODE) top += CODE_H;
	if (view != SAVES_HOME) l.line = (SDL_Rect){ 10, top + (view == SAVES_RESULT ? 6 : 0), width - 20, buttons_y - top - 6 };
	int bw = cols == 1 ? inner : (inner - 4) / 2;
	for (int i = 0; i < n && i < SAVES_ACTIONS; ++i) {
		int row = i / cols, col = i % cols;
		bool whole = cols == 2 && i == n - 1 && n % 2;
		SDL_Rect r = { 6 + col * (bw + 4), buttons_y + row * ROW, whole ? inner : bw, ROW - 2 };
		l.buttons[l.count++] = (SavesButton){ r, actions[i] };
	}
	return l;
}

static bool ahead(SDL_Rect from, SDL_Rect to, int dx, int dy) {
	if (dx > 0) return to.x >= from.x + from.w;
	if (dx < 0) return to.x + to.w <= from.x;
	if (dy > 0) return to.y >= from.y + from.h;
	return dy < 0 && to.y + to.h <= from.y;
}

int saves_layout_move(const SavesLayout *l, int focus, int dx, int dy) {
	int current = -1;
	for (int i = 0; i < l->count; ++i) if (l->buttons[i].action == focus) current = i;
	if (current < 0) return l->count ? l->buttons[0].action : focus;
	SDL_Rect from = l->buttons[current].rect;
	int best = current, best_across = INT_MAX, best_along = INT_MAX;
	for (int i = 0; i < l->count; ++i) {
		SDL_Rect to = l->buttons[i].rect;
		if (!ahead(from, to, dx, dy)) continue;
		int x = (2 * to.x + to.w) - (2 * from.x + from.w);
		int y = (2 * to.y + to.h) - (2 * from.y + from.h);
		int along = dx ? x * dx : y * dy;
		int across = dx ? y : x;
		if (along <= 0) continue;
		if (across < 0) across = -across;
		if (across > best_across || (across == best_across && along >= best_along)) continue;
		best_across = across;
		best_along = along;
		best = i;
	}
	return l->buttons[best].action;
}

/* Home, on Options' measures (options_draw.c): the window 7 in from the
 * sides, 3 between its parts; under the band the pages' tabs (none for
 * one page), this device's strip, the page's lines in their window, the
 * information box to the foot with INFORMATION over its page. On the
 * GBA's own 160 rows each part as small as keeps its margins (lines 12,
 * a strip of one line in the box's edge alone), so the box still holds
 * two lines; a taller canvas gives lines 13, held upright 18 (a
 * thumb's), and the strip its second line */
#define SIDE 7
#define GAP 3
#define LINES_MOST 4

SavesLayout saves_home_layout(int width, int height, int pages, const int *lines, int n) {
	SavesLayout l = { 0 };
	bool compact = height < 180;
	int inner = width - 2 * SIDE, y = compact ? 22 : 23;
	int line_h = height >= 240 ? 18 : compact ? 12 : 13, pad = compact ? 1 : 2;
	l.back = (SDL_Rect){ 0, 0, 76, 20 };
	if (pages > 1) {
		l.tabs = (SDL_Rect){ SIDE, y, inner, LOOK_PILL_H };
		int left = SIDE + LOOK_TABS_IN, right = SIDE + inner - LOOK_TABS_IN, tw = (right - left - (pages - 1) * LOOK_TAB_GAP) / pages;
		/* (each tab's tap a little taller than its pill) */
		for (int p = 0; p < pages && p < 3; ++p) l.tab[p] = (SDL_Rect){ left + p * (tw + LOOK_TAB_GAP), y - 2, tw, LOOK_PILL_H + 4 };
		l.tab_count = pages < 3 ? pages : 3;
		y += LOOK_PILL_H + GAP;
	}
	l.here = (SDL_Rect){ SIDE, y, inner, height >= 200 ? 28 : compact ? 15 : 17 };
	y += l.here.h + GAP;
	l.list = (SDL_Rect){ SIDE, y, inner, LINES_MOST * line_h + 2 * LOOK_FRAME + 2 * pad };
	/* (each line the window's field wide, Delete a line's room apart) */
	for (int i = 0, row = 0; i < n && row < LINES_MOST; ++i, ++row) {
		if (lines[i] == SAVES_DELETE && row) ++row;
		SDL_Rect r = { SIDE + LOOK_FRAME, y + LOOK_FRAME + pad + row * line_h, inner - 2 * LOOK_FRAME, line_h };
		l.buttons[l.count++] = (SavesButton){ r, lines[i] };
	}
	y += l.list.h + GAP;
	l.info = (SDL_Rect){ SIDE, y, inner, height - GAP - y };
	return l;
}
