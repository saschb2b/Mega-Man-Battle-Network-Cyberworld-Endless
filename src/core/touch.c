#include "touch.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#ifdef __ANDROID__
#include <jni.h>
#endif
#ifdef CW_IOS
#include "ios.h"
#endif

#include "buttons.h"
#include "director.h"
#include "platform.h"
#include "touch_art.h"

#define FINGERS 10

/* the PET's colours */
static const SDL_Color CYAN = { 66, 198, 231, 255 }, CYAN_HI = { 150, 230, 255, 255 }, NAVY = { 16, 54, 74, 255 },
	DEEP = { 10, 30, 44, 255 }, DARK = { 8, 20, 30, 255 }, GOLD = { 255, 214, 16, 255 }, ORANGE = { 222, 132, 0, 255 },
	GREEN_HI = { 74, 231, 115, 255 }, GREEN = { 0, 123, 74, 255 }, WHITE = { 240, 250, 255, 255 };

static SDL_Color with_alpha(SDL_Color c, int a) { c.a = (Uint8)a; return c; }
/* (a fill lit by a thumb) */
static SDL_Color bright(SDL_Color c) {
	SDL_Color b = { (Uint8)fminf(255, c.r * 1.35f + 20), (Uint8)fminf(255, c.g * 1.35f + 20), (Uint8)fminf(255, c.b * 1.35f + 20), c.a };
	return b;
}

static TouchPrefs prefs, before;   /* (before: as they were when the editor opened, for CANCEL) */
static TouchLayout lay;
static TouchScreen scr;
static char prefs_path[600];
static bool prefs_read, shown, always;
static int warm;    /* (the pressed looks made for this layout so far) */
static uint32_t taken;
/* What a finger does: nothing yet (it went down between the controls),
 * steers the D-pad, presses a button (and rolls onto the next), taps MENU. */
enum { F_NONE, F_STEER, F_PRESS, F_MENU };
static struct { bool on; SDL_FingerID id; float x, y; int mode, control; uint32_t dir; } fingers[FINGERS];

/* their menu over the paused game, and its editor */
enum { PAGE_NONE, PAGE_MENU, PAGE_EDIT };
static int page;
static int sel = -1;                  /* the editor's chosen control */
enum { G_NONE, G_WIDGET, G_MOVE, G_SIZE };
/* What each finger does on the menu: works a widget, moves the chosen
 * control, or sizes it (a corner, or a second finger's pinch). */
static struct { int what, widget; float dx, dy, d0, x0, y0; int size0; bool moving; } grab[FINGERS];

bool touch_shown(void) { return shown; }
bool touch_paused(void) { return shown && page != PAGE_NONE; }

static void close_menu(void);

bool touch_show(bool on) {
	if (always) on = true;
	if (on == shown) return false;
	shown = on;
	/* (a key or a button put them away: their menu keeps what it has) */
	if (!on && page) close_menu();
	memset(fingers, 0, sizeof fingers);
	if (!on) art_flush();
	return true;
}

void touch_always(void) { always = shown = true; }

void touch_release(void) {
	memset(fingers, 0, sizeof fingers);
	memset(grab, 0, sizeof grab);
}

void touch_reset_art(void) { art_flush(); }

#ifdef CW_IOS
/* (an iPhone's notch, rounded corners and home indicator: each control
 * moved in past what they cover, as Android's own layout leaves to the
 * system) */
static void safe_area(void) {
	float top, left, bottom, right;
	int ww = 0, wh = 0;
	ios_safe_insets(P.window, &top, &left, &bottom, &right);
	SDL_GetWindowSize(P.window, &ww, &wh);
	if (ww <= 0) return;
	float k = (float)P.screen_w / (float)ww;   /* (the screen's pixels to its points) */
	for (int c = 0; c < TOUCH_CONTROLS; ++c) {
		TouchBox *b = &lay.box[c];
		float x0 = left * k + b->w / 2, x1 = (float)P.screen_w - right * k - b->w / 2;
		float y0 = top * k + b->h / 2, y1 = (float)P.screen_h - bottom * k - b->h / 2;
		if (x0 <= x1) b->cx = fminf(fmaxf(b->cx, x0), x1);
		if (y0 <= y1) b->cy = fminf(fmaxf(b->cy, y0), y1);
	}
}

#endif
void touch_relayout(void) {
	/* (the defaults until touch.ini is read: headless runs read none) */
	if (!prefs_read) { touch_prefs_default(&prefs); prefs_read = true; }
	int ox = (P.screen_w - P.w * P.scale) / 2, oy = (P.screen_h - P.h * P.scale) / 2;
	scr.w = P.screen_w;
	scr.h = P.screen_h;
	scr.dp = P.dp;
	scr.px = ox + P.core_x * P.scale;
	scr.py = oy + P.core_y * P.scale;
	scr.pw = CORE_W * P.scale;
	scr.ph = CORE_H * P.scale;
	touch_layout_for(&scr, &prefs, &lay);
#ifdef CW_IOS
	safe_area();
#endif
	warm = 0;
}

void touch_load(const char *path) {
	snprintf(prefs_path, sizeof prefs_path, "%s", path);
	static char text[8192];
	FILE *f = fopen(path, "r");
	size_t n = f ? fread(text, 1, sizeof text - 1, f) : 0;
	if (f) fclose(f);
	text[n] = 0;
	touch_prefs_parse(text, &prefs);
	prefs_read = true;
	touch_relayout();
}

static void save(void) {
	if (!prefs_path[0]) return;
	static char text[8192];
	int n = touch_prefs_format(&prefs, text, sizeof text);
	FILE *f = fopen(prefs_path, "w");
	if (!f) return;
	fwrite(text, 1, (size_t)n, f);
	fclose(f);
	platform_persist();
}

/* ---- haptics: a short tick under the thumb for each press ---- */

static bool haptics_exist(void) {
#if defined(__ANDROID__) || defined(CW_IOS)
	return true;
#elif defined(__EMSCRIPTEN__)
	static int has = -1;
	if (has < 0) has = emscripten_run_script_int("typeof navigator.vibrate === 'function' ? 1 : 0");
	return has == 1;
#else
	return false;
#endif
}

static void tick(void) {
	if (!prefs.haptics || !haptics_exist()) return;
#if defined(__ANDROID__)
	/* (the activity's own key feedback, as Android's keyboard gives, under
	 * the system's touch feedback setting; GameActivity.haptic) */
	static bool missing;
	JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	if (env && activity && !missing) {
		jclass cls = (*env)->GetObjectClass(env, activity);
		jmethodID mid = (*env)->GetMethodID(env, cls, "haptic", "()V");
		if (mid) (*env)->CallVoidMethod(env, activity, mid);
		if ((*env)->ExceptionCheck(env)) { (*env)->ExceptionClear(env); missing = true; }
		(*env)->DeleteLocalRef(env, cls);
	}
	if (env && activity) (*env)->DeleteLocalRef(env, activity);
#elif defined(CW_IOS)
	ios_haptic();
#elif defined(__EMSCRIPTEN__)
	emscripten_run_script("navigator.vibrate(12)");
#endif
}

/* ---- the fingers on the controls ---- */

/* (the D-pad's diagonals where MegaMan walks, whose walkways run along
 * them; elsewhere, a battle's panels, the menus and the Custom screen,
 * they only get in the way) */
static bool four_way(void) { return !director_on_map(); }

static bool is_button(int c) { return touch_button(c) != 0; }

/* A finger, down or moved, takes or keeps what it reaches. */
static void finger_at(int k, float x, float y, bool down) {
	fingers[k].x = x;
	fingers[k].y = y;
	int mode = fingers[k].mode;
	if (mode == F_STEER) {
		uint32_t d = touch_dpad_steer(&lay, x, y, fingers[k].dir, four_way());
		if (d && d != fingers[k].dir) tick();
		fingers[k].dir = d;
		return;
	}
	if (mode == F_MENU) return;
	int c = touch_control_at(&lay, x, y, mode == F_PRESS ? fingers[k].control : -1);
	if (mode == F_NONE) {
		/* (one that went down between them takes the first it reaches;
		 * MENU only by a tap on it) */
		if (c == TOUCH_DPAD) {
			fingers[k].mode = F_STEER;
			fingers[k].dir = 0;
			finger_at(k, x, y, down);
		} else if (is_button(c)) {
			fingers[k].mode = F_PRESS;
			fingers[k].control = c;
			taken |= touch_button(c);
			tick();
		} else if (c == TOUCH_MENU && down) {
			fingers[k].mode = F_MENU;
			fingers[k].control = c;
		}
		return;
	}
	/* a thumb rolls from B onto A (and off them onto nothing) */
	int now = is_button(c) ? c : -1;
	if (now >= 0 && now != fingers[k].control) {
		taken |= touch_button(now);
		tick();
	}
	fingers[k].control = now;
}

static void open_menu(void);
static void menu_finger(uint32_t type, int k, float x, float y);

bool touch_event(const SDL_Event *e) {
	/* (a touchpad's fingers move a pointer: they touch no screen) */
	if (SDL_GetTouchDeviceType(e->tfinger.touchId) != SDL_TOUCH_DEVICE_DIRECT) return false;
	return touch_finger(e->type, e->tfinger.fingerId, e->tfinger.x * P.screen_w, e->tfinger.y * P.screen_h);
}

bool touch_finger(uint32_t type, SDL_FingerID id, float x, float y) {
	int k = 0;
	while (k < FINGERS && !(fingers[k].on && fingers[k].id == id)) ++k;
	if (type == SDL_FINGERDOWN) {
		if (touch_show(true)) return true;
		if (k == FINGERS) for (k = 0; k < FINGERS && fingers[k].on; ++k) {}
		if (k == FINGERS) return false;
		memset(&fingers[k], 0, sizeof fingers[k]);
		memset(&grab[k], 0, sizeof grab[k]);
		fingers[k].on = true;
		fingers[k].id = id;
		fingers[k].control = -1;
		if (page) menu_finger(type, k, x, y);
		else finger_at(k, x, y, true);
		return false;
	}
	if (k == FINGERS) return false;
	if (page) {
		menu_finger(type, k, x, y);
	} else if (type == SDL_FINGERUP) {
		/* (MENU opens on a tap that lifts on it) */
		if (fingers[k].mode == F_MENU && fingers[k].control == TOUCH_MENU && touch_control_at(&lay, x, y, TOUCH_MENU) == TOUCH_MENU) open_menu();
	} else {
		finger_at(k, x, y, false);
	}
	if (type == SDL_FINGERUP) fingers[k].on = false;
	return false;
}

uint32_t touch_held(void) {
	uint32_t bits = 0;
	if (page) return 0;
	for (int k = 0; k < FINGERS; ++k) {
		if (!fingers[k].on) continue;
		if (fingers[k].mode == F_STEER) bits |= fingers[k].dir;
		else if (fingers[k].mode == F_PRESS) bits |= touch_button(fingers[k].control);
	}
	return bits;
}

uint32_t touch_taken(void) {
	uint32_t t = page ? 0 : taken;
	taken = 0;
	return t;
}

/* ---- their menu and the editor, over the paused game ---- */

/* The widgets, placed afresh for each use (the screen turns, the rows
 * change): what they are and where. */
enum {
	U_NONE, U_SIZE, U_OPACITY, U_HAPTICS_ON, U_HAPTICS_OFF, U_EDIT, U_RESUME,
	U_CSIZE, U_CALPHA, U_DEFAULT, U_LARGE, U_LEFT, U_COMPACT, U_RESET, U_CANCEL, U_DONE,
};
enum { K_SLIDER, K_CHIP, K_BUTTON, K_MAIN };
typedef struct { int id, kind; float x, y, w, h; const char *text; } Widget;
static Widget ui[16];
static int nui;
static struct { float x, y, w, h; } panel;
static float row_h, pad_x, text_y[3];   /* (the title's, the chosen control's name's and the hint's middles) */
static int ts_title, ts, ts_small;

static const char *const control_titles[TOUCH_CONTROLS] = { "D-PAD", "A BUTTON", "B BUTTON", "L BUTTON", "R BUTTON", "START", "SELECT", "MENU" };

static void add(int id, int kind, float x, float y, float w, float h, const char *text) {
	if (nui >= (int)(sizeof ui / sizeof *ui)) return;
	Widget *g = &ui[nui++];
	g->id = id; g->kind = kind; g->x = x; g->y = y; g->w = w; g->h = h; g->text = text;
}

/* Chips across a row from x, `n` of them, fitting `room`. */
static void chips(float x, float y, float room, float h, const int *ids, const char *const *names, int n) {
	float D = lay.dp, gap = 8 * D, total;
	for (;;) {
		total = gap * (n - 1);
		for (int i = 0; i < n; ++i) total += art_text_width(names[i], ts_small) + 24 * D;
		if (total <= room || ts_small <= 1) break;
		--ts_small;
	}
	for (int i = 0; i < n; ++i) {
		float w = art_text_width(names[i], ts_small) + 24 * D;
		add(ids[i], K_CHIP, x, y, w, h, names[i]);
		x += w + gap;
	}
}

static int shape_places(void) {
	int n = 0;
	for (int c = 0; c < TOUCH_CONTROLS; ++c) {
		const TouchPlace *q = &prefs.place[lay.shape][c];
		n += q->moved || q->size || q->alpha;
	}
	return n;
}

static void place_ui(void) {
	nui = 0;
	float D = lay.dp, W = (float)scr.w, H = (float)scr.h;
	bool edit = page == PAGE_EDIT;
	int rows = edit ? 3 : haptics_exist() ? 3 : 2;
	float pad = 14 * D, title = 40 * D, name = edit ? 28 * D : 0, buttons = 52 * D;
	row_h = 46 * D;
	float pw = fminf(W - 24 * D, 420 * D), ph = pad + title + name + rows * row_h + buttons + pad;
	/* (over the picture, clear of the controls under it or beside it) */
	float top = 8 * D, bottom = H - 8 * D;
	if (lay.shape == TOUCH_BELOW) {
		top = fmaxf(top, scr.py - 16 * D);
		for (int c = 0; c < TOUCH_CONTROLS; ++c)
			if (lay.box[c].cy > scr.py + scr.ph / 2) bottom = fminf(bottom, lay.box[c].cy - lay.box[c].h / 2 - 6 * D);
	} else if (lay.shape == TOUCH_SIDE) {
		pw = fminf(pw, scr.pw + 8 * D);
	}
	float k = ph > bottom - top ? fmaxf(0.6f, (bottom - top) / ph) : 1;
	pad *= k; title *= k; name *= k; row_h *= k; buttons *= k; ph *= k;
	pad_x = pad;
	panel.w = pw;
	panel.h = ph;
	panel.x = (W - pw) / 2;
	panel.y = lay.shape == TOUCH_BELOW ? top : top + (bottom - top - ph) / 2;
	ts_title = (int)fmaxf(1, roundf(3.0f * D * k));
	ts = (int)fmaxf(1, roundf(2.3f * D * k));
	ts_small = (int)fmaxf(1, roundf(1.9f * D * k));
	float x0 = panel.x + pad, x1 = panel.x + pw - pad, y = panel.y + pad;
	text_y[0] = y + title / 2;
	y += title;
	if (edit) { text_y[1] = y + name / 2; y += name; }
	/* sliders: a label, the track, the value */
	/* (the value clear of the knob at the track's end) */
	float label_w = art_text_width("OPACITY", ts) + 14 * D, value_w = art_text_width("100%", ts) + 22 * D;
	add(edit ? U_CSIZE : U_SIZE, K_SLIDER, x0 + label_w, y, x1 - value_w - x0 - label_w, row_h, "SIZE");
	y += row_h;
	add(edit ? U_CALPHA : U_OPACITY, K_SLIDER, x0 + label_w, y, x1 - value_w - x0 - label_w, row_h, "OPACITY");
	y += row_h;
	float chip_h = fminf(34 * D, row_h - 8 * D);
	if (edit) {
		static const int ids[] = { U_DEFAULT, U_LARGE, U_LEFT, U_COMPACT };
		static const char *const names[] = { "DEFAULT", "LARGE", "LEFT-HANDED", "COMPACT" };
		/* (LARGE where there is room for it: a phone's controls are about
		 * as large as fit) */
		int ids2[4], n = 0;
		const char *names2[4];
		for (int i = 0; i < 4; ++i)
			if (ids[i] != U_LARGE || lay.size_max >= 120) { ids2[n] = ids[i]; names2[n++] = names[i]; }
		chips(x0, y + (row_h - chip_h) / 2, x1 - x0, chip_h, ids2, names2, n);
		y += row_h;
	} else if (haptics_exist()) {
		static const int ids[] = { U_HAPTICS_ON, U_HAPTICS_OFF };
		static const char *const names[] = { "ON", "OFF" };
		chips(x0 + label_w, y + (row_h - chip_h) / 2, x1 - x0 - label_w, chip_h, ids, names, 2);
		y += row_h;
	}
	/* the way out */
	float bh = fminf(40 * D, buttons - 10 * D), by = y + (buttons - bh) / 2, gap = 10 * D;
	if (edit) {
		float bw = (x1 - x0 - 2 * gap) / 3;
		add(U_RESET, K_BUTTON, x0, by, bw, bh, "RESET");
		add(U_CANCEL, K_BUTTON, x0 + bw + gap, by, bw, bh, "CANCEL");
		add(U_DONE, K_MAIN, x0 + 2 * (bw + gap), by, bw, bh, "DONE");
	} else {
		float bw = (x1 - x0 - gap) / 2;
		add(U_EDIT, K_BUTTON, x0, by, bw, bh, "EDIT LAYOUT");
		add(U_RESUME, K_MAIN, x0 + bw + gap, by, bw, bh, "RESUME");
	}
	/* the editor's hint under the panel, and under a control there (MENU) */
	text_y[2] = panel.y + panel.h + 16 * D;
	float half = art_text_width("DRAG IT TO MOVE IT, PINCH OR A CORNER TO SIZE IT", ts_small) / 2.f;
	for (int c = 0; c < TOUCH_CONTROLS; ++c) {
		const TouchBox *b = &lay.box[c];
		if (fabsf(b->cy - text_y[2]) < b->h / 2 + 10 * D && fabsf(b->cx - W / 2) < b->w / 2 + half) text_y[2] = b->cy + b->h / 2 + 14 * D;
	}
}

static int widget_at(float x, float y) {
	for (int i = 0; i < nui; ++i) {
		const Widget *g = &ui[i];
		/* (a slider's whole row, and a finger's width round its ends) */
		float grow = g->kind == K_SLIDER ? 12 * lay.dp : 4 * lay.dp;
		if (x >= g->x - grow && x < g->x + g->w + grow && y >= g->y && y < g->y + g->h) return i;
	}
	return -1;
}

static TouchPlace *chosen(void) { return sel >= 0 ? &prefs.place[lay.shape][sel] : NULL; }

/* A slider's range and value. */
static void slider_range(int id, int *lo, int *hi, int *v) {
	TouchPlace *q = chosen();
	switch (id) {
	case U_SIZE: *lo = TOUCH_SIZE_MIN; *hi = lay.size_max; *v = prefs.size < *hi ? prefs.size : *hi; break;
	case U_OPACITY: *lo = TOUCH_ALPHA_MIN; *hi = 100; *v = prefs.opacity; break;
	case U_CSIZE: *lo = TOUCH_SIZE_MIN; *hi = TOUCH_SIZE_MAX; *v = q && q->size ? q->size : 100; break;
	default: *lo = TOUCH_ALPHA_MIN; *hi = 100; *v = q && q->alpha ? q->alpha : 100; break;
	}
}

static bool widget_off(const Widget *g) { return (g->id == U_CSIZE || g->id == U_CALPHA || g->id == U_RESET) && sel < 0 && (g->id != U_RESET || !shape_places()); }

static void slide(const Widget *g, float x) {
	if (widget_off(g)) return;
	int lo, hi, v;
	slider_range(g->id, &lo, &hi, &v);
	float t = fmaxf(0, fminf(1, (x - g->x) / g->w));
	/* (in steps of five) */
	v = lo + (int)roundf(t * (hi - lo) / 5) * 5;
	if (v > hi) v = hi;
	TouchPlace *q = chosen();
	switch (g->id) {
	case U_SIZE: prefs.size = v; break;
	case U_OPACITY: prefs.opacity = v; break;
	case U_CSIZE: if (q) q->size = (int16_t)(v == 100 ? 0 : v); break;
	case U_CALPHA: if (q) q->alpha = (int16_t)(v == 100 ? 0 : v); break;
	default: break;
	}
	touch_relayout();
}

static void clear_shape(void) { memset(prefs.place[lay.shape], 0, sizeof prefs.place[lay.shape]); }

static bool chip_lit(int id) {
	switch (id) {
	case U_HAPTICS_ON: return prefs.haptics;
	case U_HAPTICS_OFF: return !prefs.haptics;
	case U_DEFAULT: return prefs.size == 100 && prefs.opacity == 100 && !prefs.left_handed && !shape_places();
	case U_LARGE: return prefs.size == 125;
	case U_LEFT: return prefs.left_handed;
	case U_COMPACT: return prefs.size == 80;
	default: return false;
	}
}

static void press(int id) {
	switch (id) {
	case U_HAPTICS_ON: prefs.haptics = true; tick(); break;
	case U_HAPTICS_OFF: prefs.haptics = false; break;
	case U_EDIT:
		before = prefs;
		page = PAGE_EDIT;
		sel = -1;
		break;
	case U_RESUME: close_menu(); return;
	/* presets: a size and a hand, this arrangement laid out afresh */
	case U_DEFAULT: prefs.size = prefs.opacity = 100; prefs.left_handed = false; clear_shape(); sel = -1; break;
	case U_LARGE: prefs.size = 125; clear_shape(); break;
	case U_COMPACT: prefs.size = 80; clear_shape(); break;
	case U_LEFT: prefs.left_handed = !prefs.left_handed; clear_shape(); break;
	case U_RESET:
		/* (the chosen control back where it was laid out, else them all) */
		if (sel >= 0) memset(&prefs.place[lay.shape][sel], 0, sizeof(TouchPlace));
		else clear_shape();
		break;
	case U_CANCEL: prefs = before; page = PAGE_MENU; sel = -1; break;
	case U_DONE: save(); page = PAGE_MENU; sel = -1; break;
	default: break;
	}
	touch_relayout();
}

static void open_menu(void) {
	page = PAGE_MENU;
	sel = -1;
	memset(grab, 0, sizeof grab);
	for (int k = 0; k < FINGERS; ++k) fingers[k].mode = F_NONE;
	tick();
}

static void close_menu(void) {
	/* (the editor left open keeps what it has) */
	page = PAGE_NONE;
	sel = -1;
	memset(grab, 0, sizeof grab);
	/* (the fingers on the menu press nothing as it goes) */
	for (int k = 0; k < FINGERS; ++k) { fingers[k].mode = F_MENU; fingers[k].control = -1; }
	save();
	touch_relayout();
}

bool touch_back(void) {
	if (!touch_paused()) return false;
	if (page == PAGE_EDIT) press(U_CANCEL);
	else close_menu();
	return true;
}

/* The corner handle of the chosen control a finger is on, or -1. */
static int handle_at(float x, float y) {
	if (sel < 0) return -1;
	const TouchBox *b = &lay.box[sel];
	float D = lay.dp, hw = b->w / 2 + 6 * D, hh = b->h / 2 + 6 * D;
	for (int i = 0; i < 4; ++i) {
		float hx = b->cx + (i & 1 ? hw : -hw), hy = b->cy + (i & 2 ? hh : -hh);
		if (hypotf(x - hx, y - hy) <= 22 * D) return i;
	}
	return -1;
}

static int size_of(int c) { const TouchPlace *q = &prefs.place[lay.shape][c]; return q->size ? q->size : 100; }

static void set_size(int c, int v) {
	v = v < TOUCH_SIZE_MIN ? TOUCH_SIZE_MIN : v > TOUCH_SIZE_MAX ? TOUCH_SIZE_MAX : v;
	prefs.place[lay.shape][c].size = (int16_t)(v == 100 ? 0 : v);
	touch_relayout();
}

static void menu_finger(uint32_t type, int k, float x, float y) {
	place_ui();
	fingers[k].x = x;
	fingers[k].y = y;
	if (type == SDL_FINGERDOWN) {
		int w = widget_at(x, y);
		if (w >= 0) {
			grab[k].what = G_WIDGET;
			grab[k].widget = w;
			if (ui[w].kind == K_SLIDER) slide(&ui[w], x);
			return;
		}
		if (page == PAGE_MENU) {
			/* (a tap beside the panel goes back to the game) */
			if (x < panel.x || x >= panel.x + panel.w || y < panel.y || y >= panel.y + panel.h) grab[k].what = G_NONE, grab[k].widget = -2;
			return;
		}
		/* the editor: a second finger on the screen pinches the control the
		 * first moves; a corner handle sizes it; else a finger takes the
		 * control it is on, or lets the chosen one go */
		for (int j = 0; j < FINGERS; ++j)
			if (j != k && fingers[j].on && grab[j].what == G_MOVE && sel >= 0) {
				grab[k].what = G_SIZE;
				grab[k].widget = j;
				grab[k].d0 = fmaxf(1, hypotf(x - fingers[j].x, y - fingers[j].y));
				grab[k].size0 = size_of(sel);
				return;
			}
		if (handle_at(x, y) >= 0) {
			grab[k].what = G_SIZE;
			grab[k].widget = -1;
			grab[k].d0 = fmaxf(1, hypotf(x - lay.box[sel].cx, y - lay.box[sel].cy));
			grab[k].size0 = size_of(sel);
			return;
		}
		if (x >= panel.x && x < panel.x + panel.w && y >= panel.y && y < panel.y + panel.h) return;
		sel = touch_control_at(&lay, x, y, -1);
		if (sel >= 0) {
			grab[k].what = G_MOVE;
			grab[k].dx = x - lay.box[sel].cx;
			grab[k].dy = y - lay.box[sel].cy;
			grab[k].x0 = x;
			grab[k].y0 = y;
		}
		return;
	}
	if (type == SDL_FINGERMOTION) {
		if (grab[k].what == G_WIDGET && ui[grab[k].widget].kind == K_SLIDER) slide(&ui[grab[k].widget], x);
		else if (grab[k].what == G_MOVE && sel >= 0) {
			/* (a tap chooses it: it moves once the finger has, a few dp) */
			if (!grab[k].moving && hypotf(x - grab[k].x0, y - grab[k].y0) < 6 * lay.dp) return;
			grab[k].moving = true;
			TouchPlace *q = &prefs.place[lay.shape][sel];
			float cx = fmaxf(0, fminf((float)scr.w, x - grab[k].dx)), cy = fmaxf(0, fminf((float)scr.h, y - grab[k].dy));
			q->moved = true;
			q->x = (int16_t)(cx * 1000 / scr.w);
			q->y = (int16_t)(cy * 1000 / scr.h);
			touch_relayout();
		} else if (grab[k].what == G_SIZE && sel >= 0) {
			int j = grab[k].widget;
			float d = j >= 0 ? hypotf(x - fingers[j].x, y - fingers[j].y) : hypotf(x - lay.box[sel].cx, y - lay.box[sel].cy);
			set_size(sel, (int)roundf(grab[k].size0 * d / grab[k].d0));
		}
		return;
	}
	/* lifted: a button pressed where it was let go */
	if (grab[k].what == G_WIDGET) {
		const Widget *g = &ui[grab[k].widget];
		if (g->kind != K_SLIDER && widget_at(x, y) == grab[k].widget && !widget_off(g)) press(g->id);
	} else if (page == PAGE_MENU && grab[k].widget == -2 &&
		(x < panel.x || x >= panel.x + panel.w || y < panel.y || y >= panel.y + panel.h)) {
		close_menu();
	}
	grab[k].what = G_NONE;
}

/* ---- drawing, at the screen's own pixels ---- */

static ArtSpec spec(int kind) {
	ArtSpec s;
	memset(&s, 0, sizeof s);
	s.kind = kind;
	return s;
}

/* (sizes whole and widths in quarter pixels: a picture is made once for them) */
static float px(float v) { return roundf(v); }
static float quarter(float v) { return roundf(v * 4) / 4; }

static void label(ArtSpec *s, const char *text, int scale, SDL_Color ink, SDL_Color outline) {
	snprintf(s->label, sizeof s->label, "%s", text);
	s->scale = scale;
	s->ink = ink;
	s->ink_edge = outline;
}

static ArtSpec disc_spec(const TouchBox *b, SDL_Color ring, SDL_Color fill, const char *name, bool lit) {
	float u = lay.unit;
	ArtSpec s = spec(ART_DISC);
	s.w = s.h = px(b->w);
	s.edge_w = quarter(2.8f * u);
	s.rim_w = quarter(1.15f * u);
	s.edge = with_alpha(ring, 235);
	s.fill = lit ? with_alpha(bright(fill), 215) : with_alpha(fill, 110);
	if (lit) { s.shrink = 0.94f; s.glow = 1.28f; s.glow_color = with_alpha(ring, 140); }
	label(&s, name, (int)fmaxf(1, b->w / 2 * 0.084f), WHITE, with_alpha(DARK, 217));
	return s;
}

static ArtSpec plate_spec(const TouchBox *b, const char *name, bool lit) {
	float u = lay.unit;
	ArtSpec s = spec(ART_PLATE);
	s.w = px(b->w);
	s.h = px(b->h);
	s.radius = s.h / 2;
	s.edge_w = quarter(2.5f * u);
	s.rim_w = quarter(1.15f * u);
	s.edge = with_alpha(CYAN, 230);
	s.fill = lit ? with_alpha(CYAN, 200) : with_alpha(NAVY, 100);
	if (lit) { s.glow = 1.25f; s.glow_color = with_alpha(CYAN_HI, 110); }
	int scale = (int)fmaxf(1, strlen(name) == 1 ? b->h * 0.076f : b->h * 0.043f + 0.3f);
	while (scale > 1 && art_text_width(name, scale) > b->w * 0.72f) --scale;
	label(&s, name, scale, WHITE, with_alpha(DARK, 217));
	return s;
}

/* The D-pad (arm -1), or one of its arms lit (0 up, 1 right, 2 down, 3 left). */
static ArtSpec dpad_spec(const TouchBox *b, int arm) {
	float u = lay.unit;
	ArtSpec s = spec(arm < 0 ? ART_DPAD : ART_ARM);
	s.w = s.h = px(b->w);
	s.edge_w = quarter(2.8f * u);
	if (arm < 0) {
		s.rim_w = quarter(1.15f * u);
		s.edge = with_alpha(CYAN, 230);
		s.fill = with_alpha(NAVY, 100);
		s.ink = with_alpha(WHITE, 235);
	} else {
		s.arm = arm;
		s.fill = with_alpha(CYAN, 225);
		s.glow_color = with_alpha(CYAN_HI, 150);
		s.ink = with_alpha(DARK, 235);
	}
	return s;
}

static const char *const control_labels[TOUCH_CONTROLS] = { "", "A", "B", "L", "R", "START", "SELECT", "MENU" };

static ArtSpec control_spec(int c, bool lit) {
	const TouchBox *b = &lay.box[c];
	if (c == TOUCH_A) return disc_spec(b, GOLD, ORANGE, control_labels[c], lit);
	if (c == TOUCH_B) return disc_spec(b, GREEN_HI, GREEN, control_labels[c], lit);
	return plate_spec(b, control_labels[c], lit);
}

/* The pressed looks made ahead, one a frame after the layout changes, so
 * a first press does not wait for its picture: the D-pad's arms, A, B,
 * then the rest. */
#define WARM_LOOKS (4 + TOUCH_CONTROLS - 1)

static void warm_up(void) {
	int i = warm++;
	ArtSpec s = i < 4 ? dpad_spec(&lay.box[TOUCH_DPAD], i) : control_spec(i - 4 + 1, true);
	art_get(&s);
}

static void draw_controls(void) {
	uint32_t held = touch_held();
	bool lit[TOUCH_CONTROLS] = { false };
	for (int k = 0; k < FINGERS; ++k)
		if (fingers[k].on && !page && (fingers[k].mode == F_PRESS || fingers[k].mode == F_MENU) && fingers[k].control >= 0)
			lit[fingers[k].control] = fingers[k].mode == F_PRESS || touch_control_at(&lay, fingers[k].x, fingers[k].y, TOUCH_MENU) == TOUCH_MENU;
	static const uint32_t arms[4] = { BTN_UP, BTN_RIGHT, BTN_DOWN, BTN_LEFT };
	for (int i = 0; i < TOUCH_CONTROLS; ++i) {
		/* (the editor's chosen one over the rest) */
		int c = sel < 0 || page != PAGE_EDIT ? i : i == TOUCH_CONTROLS - 1 ? sel : i < sel ? i : i + 1;
		const TouchBox *b = &lay.box[c];
		float a = lay.alpha[c];
		if (c == TOUCH_DPAD) {
			ArtSpec s = dpad_spec(b, -1);
			art_draw(&s, b->cx, b->cy, a);
			for (int arm = 0; arm < 4; ++arm) {
				if (!(held & arms[arm])) continue;
				s = dpad_spec(b, arm);
				art_draw(&s, b->cx, b->cy, a);
			}
			continue;
		}
		ArtSpec s = control_spec(c, lit[c]);
		art_draw(&s, b->cx, b->cy, a);
	}
}

static void draw_selection(void) {
	if (sel < 0) return;
	const TouchBox *b = &lay.box[sel];
	float D = lay.dp;
	bool round = sel == TOUCH_DPAD || sel == TOUCH_A || sel == TOUCH_B;
	ArtSpec s = spec(ART_DASHED);
	s.round = round;
	s.w = px(b->w + 12 * D);
	s.h = px(b->h + 12 * D);
	s.radius = round ? s.w / 2 : s.h / 2;
	s.edge_w = quarter(2.2f * D);
	s.edge = GOLD;
	art_draw(&s, b->cx, b->cy, 1);
	float hw = b->w / 2 + 6 * D, hh = b->h / 2 + 6 * D;
	for (int i = 0; i < 4; ++i) {
		ArtSpec h = spec(ART_PLATE);
		h.w = h.h = px(12 * D);
		h.radius = px(3 * D);
		h.edge_w = quarter(1.5f * D);
		h.edge = DARK;
		h.fill = GOLD;
		art_draw(&h, b->cx + (i & 1 ? hw : -hw), b->cy + (i & 2 ? hh : -hh), 1);
	}
}

static bool widget_held(int i) {
	for (int k = 0; k < FINGERS; ++k)
		if (fingers[k].on && grab[k].what == G_WIDGET && grab[k].widget == i) return true;
	return false;
}

static void draw_slider(const Widget *g, int i) {
	float D = lay.dp, cy = g->y + g->h / 2, a = widget_off(g) ? 0.35f : 1;
	int lo, hi, v;
	slider_range(g->id, &lo, &hi, &v);
	art_text(panel.x + pad_x, cy, g->text, ts, with_alpha(WHITE, (int)(255 * a)), with_alpha(DARK, (int)(255 * a)), -1);
	ArtSpec t = spec(ART_PLATE);
	t.w = px(g->w);
	t.h = px(6 * D);
	t.radius = t.h / 2;
	t.edge_w = quarter(1.2f * D);
	t.edge = CYAN;
	t.fill = NAVY;
	art_draw(&t, g->x + g->w / 2, cy, a);
	float f = hi > lo ? (float)(v - lo) / (hi - lo) : 1, kx = g->x + g->w * f;
	/* (the track lit up to the knob) */
	SDL_Rect clip = { (int)g->x - 2, (int)(cy - 8 * D), (int)(kx - g->x) + 2, (int)(16 * D) + 1 };
	SDL_RenderSetClipRect(P.renderer, &clip);
	t.fill = CYAN;
	art_draw(&t, g->x + g->w / 2, cy, a);
	SDL_RenderSetClipRect(P.renderer, NULL);
	ArtSpec k = spec(ART_DISC);
	k.w = k.h = px((widget_held(i) ? 24 : 20) * D);
	k.edge_w = quarter(1.6f * D);
	k.edge = DARK;
	k.fill = WHITE;
	art_draw(&k, kx, cy, a);
	char value[8];
	snprintf(value, sizeof value, "%d%%", v);
	art_text(panel.x + panel.w - pad_x, cy, value, ts, with_alpha(WHITE, (int)(255 * a)), with_alpha(DARK, (int)(255 * a)), 1);
}

static void draw_menu(void) {
	float D = lay.dp;
	place_ui();
	ArtSpec p = spec(ART_PLATE);
	p.w = px(panel.w);
	p.h = px(panel.h);
	p.radius = px(10 * D);
	p.edge_w = quarter(2 * D);
	p.edge = CYAN;
	p.fill = with_alpha(DEEP, 238);
	art_draw(&p, panel.x + panel.w / 2, panel.y + panel.h / 2, 1);
	bool edit = page == PAGE_EDIT;
	art_text(panel.x + panel.w / 2, text_y[0], edit ? "EDIT TOUCH CONTROLS" : "TOUCH CONTROLS", ts_title, CYAN_HI, DARK, 0);
	if (edit) {
		if (sel >= 0) art_text(panel.x + panel.w / 2, text_y[1], control_titles[sel], ts, GOLD, DARK, 0);
		else art_text(panel.x + panel.w / 2, text_y[1], "TAP A BUTTON TO CHOOSE IT", ts_small, with_alpha(WHITE, 170), DARK, 0);
	}
	if (!edit && haptics_exist()) {
		for (int i = 0; i < nui; ++i)
			if (ui[i].id == U_HAPTICS_ON) art_text(panel.x + pad_x, ui[i].y + ui[i].h / 2, "HAPTICS", ts, WHITE, DARK, -1);
	}
	for (int i = 0; i < nui; ++i) {
		const Widget *g = &ui[i];
		if (g->kind == K_SLIDER) { draw_slider(g, i); continue; }
		bool main = g->kind == K_MAIN, chip = g->kind == K_CHIP, lit = chip && chip_lit(g->id), held = widget_held(i);
		ArtSpec b = spec(ART_PLATE);
		b.w = px(g->w);
		b.h = px(g->h);
		b.radius = chip ? b.h / 2 : px(8 * D);
		b.edge_w = quarter((chip ? 1.5f : 2) * D);
		b.edge = main ? GOLD : CYAN;
		SDL_Color fill = main ? GOLD : lit ? CYAN : NAVY;
		b.fill = held ? bright(fill) : fill;
		bool dark_ink = main || lit;
		label(&b, g->text, chip ? ts_small : ts, dark_ink ? DARK : WHITE, dark_ink ? b.fill : DARK);
		art_draw(&b, g->x + g->w / 2, g->y + g->h / 2, widget_off(g) ? 0.35f : 1);
	}
	if (edit && text_y[2] + 10 * D < scr.h)
		art_text(scr.w / 2.f, text_y[2], sel >= 0 ? "DRAG IT TO MOVE IT, PINCH OR A CORNER TO SIZE IT" : "DRAG A BUTTON TO MOVE IT",
			ts_small, CYAN_HI, DARK, 0);
}

void touch_draw(void) {
	art_tick();
	if (!shown) return;
	SDL_SetRenderDrawBlendMode(P.renderer, SDL_BLENDMODE_BLEND);
	if (page) {
		/* the paused game dimmed under them */
		SDL_SetRenderDrawColor(P.renderer, 4, 10, 16, 120);
		SDL_RenderFillRect(P.renderer, NULL);
	}
	draw_controls();
	if (warm < WARM_LOOKS && !page) warm_up();
	if (page == PAGE_EDIT) draw_selection();
	if (page) draw_menu();
}
