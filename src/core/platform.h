/* Window, logical canvas, input and timing. */
#ifndef PLATFORM_H
#define PLATFORM_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

/* The battle view is the GBA's 240x160. The canvas grows to fill the
 * screen at the largest whole-number scale that still fits that view:
 * 1280x960 -> 256x192 at 5x, 1920x1080 -> 320x180 at 6x. */
#define CORE_W 240
#define CORE_H 160

enum {
	BTN_UP = 1 << 0,
	BTN_DOWN = 1 << 1,
	BTN_LEFT = 1 << 2,
	BTN_RIGHT = 1 << 3,
	BTN_A = 1 << 4,      /* chip / confirm */
	BTN_B = 1 << 5,      /* buster / back */
	BTN_L = 1 << 6,
	BTN_R = 1 << 7,
	BTN_START = 1 << 8,
	BTN_SELECT = 1 << 9,
};

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *canvas;
	int screen_w, screen_h;
	int w, h;          /* logical canvas */
	int scale;
	int core_x, core_y; /* top-left of the centered 240x160 core */
	uint32_t held, pressed, released, repeat;
	int repeat_timer[16];
	bool quit;
	bool headless;
	bool forced;       /* a fixed size (--size, headless): the window never relays out */
	bool fullscreen;
	uint64_t frame;
	bool keyboard_last; /* last input came from the keyboard */
	/* Full-screen effects, set by the scene each frame while drawing and
	 * applied to the canvas before it is shown (the GBA's MOSAIC register and
	 * palette fades). fade is 0..16 toward fade_color. */
	int fx_mosaic;
	int fx_fade;
	SDL_Color fx_fade_color;
	SDL_Texture *fx_copy;
} Platform;

extern Platform P;

/* A fullscreen window at the desktop's size, or a resizable one at the
 * largest whole scale that fits; F11 or Alt+Enter switch between them. */
bool platform_init(int force_w, int force_h, bool headless, bool fullscreen);
void platform_shutdown(void);
void platform_poll(void);
void platform_begin_frame(void);
void platform_end_frame(void);
/* Apply fx_mosaic and fx_fade to the canvas (called once the scene has drawn). */
void platform_apply_effects(void);
bool platform_save_canvas(const char *path);
/* Files were written: in a browser, keep them (IndexedDB); elsewhere nothing. */
void platform_persist(void);
/* Inject buttons for scripted tests; merged with real input. */
void platform_inject(uint32_t buttons);

static inline bool btn_pressed(uint32_t b) { return (P.pressed & b) != 0; }
static inline bool btn_held(uint32_t b) { return (P.held & b) != 0; }
static inline bool btn_released(uint32_t b) { return (P.released & b) != 0; }
/* Pressed, or held long enough to auto-repeat (menus). */
static inline bool btn_repeat(uint32_t b) { return (P.repeat & b) != 0; }

#endif
