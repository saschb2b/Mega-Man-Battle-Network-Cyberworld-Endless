#include "cinema.h"

#include <stdio.h>
#include <string.h>

#include "emu.h"
#include "gfx.h"
#include "platform.h"

#define BAR_H      18   /* letterbox bars at full height */
#define BAR_FRAMES 12
#define EASE_IN    16   /* frames a card's parts take to slide in */
#define FADE_OUT   14

enum { CARD_NONE, CARD_TITLE, CARD_AREA };

static struct {
	int input;
	uint32_t walk;
	bool bars;
	int bar;                 /* 0 .. BAR_FRAMES */
	int flash, flash_len;
	int shake, shake_amp;
	int card, card_t, card_len;
	char top[48], name[48], line1[48], line2[48];
	SDL_Color accent;
} C;

void cinema_reset(void) { memset(&C, 0, sizeof C); }

void cinema_input(int mode) { C.input = mode; }
void cinema_walk(uint32_t keys) { C.walk = keys; }
void cinema_letterbox(bool on) { C.bars = on; }
void cinema_flash(int frames) { C.flash = C.flash_len = frames; }
void cinema_shake(int frames, int amplitude) { C.shake = frames; C.shake_amp = amplitude; }
bool cinema_busy(void) { return C.card != CARD_NONE; }

static void card(int kind, const char *top, const char *name, const char *l1, const char *l2, SDL_Color accent, int frames) {
	C.card = kind;
	C.card_t = 0;
	C.card_len = frames;
	snprintf(C.top, sizeof C.top, "%s", top ? top : "");
	snprintf(C.name, sizeof C.name, "%s", name ? name : "");
	snprintf(C.line1, sizeof C.line1, "%s", l1 ? l1 : "");
	snprintf(C.line2, sizeof C.line2, "%s", l2 ? l2 : "");
	C.accent = accent;
}

void cinema_title(const char *top, const char *name, const char *epithet, SDL_Color accent, int frames) {
	card(CARD_TITLE, top, name, epithet, NULL, accent, frames);
}

void cinema_card(const char *small, const char *big, const char *line1, const char *line2, SDL_Color accent, int frames) {
	card(CARD_AREA, small, big, line1, line2, accent, frames);
}

uint32_t cinema_keys(uint32_t keys) {
	if (C.input == CINEMA_HOLD) return 0;
	if (C.input == CINEMA_TALK) return keys & (KEY_A | KEY_B);
	if (C.input == CINEMA_WALK) return C.walk;
	return keys;
}

void cinema_update(void) {
	if (C.bars && C.bar < BAR_FRAMES) ++C.bar;
	if (!C.bars && C.bar > 0) --C.bar;
	if (C.flash > 0) --C.flash;
	if (C.shake > 0) --C.shake;
	if (C.card && ++C.card_t >= C.card_len) C.card = CARD_NONE;
}

void cinema_offset(int *dx, int *dy) {
	*dx = *dy = 0;
	if (C.shake <= 0) return;
	/* a hard jolt that settles */
	int a = C.shake_amp * C.shake / (C.shake + 8) + 1;
	*dx = (C.shake & 2) ? a : -a;
	*dy = (C.shake & 4) ? a / 2 : -a / 2;
}

/* 0..255: how far a part has come in, easing out, `delay` frames late. */
static int ease(int t, int delay) {
	int u = t - delay;
	if (u <= 0) return 0;
	if (u >= EASE_IN) return 255;
	int v = 255 - (EASE_IN - u) * (EASE_IN - u) * 255 / (EASE_IN * EASE_IN);
	return v;
}

/* The card's opacity: in over a few frames, out at its end. */
static int card_alpha(void) {
	int left = C.card_len - C.card_t;
	int a = C.card_t < 6 ? C.card_t * 255 / 6 : 255;
	if (left < FADE_OUT) a = a * left / FADE_OUT;
	return a;
}

static SDL_Color with_alpha(SDL_Color c, int a) { c.a = (Uint8)(c.a * a / 255); return c; }

static void draw_title(int x0, int y0) {
	int a = card_alpha();
	int open = ease(C.card_t, 0);
	/* a dark band opening from the middle, edged in the guardian's colour */
	int band = 64 * open / 255, mid = y0 + 78;
	fill_rect(x0, mid - band / 2, CORE_W, band, rgba(0, 0, 16, 190 * a / 255));
	int line = CORE_W * ease(C.card_t, 4) / 255;
	fill_rect(x0 + (CORE_W - line) / 2, mid - band / 2, line, 1, with_alpha(C.accent, a));
	fill_rect(x0 + (CORE_W - line) / 2, mid + band / 2 - 1, line, 1, with_alpha(C.accent, a));
	if (band < 60) return;
	/* the area it keeps, its name sliding in from the right, its epithet from the left */
	text_draw(x0 + CORE_W / 2, mid - 30, C.top, with_alpha(rgba(200, 200, 216, 255), a * ease(C.card_t, 6) / 255), TEXT_CENTER);
	int in = ease(C.card_t, 8);
	int nx = x0 + CORE_W / 2 + (255 - in) * 90 / 255;
	text_draw_scaled(nx + 1, mid - 15, C.name, with_alpha(C.accent, a * in / 255), TEXT_CENTER, 2);
	text_draw_scaled(nx, mid - 16, C.name, with_alpha(WHITE, a * in / 255), TEXT_CENTER, 2);
	int ep = ease(C.card_t, 16);
	int ex = x0 + CORE_W / 2 - (255 - ep) * 90 / 255;
	text_draw(ex, mid + 14, C.line1, with_alpha(C.accent, a * ep / 255), TEXT_CENTER);
}

static void draw_area(int x0, int y0) {
	int a = card_alpha();
	int y = y0 + 44;
	fill_rect(x0, y - 8, CORE_W, 72, rgba(0, 0, 16, 150 * a / 255));
	text_draw(x0 + CORE_W / 2, y, C.top, with_alpha(C.accent, a * ease(C.card_t, 0) / 255), TEXT_CENTER);
	int in = ease(C.card_t, 4);
	text_draw_scaled(x0 + CORE_W / 2, y + 12 - (255 - in) * 8 / 255, C.name, with_alpha(WHITE, a * in / 255), TEXT_CENTER, 2);
	int line = 150 * ease(C.card_t, 10) / 255;
	fill_rect(x0 + (CORE_W - line) / 2, y + 38, line, 1, with_alpha(C.accent, a));
	text_draw(x0 + CORE_W / 2, y + 42, C.line1, with_alpha(rgba(216, 216, 232, 255), a * ease(C.card_t, 14) / 255), TEXT_CENTER);
	text_draw(x0 + CORE_W / 2, y + 52, C.line2, with_alpha(rgba(216, 216, 232, 255), a * ease(C.card_t, 18) / 255), TEXT_CENTER);
}

void cinema_draw(void) {
	int x0 = P.core_x, y0 = P.core_y;
	int bar = BAR_H * C.bar / BAR_FRAMES;
	if (bar) {
		fill_rect(x0, y0, CORE_W, bar, BLACK);
		fill_rect(x0, y0 + CORE_H - bar, CORE_W, bar, BLACK);
	}
	if (C.card == CARD_TITLE) draw_title(x0, y0);
	if (C.card == CARD_AREA) draw_area(x0, y0);
	if (C.flash > 0) fill_rect(x0, y0, CORE_W, CORE_H, rgba(255, 255, 255, 230 * C.flash / C.flash_len));
}
