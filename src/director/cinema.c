#include "cinema.h"

#include <math.h>
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
	bool off_map;
	int input;
	bool b_latch;   /* a B held as a talk took the keys: kept from it until let go */
	uint32_t walk;
	bool bars;
	int bar;                 /* 0 .. BAR_FRAMES */
	int flash, flash_len;
	int shake, shake_amp;
	int card, card_t, card_len;
	char top[48], name[48], line1[48], line2[48];
	SDL_Color accent;
	int arrow_dir, arrow_t, arrow_len;   /* the way-on arrow: screen direction 0-7, frames shown, of */
	bool arrow_heal;                     /* it leads to the Recovery Mr. Prog: green, the map's Heal */
	char note[24];
	int note_t, note_len;                /* a corner note: frames shown, of */
} C;

void cinema_reset(void) { memset(&C, 0, sizeof C); }

/* (a talk that opens on its own takes a B held to run as a press to
 * hurry it: a player running into a guardian's last-stop words saw none
 * of them; the B counts again once let go) */
void cinema_input(int mode) {
	if (mode == CINEMA_TALK && C.input != CINEMA_TALK) C.b_latch = true;
	C.input = mode;
}
int cinema_input_mode(void) { return C.input; }
void cinema_walk(uint32_t keys) { C.walk = keys; }
void cinema_letterbox(bool on) { C.bars = on; }
void cinema_flash(int frames) { C.flash = C.flash_len = frames; }
void cinema_shake(int frames, int amplitude) { C.shake = frames; C.shake_amp = amplitude; }
bool cinema_busy(void) { return C.card != CARD_NONE; }
void cinema_arrow(int dir, int frames) { C.arrow_dir = dir & 7; C.arrow_t = 0; C.arrow_len = frames; }
void cinema_arrow_extend(int frames) { if (C.arrow_len && C.arrow_len - C.arrow_t < frames) C.arrow_len = C.arrow_t + frames; }
bool cinema_arrow_on(void) { return C.arrow_len > 0; }
int cinema_arrow_age(void) { return C.arrow_len ? C.arrow_t : 0; }
void cinema_arrow_turn(int dir) { C.arrow_dir = dir & 7; }
void cinema_arrow_heal(bool heal) { C.arrow_heal = heal; }
int cinema_arrow_dir(void) { return C.arrow_dir; }
void cinema_note(const char *text, int frames) { snprintf(C.note, sizeof C.note, "%s", text); C.note_t = 0; C.note_len = frames; }

int cinema_note_height(void) { return C.note_len && !C.off_map ? TEXT_H + 6 : 0; }

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

void cinema_on_map(bool on_map) { C.off_map = !on_map; }

void cinema_card_yield(void) {
	if (C.card == CARD_AREA && C.card_len - C.card_t > FADE_OUT) C.card_t = C.card_len - FADE_OUT;
}

int cinema_card_age(void) { return C.card == CARD_AREA ? C.card_t : -1; }

uint32_t cinema_keys(uint32_t keys) {
	if (C.off_map) return keys;
	if (C.input == CINEMA_HOLD) return 0;
	if (C.input == CINEMA_TALK) {
		C.b_latch &= (keys & KEY_B) != 0;
		return keys & (C.b_latch ? KEY_A : KEY_A | KEY_B);
	}
	if (C.input == CINEMA_WALK) return C.walk;
	return keys;
}

void cinema_update(void) {
	if (C.bars && C.bar < BAR_FRAMES) ++C.bar;
	if (!C.bars && C.bar > 0) --C.bar;
	if (C.flash > 0) --C.flash;
	if (C.shake > 0) --C.shake;
	/* (a card waits out a shop, the PET or a battle, unseen) */
	if (C.card && !C.off_map && ++C.card_t >= C.card_len) C.card = CARD_NONE;
	if (C.arrow_len && ++C.arrow_t >= C.arrow_len) C.arrow_len = 0;
	/* (and a note waits out a menu, unseen, as a card does) */
	if (C.note_len && !C.off_map && ++C.note_t >= C.note_len) C.note_len = 0;
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

/* A card's big name: twice the size where it fits the picture ("Robot
 * Control Comp" does not). */
static int name_scale(const char *s) { return text_width(s) * 2 <= CORE_W - 8 ? 2 : 1; }

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
	int sc = name_scale(C.name), ny = mid - 16 + (2 - sc) * TEXT_H / 2;
	text_draw_scaled(nx + 1, ny + 1, C.name, with_alpha(C.accent, a * in / 255), TEXT_CENTER, sc);
	text_draw_scaled(nx, ny, C.name, with_alpha(WHITE, a * in / 255), TEXT_CENTER, sc);
	int ep = ease(C.card_t, 16);
	int ex = x0 + CORE_W / 2 - (255 - ep) * 90 / 255;
	text_draw(ex, mid + 14, C.line1, with_alpha(C.accent, a * ep / 255), TEXT_CENTER);
}

static void draw_area(int x0, int y0) {
	int a = card_alpha();
	int y = y0 + 44;
	fill_rect(x0, y - 8, CORE_W, C.line2[0] ? 80 : 68, rgba(0, 0, 16, 150 * a / 255));
	text_draw(x0 + CORE_W / 2, y, C.top, with_alpha(C.accent, a * ease(C.card_t, 0) / 255), TEXT_CENTER);
	int in = ease(C.card_t, 4);
	int sc = name_scale(C.name);
	text_draw_scaled(x0 + CORE_W / 2, y + 12 + (2 - sc) * TEXT_H / 2 - (255 - in) * 8 / 255, C.name, with_alpha(WHITE, a * in / 255), TEXT_CENTER, sc);
	int line = 150 * ease(C.card_t, 10) / 255;
	fill_rect(x0 + (CORE_W - line) / 2, y + 38, line, 1, with_alpha(C.accent, a));
	text_draw(x0 + CORE_W / 2, y + 42, C.line1, with_alpha(rgba(216, 216, 232, 255), a * ease(C.card_t, 14) / 255), TEXT_CENTER);
	text_draw(x0 + CORE_W / 2, y + 42 + TEXT_H, C.line2, with_alpha(rgba(216, 216, 232, 255), a * ease(C.card_t, 18) / 255), TEXT_CENTER);
}

/* A filled triangle, pixel by pixel on the canvas. */
static void tri(float ax, float ay, float bx, float by, float cx, float cy, SDL_Color c) {
	int x0 = (int)fminf(ax, fminf(bx, cx)), x1 = (int)fmaxf(ax, fmaxf(bx, cx)) + 1;
	int y0 = (int)fminf(ay, fminf(by, cy)), y1 = (int)fmaxf(ay, fmaxf(by, cy)) + 1;
	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x) {
			float px = x + 0.5f, py = y + 0.5f;
			float d1 = (px - bx) * (ay - by) - (ax - bx) * (py - by);
			float d2 = (px - cx) * (by - cy) - (bx - cx) * (py - cy);
			float d3 = (px - ax) * (cy - ay) - (cx - ax) * (py - ay);
			bool neg = d1 < 0 || d2 < 0 || d3 < 0, pos = d1 > 0 || d2 > 0 || d3 > 0;
			if (!(neg && pos)) fill_rect(x, y, 1, 1, c);
		}
}

/* The way on: an arrow a little off the middle of the picture, where the
 * camera keeps MegaMan, pointing along the route (0 right, then clockwise
 * in eighths), bobbing that way; it fades out at its end. Green while it
 * leads to the heal: a playtester told it led there first followed it,
 * then heard the exit's way, and could not tell which it showed (session
 * 62). */
static void draw_arrow(int x0, int y0) {
	if (!C.arrow_len || C.off_map) return;
	int left = C.arrow_len - C.arrow_t;
	int a = C.arrow_t < 8 ? C.arrow_t * 255 / 8 : left < 16 ? left * 255 / 16 : 255;
	float ang = (float)C.arrow_dir * 3.14159265f / 4.0f;
	float ux = cosf(ang), uy = sinf(ang) * 0.5f;   /* (a diagonal along the isometric floor's, as a walkway runs) */
	float n = sqrtf(ux * ux + uy * uy);
	ux /= n; uy /= n;
	float bob = 3.0f * sinf((float)C.arrow_t * 0.25f);
	/* (clear of the chat box below when it points down) */
	float cx = (float)x0 + 120 + ux * (32 + bob), cy = (float)y0 + 66 + uy * (30 + bob);
	/* two slender heads, one behind the other (">>"), outlined */
	for (int head = 1; head >= 0; --head) {
		float hx = cx - ux * 9 * (float)head, hy = cy - uy * 9 * (float)head;
		int ha = head ? a * 3 / 5 : a;
		for (int pass = 0; pass < 2; ++pass) {
			float s = pass ? 1.0f : 1.4f, w = pass ? 5.0f : 7.0f;
			SDL_Color col = !pass ? rgba(0, 24, 64, (Uint8)(ha * 3 / 4))
				: C.arrow_heal ? rgba(90, 255, 120, (Uint8)ha) : rgba(120, 248, 255, (Uint8)ha);
			float tx = hx + ux * 8 * s, ty = hy + uy * 8 * s;
			float bx = hx - ux * 4, by = hy - uy * 4;
			tri(tx, ty, bx - uy * w, by + ux * w, bx + uy * w, by - ux * w, col);
		}
	}
}

/* The note: a small dark box in the top right corner, the HP's opposite,
 * fading in and out. */
void cinema_note_box(int x0, int y0, const char *text, int t, int len) {
	int left = len - t;
	int a = t < 8 ? t * 255 / 8 : left < 16 ? left * 255 / 16 : 255;
	int w = text_width(text) + 8, x = x0 + CORE_W - w - 3, y = y0 + 3;
	fill_rect(x, y, w, TEXT_H + 4, rgba(0, 16, 40, 170 * a / 255));
	fill_rect(x, y + TEXT_H + 3, w, 1, rgba(120, 248, 255, 200 * a / 255));
	text_draw(x + 4, y + 2, text, rgba(200, 236, 255, (Uint8)a), TEXT_LEFT);
}

static void draw_note(int x0, int y0) {
	if (!C.note_len || C.off_map) return;
	cinema_note_box(x0, y0, C.note, C.note_t, C.note_len);
}

void cinema_draw(void) {
	int x0 = P.core_x, y0 = P.core_y;
	/* (the bars stage the map: a battle that began under them, issue #24,
	 * or a menu, is drawn whole) */
	int bar = C.off_map ? 0 : BAR_H * C.bar / BAR_FRAMES;
	if (bar) {
		fill_rect(x0, y0, CORE_W, bar, BLACK);
		fill_rect(x0, y0 + CORE_H - bar, CORE_W, bar, BLACK);
	}
	draw_arrow(x0, y0);
	draw_note(x0, y0);
	if (C.card == CARD_TITLE && !C.off_map) draw_title(x0, y0);
	if (C.card == CARD_AREA && !C.off_map) draw_area(x0, y0);
	if (C.flash > 0) fill_rect(x0, y0, CORE_W, CORE_H, rgba(255, 255, 255, 230 * C.flash / C.flash_len));
}
