#include "network_state.h"

#include <stdio.h>
#include <string.h>

#include "gfx.h"
#include "minifont.h"
#include "network_text.h"
#include "pixfont.h"
#include "platform.h"
#include "second_frame.h"
#include "text_input.h"

static const char keys[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 _-.:/[]";
static char last_edit[NETWORK_URL];
#define KEY_COLUMNS 12
#define KEY_ROWS 6
#define KEY_COUNT (KEY_COLUMNS * KEY_ROWS)

static SDL_Rect grid(void) {
	int w = P.w - 16;
	if (w > 288) w = 288;
	w -= w % KEY_COLUMNS;
	return (SDL_Rect){ (P.w - w) / 2, 64, w, KEY_ROWS * 11 };
}

void network_edit_begin(bool name, bool adding) {
	N.view = name ? NETWORK_EDIT_NAME : NETWORK_EDIT_SERVER;
	N.adding = adding;
	N.key = 0;
	N.note[0] = 0;
	snprintf(N.edit, sizeof N.edit, "%s", name ? N.prefs.name : adding ? "wss://" : N.prefs.servers[N.prefs.selected]);
	snprintf(last_edit, sizeof last_edit, "%s", N.edit);
	text_input_begin(N.edit, name ? NETWORK_NAME : sizeof N.edit);
}

static void done(void) {
	size_t n = strlen(N.edit);
	while (n && N.edit[n - 1] == ' ') N.edit[--n] = 0;
	bool name = N.view == NETWORK_EDIT_NAME;
	if (!(name ? network_name_valid(N.edit) : network_url_valid(N.edit))) {
		snprintf(N.note, sizeof N.note, "%s", network_word(name ? NW_NAME_INVALID : NW_URL_INVALID));
		return;
	}
	NetworkPrefs previous = N.prefs;
	if (name) snprintf(N.prefs.name, sizeof N.prefs.name, "%.*s", NETWORK_NAME - 1, N.edit);
	else {
		if (N.adding) N.prefs.selected = N.prefs.count++;
		if (strcmp(N.prefs.servers[N.prefs.selected], N.edit)) N.prefs.consent &= ~(1u << N.prefs.selected);
		snprintf(N.prefs.servers[N.prefs.selected], NETWORK_URL, "%s", N.edit);
	}
	if (!network_store()) { N.prefs = previous; return; }
	text_input_end();
	N.view = NETWORK_HOME;
	N.row = name ? NETWORK_ROW_NAME : NETWORK_ROW_SERVER;
	N.note[0] = 0;
}

static void key_choose(void) {
	N.note[0] = 0;
	if (N.key < (int)strlen(keys)) {
		char letter[2] = { keys[N.key], 0 };
		text_input_add(letter);
	} else if (N.key == KEY_COUNT) text_input_erase();
	else if (N.key == KEY_COUNT + 1) done();
	else if (N.key == KEY_COUNT + 2) network_back();
}

static void cursor(void) {
	if (N.key >= KEY_COUNT) {
		if (btn_repeat(BTN_LEFT)) N.key = KEY_COUNT + (N.key - KEY_COUNT + 2) % 3;
		if (btn_repeat(BTN_RIGHT)) N.key = KEY_COUNT + (N.key - KEY_COUNT + 1) % 3;
		if (btn_repeat(BTN_UP)) N.key = KEY_COUNT - KEY_COLUMNS + (N.key - KEY_COUNT) * 4;
		if (btn_repeat(BTN_DOWN)) N.key = (N.key - KEY_COUNT) * 4;
		return;
	}
	int row = N.key / KEY_COLUMNS, col = N.key % KEY_COLUMNS;
	if (btn_repeat(BTN_LEFT)) col = (col + KEY_COLUMNS - 1) % KEY_COLUMNS;
	if (btn_repeat(BTN_RIGHT)) col = (col + 1) % KEY_COLUMNS;
	if (btn_repeat(BTN_UP)) --row;
	if (btn_repeat(BTN_DOWN)) ++row;
	N.key = row < 0 || row >= KEY_ROWS ? KEY_COUNT + col / 4 : row * KEY_COLUMNS + col;
	if (N.key < KEY_COUNT && N.key >= (int)strlen(keys)) N.key = KEY_COUNT + col / 4;
}

void network_edit_update(void) {
	if (strcmp(last_edit, N.edit)) { N.note[0] = 0; snprintf(last_edit, sizeof last_edit, "%s", N.edit); }
	int result = text_input_result();
	if (result == TEXT_INPUT_CANCEL || btn_pressed(BTN_B)) { network_back(); return; }
	if (result == TEXT_INPUT_DONE || btn_pressed(BTN_START)) { done(); return; }
	if (btn_repeat(BTN_L)) { text_input_erase(); N.note[0] = 0; }
	cursor();
	SDL_Rect r = grid();
	int x, y;
	if (platform_tap(&x, &y) && x >= r.x && x < r.x + r.w) {
		if (y >= r.y && y < r.y + r.h) { N.key = (y - r.y) / 11 * KEY_COLUMNS + (x - r.x) / (r.w / KEY_COLUMNS); key_choose(); }
		else if (y >= 133 && y < 149) { N.key = KEY_COUNT + (x - r.x) / (r.w / 3); key_choose(); }
	}
	if (btn_pressed(BTN_A)) key_choose();
}

void network_edit_draw(void) {
	const char *heading = network_word(N.view == NETWORK_EDIT_NAME ? NW_EDIT_NAME : NW_EDIT_SERVER);
	pixfont_draw(8, 27, heading, PET_WHITE, PET_EXIT, PIXFONT_LEFT, 1);
	fill_rect(8, 40, P.w - 16, 19, PET_EXIT);
	const char *visible = N.edit;
	while (*visible && pixfont_width(visible, 1) > P.w - 26) ++visible;
	pixfont_draw(12, 45, visible, PET_CYAN_HI, PET_EXIT, PIXFONT_LEFT, 1);
	SDL_Rect r = grid();
	int width = r.w / KEY_COLUMNS;
	for (int i = 0; i < (int)strlen(keys); ++i) {
		int x = r.x + i % KEY_COLUMNS * width, y = r.y + i / KEY_COLUMNS * 11;
		fill_rect(x, y, width - 1, 10, N.key == i ? PET_GOLD : PET_SLOT);
		char letter[2] = { keys[i], 0 };
		pixfont_draw(x + width / 2, y + 1, keys[i] == ' ' ? network_word(NW_SPACE) : letter,
		             N.key == i ? PET_EXIT : PET_WHITE, PET_SLOT, PIXFONT_CENTER, 1);
	}
	for (int i = 0; i < 3; ++i) {
		int x = r.x + i * (r.w / 3);
		fill_rect(x, 133, r.w / 3 - 2, 16, N.key == KEY_COUNT + i ? PET_GOLD : PET_SLOT);
		pixfont_draw(x + r.w / 6, 137, network_word(NW_ERASE + i), N.key == KEY_COUNT + i ? PET_EXIT : PET_WHITE, PET_SLOT, PIXFONT_CENTER, 1);
	}
	minifont_draw_centered(P.w / 2, P.h - 7, network_word(NW_EDIT_HINT), PET_WHITE, 1);
}
