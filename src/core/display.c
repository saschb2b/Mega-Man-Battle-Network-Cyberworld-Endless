/* display.h. The scale is read by the layout (platform.c's game_canvas,
 * the 3DS's present); the window and smooth motion are the platform's
 * own (P.fullscreen, P.blend), set through it; the GBA's screen's are
 * lcd_frame.c's and lcd_grid.c's. */
#include "display.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
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
/* (interframe_blending's words, LcdBlend's order; mGBA's mix and mix_smart read too) */
static const char *const blends[LCD_BLEND_COUNT] = { "off", "simple", "smart", "lcd_ghosting", "lcd_ghosting_fast" };
/* (colour_correction's, LcdColour's order: the shaders' names) */
static const char *const colours[LCD_COLOUR_COUNT] = { "off", "gba", "vba", "nds", "psp" };

DisplayScale display_scale(void) { return scale; }

DisplayScale display_scale_default(void) {
#ifdef __3DS__
	return SCALE_FILL;
#else
	return SCALE_AUTO;
#endif
}

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

static LcdBlend blend_read(const char *word) {
	for (int b = 0; b < LCD_BLEND_COUNT; ++b)
		if (!strcmp(word, blends[b])) return (LcdBlend)b;
	return !strcmp(word, "mix") ? LCD_BLEND_SIMPLE : !strcmp(word, "mix_smart") ? LCD_BLEND_SMART : LCD_BLEND_OFF;
}

/* (in either case, as mGBA spells GBA; ds for nds, and on for gba, the
 * one there was first) */
static LcdColour colour_read(const char *word) {
	char low[8];
	size_t n = strlen(word);
	if (n >= sizeof low) return LCD_COLOUR_OFF;
	for (size_t i = 0; i <= n; ++i) low[i] = (char)tolower((unsigned char)word[i]);
	for (int c = 0; c < LCD_COLOUR_COUNT; ++c)
		if (!strcmp(low, colours[c])) return (LcdColour)c;
	return !strcmp(low, "ds") ? LCD_COLOUR_NDS : yes(low) ? LCD_COLOUR_GBA : LCD_COLOUR_OFF;
}

/* The GBA's screen's lines (the command line's, which settings.ini's leave
 * as they set them: `forced`, a bit each) */
static const char *const lcd_keys[] = { "colour_correction", "interframe_blending", "lcd3x" };
static unsigned forced;

static bool lcd_setting(const char *key, const char *val) {
	int k = 0;
	while (k < 3 && strcmp(key, lcd_keys[k])) ++k;
	/* (as mGBA spells it) */
	if (!strcmp(key, "color_correction")) k = 0;
	if (k == 3) return false;
	if (forced & 1u << k) return true;
	if (k == 0) lcd_set_colour(colour_read(val));
	else if (k == 1) lcd_set_blend(blend_read(val));
	else lcd_set_grid(yes(val));
	return true;
}

/* settings.ini's voxel (docs/engine/voxel.md): on or off, and a dev's
 * debug; the camera's angle, scale and depth the command line's alone,
 * while the look is being chosen */
static int voxel;
static float voxel_cam[3] = { 25.0f, 1.0f, 0.0f };
static unsigned voxel_forced;
static int voxel_read(const char *val) { return !strcmp(val, "debug") ? DISPLAY_VOXEL_DEBUG : yes(val) ? DISPLAY_VOXEL_ON : DISPLAY_VOXEL_OFF; }

int display_voxel(void) { return display_has_voxel() ? voxel : DISPLAY_VOXEL_OFF; }
float display_voxel_pitch(void) { return voxel_cam[0]; }
float display_voxel_zoom(void) { return voxel_cam[1]; }
float display_voxel_depth(void) { return voxel_cam[2]; }

bool display_has_voxel(void) {
#if defined(__3DS__) || defined(__vita__)
	return false;
#else
	return true;
#endif
}

bool display_setting(const char *key, const char *val) {
	if (!strcmp(key, "smooth_motion")) P.blend = yes(val);
	else if (!strcmp(key, "screen")) set(display_scale_read(val));
	else if (!strcmp(key, "voxel")) { if (!(voxel_forced & 1u)) voxel = voxel_read(val); }
	else return lcd_setting(key, val);
	return true;
}

bool display_arg(const char *flag, const char *val) {
	static const char *const cams[] = { "--voxel-pitch", "--voxel-zoom", "--voxel-depth" };
	for (int k = 0; k < 3; ++k)
		if (!strcmp(flag, cams[k])) { voxel_cam[k] = (float)atof(val); return true; }
	if (!strcmp(flag, "--voxel")) {
		voxel = voxel_read(val);
		voxel_forced |= 1u;
		return true;
	}
	static const char *const flags[] = { "--colour-correction", "--interframe-blending", "--lcd3x" };
	for (int k = 0; k < 3; ++k)
		if (!strcmp(flag, flags[k])) {
			forced &= ~(1u << k);
			lcd_setting(lcd_keys[k], val);
			forced |= 1u << k;
			return true;
		}
	return false;
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

bool display_has_lcd3x(void) {
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

LcdColour display_colour_next(int dir) { return (LcdColour)((lcd_colour() + LCD_COLOUR_COUNT + (dir < 0 ? -1 : 1)) % LCD_COLOUR_COUNT); }

void display_colour_keep(LcdColour c) {
	lcd_set_colour(c);
	keep("colour_correction", colours[lcd_colour()], options_colour_note());
}

LcdBlend display_blend_next(int dir) { return (LcdBlend)((lcd_blend() + LCD_BLEND_COUNT + (dir < 0 ? -1 : 1)) % LCD_BLEND_COUNT); }

void display_blend_keep(LcdBlend b) {
	lcd_set_blend(b);
	keep("interframe_blending", blends[lcd_blend()], options_blend_note());
}

void display_voxel_keep(bool on) {
	voxel = on && display_has_voxel() ? DISPLAY_VOXEL_ON : DISPLAY_VOXEL_OFF;
	keep("voxel", voxel ? "on" : "off", options_voxel_note());
}

void display_lcd3x_keep(bool on) {
	lcd_set_grid(on);
	keep("lcd3x", lcd_grid() ? "on" : "off", options_lcd3x_note());
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
