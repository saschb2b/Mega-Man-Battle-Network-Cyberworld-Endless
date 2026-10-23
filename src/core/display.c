/* display.h. The scale is read by the layout (platform.c's game_canvas,
 * the 3DS's present); the window and smooth motion are the platform's
 * own (P.fullscreen, P.blend), set through it. */
#include "display.h"

#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "game.h"
#include "options_text.h"
#include "platform.h"
#include "settings_line.h"

#ifdef __3DS__
static DisplayScale scale = SCALE_FILL;   /* (the 3DS's auto: its top screen filled) */
#else
static DisplayScale scale = SCALE_AUTO;
#endif

static const char *const keys[SCALE_COUNT] = { "auto", "whole", "fill", "stretch" };

DisplayScale display_scale(void) { return scale; }

DisplayScale display_scale_read(const char *word) {
	for (int s = 0; s < SCALE_COUNT; ++s)
		if (!strcmp(word, keys[s])) return (DisplayScale)s;
	/* (integer scaling, as Options says it and emulators do) */
	return !strcmp(word, "integer") ? SCALE_WHOLE : SCALE_AUTO;
}

static bool yes(const char *val) { return !strcmp(val, "on") || !strcmp(val, "yes") || !strcmp(val, "1"); }

/* (the 3DS's auto is its fill) */
static void set(DisplayScale s) {
#ifdef __3DS__
	if (s == SCALE_AUTO) s = SCALE_FILL;
#endif
	scale = s;
}

bool display_setting(const char *key, const char *val) {
	if (!strcmp(key, "smooth_motion")) P.blend = yes(val);
	else if (!strcmp(key, "screen")) set(display_scale_read(val));
	else return false;
	return true;
}

bool display_has_scale(void) {
#ifdef __EMSCRIPTEN__
	return false;
#else
	return true;
#endif
}

bool display_has_window(void) {
#ifdef CW_DESKTOP
	return true;
#else
	return false;
#endif
}

bool display_has_smooth(void) {
#ifdef __3DS__
	return false;
#else
	return true;
#endif
}

DisplayScale display_scale_next(int dir) {
#ifdef __3DS__
	static const DisplayScale order[] = { SCALE_FILL, SCALE_WHOLE, SCALE_STRETCH };
#else
	static const DisplayScale order[] = { SCALE_AUTO, SCALE_WHOLE, SCALE_FILL, SCALE_STRETCH };
#endif
	int n = (int)(sizeof order / sizeof *order), at = 0;
	for (int i = 0; i < n; ++i)
		if (order[i] == scale) at = i;
	return order[(at + n + (dir < 0 ? -1 : 1)) % n];
}

/* settings.ini's `key` set to `value`, and the files kept */
static void keep(const char *key, const char *value, const char *note) {
	char path[600];
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	settings_line_write(path, key, value, note);
	platform_persist();
}

void display_scale_use(DisplayScale s) {
	set(s);
	platform_relayout();
}

void display_scale_keep(DisplayScale s) {
	display_scale_use(s);
	keep("screen", keys[scale], options_screen_note());
}

void display_fullscreen_keep(bool on) {
	platform_set_fullscreen(on);
	keep("fullscreen", on ? "on" : "off", options_fullscreen_note());
}

void display_smooth_keep(bool on, bool tell_page) {
	platform_set_smooth(on);
	keep("smooth_motion", on ? "on" : "off", options_smooth_note());
#ifdef __EMSCRIPTEN__
	if (tell_page) emscripten_run_script(on ? "if (Module.cwSmooth) Module.cwSmooth(true);" : "if (Module.cwSmooth) Module.cwSmooth(false);");
#else
	(void)tell_page;
#endif
}

#ifdef __EMSCRIPTEN__
/* The page's Smooth motion button (web/play/app.js): set here as Options sets it. */
void cw_set_smooth(int on);
EMSCRIPTEN_KEEPALIVE void cw_set_smooth(int on) { display_smooth_keep(on != 0, false); }
#endif

int display_fullscreen_kept(const char *data_dir) {
	char path[600], val[16];
	snprintf(path, sizeof path, "%s/settings.ini", data_dir);
	if (!settings_line_read(path, "fullscreen", val, sizeof val)) return -1;
	return yes(val) ? 1 : 0;
}
