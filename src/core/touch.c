#include "touch.h"

#include <math.h>
#include <string.h>

#include "buttons.h"
#include "gfx.h"
#include "minifont.h"
#include "platform.h"

#define FINGERS 10

static TouchLayout lay;
static bool shown, always;
static uint32_t taken;
static struct { bool on; SDL_FingerID id; int x, y, from; } fingers[FINGERS];

bool touch_shown(void) { return shown; }

bool touch_show(bool on) {
	if (always) on = true;
	if (on == shown) return false;
	shown = on;
	memset(fingers, 0, sizeof fingers);
	return true;
}

void touch_always(void) { always = shown = true; }

void touch_relayout(void) { touch_layout_for(P.w, P.h, P.core_x, P.core_y, &lay); }

/* A finger's place on the canvas: the event gives it as a fraction of the
 * window, and the canvas stands scaled in its middle (platform_end_frame). */
static void to_canvas(float fx, float fy, int *x, int *y) {
	int ox = (P.screen_w - P.w * P.scale) / 2, oy = (P.screen_h - P.h * P.scale) / 2;
	*x = ((int)(fx * (float)P.screen_w) - ox) / P.scale;
	*y = ((int)(fy * (float)P.screen_h) - oy) / P.scale;
}

bool touch_event(const SDL_Event *e) {
	/* (a touchpad's fingers move a pointer: they touch no screen) */
	if (SDL_GetTouchDeviceType(e->tfinger.touchId) != SDL_TOUCH_DEVICE_DIRECT) return false;
	int x, y, k = 0;
	to_canvas(e->tfinger.x, e->tfinger.y, &x, &y);
	while (k < FINGERS && !(fingers[k].on && fingers[k].id == e->tfinger.fingerId)) ++k;
	if (e->type == SDL_FINGERDOWN) {
		if (touch_show(true)) return true;
		for (k = 0; k < FINGERS && fingers[k].on; ++k) {}
		if (k == FINGERS) return false;
		fingers[k].on = true;
		fingers[k].id = e->tfinger.fingerId;
		fingers[k].x = x;
		fingers[k].y = y;
		fingers[k].from = touch_control_at(&lay, x, y);
		taken |= touch_hit(&lay, x, y, fingers[k].from);
	} else if (k < FINGERS) {
		if (e->type == SDL_FINGERUP) {
			fingers[k].on = false;
			return false;
		}
		fingers[k].x = x;
		fingers[k].y = y;
		/* (one that went down on nothing takes the first control it reaches) */
		if (fingers[k].from < 0) fingers[k].from = touch_control_at(&lay, x, y);
	}
	return false;
}

uint32_t touch_held(void) {
	uint32_t bits = 0;
	for (int k = 0; k < FINGERS; ++k)
		if (fingers[k].on) bits |= touch_hit(&lay, fingers[k].x, fingers[k].y, fingers[k].from);
	return bits;
}

uint32_t touch_taken(void) {
	uint32_t t = taken;
	taken = 0;
	return t;
}

/* ---- drawing: pixel art on the canvas, in the PET's colours ---- */

typedef struct { SDL_Color edge, fill, lit, text; } Look;

/* Half the width of a disc of radius r on row dy. */
static int span(int r, int dy) { return dy * dy > r * r ? -1 : (int)sqrt((double)(r * r - dy * dy) + 0.3); }

/* A round button: an edge a pixel wide round its fill, drawn without
 * overlap so a see-through one stays even. */
static void disc(int cx, int cy, int r, SDL_Color edge, SDL_Color fill) {
	for (int dy = -r; dy <= r; ++dy) {
		int wo = span(r, dy), wi = span(r - 1, dy);
		if (wi < 0) { fill_rect(cx - wo, cy + dy, 2 * wo + 1, 1, edge); continue; }
		fill_rect(cx - wo, cy + dy, wo - wi, 1, edge);
		fill_rect(cx + wi + 1, cy + dy, wo - wi, 1, edge);
		fill_rect(cx - wi, cy + dy, 2 * wi + 1, 1, fill);
	}
}

/* A plate with its corners cut. */
static void plate(const TouchBox *b, SDL_Color edge, SDL_Color fill) {
	fill_rect(b->x + 1, b->y, b->w - 2, 1, edge);
	fill_rect(b->x + 1, b->y + b->h - 1, b->w - 2, 1, edge);
	fill_rect(b->x, b->y + 1, 1, b->h - 2, edge);
	fill_rect(b->x + b->w - 1, b->y + 1, 1, b->h - 2, edge);
	fill_rect(b->x + 1, b->y + 1, b->w - 2, b->h - 2, fill);
}

static void label(int cx, int cy, const char *s, int scale, SDL_Color c) {
	minifont_draw_centered(cx + 1, cy - 5 * scale / 2 + 1, s, rgba(16, 32, 40, c.a), scale);
	minifont_draw_centered(cx, cy - 5 * scale / 2, s, c, scale);
}

/* An arrow of n rows pointing (dx, dy) out of the D-pad, its tip at (x, y). */
static void arrow(int x, int y, int dx, int dy, int n, SDL_Color c) {
	for (int i = 0; i < n; ++i) {
		if (dx) fill_rect(x - dx * i, y - i, 1, 2 * i + 1, c);
		else fill_rect(x - i, y - dy * i, 2 * i + 1, 1, c);
	}
}

static void draw_dpad(const TouchBox *b, uint32_t held, const Look *k) {
	int r = b->w / 2, cx = b->x + r, cy = b->y + r;
	int a = (r * 2 / 3) | 1, h = a / 2;
	/* the cross as five boxes that do not overlap: the middle and four arms */
	struct { uint32_t bit; TouchBox box; int ax, ay; } arm[4] = {
		{ BTN_UP, { cx - h, cy - r, a, r - h }, 0, -1 },
		{ BTN_DOWN, { cx - h, cy + h + 1, a, r - h }, 0, 1 },
		{ BTN_LEFT, { cx - r, cy - h, r - h, a }, -1, 0 },
		{ BTN_RIGHT, { cx + h + 1, cy - h, r - h, a }, 1, 0 },
	};
	bool edged = k->fill.a == 255;
	for (int i = 0; i < 4; ++i) {
		const TouchBox *m = &arm[i].box;
		bool lit = (held & arm[i].bit) != 0;
		if (edged) fill_rect(m->x, m->y, m->w, m->h, k->edge);
		/* (the edge's side toward the middle stays open) */
		int ix = m->x + (edged && arm[i].ax <= 0), iy = m->y + (edged && arm[i].ay <= 0);
		int iw = m->w - (edged ? (arm[i].ax ? 1 : 2) : 0), ih = m->h - (edged ? (arm[i].ay ? 1 : 2) : 0);
		fill_rect(ix, iy, iw, ih, lit ? k->lit : k->fill);
		int n = a / 4 > 2 ? a / 4 : 2;
		int tx = cx + arm[i].ax * (r - 3), ty = cy + arm[i].ay * (r - 3);
		arrow(tx, ty, arm[i].ax, arm[i].ay, n, lit ? k->fill : k->text);
	}
	fill_rect(cx - h, cy - h, a, a, k->fill);
	if (edged) disc(cx, cy, h / 2 > 1 ? h / 2 : 1, k->edge, k->fill);
}

void touch_draw(void) {
	if (!shown) return;
	uint32_t held = touch_held();
	int alpha = lay.over ? 110 : 255, text = lay.over ? 170 : 255;
	Look base = { rgba(66, 198, 231, alpha), rgba(16, 54, 74, alpha), rgba(66, 198, 231, alpha), rgba(214, 244, 255, text) };
	Look a = { rgba(255, 214, 16, alpha), rgba(222, 132, 0, alpha), rgba(255, 238, 120, alpha), rgba(255, 255, 255, text) };
	Look b = { rgba(74, 231, 115, alpha), rgba(0, 123, 74, alpha), rgba(140, 255, 170, alpha), rgba(255, 255, 255, text) };
	draw_dpad(&lay.box[TOUCH_DPAD], held, &base);
	struct { int c; uint32_t bit; const char *name; const Look *look; int scale; } keys[] = {
		{ TOUCH_A, BTN_A, "A", &a, 2 }, { TOUCH_B, BTN_B, "B", &b, 2 },
		{ TOUCH_L, BTN_L, "L", &base, 2 }, { TOUCH_R, BTN_R, "R", &base, 2 },
		{ TOUCH_SELECT, BTN_SELECT, "SELECT", &base, 1 }, { TOUCH_START, BTN_START, "START", &base, 1 },
	};
	for (size_t i = 0; i < sizeof keys / sizeof *keys; ++i) {
		const TouchBox *m = &lay.box[keys[i].c];
		const Look *k = keys[i].look;
		bool lit = (held & keys[i].bit) != 0;
		int cx = m->x + m->w / 2, cy = m->y + m->h / 2;
		if (keys[i].c == TOUCH_A || keys[i].c == TOUCH_B) disc(cx, cy, m->w / 2, k->edge, lit ? k->lit : k->fill);
		else plate(m, k->edge, lit ? k->lit : k->fill);
		label(cx, cy, keys[i].name, keys[i].scale, lit ? k->fill : k->text);
	}
}
