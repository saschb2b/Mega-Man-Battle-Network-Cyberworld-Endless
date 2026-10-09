#include "saves_layout.h"

#include <limits.h>

static void add(SavesLayout *l, int x, int y, int w, int h, int action) {
	l->buttons[l->count++] = (SavesButton){ { x, y, w, h }, action };
}

static void pair(SavesLayout *l, int w, int y, int left, int right) {
	int bw = (w - 16) / 2;
	add(l, 6, y, bw, 16, left);
	add(l, 10 + bw, y, bw, 16, right);
}

SavesLayout saves_layout(int width, int height, SavesView view, bool resume, bool result_undo) {
	SavesLayout l = { 0 };
	l.back = (SDL_Rect){ width - 48, 2, 44, 16 };
	l.hint = (SDL_Rect){ 8, height - 10, width - 16, 9 };
	bool narrow = width < 230;
	if (view == SAVES_HOME) {
		l.hero = (SDL_Rect){ 6, 24, width - 12, 45 };
		l.status = (SDL_Rect){ 6, 70, width - 12, 24 };
		if (narrow) {
			static const int actions[] = { SAVES_EXPORT, SAVES_IMPORT, SAVES_UNDO, SAVES_DETAILS, SAVES_OPTIONS };
			for (int i = 0; i < 5; ++i) add(&l, 6, 96 + i * 18, width - 12, 16, actions[i]);
			l.hint = (SDL_Rect){ 8, 189, width - 16, height - 191 };
		} else {
			pair(&l, width, 96, SAVES_EXPORT, SAVES_IMPORT);
			pair(&l, width, 114, SAVES_UNDO, SAVES_DETAILS);
			add(&l, 6, 132, width - 12, 16, SAVES_OPTIONS);
		}
	} else if (view == SAVES_TRANSFER) {
		l.status = (SDL_Rect){ 6, 24, width - 12, 25 };
		add(&l, 6, 53, width - 12, 18, SAVES_AUTO);
		add(&l, 6, 74, width - 12, 18, SAVES_FOLDER);
		add(&l, 6, 95, width - 12, 18, SAVES_RETRY);
		l.hint = (SDL_Rect){ 8, 118, width - 16, height - 120 };
	} else if (view == SAVES_COMPARE) {
		if (narrow) {
			l.hero = (SDL_Rect){ 6, 24, width - 12, 42 };
			l.file = (SDL_Rect){ 6, 69, width - 12, 42 };
			l.note = (SDL_Rect){ 8, 114, width - 16, 44 };
			static const int actions[] = { SAVES_KEEP, SAVES_TAKE, SAVES_HERE_DETAILS, SAVES_FILE_DETAILS };
			for (int i = 0; i < 4; ++i) add(&l, 6, height - 74 + i * 18, width - 12, 16, actions[i]);
		} else {
			int bw = (width - 16) / 2;
			l.hero = (SDL_Rect){ 6, 24, bw, 58 };
			l.file = (SDL_Rect){ bw + 10, 24, bw, 58 };
			l.note = (SDL_Rect){ 8, 85, width - 16, 32 };
			pair(&l, width, height - 41, SAVES_KEEP, SAVES_TAKE);
			pair(&l, width, height - 23, SAVES_HERE_DETAILS, SAVES_FILE_DETAILS);
		}
		l.hint.h = 0;
	} else if (view == SAVES_DETAIL) {
		l.content = (SDL_Rect){ 8, 38, width - 16, height - 68 };
		pair(&l, width, height - 26, SAVES_PREV, SAVES_NEXT);
	} else {
		int action = resume ? SAVES_CONTINUE : SAVES_DONE;
		int y = height - (narrow && result_undo ? 61 : 43);
		l.note = (SDL_Rect){ 10, 28, width - 20, y - 32 };
		if (narrow && result_undo) {
			add(&l, 6, y, width - 12, 18, action);
			add(&l, 6, y + 20, width - 12, 16, SAVES_UNDO);
		} else if (result_undo) pair(&l, width, y, action, SAVES_UNDO);
		else add(&l, 6, y, width - 12, 18, action);
		add(&l, 6, height - 23, width - 12, 16, SAVES_DETAILS);
		l.hint.h = 0;
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
