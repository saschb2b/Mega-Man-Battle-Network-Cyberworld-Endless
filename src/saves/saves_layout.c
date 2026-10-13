/* saves_layout.h. Buttons two a row where the canvas is wide, one a row
 * where it is narrow (a phone held upright), the last of an odd count
 * the row's width; under the panels at home, at the foot of the others. */
#include "saves_layout.h"

#include <limits.h>

#define ROW 18     /* a button's height and its gap */
#define NARROW 230

SavesLayout saves_layout(int width, int height, SavesView view, const int *actions, int n) {
	SavesLayout l = { 0 };
	bool narrow = width < NARROW;
	int cols = narrow ? 1 : 2, rows = (n + cols - 1) / cols, inner = width - 12;
	l.back = (SDL_Rect){ width - 48, 2, 44, 16 };
	int top = 24;
	if (view == SAVES_HOME) {
		l.here = (SDL_Rect){ 6, top, inner, 48 };
		l.line = (SDL_Rect){ 8, top + 51, width - 16, 9 };
		top += 64;
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
