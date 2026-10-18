#include "online_field.h"

#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "gfx.h"
#include "minifont.h"
#include "network_prefs.h"
#include "online_text.h"
#include "pixfont.h"
#include "platform.h"
#include "second_frame.h"
#include "text_input.h"

static const char keys[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 _-.:/[]";
#define KEY_COLUMNS 12
#define KEY_ROWS 6
#define KEY_COUNT (KEY_COLUMNS * KEY_ROWS)
#define BUTTONS 3   /* ERASE, OK, BACK after the keys */
#define KEY_W 18
#define KEY_H 11
#define GRID_Y 48
#define BUTTON_Y 118

static struct {
	bool shown;
	FieldKind kind;
	char text[NETWORK_URL];
	int key;
	const char *note;
} F;

/* the panel's top left: 240 x 160 in the middle of the canvas */
static int left(void) { return (P.w - 240) / 2; }
static int top(void) { return (P.h - 160) / 2; }

static bool back(void) {
	F.note = NULL;
	F.key = KEY_COUNT + 2;
	return true;
}

void online_field_open(FieldKind kind, const char *initial) {
	memset(&F, 0, sizeof F);
	F.shown = true;
	F.kind = kind;
	snprintf(F.text, sizeof F.text, "%s", initial ? initial : "");
	text_input_begin(F.text, kind == FIELD_NAME ? NETWORK_NAME : sizeof F.text);
	platform_own_taps(true);
	platform_on_back(back);
}

bool online_field_shown(void) { return F.shown; }

static int finish(int result) {
	text_input_end();
	platform_own_taps(false);
	platform_on_back(NULL);
	F.shown = false;
	audio_sfx(result == FIELD_DONE ? SFX_SELECT : SFX_CANCEL);
	return result;
}

bool online_field_url(const char *typed, char *out, size_t cap) {
	char url[NETWORK_URL];
	bool whole = !strncmp(typed, "ws://", 5) || !strncmp(typed, "wss://", 6);
	snprintf(url, sizeof url, "%s%s", whole ? "" : "wss://", typed);
	/* (the path the server answers on, where only a host was typed) */
	const char *host = strstr(url, "://") + 3;
	if (!strchr(host, '/') && strlen(url) + 9 < sizeof url) strcat(url, "/presence");
	if (!network_url_valid(url) || strlen(url) >= cap) return false;
	snprintf(out, cap, "%s", url);
	return true;
}

void online_field_host(const char *url, char *out, size_t cap) {
	const char *host = strstr(url, "://");
	host = host ? host + 3 : url;
	size_t n = strcspn(host, "/");
	if (n >= cap) n = cap - 1;
	memcpy(out, host, n);
	out[n] = 0;
}

static int done(char *out, size_t cap) {
	size_t n = strlen(F.text);
	while (n && F.text[n - 1] == ' ') F.text[--n] = 0;
	if (F.kind == FIELD_NAME) {
		if (!network_name_valid(F.text) || n >= cap) { F.note = online_field_bad_name; return FIELD_EDITING; }
		snprintf(out, cap, "%s", F.text);
	} else if (!online_field_url(F.text, out, cap)) { F.note = online_field_bad_address; return FIELD_EDITING; }
	return finish(FIELD_DONE);
}

static int press(char *out, size_t cap) {
	F.note = NULL;
	if (F.key < (int)strlen(keys)) {
		char letter[2] = { keys[F.key], 0 };
		text_input_add(letter);
	} else if (F.key == KEY_COUNT) text_input_erase();
	else if (F.key == KEY_COUNT + 1) return done(out, cap);
	else if (F.key == KEY_COUNT + 2) return finish(FIELD_CANCELLED);
	return FIELD_EDITING;
}

static void cursor(void) {
	if (F.key >= KEY_COUNT) {
		int b = F.key - KEY_COUNT;
		if (btn_repeat(BTN_LEFT)) F.key = KEY_COUNT + (b + BUTTONS - 1) % BUTTONS;
		if (btn_repeat(BTN_RIGHT)) F.key = KEY_COUNT + (b + 1) % BUTTONS;
		if (btn_repeat(BTN_UP)) F.key = KEY_COUNT - KEY_COLUMNS + b * 4;
		if (btn_repeat(BTN_DOWN)) F.key = b * 4;
		return;
	}
	int row = F.key / KEY_COLUMNS, col = F.key % KEY_COLUMNS;
	if (btn_repeat(BTN_LEFT)) col = (col + KEY_COLUMNS - 1) % KEY_COLUMNS;
	if (btn_repeat(BTN_RIGHT)) col = (col + 1) % KEY_COLUMNS;
	if (btn_repeat(BTN_UP)) --row;
	if (btn_repeat(BTN_DOWN)) ++row;
	F.key = row < 0 || row >= KEY_ROWS ? KEY_COUNT + col / 4 : row * KEY_COLUMNS + col;
	if (F.key < KEY_COUNT && F.key >= (int)strlen(keys)) F.key = KEY_COUNT + col / 4;
}

/* the key or button under canvas pixel (x, y); -1 none */
static int key_at(int x, int y) {
	int gx = left() + (240 - KEY_COLUMNS * KEY_W) / 2, gy = top() + GRID_Y;
	if (x >= gx && x < gx + KEY_COLUMNS * KEY_W && y >= gy && y < gy + KEY_ROWS * KEY_H) {
		int k = (y - gy) / KEY_H * KEY_COLUMNS + (x - gx) / KEY_W;
		return k < (int)strlen(keys) ? k : -1;
	}
	if (y >= top() + BUTTON_Y && y < top() + BUTTON_Y + 14 && x >= left() + 8 && x < left() + 232) return KEY_COUNT + (x - left() - 8) / 75;
	return -1;
}

int online_field_update(char *out, size_t cap) {
	if (!F.shown) return FIELD_CANCELLED;
	int typed = text_input_result();
	if (typed == TEXT_INPUT_CANCEL || btn_pressed(BTN_B)) return finish(FIELD_CANCELLED);
	if (typed == TEXT_INPUT_DONE || btn_pressed(BTN_START)) return done(out, cap);
	if (btn_repeat(BTN_L)) { text_input_erase(); F.note = NULL; }
	cursor();
	int x, y;
	if (platform_tap(&x, &y)) {
		int k = key_at(x, y);
		if (k >= 0) { F.key = k; return press(out, cap); }
	}
	if (btn_pressed(BTN_A)) return press(out, cap);
	return FIELD_EDITING;
}

void online_field_draw(void) {
	if (!F.shown) return;
	int x0 = left(), y0 = top();
	fill_rect(0, 0, P.w, P.h, rgba(0, 0, 0, 160));
	fill_rect(x0, y0, 240, 160, PET_LINE);
	fill_rect(x0 + 1, y0 + 1, 238, 158, PET_EXIT);
	pixfont_draw(x0 + 8, y0 + 6, F.kind == FIELD_NAME ? online_field_name_title : online_field_address_title, PET_GOLD, PET_EXIT, PIXFONT_LEFT, 1);
	fill_rect(x0 + 8, y0 + 18, 224, 15, PET_SLOT);
	const char *shown = F.text;
	while (*shown && pixfont_width(shown, 1) > 214) ++shown;
	pixfont_draw(x0 + 12, y0 + 22, shown, PET_CYAN_HI, PET_EXIT, PIXFONT_LEFT, 1);
	int caret = x0 + 13 + pixfont_width(shown, 1);
	if (SDL_GetTicks() / 400 % 2) fill_rect(caret, y0 + 21, 1, 9, PET_WHITE);
	const char *note = F.note ? F.note : F.kind == FIELD_NAME ? online_field_name_hint : online_field_address_hint;
	minifont_draw(x0 + 8, y0 + 38, note, F.note ? PET_GOLD : PET_DIM, 1);
	int gx = x0 + (240 - KEY_COLUMNS * KEY_W) / 2;
	for (int i = 0; i < (int)strlen(keys); ++i) {
		int kx = gx + i % KEY_COLUMNS * KEY_W, ky = y0 + GRID_Y + i / KEY_COLUMNS * KEY_H;
		bool on = F.key == i;
		fill_rect(kx, ky, KEY_W - 1, KEY_H - 1, on ? PET_GOLD : PET_SLOT);
		char letter[2] = { keys[i], 0 };
		pixfont_draw(kx + KEY_W / 2, ky + 1, keys[i] == ' ' ? online_field_space : letter, on ? PET_EXIT : PET_WHITE, PET_SLOT, PIXFONT_CENTER, 1);
	}
	for (int i = 0; i < BUTTONS; ++i) {
		int bx = x0 + 8 + i * 75;
		bool on = F.key == KEY_COUNT + i;
		fill_rect(bx, y0 + BUTTON_Y, 73, 14, on ? PET_GOLD : PET_NAVY);
		pixfont_draw(bx + 36, y0 + BUTTON_Y + 3, online_field_buttons[i], on ? PET_EXIT : PET_WHITE, PET_EXIT, PIXFONT_CENTER, 1);
	}
	minifont_draw_centered(x0 + 120, y0 + 146, online_field_keys_hint, PET_WHITE, 1);
}
