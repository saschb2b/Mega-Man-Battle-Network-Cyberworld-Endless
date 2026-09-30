#include "touch.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "buttons.h"
#include "gfx.h"
#include "minifont.h"
#include "platform.h"

#define FINGERS 10

static TouchLayout lay;
static bool shown, always;
static uint32_t taken;
static struct { bool on; SDL_FingerID id; int x, y, from; uint32_t dir; } fingers[FINGERS];

/* the player's arrangement and its editor */
static TouchCustom custom;
static char custom_path[600];
static bool offered, editing;
static int sel = -1;                  /* the control last touched in the editor */
static struct { bool on; SDL_FingerID id; int dx, dy; } drag;
enum { TOOL_SMALLER, TOOL_BIGGER, TOOL_RESET, TOOL_DONE, TOOLS };
static TouchBox edit_plate, tool[TOOLS];
static const char *const tool_names[TOOLS] = { "-", "+", "RESET", "DONE" };

bool touch_shown(void) { return shown; }

static void save(void);

bool touch_show(bool on) {
	if (always) on = true;
	if (on == shown) return false;
	shown = on;
	memset(fingers, 0, sizeof fingers);
	/* (a key or a button put them away: the editor keeps what it has) */
	if (!on && editing) { save(); editing = false; }
	return true;
}

void touch_always(void) { always = shown = true; }

void touch_release(void) { memset(fingers, 0, sizeof fingers); }

void touch_relayout(void) {
	touch_layout_for(P.w, P.h, P.core_x, P.core_y, &lay);
	touch_layout_custom(&lay, P.w, P.h, &custom);
	/* the editor's plates across the canvas's top, EDIT CONTROLS there too */
	static const int widths[TOOLS] = { 22, 22, 44, 40 };
	int total = 0;
	for (int i = 0; i < TOOLS; ++i) total += widths[i] + (i ? 4 : 0);
	for (int i = 0, x = (P.w - total) / 2; i < TOOLS; x += widths[i] + 4, ++i) tool[i] = (TouchBox){ x, 3, widths[i], 16 };
	edit_plate = (TouchBox){ (P.w - 84) / 2, 3, 84, 16 };
}

void touch_load(const char *path) {
	snprintf(custom_path, sizeof custom_path, "%s", path);
	static char text[4096];
	FILE *f = fopen(path, "r");
	size_t n = f ? fread(text, 1, sizeof text - 1, f) : 0;
	if (f) fclose(f);
	text[n] = 0;
	touch_custom_parse(text, &custom);
	touch_relayout();
}

static void save(void) {
	if (!custom_path[0]) return;
	static char text[4096];
	int n = touch_custom_format(&custom, text, sizeof text);
	FILE *f = fopen(custom_path, "w");
	if (!f) return;
	fwrite(text, 1, (size_t)n, f);
	fclose(f);
	platform_persist();
}

void touch_offer_edit(bool on) { offered = on; }
bool touch_editing(void) { return editing; }

static bool in_box(const TouchBox *b, int x, int y) { return x >= b->x && y >= b->y && x < b->x + b->w && y < b->y + b->h; }

/* A finger down in the editor: a plate, else the control it grabs. */
static void edit_down(SDL_FingerID id, int x, int y) {
	TouchPlace *p = custom.place[touch_shape(&lay)];
	for (int i = 0; i < TOOLS; ++i) {
		if (!in_box(&tool[i], x, y)) continue;
		if ((i == TOOL_SMALLER || i == TOOL_BIGGER) && sel >= 0) {
			int s = (p[sel].size ? p[sel].size : 100) + (i == TOOL_BIGGER ? 10 : -10);
			s = s < TOUCH_SIZE_MIN ? TOUCH_SIZE_MIN : s > TOUCH_SIZE_MAX ? TOUCH_SIZE_MAX : s;
			p[sel].size = (int16_t)(s == 100 ? 0 : s);
		} else if (i == TOOL_RESET) {
			memset(p, 0, sizeof(TouchPlace) * TOUCH_CONTROLS);
			sel = -1;
		} else if (i == TOOL_DONE) {
			save();
			editing = false;
			sel = -1;
		}
		touch_relayout();
		return;
	}
	sel = touch_control_at(&lay, x, y);
	if (sel < 0) return;
	const TouchBox *b = &lay.box[sel];
	drag.on = true;
	drag.id = id;
	drag.dx = x - (b->x + b->w / 2);
	drag.dy = y - (b->y + b->h / 2);
}

static void edit_move(SDL_FingerID id, int x, int y) {
	if (!drag.on || id != drag.id || sel < 0 || P.w <= 0 || P.h <= 0) return;
	TouchPlace *p = &custom.place[touch_shape(&lay)][sel];
	int cx = x - drag.dx, cy = y - drag.dy;
	cx = cx < 0 ? 0 : cx > P.w ? P.w : cx;
	cy = cy < 0 ? 0 : cy > P.h ? P.h : cy;
	p->moved = true;
	p->x = (int16_t)(cx * 1000 / P.w);
	p->y = (int16_t)(cy * 1000 / P.h);
	touch_relayout();
}

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
	int x, y;
	to_canvas(e->tfinger.x, e->tfinger.y, &x, &y);
	return touch_finger(e->type, e->tfinger.fingerId, x, y);
}

bool touch_finger(uint32_t type, SDL_FingerID id, int x, int y) {
	int k = 0;
	while (k < FINGERS && !(fingers[k].on && fingers[k].id == id)) ++k;
	/* (the editor takes the fingers: they press nothing) */
	if (editing) {
		if (type == SDL_FINGERDOWN) edit_down(id, x, y);
		else if (type == SDL_FINGERMOTION) edit_move(id, x, y);
		else if (type == SDL_FINGERUP && drag.on && drag.id == id) drag.on = false;
		return false;
	}
	if (type == SDL_FINGERDOWN) {
		if (touch_show(true)) return true;
		if (offered && in_box(&edit_plate, x, y)) {
			editing = true;
			sel = -1;
			drag.on = false;
			memset(fingers, 0, sizeof fingers);
			return false;
		}
		for (k = 0; k < FINGERS && fingers[k].on; ++k) {}
		if (k == FINGERS) return false;
		fingers[k].on = true;
		fingers[k].id = id;
		fingers[k].x = x;
		fingers[k].y = y;
		fingers[k].from = touch_control_at(&lay, x, y);
		fingers[k].dir = fingers[k].from == TOUCH_DPAD ? touch_dpad_steer(&lay, x, y, 0) : 0;
		taken |= touch_hit(&lay, x, y, fingers[k].from);
	} else if (k < FINGERS) {
		if (type == SDL_FINGERUP) {
			fingers[k].on = false;
			return false;
		}
		fingers[k].x = x;
		fingers[k].y = y;
		/* (one that went down on nothing takes the first control it reaches) */
		if (fingers[k].from < 0) fingers[k].from = touch_control_at(&lay, x, y);
		if (fingers[k].from == TOUCH_DPAD) fingers[k].dir = touch_dpad_steer(&lay, x, y, fingers[k].dir);
	}
	return false;
}

uint32_t touch_held(void) {
	uint32_t bits = 0;
	if (editing) return 0;
	for (int k = 0; k < FINGERS; ++k)
		if (fingers[k].on) bits |= fingers[k].from == TOUCH_DPAD ? fingers[k].dir : touch_hit(&lay, fingers[k].x, fingers[k].y, fingers[k].from);
	return bits;
}

uint32_t touch_taken(void) {
	uint32_t t = editing ? 0 : taken;
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
	Look gold = { rgba(255, 214, 16, 255), rgba(74, 49, 0, 235), rgba(255, 238, 120, 255), rgba(255, 238, 120, 255) };
	if (editing) {
		/* the control last touched framed, the plates, and what to do */
		if (sel >= 0) {
			const TouchBox *m = &lay.box[sel];
			SDL_Color c = gold.edge;
			fill_rect(m->x - 2, m->y - 2, m->w + 4, 1, c);
			fill_rect(m->x - 2, m->y + m->h + 1, m->w + 4, 1, c);
			fill_rect(m->x - 2, m->y - 2, 1, m->h + 4, c);
			fill_rect(m->x + m->w + 1, m->y - 2, 1, m->h + 4, c);
		}
		for (int i = 0; i < TOOLS; ++i) {
			bool off = (i == TOOL_SMALLER || i == TOOL_BIGGER) && sel < 0;
			plate(&tool[i], off ? base.edge : gold.edge, off ? base.fill : gold.fill);
			label(tool[i].x + tool[i].w / 2, tool[i].y + tool[i].h / 2, tool_names[i], i < TOOL_RESET ? 2 : 1, off ? base.text : gold.text);
		}
		label(P.w / 2, tool[0].y + 26, sel < 0 ? "DRAG A BUTTON TO MOVE IT" : "- AND + SIZE IT", 1, base.text);
	} else if (offered) {
		plate(&edit_plate, gold.edge, gold.fill);
		label(edit_plate.x + edit_plate.w / 2, edit_plate.y + edit_plate.h / 2, "EDIT CONTROLS", 1, gold.text);
	}
}
