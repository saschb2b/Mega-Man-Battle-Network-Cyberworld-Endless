#include "platform.h"

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
#include "touch.h"

Platform P;

static SDL_GameController *pads[4];
static uint32_t injected;
static uint32_t pad_bits, key_bits;
static void keys_default(void);
static uint32_t tapped;   /* pressed since the last poll: a tap released in the same frame still counts */

static void open_pads(void) {
	for (int i = 0; i < SDL_NumJoysticks() && i < 4; ++i) {
		if (!pads[i] && SDL_IsGameController(i)) pads[i] = SDL_GameControllerOpen(i);
	}
}

bool platform_pad_present(void) {
	for (int i = 0; i < 4; ++i) if (pads[i]) return true;
	return false;
}

static void layout_canvas(void) {
	int sx = P.screen_w / CORE_W, sy = P.screen_h / CORE_H;
	P.scale = sx < sy ? sx : sy;
	if (P.scale < 1) P.scale = 1;
	if (touch_shown()) P.scale = touch_fit_scale(P.screen_w, P.screen_h, P.scale);
	P.w = P.screen_w / P.scale;
	P.h = P.screen_h / P.scale;
	if (P.w < CORE_W) P.w = CORE_W;
	if (P.h < CORE_H) P.h = CORE_H;
	P.core_x = (P.w - CORE_W) / 2;
	P.core_y = (P.h - CORE_H) / 2;
	/* on a tall screen the touch controls take the room under the picture */
	int top = touch_shown() ? touch_picture_top(P.w, P.h) : -1;
	if (top >= 0) P.core_y = top;
	touch_relayout();
	if (P.canvas) SDL_DestroyTexture(P.canvas);
	if (P.fx_copy) SDL_DestroyTexture(P.fx_copy);
	P.canvas = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, P.w, P.h);
	SDL_SetTextureScaleMode(P.canvas, SDL_ScaleModeNearest);
	P.fx_copy = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, P.w, P.h);
	SDL_SetTextureScaleMode(P.fx_copy, SDL_ScaleModeNearest);
}

/* The window's new size (resized, or in or out of fullscreen): the canvas
 * follows at the largest whole scale. */
static void resized(void) {
	if (P.forced) return;
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

bool platform_init(int force_w, int force_h, bool headless, bool fullscreen) {
	P.headless = headless;
	keys_default();
	if (headless) {
		SDL_SetHint(SDL_HINT_VIDEODRIVER, "dummy");
		SDL_SetHint(SDL_HINT_AUDIODRIVER, "dummy");
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
#ifdef __ANDROID__
	/* a phone turns (the touch controls go under the picture or beside it),
	 * and Back is Escape (the quit prompt), not the end of the app */
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown");
	SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
	/* (a phone's tilt is no controller) */
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif
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
#ifdef __ANDROID__
	/* (SDL locks a window that cannot resize to the way the phone is held
	 * at the start, whatever the hint allows) */
	flags |= SDL_WINDOW_RESIZABLE;
#endif
	P.window = SDL_CreateWindow("Mega Man Battle Network: Cyberworld Endless",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ww, wh, flags);
	if (!P.window) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); return false; }
#ifdef CW_DESKTOP
	SDL_Surface *icon = SDL_CreateRGBSurfaceWithFormatFrom((void *)app_icon_rgba, APP_ICON_SIZE, APP_ICON_SIZE, 32,
		APP_ICON_SIZE * 4, SDL_PIXELFORMAT_RGBA32);
	if (icon) { SDL_SetWindowIcon(P.window, icon); SDL_FreeSurface(icon); }
#endif
	Uint32 rflags = headless ? SDL_RENDERER_SOFTWARE : (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	P.renderer = SDL_CreateRenderer(P.window, -1, rflags);
	if (!P.renderer) P.renderer = SDL_CreateRenderer(P.window, -1, SDL_RENDERER_SOFTWARE);
	if (!P.renderer) { fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError()); return false; }
	SDL_GetRendererOutputSize(P.renderer, &P.screen_w, &P.screen_h);
	if (force_w && force_h) { P.screen_w = force_w; P.screen_h = force_h; }
	layout_canvas();
	SDL_ShowCursor(P.fullscreen || headless ? SDL_DISABLE : SDL_ENABLE);
	open_pads();
#ifdef __ANDROID__
	/* the buttons from the start, until a controller's first press (a
	 * handheld's own controls: its first START); a keyboard with arrow keys
	 * counts as a controller here, so its presence decides nothing */
	if (touch_show(true)) layout_canvas();
	SDL_Log("screen %dx%d, canvas %dx%d at %dx, touch controls %s", P.screen_w, P.screen_h, P.w, P.h, P.scale, touch_shown() ? "shown" : "hidden");
	for (int i = 0; i < SDL_NumJoysticks(); ++i) SDL_Log("controller %d: %s%s", i, SDL_JoystickNameForIndex(i), SDL_IsGameController(i) ? " (a gamepad)" : "");
#endif
	SDL_RendererInfo info;
	SDL_GetRendererInfo(P.renderer, &info);
	printf("display %dx%d renderer %s canvas %dx%d at %dx\n", P.screen_w, P.screen_h, info.name, P.w, P.h, P.scale);
	return true;
}

void platform_shutdown(void) {
	for (int i = 0; i < 4; ++i) if (pads[i]) SDL_GameControllerClose(pads[i]);
	if (P.canvas) SDL_DestroyTexture(P.canvas);
	if (P.fx_copy) SDL_DestroyTexture(P.fx_copy);
	if (P.renderer) SDL_DestroyRenderer(P.renderer);
	if (P.window) SDL_DestroyWindow(P.window);
	SDL_Quit();
}

/* The keyboard, after Capcom's own PC layout for Battle Network (the Legacy
 * Collection): WASD moves, J and K are A and B, Q and E are L and R, Enter
 * is Start and R is Select. The arrows with X and Z also work, as on most
 * GBA emulators. Keys are positions (scancodes), not letters, so an AZERTY
 * keyboard moves with ZQSD. keys.ini in the data folder changes them. */
static const struct { const char *name; uint32_t bit; const char *keys; } key_defaults[] = {
	{ "UP", BTN_UP, "W, Up" },
	{ "DOWN", BTN_DOWN, "S, Down" },
	{ "LEFT", BTN_LEFT, "A, Left" },
	{ "RIGHT", BTN_RIGHT, "D, Right" },
	{ "A", BTN_A, "J, X" },
	{ "B", BTN_B, "K, Z" },
	{ "L", BTN_L, "Q" },
	{ "R", BTN_R, "E" },
	{ "START", BTN_START, "Return, Keypad Enter" },
	{ "SELECT", BTN_SELECT, "R, Backspace" },
};
static uint32_t key_map[SDL_NUM_SCANCODES];

/* "J, X" -> the button on each key; false and a message for an unknown name */
static bool bind_keys(uint32_t bit, const char *list, const char *where) {
	char buf[256];
	snprintf(buf, sizeof buf, "%s", list);
	bool ok = true;
	char *save = NULL;
	for (char *t = strtok_r(buf, ",", &save); t; t = strtok_r(NULL, ",", &save)) {
		while (*t == ' ' || *t == '\t') ++t;
		char *e = t + strlen(t);
		while (e > t && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r' || e[-1] == '\n')) *--e = 0;
		if (!*t) continue;
		SDL_Scancode sc = SDL_GetScancodeFromName(t);
		if (sc == SDL_SCANCODE_UNKNOWN || sc == SDL_SCANCODE_ESCAPE || sc == SDL_SCANCODE_F11) {
			fprintf(stderr, "%s: no key called \"%s\"%s\n", where, t,
				sc == SDL_SCANCODE_UNKNOWN ? "" : " (Escape and F11 are taken)");
			ok = false;
			continue;
		}
		key_map[sc] |= bit;
	}
	return ok;
}

static void keys_default(void) {
	memset(key_map, 0, sizeof key_map);
	for (size_t i = 0; i < sizeof key_defaults / sizeof *key_defaults; ++i)
		bind_keys(key_defaults[i].bit, key_defaults[i].keys, "defaults");
}

void platform_load_keys(const char *path) {
	keys_default();
	FILE *f = fopen(path, "r");
	if (!f) {
		f = fopen(path, "w");
		if (!f) return;
		fprintf(f,
			"# Cyberworld Endless: the keyboard. Each line gives a Game Boy Advance\n"
			"# button its keys, separated by commas. Keys are named as on a US\n"
			"# keyboard (A-Z, 0-9, Up, Down, Left, Right, Space, Return, Backspace, Tab,\n"
			"# Left Shift, Right Shift, Left Ctrl, Keypad 8, Keypad Enter...) and mean\n"
			"# that position: on an AZERTY keyboard W is the key marked Z. Escape (quit)\n"
			"# and F11 (fullscreen) are taken. Delete this file for the defaults.\n\n");
		for (size_t i = 0; i < sizeof key_defaults / sizeof *key_defaults; ++i)
			fprintf(f, "%-6s = %s\n", key_defaults[i].name, key_defaults[i].keys);
		fclose(f);
		platform_persist();
		return;
	}
	char line[256];
	int n = 0;
	while (fgets(line, sizeof line, f)) {
		++n;
		char *eq = strchr(line, '=');
		char *hash = strchr(line, '#');
		if (hash && (!eq || hash < eq)) continue;
		if (!eq) continue;
		*eq = 0;
		char name[16] = "";
		sscanf(line, " %15s", name);
		size_t i = 0;
		while (i < sizeof key_defaults / sizeof *key_defaults && SDL_strcasecmp(name, key_defaults[i].name)) ++i;
		char where[600];
		snprintf(where, sizeof where, "%s:%d", path, n);
		if (i == sizeof key_defaults / sizeof *key_defaults) { fprintf(stderr, "%s: no button called \"%s\"\n", where, name); continue; }
		/* the file's keys replace the defaults for this button */
		for (int sc = 0; sc < SDL_NUM_SCANCODES; ++sc) key_map[sc] &= ~key_defaults[i].bit;
		bind_keys(key_defaults[i].bit, eq + 1, where);
	}
	fclose(f);
}

static uint32_t key_button(SDL_Scancode sc) {
	return (unsigned)sc < SDL_NUM_SCANCODES ? key_map[sc] : 0;
}

static uint32_t pad_button(Uint8 b) {
	switch (b) {
	case SDL_CONTROLLER_BUTTON_DPAD_UP: return BTN_UP;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return BTN_DOWN;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return BTN_LEFT;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return BTN_RIGHT;
	case SDL_CONTROLLER_BUTTON_A: return BTN_A;
	case SDL_CONTROLLER_BUTTON_B: return BTN_B;
	case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return BTN_L;
	case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return BTN_R;
	case SDL_CONTROLLER_BUTTON_START: return BTN_START;
	case SDL_CONTROLLER_BUTTON_BACK: return BTN_SELECT;
	default: return 0;
	}
}

static uint32_t stick_bits(void) {
	uint32_t bits = 0;
	for (int i = 0; i < 4; ++i) {
		if (!pads[i]) continue;
		int x = SDL_GameControllerGetAxis(pads[i], SDL_CONTROLLER_AXIS_LEFTX);
		int y = SDL_GameControllerGetAxis(pads[i], SDL_CONTROLLER_AXIS_LEFTY);
		if (x < -16000) bits |= BTN_LEFT;
		if (x > 16000) bits |= BTN_RIGHT;
		if (y < -16000) bits |= BTN_UP;
		if (y > 16000) bits |= BTN_DOWN;
		if (SDL_GameControllerGetAxis(pads[i], SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 16000) bits |= BTN_L;
		if (SDL_GameControllerGetAxis(pads[i], SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000) bits |= BTN_R;
	}
	return bits;
}

void platform_inject(uint32_t buttons) { injected = buttons; }

void platform_poll(void) {
#ifdef __EMSCRIPTEN__
	follow_page();
#endif
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT: P.quit = true; break;
		case SDL_FINGERDOWN:
		case SDL_FINGERMOTION:
		case SDL_FINGERUP:
			if (touch_event(&e)) layout_canvas();
			break;
		case SDL_KEYDOWN: {
			uint32_t b = key_button(e.key.keysym.scancode);
			/* Alt+Enter is fullscreen, not Start */
			if (e.key.keysym.mod & KMOD_ALT) b &= ~BTN_START;
			if (!e.key.repeat) { key_bits |= b; tapped |= b; if (b) P.keyboard_last = true; }
			/* the keyboard's hands put the touch controls away */
			if (b && !e.key.repeat && touch_show(false)) layout_canvas();
#ifndef __EMSCRIPTEN__
			/* (in a browser the page keeps Escape and fullscreen; Android's
			 * Back is Escape) */
			if ((e.key.keysym.scancode == SDL_SCANCODE_ESCAPE || e.key.keysym.scancode == SDL_SCANCODE_AC_BACK) && !e.key.repeat) {
				/* the first Escape asks, the second within two seconds quits */
				if (P.quit_prompt > 0) P.quit = true;
				else { P.quit_prompt = 120; P.quit_pad = false; }
			}
			/* F11 or Alt+Enter: fullscreen and back */
			if (!e.key.repeat && !P.headless && (e.key.keysym.sym == SDLK_F11 ||
				(e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_ALT))))
				set_fullscreen(!P.fullscreen);
#endif
			break;
		}
		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) resized();
			/* keys let go of in another window would stay held */
			if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) key_bits = 0;
			break;
		case SDL_KEYUP: key_bits &= ~key_button(e.key.keysym.scancode); break;
		case SDL_CONTROLLERBUTTONDOWN:
			pad_bits |= pad_button(e.cbutton.button);
			tapped |= pad_button(e.cbutton.button);
			P.keyboard_last = false;
			if (touch_show(false)) layout_canvas();
			break;
		case SDL_CONTROLLERBUTTONUP: pad_bits &= ~pad_button(e.cbutton.button); break;
		case SDL_CONTROLLERDEVICEADDED: open_pads(); break;
		default: break;
		}
	}
	if (P.quit_prompt > 0) --P.quit_prompt;
#ifndef __EMSCRIPTEN__
	/* a controller's Escape: SELECT and START held a second asks, and
	 * held again while it asks quits (a handheld's Steam Deck or a pad on
	 * the couch has no keyboard; PortMaster's own hotkey is the same pair) */
	static int pair_held;
	if ((pad_bits & (BTN_SELECT | BTN_START)) == (BTN_SELECT | BTN_START)) {
		if (++pair_held == 60) {
			if (P.quit_prompt > 0 && P.quit_pad) P.quit = true;
			else { P.quit_prompt = 180; P.quit_pad = true; }
		}
	} else pair_held = 0;
#endif
	tapped |= touch_taken();
	uint32_t now = key_bits | pad_bits | stick_bits() | injected | tapped | touch_held();
	tapped = 0;
	P.pressed = now & ~P.held;
	P.released = P.held & ~now;
	P.held = now;
	P.repeat = P.pressed;
	for (int i = 0; i < 10; ++i) {
		uint32_t b = 1u << i;
		if (!(now & b)) { P.repeat_timer[i] = 0; continue; }
		int t = ++P.repeat_timer[i];
		if (t > 18 && (t - 18) % 5 == 0) P.repeat |= b;
	}
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

void platform_end_frame(void) {
	SDL_SetRenderTarget(P.renderer, NULL);
	SDL_SetRenderDrawColor(P.renderer, 0, 0, 0, 255);
	SDL_RenderClear(P.renderer);
	SDL_Rect dst = { (P.screen_w - P.w * P.scale) / 2, (P.screen_h - P.h * P.scale) / 2, P.w * P.scale, P.h * P.scale };
	SDL_RenderCopy(P.renderer, P.canvas, NULL, &dst);
	SDL_RenderPresent(P.renderer);
	++P.frame;
	{ extern uint64_t audio_log_frame; audio_log_frame = P.frame; }
}

void platform_persist(void) {
#ifdef __EMSCRIPTEN__
	/* the browser's files live in memory until they are synced to IndexedDB */
	emscripten_run_script("if (typeof Module.persist === 'function') Module.persist();");
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
