#include "network_state.h"

#include <stdio.h>
#include <string.h>

#include "gfx.h"
#include "minifont.h"
#include "network_text.h"
#include "pixfont.h"
#include "platform.h"
#include "second_frame.h"

static void words(int x, int y, const char *s, SDL_Color color) {
	pixfont_draw(x, y, s, color, PET_EXIT, PIXFONT_LEFT, 1);
}

static void clipped(int x, int y, const char *s, int width, SDL_Color color) {
	char line[NETWORK_URL];
	snprintf(line, sizeof line, "%s", s);
	size_t n = strlen(line);
	if (pixfont_width(line, 1) > width) {
		while (n && pixfont_width(line, 1) + pixfont_width("...", 1) > width) line[--n] = 0;
		snprintf(line + n, sizeof line - n, "...");
	}
	words(x, y, line, color);
}

SDL_Rect network_home_rect(int row) {
	static const int top[] = { 27, 51, 86, 108, 130 };
	static const int height[] = { 21, 32, 19, 19, 19 };
	int w = P.w - 16;
	if (w > 360) w = 360;
	return (SDL_Rect){ (P.w - w) / 2, top[row], w, height[row] };
}

static void button(SDL_Rect r, const char *label, bool selected) {
	fill_rect(r.x, r.y, r.w, r.h, selected ? PET_GOLD : PET_LINE);
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, selected ? PET_NAVY : PET_SLOT);
	pixfont_draw(r.x + r.w / 2, r.y + (r.h - PIXFONT_H) / 2, label, PET_WHITE, PET_EXIT, PIXFONT_CENTER, 1);
}

static void privacy(void) {
	words(9, 28, network_word(NW_PRIVACY_TITLE), PET_GOLD);
	clipped(9, 42, N.prefs.servers[N.prefs.selected], P.w - 18, PET_CYAN_HI);
	char lines[10][96];
	int n = pixfont_wrap(network_word(NW_PRIVACY_BODY), P.w - 20, 1, lines, 10);
	for (int i = 0; i < n; ++i) words(10, 58 + i * PIXFONT_LINE, lines[i], PET_WHITE);
	int w = (P.w - 24) / 2;
	button((SDL_Rect){ 8, P.h - 29, w, 20 }, network_word(NW_BACK), !N.privacy);
	button((SDL_Rect){ P.w / 2 + 4, P.h - 29, w, 20 }, network_word(NW_CONNECT), N.privacy != 0);
}

static void home(void) {
	static const int labels[] = { NW_NAME, NW_SERVER, NW_ADD, NW_JOIN, NW_BACK };
	for (int i = 0; i < NETWORK_ROWS; ++i) {
		SDL_Rect r = network_home_rect(i);
		button(r, i < 2 ? "" : network_word(labels[i]), N.row == i);
		if (i >= 2) continue;
		words(r.x + 5, r.y + 4, network_word(labels[i]), PET_GOLD);
		if (!i) clipped(r.x + 49, r.y + 4, N.prefs.name, r.w - 54, PET_WHITE);
		else {
			char count[24];
			snprintf(count, sizeof count, "%u / %u", N.prefs.selected + 1, N.prefs.count);
			words(r.x + r.w - 34, r.y + 4, count, PET_CYAN_HI);
			clipped(r.x + 5, r.y + 17, N.prefs.servers[N.prefs.selected], r.w - 10, PET_WHITE);
		}
	}
	const char *hint = N.row == NETWORK_ROW_SERVER ? network_word(NW_SERVER_HELP) : network_word(NW_MENU_HINT);
	minifont_draw_centered(P.w / 2, P.h - 7, hint, PET_WHITE, 1);
}

static void note(void) {
	if (!N.note[0]) return;
	char lines[8][96];
	int n = pixfont_wrap(N.note, P.w - 30, 1, lines, 8);
	int h = n * PIXFONT_LINE + 12, y = (P.h - h) / 2;
	fill_rect(8, y, P.w - 16, h, PET_GOLD);
	fill_rect(9, y + 1, P.w - 18, h - 2, PET_EXIT);
	for (int i = 0; i < n; ++i) words(15, y + 6 + i * PIXFONT_LINE, lines[i], PET_WHITE);
}

void network_draw(void) {
	fill_rect(0, 0, P.w, P.h, PET_GREEN);
	fill_rect(0, 0, P.w, 21, PET_DARK);
	fill_rect(0, 21, P.w, 1, PET_LINE);
	words(8, 6, network_word(NW_TITLE), PET_WHITE);
	second_stripes(14 + pixfont_width(network_word(NW_TITLE), 1), 4);
	if (N.view == NETWORK_PRIVACY) privacy();
	else if (N.view == NETWORK_EDIT_NAME || N.view == NETWORK_EDIT_SERVER) network_edit_draw();
	else home();
	note();
}
