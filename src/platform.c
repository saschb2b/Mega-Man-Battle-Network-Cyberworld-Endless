#include "platform.h"

#include <stdio.h>
#include <string.h>

Platform P;

static SDL_GameController *pads[4];
static uint32_t injected;
static uint32_t pad_bits, key_bits;

static void open_pads(void) {
	for (int i = 0; i < SDL_NumJoysticks() && i < 4; ++i) {
		if (!pads[i] && SDL_IsGameController(i)) pads[i] = SDL_GameControllerOpen(i);
	}
}

static void layout_canvas(void) {
	int sx = P.screen_w / CORE_W, sy = P.screen_h / CORE_H;
	P.scale = sx < sy ? sx : sy;
	if (P.scale < 1) P.scale = 1;
	P.w = P.screen_w / P.scale;
	P.h = P.screen_h / P.scale;
	if (P.w < CORE_W) P.w = CORE_W;
	if (P.h < CORE_H) P.h = CORE_H;
	P.core_x = (P.w - CORE_W) / 2;
	P.core_y = (P.h - CORE_H) / 2;
	if (P.canvas) SDL_DestroyTexture(P.canvas);
	if (P.fx_copy) SDL_DestroyTexture(P.fx_copy);
	P.canvas = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, P.w, P.h);
	SDL_SetTextureScaleMode(P.canvas, SDL_ScaleModeNearest);
	P.fx_copy = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, P.w, P.h);
	SDL_SetTextureScaleMode(P.fx_copy, SDL_ScaleModeNearest);
}

bool platform_init(int force_w, int force_h, bool headless) {
	P.headless = headless;
	if (headless) {
		SDL_SetHint(SDL_HINT_VIDEODRIVER, "dummy");
		SDL_SetHint(SDL_HINT_AUDIODRIVER, "dummy");
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0) {
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
			fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
			return false;
		}
	}
	Uint32 flags = SDL_WINDOW_SHOWN;
	int ww = force_w, wh = force_h;
	if (!ww || !wh) {
		SDL_DisplayMode dm;
		if (SDL_GetDesktopDisplayMode(0, &dm) == 0) { ww = dm.w; wh = dm.h; }
		else { ww = CORE_W * 4; wh = CORE_H * 4; }
		if (!headless) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	P.window = SDL_CreateWindow("Mega Man Battle Network: Cyberworld Endless",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ww, wh, flags);
	if (!P.window) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); return false; }
	Uint32 rflags = headless ? SDL_RENDERER_SOFTWARE : (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	P.renderer = SDL_CreateRenderer(P.window, -1, rflags);
	if (!P.renderer) P.renderer = SDL_CreateRenderer(P.window, -1, SDL_RENDERER_SOFTWARE);
	if (!P.renderer) { fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError()); return false; }
	SDL_GetRendererOutputSize(P.renderer, &P.screen_w, &P.screen_h);
	if (force_w && force_h) { P.screen_w = force_w; P.screen_h = force_h; }
	layout_canvas();
	SDL_ShowCursor(SDL_DISABLE);
	open_pads();
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

static uint32_t key_button(SDL_Keycode k) {
	switch (k) {
	case SDLK_UP: return BTN_UP;
	case SDLK_DOWN: return BTN_DOWN;
	case SDLK_LEFT: return BTN_LEFT;
	case SDLK_RIGHT: return BTN_RIGHT;
	case SDLK_x: case SDLK_SPACE: return BTN_A;
	case SDLK_z: case SDLK_BACKSPACE: return BTN_B;
	case SDLK_a: case SDLK_q: return BTN_L;
	case SDLK_s: case SDLK_w: return BTN_R;
	case SDLK_RETURN: return BTN_START;
	case SDLK_RSHIFT: case SDLK_TAB: return BTN_SELECT;
	default: return 0;
	}
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
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT: P.quit = true; break;
		case SDL_KEYDOWN:
			if (!e.key.repeat) { key_bits |= key_button(e.key.keysym.sym); P.keyboard_last = true; }
			if (e.key.keysym.sym == SDLK_ESCAPE) P.quit = true;
			break;
		case SDL_KEYUP: key_bits &= ~key_button(e.key.keysym.sym); break;
		case SDL_CONTROLLERBUTTONDOWN: pad_bits |= pad_button(e.cbutton.button); P.keyboard_last = false; break;
		case SDL_CONTROLLERBUTTONUP: pad_bits &= ~pad_button(e.cbutton.button); break;
		case SDL_CONTROLLERDEVICEADDED: open_pads(); break;
		default: break;
		}
	}
	uint32_t now = key_bits | pad_bits | stick_bits() | injected;
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
