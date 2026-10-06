#include "platform.h"
#include "platform_second.h"
#include "present_3ds.h"

#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#ifdef CW_DESKTOP
#include <stdlib.h>
#include "app_icon.h"
#endif
#include "analytics_net.h"
#include "audio.h"
#include "controls.h"
#include "emu.h"
#include "frame_log.h"
#include "gfx.h"
#include "keymap.h"
#include "mirror.h"
#include "pads.h"
#include "touch.h"
#ifdef CW_IOS
#include "ios.h"
#endif

Platform P;

/* The canvas's pixels, the order the textures drawn on it have: on the
 * 3DS one copy a frame turns them into the GPU's (a canvas in the GPU's
 * order converted every sprite and letter drawn on it, 18 ms a frame
 * under a title card) */
#define CANVAS_FORMAT SDL_PIXELFORMAT_ARGB8888

static uint32_t injected;
static uint32_t key_bits;
static uint32_t key_tapped;   /* pressed since the last poll: a tap released in the same frame still counts */

bool platform_pad_present(void) { return pads_present(); }

/* Smooth motion (settings.ini): the game's two latest frames, each kept
 * whole as it ends, mixed at each refresh of the display by how far the
 * next one is due (main.c's loop). The game runs at the GBA's 60 frames a
 * second; a 90 or 144 Hz display shows each for one refresh or two, or two
 * or three, in turn, which reads as judder; mixed, motion is even, a little
 * blurred, and a frame later. */
static SDL_Texture *blend_tex[2];
static int blend_cur = -1, blend_n;   /* the latest kept (-1 none), how many (0-2) */

static void blend_reset(void) {
	for (int i = 0; i < 2; ++i)
		if (blend_tex[i]) { SDL_DestroyTexture(blend_tex[i]); blend_tex[i] = NULL; }
	blend_cur = -1;
	blend_n = 0;
}

static void blend_keep(void) {
	int next = blend_cur < 0 ? 0 : blend_cur ^ 1;
	if (!blend_tex[next]) {
		blend_tex[next] = SDL_CreateTexture(P.renderer, CANVAS_FORMAT, SDL_TEXTUREACCESS_TARGET, P.w, P.h);
		if (!blend_tex[next]) return;
		SDL_SetTextureScaleMode(blend_tex[next], SDL_ScaleModeNearest);
	}
	SDL_BlendMode mode;
	SDL_GetTextureBlendMode(P.canvas, &mode);
	SDL_SetTextureBlendMode(P.canvas, SDL_BLENDMODE_NONE);
	SDL_SetRenderTarget(P.renderer, blend_tex[next]);
	SDL_RenderCopy(P.renderer, P.canvas, NULL, NULL);
	SDL_SetTextureBlendMode(P.canvas, mode);
	blend_cur = next;
	if (blend_n < 2) ++blend_n;
}

static float forced_dpi;

void platform_set_dpi(float dpi) { forced_dpi = dpi; }

/* Screen pixels to a dp: Android's density (its dp), a page's device
 * pixels to a CSS pixel, a display's dots per inch; where none is told, a
 * handheld's screen of 400 dp across its short side, and never less than
 * one of 800 (a desktop's claimed 96 dpi). */
static float density(void) {
	if (forced_dpi > 0) return forced_dpi / 160;
	int short_side = P.screen_w < P.screen_h ? P.screen_w : P.screen_h;
	float dp = 0;
	/* (the browser's pixels to its CSS pixels; an iPhone's to its points,
	 * a 163rd of an inch, which SDL's DPI table knows not for every model) */
#if defined(__EMSCRIPTEN__) || defined(CW_IOS)
	int ww = 0, wh = 0;
	SDL_GetWindowSize(P.window, &ww, &wh);
	if (ww > 0) dp = (float)P.screen_w / (float)ww;
#elif !defined(__3DS__)
	float ddpi = 0;
	if (P.window && SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(P.window), &ddpi, NULL, NULL) == 0 && ddpi > 0) dp = ddpi / 160;
	else dp = short_side / 400.f;
#endif
	if (dp < short_side / 800.f) dp = short_side / 800.f;
	return dp > 0 ? dp : 1;
}

/* settings.ini's screen: the picture at the largest whole scale (whole), at
 * the largest that fits (fill), or (auto) filling where the whole one leaves
 * it a quarter smaller or more: an RG35XX Pro's 640 x 480 drew it at
 * 480 x 320, half the screen (issue #36), where a Nova's 1280 x 960 gives
 * 5x against 5.33 and a Flip 2's 1920 x 1080 6x against 6.75 */
enum { SCREEN_AUTO, SCREEN_WHOLE, SCREEN_FILL };
#if !defined(__3DS__) && !defined(__EMSCRIPTEN__)
static int screen_mode = SCREEN_AUTO;
#endif
#define FILL_GAIN 1.25f

/* The scale the canvas fills the screen at, or 0 at a whole one: under the
 * touch controls only a phone's width, held upright (touch_upright_fill: a
 * 1080-wide screen drew the picture 960 wide, and the owner, playing the
 * APK upright, found it small); never on the 3DS (present_3ds.c fills its
 * top screen), nor on a page that sizes its canvas (all but a phone's). */
static float fill_scale(void) {
#if defined(__3DS__)
	return 0;
#elif defined(__EMSCRIPTEN__)
	return touch_shown() ? touch_upright_fill(P.screen_w, P.screen_h, P.dp, P.scale) : 0;
#else
	if (screen_mode == SCREEN_WHOLE) return 0;
	if (touch_shown()) return touch_upright_fill(P.screen_w, P.screen_h, P.dp, P.scale);
	float fx = (float)P.screen_w / CORE_W, fy = (float)P.screen_h / CORE_H, f = fx < fy ? fx : fy;
	if (f <= (float)P.scale || (screen_mode == SCREEN_AUTO && f < (float)P.scale * FILL_GAIN)) return 0;
	return f;
#endif
}

/* The top of an iPhone's safe area, in the screen's pixels: under its notch
 * or Dynamic Island, held upright; 0 elsewhere. */
static int safe_top(void) {
#ifdef CW_IOS
	float top, left, bottom, right;
	int ww = 0, wh = 0;
	ios_safe_insets(P.window, &top, &left, &bottom, &right);
	SDL_GetWindowSize(P.window, &ww, &wh);
	return ww > 0 ? (int)(top * (float)P.screen_w / (float)ww + 0.5f) : 0;
#else
	return 0;
#endif
}

/* A screen of its own (platform_own_taps): its taps and the mouse's, files
 * dropped on the window, and its own back */
static bool own_taps;
static int tap_x = -1, tap_y = -1, point_x = -1, point_y = -1;
static char dropped[1024];
#ifndef __EMSCRIPTEN__
static bool (*back_hook)(void);   /* (a browser's page keeps Escape) */
#endif

/* The game's canvas: the picture at the largest whole scale, or filling
 * the screen (fill_scale), the touch controls round it */
static void game_canvas(void) {
	int sx = P.screen_w / CORE_W, sy = P.screen_h / CORE_H;
	P.scale = sx < sy ? sx : sy;
	if (P.scale < 1) P.scale = 1;
	if (touch_shown()) P.scale = touch_fit_scale(P.screen_w, P.screen_h, P.dp, P.scale);
	P.fill = fill_scale();
	P.w = P.fill > 0 ? (int)((float)P.screen_w / P.fill) : P.screen_w / P.scale;
	P.h = P.fill > 0 ? (int)((float)P.screen_h / P.fill) : P.screen_h / P.scale;
	if (P.w < CORE_W) P.w = CORE_W;
	if (P.h < CORE_H) P.h = CORE_H;
	P.core_x = (P.w - CORE_W) / 2;
	P.core_y = (P.h - CORE_H) / 2;
	/* on a tall screen the touch controls take the room under the picture,
	 * which moves up clear of the status bar */
	float k = P.fill > 0 ? P.fill : (float)P.scale;
	int top = touch_shown() ? touch_picture_top(P.screen_w, P.screen_h, P.dp, k) : -1;
	/* (and clear of an iPhone's notch or island, which a status bar's room is not) */
	if (top >= 0 && top < safe_top()) top = safe_top();
	if (top >= 0) {
		float oy = ((float)P.screen_h - (float)P.h * k) / 2;
		int y = (int)(((float)top - oy) / k + 0.999f);
		if (y >= 0 && y + CORE_H <= P.h) P.core_y = y;
	}
}

/* ... and a screen of its own's, a menu's: whole scales alone, the
 * largest that leaves it 240 x 160 or more, or 160 x 240 on a screen held
 * upright, where the picture's 240 across made a phone's letters small */
static void menu_canvas(void) {
	bool upright = P.screen_h > P.screen_w;
	int sx = P.screen_w / (upright ? CORE_H : CORE_W), sy = P.screen_h / (upright ? CORE_W : CORE_H);
	P.scale = sx < sy ? sx : sy;
	if (P.scale < 1) P.scale = 1;
	P.fill = 0;
	P.w = P.screen_w / P.scale;
	P.h = P.screen_h / P.scale;
	P.core_x = (P.w - CORE_W) / 2;
	P.core_y = (P.h - CORE_H) / 2;
}

static void layout_canvas(void) {
	P.dp = density();
	if (own_taps) menu_canvas();
	else game_canvas();
	touch_relayout();
	if (P.canvas) SDL_DestroyTexture(P.canvas);
	if (P.fx_copy) SDL_DestroyTexture(P.fx_copy);
	blend_reset();
	P.canvas = SDL_CreateTexture(P.renderer, CANVAS_FORMAT, SDL_TEXTUREACCESS_TARGET, P.w, P.h);
	SDL_SetTextureScaleMode(P.canvas, SDL_ScaleModeNearest);
	P.fx_copy = SDL_CreateTexture(P.renderer, CANVAS_FORMAT, SDL_TEXTUREACCESS_TARGET, P.w, P.h);
	SDL_SetTextureScaleMode(P.fx_copy, SDL_ScaleModeNearest);
	if (P.sharp) SDL_DestroyTexture(P.sharp);
	P.sharp = NULL;
	if (P.fill > 0) {
		int whole = (int)P.fill + 1;   /* (the whole scale just over the fill) */
		P.sharp = SDL_CreateTexture(P.renderer, CANVAS_FORMAT, SDL_TEXTUREACCESS_TARGET, P.w * whole, P.h * whole);
		if (P.sharp) SDL_SetTextureScaleMode(P.sharp, SDL_ScaleModeLinear);
		else P.fill = 0;   /* (no memory for it: the canvas at the whole scale, a little wide) */
	}
}

/* The canvas's scale as the log says it: "5x", or "2.67x (fill)". */
static const char *scale_words(void) {
	static char s[24];
	if (P.fill > 0) snprintf(s, sizeof s, "%.2fx (fill)", (double)P.fill);
	else snprintf(s, sizeof s, "%dx", P.scale);
	return s;
}

/* The screen as the renderer's target, its viewport the whole of it (a
 * turn's, taken up here rather than by SDL: turns_here). */
static void screen_target(void) {
	SDL_SetRenderTarget(P.renderer, NULL);
	SDL_RenderSetViewport(P.renderer, NULL);
}

/* The window's new size (resized, or in or out of fullscreen): the canvas
 * follows at the largest whole scale. */
static void resized(void) {
	if (P.forced) return;
	screen_target();
	SDL_GetRendererOutputSize(P.renderer, &P.screen_w, &P.screen_h);
	layout_canvas();
}

#ifdef __EMSCRIPTEN__
/* The canvas's size on the page (CSS pixels) when the page sizes it (a
 * phone's whole screen): the window follows it, drawn at the device's
 * pixels. Asked every frame: a turned phone or the page going fullscreen
 * moves it, and SDL takes the size of a moment where it measures 0. */
static void follow_page(void) {
	double w, h;
	if (P.forced || P.headless || emscripten_get_element_css_size("#canvas", &w, &h) != EMSCRIPTEN_RESULT_SUCCESS || w < 2 || h < 2) return;
	int cw, ch;
	SDL_GetWindowSize(P.window, &cw, &ch);
	if ((int)w == cw && (int)h == ch && P.screen_w > 1) return;
	SDL_SetWindowSize(P.window, (int)w, (int)h);
	resized();
}
#endif

#ifndef __EMSCRIPTEN__
static void set_fullscreen(bool on) {
	P.fullscreen = on;
	SDL_SetWindowFullscreen(P.window, on ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	SDL_ShowCursor(on ? SDL_DISABLE : SDL_ENABLE);
	resized();
}
#endif

/* A point on the screen (its pixels) as the canvas pixel under it */
static void canvas_point(float sx, float sy, int *x, int *y) {
	float k = P.fill > 0 ? P.fill : (float)P.scale;
	float ox = ((float)P.screen_w - (float)P.w * k) / 2, oy = ((float)P.screen_h - (float)P.h * k) / 2;
	*x = (int)((sx - ox) / k);
	*y = (int)((sy - oy) / k);
}

void platform_own_taps(bool on) {
	static bool touch_was;
	if (on == own_taps) return;
	if (on) touch_was = touch_shown();
	own_taps = on;
	tap_x = tap_y = point_x = point_y = -1;
	dropped[0] = 0;
	/* (the touch controls away, and back as they were; the mouse's cursor
	 * shown over it, a fullscreen game's too) */
	touch_show(on ? false : touch_was);
	layout_canvas();
#ifdef CW_DESKTOP
	if (!P.headless) SDL_ShowCursor(on || !P.fullscreen ? SDL_ENABLE : SDL_DISABLE);
	SDL_EventState(SDL_DROPFILE, on ? SDL_ENABLE : SDL_DISABLE);
#endif
}

bool platform_tap(int *x, int *y) {
	if (tap_x < 0) return false;
	*x = tap_x;
	*y = tap_y;
	tap_x = tap_y = -1;
	return true;
}

bool platform_pointer(int *x, int *y) {
	*x = point_x;
	*y = point_y;
	return point_x >= 0;
}

bool platform_dropped(char *path, size_t n) {
	if (!dropped[0]) return false;
	snprintf(path, n, "%s", dropped);
	dropped[0] = 0;
	return true;
}

void platform_on_back(bool (*back)(void)) {
#ifndef __EMSCRIPTEN__
	back_hook = back;
#else
	(void)back;
#endif
}

void platform_safe_edges(int *top, int *left, int *bottom, int *right) {
	*top = *left = *bottom = *right = 0;
#ifdef CW_IOS
	float t, l, b, r, k = P.fill > 0 ? P.fill : (float)P.scale;
	int ww = 0, wh = 0;
	ios_safe_insets(P.window, &t, &l, &b, &r);
	SDL_GetWindowSize(P.window, &ww, &wh);
	if (ww <= 0 || wh <= 0) return;
	/* (the window's points as the screen's pixels, then the canvas's, past
	 * its border on each side) */
	float px = (float)P.screen_w / (float)ww, py = (float)P.screen_h / (float)wh;
	float ox = ((float)P.screen_w - (float)P.w * k) / 2, oy = ((float)P.screen_h - (float)P.h * k) / 2;
	*top = t > 0 ? (int)((t * py - oy) / k + 0.999f) : 0;
	*bottom = b > 0 ? (int)((b * py - oy) / k + 0.999f) : 0;
	*left = l > 0 ? (int)((l * px - ox) / k + 0.999f) : 0;
	*right = r > 0 ? (int)((r * px - ox) / k + 0.999f) : 0;
	if (*top < 0) *top = 0;
	if (*bottom < 0) *bottom = 0;
	if (*left < 0) *left = 0;
	if (*right < 0) *right = 0;
#endif
}

/* The mouse over a screen of its own (a finger's own mouse aside): where
 * it is, and where its left button let go */
static void mouse(const SDL_Event *e) {
	if (!own_taps) return;
	bool motion = e->type == SDL_MOUSEMOTION;
	if ((motion ? e->motion.which : e->button.which) == SDL_TOUCH_MOUSEID) return;
	if (!motion && e->button.button != SDL_BUTTON_LEFT) return;
	int ww = 0, wh = 0;
	SDL_GetWindowSize(P.window, &ww, &wh);
	/* (the window's points as the renderer's pixels: a high-DPI window's differ) */
	float kx = ww > 0 ? (float)P.screen_w / (float)ww : 1, ky = wh > 0 ? (float)P.screen_h / (float)wh : 1;
	if (motion) canvas_point((float)e->motion.x * kx, (float)e->motion.y * ky, &point_x, &point_y);
	else canvas_point((float)e->button.x * kx, (float)e->button.y * ky, &tap_x, &tap_y);
}

void platform_finger(uint32_t type, SDL_FingerID id, float x, float y) {
	/* (a screen that takes its own taps: where on the canvas a finger is,
	 * and where it lifted) */
	if (own_taps) {
		if (type == SDL_FINGERUP) {
			canvas_point(x, y, &tap_x, &tap_y);
			point_x = point_y = -1;
		} else canvas_point(x, y, &point_x, &point_y);
		return;
	}
	/* (the controls screen takes the fingers while it is open) */
	if (controls_shown()) {
		int cx, cy;
		canvas_point(x, y, &cx, &cy);
		controls_finger(type, id, cx, cy);
		return;
	}
	if (touch_finger(type, id, x, y)) layout_canvas();
}

#ifndef __3DS__
/* A finger on the screen: the touch controls', the controls screen's, or
 * on iOS a screen's own tap (a touchpad's fingers move a pointer: they
 * touch no screen) */
static void finger(const SDL_Event *e) {
	if (SDL_GetTouchDeviceType(e->tfinger.touchId) != SDL_TOUCH_DEVICE_DIRECT) return;
	platform_finger(e->type, e->tfinger.fingerId, e->tfinger.x * (float)P.screen_w, e->tfinger.y * (float)P.screen_h);
}
#endif

/* A phone's own, before SDL starts: Android's and the iPhone's hints. */
static void phone_hints(void) {
#ifdef __ANDROID__
	/* a phone turns (the touch controls go under the picture or beside it),
	 * and Back is Escape (the quit prompt), not the end of the app */
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown");
	SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
	/* (a phone's tilt is no controller) */
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif
#ifdef CW_IOS
	/* an iPhone turns as an Android phone does; the home indicator fades
	 * till a swipe at the screen's foot (two swipes to leave), and the
	 * phone's tilt is no controller */
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait");
	SDL_SetHint(SDL_HINT_IOS_HIDE_HOME_INDICATOR, "2");
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif
}

/* ... and its window's flags: SDL locks a window that cannot resize to
 * the way the phone is held at the start, whatever the hint allows; on
 * iOS, the screen's own pixels, not its points */
static Uint32 phone_window_flags(void) {
#if defined(__ANDROID__)
	return SDL_WINDOW_RESIZABLE;
#elif defined(CW_IOS)
	return SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#else
	return 0;
#endif
}

#ifdef CW_IOS
/* The app sent to the background and back, as it happens (an event watch,
 * as iOS wants these: the app may be held right after them): main.c's
 * loop keeps the run and draws nothing meanwhile */
static int app_moved(void *user, SDL_Event *e) {
	(void)user;
	if (e->type == SDL_APP_WILLENTERBACKGROUND) P.background = true;
	else if (e->type == SDL_APP_DIDENTERFOREGROUND) P.background = false;
	return 0;
}
#endif

#ifdef __ANDROID__
/* Android sends a turned phone's new size from Java's thread, and SDL's
 * renderer took it up there, under the game's own drawing: it switched the
 * renderer's targets mid-frame, and a turn left the game laid out 240
 * pixels wide in a corner (the canvas's size read as the screen's) or the
 * screen black till the app was closed, the emulator's from its first
 * turn. Kept from that thread, a turn waits for the game's own
 * (follow_screen, resized). */
static SDL_threadID game_thread;
static SDL_atomic_t turned;

static int SDLCALL turns_here(void *user, SDL_Event *e) {
	(void)user;
	if (e->type != SDL_WINDOWEVENT || SDL_ThreadID() == game_thread) return 1;
	if (e->window.event != SDL_WINDOWEVENT_SIZE_CHANGED && e->window.event != SDL_WINDOWEVENT_RESIZED) return 1;
	SDL_AtomicSet(&turned, 1);
	return 0;
}
#endif

/* ... and once its window is up: the buttons from the start, until a
 * controller's first press (a handheld's own controls: its first START);
 * a keyboard with arrow keys counts as a controller here, so its presence
 * decides nothing */
static void phone_controls(void) {
#if defined(__ANDROID__) || defined(CW_IOS)
	if (touch_show(true)) layout_canvas();
#endif
#ifdef __ANDROID__
	game_thread = SDL_ThreadID();
	SDL_SetEventFilter(turns_here, NULL);
	/* (setting a filter drops the events waiting: a size that came before
	 * it, the navigation bar hidden as the game started, read again) */
	SDL_AtomicSet(&turned, 1);
	SDL_Log("screen %dx%d, canvas %dx%d at %s, touch controls %s", P.screen_w, P.screen_h, P.w, P.h, scale_words(), touch_shown() ? "shown" : "hidden");
#endif
#ifdef CW_IOS
	SDL_AddEventWatch(app_moved, NULL);
	/* (a Battle Network ROM the start's look refused beside BN6's, said
	 * over the game: ios.m) */
	ios_rom_note(P.window);
#endif
}

bool platform_init(int force_w, int force_h, bool headless, bool fullscreen) {
	P.headless = headless;
	keymap_default();
	if (headless) {
		/* (the variables, not their hints: SDL before 2.0.22, as on older
		 * handhelds, has no hints for them) */
		SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
		SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	phone_hints();
#ifdef CW_DESKTOP
	/* the window's class is the application ID, which the .desktop file names */
#ifndef _WIN32
	setenv("SDL_VIDEO_X11_WMCLASS", APP_ID, 0);
	setenv("SDL_VIDEO_WAYLAND_WMCLASS", APP_ID, 0);
#endif
	SDL_SetHint("SDL_APP_NAME", "Cyberworld Endless");
#endif
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0) {
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
			fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
			return false;
		}
	}
	Uint32 flags = SDL_WINDOW_SHOWN;
	int ww = force_w, wh = force_h;
	P.forced = force_w && force_h;
	if (!P.forced) {
		SDL_DisplayMode dm;
		if (SDL_GetDesktopDisplayMode(0, &dm) != 0) { dm.w = CORE_W * 4; dm.h = CORE_H * 4; }
		if (fullscreen && !headless) {
			ww = dm.w; wh = dm.h;
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		} else {
			/* a window at the largest whole scale that leaves room around it */
			int sx = dm.w * 85 / 100 / CORE_W, sy = dm.h * 85 / 100 / CORE_H, s = sx < sy ? sx : sy;
			if (s < 1) s = 1;
			ww = CORE_W * s; wh = CORE_H * s;
			if (!headless) flags |= SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
		}
	}
	P.fullscreen = fullscreen && !headless;
	flags |= phone_window_flags();
	P.window = SDL_CreateWindow("Mega Man Battle Network: Cyberworld Endless",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ww, wh, flags);
	if (!P.window) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); return false; }
#ifdef CW_DESKTOP
	/* (SDL takes the pixels as void *, and only reads them) */
	union { const void *c; void *v; } pixels = { app_icon_rgba };
	SDL_Surface *icon = SDL_CreateRGBSurfaceWithFormatFrom(pixels.v, APP_ICON_SIZE, APP_ICON_SIZE, 32,
		APP_ICON_SIZE * 4, SDL_PIXELFORMAT_RGBA32);
	if (icon) { SDL_SetWindowIcon(P.window, icon); SDL_FreeSurface(icon); }
#endif
#ifdef _WIN32
	/* Direct3D 11 first: SDL's lets the GPU queue one frame at most
	 * (SetMaximumFrameLatency), where Direct3D 9, SDL2's first choice,
	 * leaves it to the driver, which queues up to three, each a frame of
	 * input lag; SDL falls back if it fails. (Naming a driver turns SDL's
	 * batching off: on again.) */
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
	SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
#endif
#ifdef __3DS__
	/* (the 3DS: the renderer draws a canvas of the GBA's size into memory
	 * the GPU puts on the screen, present_3ds.c) */
	SDL_Surface *screen = present3ds_init(CORE_W, CORE_H);
	P.renderer = screen ? SDL_CreateSoftwareRenderer(screen) : NULL;
	if (!P.renderer) { fprintf(stderr, "3ds: the GPU's present did not start: %s\n", SDL_GetError()); return false; }
#else
	Uint32 rflags = headless ? SDL_RENDERER_SOFTWARE : (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	P.renderer = SDL_CreateRenderer(P.window, -1, rflags);
	if (!P.renderer) P.renderer = SDL_CreateRenderer(P.window, -1, SDL_RENDERER_SOFTWARE);
	if (!P.renderer) { fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError()); return false; }
#endif
	SDL_GetRendererOutputSize(P.renderer, &P.screen_w, &P.screen_h);
	if (force_w && force_h) { P.screen_w = force_w; P.screen_h = force_h; }
	layout_canvas();
	SDL_ShowCursor(P.fullscreen || headless ? SDL_DISABLE : SDL_ENABLE);
	pads_open();
	phone_controls();
	SDL_RendererInfo info;
	SDL_GetRendererInfo(P.renderer, &info);
	printf("display %dx%d renderer %s canvas %dx%d at %s\n", P.screen_w, P.screen_h, info.name, P.w, P.h, scale_words());
	return true;
}

void platform_shutdown(void) {
	/* (the saves as the game ends them, copied before it goes: mirror.h;
	 * a statistics' request under way given up) */
	mirror_flush();
	analytics_net_quit();
	pads_close();
	blend_reset();
	if (P.canvas) SDL_DestroyTexture(P.canvas);
	if (P.fx_copy) SDL_DestroyTexture(P.fx_copy);
	if (P.sharp) SDL_DestroyTexture(P.sharp);
	if (P.renderer) SDL_DestroyRenderer(P.renderer);
#ifdef __3DS__
	present3ds_exit();
#endif
	if (P.window) SDL_DestroyWindow(P.window);
	SDL_Quit();
}

#ifdef __3DS__
/* (settings.ini's screen: fill, the picture at the top screen's height, or
 * whole, at 1x in its middle) */
static bool fill_3ds = true;
#endif

#ifdef __EMSCRIPTEN__
/* The page's Smooth motion button, while the game runs (web/play/app.js). */
void cw_set_smooth(int on);
EMSCRIPTEN_KEEPALIVE void cw_set_smooth(int on) { P.blend = on != 0; }
#endif

void platform_load_settings(const char *path) {
	FILE *f = fopen(path, "r");
	if (!f) {
		f = fopen(path, "w");
		if (!f) return;
		fprintf(f,
			"# Cyberworld Endless: settings. Delete this file for the defaults.\n\n"
			"# smooth_motion: the game runs at the GBA's 60 frames a second. A 90, 144\n"
			"# or 165 Hz screen shows each frame for one refresh or two (or two or\n"
			"# three) in turn, which reads as judder. on mixes the two latest frames at\n"
			"# every refresh, so motion is even, a little blurred and a frame later;\n"
			"# off shows the game's own frames, sharp (best on 60 and 120 Hz).\n"
			"smooth_motion = off\n\n"
			"# screen: whole draws the game's picture at the largest whole scale, every\n"
			"# pixel the same size; fill makes it as big as the screen holds, each pixel's\n"
			"# edge a little soft; auto fills where a whole scale would leave it a quarter\n"
			"# smaller or more (a 640 x 480 screen), else whole.\n"
			"screen = auto\n");
		fclose(f);
		platform_persist();
		return;
	}
	char line[256];
	while (fgets(line, sizeof line, f)) {
		char key[64], val[64];
		if (line[0] == '#' || sscanf(line, " %63[a-z_] = %63s", key, val) != 2) continue;
		if (!strcmp(key, "smooth_motion")) P.blend = !strcmp(val, "on") || !strcmp(val, "yes") || !strcmp(val, "1");
#ifdef __3DS__
		if (!strcmp(key, "screen")) fill_3ds = strcmp(val, "whole") != 0;
#elif !defined(__EMSCRIPTEN__)
		if (!strcmp(key, "screen")) screen_mode = !strcmp(val, "whole") ? SCREEN_WHOLE : !strcmp(val, "fill") ? SCREEN_FILL : SCREEN_AUTO;
#endif
		/* (not written by default: for a report of a machine's pacing) */
		if (!strcmp(key, "frame_log")) platform_frame_log = !strcmp(val, "on") || !strcmp(val, "yes") || !strcmp(val, "1");
	}
	fclose(f);
	/* (the screen setting read after the window opened; logged where it
	 * changed the scale, as the display line before it was printed with
	 * the default's) */
	if (P.renderer && !P.headless) {
		char was[24];
		snprintf(was, sizeof was, "%s", scale_words());
		layout_canvas();
		if (strcmp(was, scale_words())) printf("screen setting: canvas %dx%d at %s\n", P.w, P.h, scale_words());
	}
}

void platform_inject(uint32_t buttons) { injected = buttons; }

/* The screen's size followed before a frame's events: a page's canvas
 * (follow_page), or a phone's turn kept from Java's thread (turns_here). */
static void follow_screen(void) {
#if defined(__EMSCRIPTEN__)
	follow_page();
#elif defined(__ANDROID__)
	if (SDL_AtomicSet(&turned, 0)) resized();
#endif
}

/* This frame's keys pressed (the controls screen's "press a key") */
static int keys_pressed[8], nkeys_pressed;

int platform_keys_pressed(int *out, int most) {
	int n = nkeys_pressed < most ? nkeys_pressed : most;
	memcpy(out, keys_pressed, (size_t)n * sizeof *out);
	return n;
}

#ifndef __EMSCRIPTEN__
/* Escape, or Android's Back: the touch controls' menu and the controls
 * screen close first; the first asks, the second within two seconds quits */
static void escape(void) {
	if (touch_back() || controls_back() || (back_hook && back_hook())) return;
	if (P.quit_prompt > 0) P.quit = true;
	else { P.quit_prompt = 120; P.quit_pad = false; }
}
#endif

static void key_down(const SDL_KeyboardEvent *k) {
	if (k->repeat) return;
	uint32_t b = keymap_button(k->keysym.scancode);
	/* Alt+Enter is fullscreen, not Start */
	if (k->keysym.mod & KMOD_ALT) b &= ~BTN_START;
	key_bits |= b;
	key_tapped |= b;
	if (b) P.keyboard_last = true;
	if (nkeys_pressed < (int)(sizeof keys_pressed / sizeof *keys_pressed)) keys_pressed[nkeys_pressed++] = k->keysym.scancode;
	/* the keyboard's hands put the touch controls away */
	if (b && touch_show(false)) layout_canvas();
#ifndef __EMSCRIPTEN__
	/* (in a browser the page keeps Escape and fullscreen; Android's Back
	 * is Escape) */
	if (k->keysym.scancode == SDL_SCANCODE_ESCAPE || k->keysym.scancode == SDL_SCANCODE_AC_BACK) escape();
	/* F11 or Alt+Enter: fullscreen and back */
	if (!P.headless && (k->keysym.sym == SDLK_F11 || (k->keysym.sym == SDLK_RETURN && (k->keysym.mod & KMOD_ALT))))
		set_fullscreen(!P.fullscreen);
#endif
}

/* keys let go of in another window would stay held, and fingers lifted
 * over another app's */
static void let_go(void) {
	key_bits = 0;
	touch_release();
}

static void poll_event(const SDL_Event *e) {
	switch (e->type) {
	case SDL_QUIT: P.quit = true; printf("quit: the window closed, or the system asked\n"); break;
	case SDL_FINGERDOWN:
	case SDL_FINGERMOTION:
	case SDL_FINGERUP:
#ifndef __3DS__
		/* (the 3DS's touch screen is its bottom one, apart from the
		 * picture: not the phone's controls round it) */
		finger(e);
#endif
		break;
	case SDL_KEYDOWN: key_down(&e->key); break;
	case SDL_KEYUP:
		key_bits &= ~keymap_button(e->key.keysym.scancode);
		break;
	case SDL_MOUSEMOTION:
	case SDL_MOUSEBUTTONUP: mouse(e); break;
	/* (a file dropped on the window: a screen of its own's) */
	case SDL_DROPFILE:
	case SDL_DROPTEXT:
		if (own_taps && e->type == SDL_DROPFILE && e->drop.file) snprintf(dropped, sizeof dropped, "%s", e->drop.file);
		SDL_free(e->drop.file);
		break;
	case SDL_WINDOWEVENT:
		if (e->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) resized();
		if (e->window.event == SDL_WINDOWEVENT_FOCUS_LOST) let_go();
		if (e->window.event == SDL_WINDOWEVENT_LEAVE) point_x = point_y = -1;
		break;
	/* (and the saves' copy made: the app may be ended there unasked) */
	case SDL_APP_WILLENTERBACKGROUND: let_go(); mirror_flush(); break;
	/* (the renderer's textures lost: the controls' art is made again) */
	case SDL_RENDER_DEVICE_RESET: touch_reset_art(); ++P.textures_lost; break;
	default:
		/* a controller's: one coming or going, a button (a press puts the
		 * touch controls away) */
		if (pads_event(e)) {
			P.keyboard_last = false;
			if (touch_show(false)) layout_canvas();
		}
		break;
	}
}

/* a controller's Escape: Back and Start held a second asks, and held again
 * while it asks quits (a handheld's Steam Deck or a pad on the couch has no
 * keyboard; PortMaster's own hotkey is the same pair): the pad's own two,
 * whatever its map, or SELECT and START as mapped */
static void quit_chord(void) {
#ifndef __EMSCRIPTEN__
	static int pair_held;
	uint32_t both = BTN_SELECT | BTN_START;
	if (pads_quit_held() || (pads_held() & both) == both) {
		if (++pair_held == 60) {
			if (P.quit_prompt > 0 && P.quit_pad) P.quit = true;
			else { P.quit_prompt = 180; P.quit_pad = true; }
		}
	} else pair_held = 0;
#endif
}

/* Held now: pressed since the last frame, and pressed or held long enough
 * to repeat (menus) */
static void press_state(uint32_t now, uint32_t *held, uint32_t *pressed, uint32_t *repeat, int *timer) {
	*pressed = now & ~*held;
	*held = now;
	*repeat = *pressed;
	for (int i = 0; i < 10; ++i) {
		uint32_t b = 1u << i;
		if (!(now & b)) { timer[i] = 0; continue; }
		int t = ++timer[i];
		if (t > 18 && (t - 18) % 5 == 0) *repeat |= b;
	}
}

void platform_poll(void) {
	follow_screen();
	pads_begin();
	nkeys_pressed = 0;
	SDL_Event e;
	while (SDL_PollEvent(&e)) poll_event(&e);
	pads_poll();
	/* (iOS's loop in the background saves the run, then polls: its copy) */
	if (P.background) mirror_flush();
	if (P.quit_prompt > 0) --P.quit_prompt;
	quit_chord();
	uint32_t touched = touch_taken() | touch_held(), keys = key_bits | key_tapped | injected;
	key_tapped = 0;
	uint32_t now = keys | touched | pads_held() | pads_taken();
	P.released = P.held & ~now;
	press_state(now, &P.held, &P.pressed, &P.repeat, P.repeat_timer);
	press_state(keys | touched | pads_menu_held() | pads_menu_taken(), &P.menu_held, &P.menu_pressed, &P.menu_repeat, P.menu_timer);
}

void platform_begin_frame(void) {
	SDL_SetRenderTarget(P.renderer, P.canvas);
	SDL_SetRenderDrawColor(P.renderer, 0, 0, 0, 255);
	SDL_RenderClear(P.renderer);
	P.fx_mosaic = 0;
	P.fx_fade = 0;
}

void platform_apply_effects(void) {
	if (P.fx_mosaic > 1 && P.fx_copy) {
		/* Horizontal mosaic: every block repeats its leftmost pixel column,
		 * with blocks aligned to the 240x160 view as on the GBA. */
		SDL_SetRenderTarget(P.renderer, P.fx_copy);
		SDL_SetTextureBlendMode(P.canvas, SDL_BLENDMODE_NONE);
		SDL_RenderCopy(P.renderer, P.canvas, NULL, NULL);
		SDL_SetTextureBlendMode(P.canvas, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(P.renderer, P.canvas);
		SDL_SetTextureBlendMode(P.fx_copy, SDL_BLENDMODE_NONE);
		int n = P.fx_mosaic;
		int start = P.core_x % n - n;
		for (int x = start; x < P.w; x += n) {
			int sx = x < 0 ? 0 : x;
			SDL_Rect src = { sx, 0, 1, P.h }, dst = { x, 0, n, P.h };
			SDL_RenderCopy(P.renderer, P.fx_copy, &src, &dst);
		}
	}
	if (P.fx_fade > 0) {
		int a = P.fx_fade >= 16 ? 255 : P.fx_fade * 255 / 16;
		SDL_SetRenderDrawBlendMode(P.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(P.renderer, P.fx_fade_color.r, P.fx_fade_color.g, P.fx_fade_color.b, (Uint8)a);
		SDL_RenderFillRect(P.renderer, NULL);
	}
}

static char screen_shot[256];

void platform_shot_screen(const char *path) { snprintf(screen_shot, sizeof screen_shot, "%s", path); }

/* The touch controls over the picture, at the screen's own pixels, and
 * the screen saved whole when a test asks. */
static void over_picture(void) {
#ifndef __3DS__
	touch_draw();
#endif
	if (!screen_shot[0]) return;
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, P.screen_w, P.screen_h, 32, SDL_PIXELFORMAT_ARGB8888);
	if (s) {
		SDL_RenderReadPixels(P.renderer, NULL, SDL_PIXELFORMAT_ARGB8888, s->pixels, s->pitch);
		if (SDL_SaveBMP(s, screen_shot) != 0) fprintf(stderr, "screen shot %s: %s\n", screen_shot, SDL_GetError());
		SDL_FreeSurface(s);
	}
	screen_shot[0] = 0;
}

/* Where the canvas lands on the screen. */
static SDL_Rect canvas_rect(void) {
	if (P.fill > 0) {
		int w = (int)((float)P.w * P.fill + 0.5f), h = (int)((float)P.h * P.fill + 0.5f);
		return (SDL_Rect){ (P.screen_w - w) / 2, (P.screen_h - h) / 2, w, h };
	}
	return (SDL_Rect){ (P.screen_w - P.w * P.scale) / 2, (P.screen_h - P.h * P.scale) / 2, P.w * P.scale, P.h * P.scale };
}

/* `older`, then `newer` at `alpha` over it (none: `older` alone), into
 * the target's rect `dst` (NULL: all of it). */
static void copy_frames(SDL_Texture *older, SDL_Texture *newer, Uint8 alpha, const SDL_Rect *dst) {
	SDL_BlendMode was;
	SDL_GetTextureBlendMode(older, &was);
	SDL_SetTextureBlendMode(older, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(P.renderer, older, NULL, dst);
	SDL_SetTextureBlendMode(older, was);
	if (!newer) return;
	SDL_SetTextureBlendMode(newer, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(newer, alpha);
	SDL_RenderCopy(P.renderer, newer, NULL, dst);
	SDL_SetTextureAlphaMod(newer, 255);
}

/* The frame (or smooth motion's two) on the cleared screen: at a whole
 * scale straight; filling it, whole-scaled into P.sharp first and from
 * there smoothly to its size (sharp bilinear: a pixel's edges a little
 * soft, no pixel drawn wider than the next). */
static void show_frames(SDL_Texture *older, SDL_Texture *newer, Uint8 alpha) {
	SDL_Rect dst = canvas_rect();
	if (P.fill > 0) {
		SDL_SetRenderTarget(P.renderer, P.sharp);
		copy_frames(older, newer, alpha, NULL);
	}
	SDL_SetRenderTarget(P.renderer, NULL);
	SDL_SetRenderDrawColor(P.renderer, 0, 0, 0, 255);
	SDL_RenderClear(P.renderer);
	if (P.fill > 0) SDL_RenderCopy(P.renderer, P.sharp, NULL, &dst);
	else copy_frames(older, newer, alpha, &dst);
}

void platform_present_blend(double w) {
	/* (a display at a multiple of 60 Hz shows every frame for the same
	 * refreshes: mixing would only show it a frame later) */
	static int hz = -1;
	if (hz < 0) {
#ifdef __EMSCRIPTEN__
		hz = 0;   /* (a page is not told its screen's refresh) */
#else
		SDL_DisplayMode m;
		hz = SDL_GetWindowDisplayMode(P.window, &m) == 0 ? m.refresh_rate : 0;
#endif
	}
	if (hz > 0 && (hz % 60 <= 1 || hz % 60 >= 59)) w = 1;
	if (blend_n == 0) show_frames(P.canvas, NULL, 255);
	else {
		SDL_Texture *cur = blend_tex[blend_cur], *prev = blend_n > 1 ? blend_tex[blend_cur ^ 1] : cur;
		if (w < 0) w = 0;
		if (w > 1) w = 1;
		show_frames(prev, cur, (Uint8)(w * 255.0 + 0.5));
	}
	over_picture();
#ifdef __3DS__
	present3ds_frame(fill_3ds, emu_threaded());
#else
	SDL_RenderPresent(P.renderer);
#endif
	frame_log_present(0);
}

/* The canvas on the display, at its scale. */
static void present_canvas(void) {
	show_frames(P.canvas, NULL, 255);
	over_picture();
#ifdef __3DS__
	present3ds_frame(fill_3ds, emu_threaded());
#else
	SDL_RenderPresent(P.renderer);
#endif
}

void platform_present_now(void) {
	if (!P.headless) present_canvas();
}

void platform_draw_over(void) { SDL_SetRenderTarget(P.renderer, P.canvas); }

void platform_end_frame(void) {
	mirror_tick();
	/* (smooth motion keeps each frame whole for the mix at the refreshes) */
	if (P.blend && !P.headless) blend_keep();
	/* (a frame the loop plays to catch up, or one smooth motion mixes: not
	 * shown here, so it waits for no refresh of the display) */
	if (P.skip_present) {
		++P.frame;
		audio_log_frame = P.frame;
		return;
	}
	uint64_t t0 = SDL_GetPerformanceCounter();
	present_canvas();
	frame_log_present(SDL_GetPerformanceCounter() - t0);
	++P.frame;
	audio_log_frame = P.frame;
}

void platform_persist(void) {
#ifdef __EMSCRIPTEN__
	/* the browser's files live in memory until they are synced to IndexedDB */
	emscripten_run_script("if (typeof Module.persist === 'function') Module.persist();");
#else
	/* (a phone's saves copied to the ROM folder a moment later: mirror.h) */
	mirror_note();
#endif
}

bool platform_save_canvas(const char *path) {
	SDL_SetRenderTarget(P.renderer, P.canvas);
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, P.w, P.h, 32, SDL_PIXELFORMAT_ARGB8888);
	if (!s) return false;
	SDL_RenderReadPixels(P.renderer, NULL, SDL_PIXELFORMAT_ARGB8888, s->pixels, s->pitch);
	bool ok = SDL_SaveBMP(s, path) == 0;
	SDL_FreeSurface(s);
	SDL_SetRenderTarget(P.renderer, NULL);
	return ok;
}
