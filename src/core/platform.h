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

/* The desktop builds' application ID: the .desktop file's name and the
 * window's class, so a dock matches the window to its pinned icon. */
#define APP_ID "io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless"

#include "buttons.h"

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *canvas;
	int screen_w, screen_h;
	int w, h;          /* logical canvas */
	int scale;
	int core_x, core_y; /* top-left of the centered 240x160 core */
	float dp;          /* screen pixels to a dp (a 160th of an inch): the touch controls' sizes */
	uint32_t held, pressed, released, repeat;
	int repeat_timer[16];
	bool quit;
	bool headless;
	bool forced;       /* a fixed size (--size, headless): the window never relays out */
	bool fullscreen;
	uint64_t frame;
	bool keyboard_last; /* last input came from the keyboard */
	bool skip_present;  /* this frame is played, not shown: the loop catching up (main.c) */
	bool blend;         /* smooth motion (settings.ini): each refresh a mix of the last two frames */
	int quit_prompt;    /* frames left of "Esc again to quit" after one Escape */
	bool quit_pad;      /* ... opened by a controller's SELECT+START, held */
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
/* The keyboard's buttons: the defaults (docs in platform.c), then path
 * (keys.ini in the data folder), which is written with the defaults when
 * it does not exist yet. */
void platform_load_keys(const char *path);
void platform_begin_frame(void);
void platform_end_frame(void);
/* Apply fx_mosaic and fx_fade to the canvas (called once the scene has drawn). */
void platform_apply_effects(void);
bool platform_save_canvas(const char *path);
/* Files were written: in a browser, keep them (IndexedDB); elsewhere nothing. */
void platform_persist(void);
/* settings.ini in the data folder (made with the defaults when missing):
 * smooth_motion = on or off (P.blend). */
void platform_load_settings(const char *path);
/* Smooth motion: the display refreshed with the game's last two frames
 * mixed, `w` (0-1) of the newer; the loop's frames were played unshown. */
void platform_present_blend(double w);
/* --frame-log (or CYBERWORLD_FRAME_LOG): a line a second of the frames'
 * pacing on stdout */
extern bool platform_frame_log;
/* A played frame's update and drawing, in performance-counter ticks, for
 * the frame log's split */
void platform_frame_parts(uint64_t update, uint64_t draw);
/* The canvas as drawn so far on the display at once, before a long wait
 * (no frame counted; none headless). */
void platform_present_now(void);
/* Draws from here onto the canvas as last shown, not cleared: a word over
 * the still picture before a long wait (then platform_present_now). */
void platform_draw_over(void);
/* --dpi: the screen's density taken as given (tests of the touch controls). */
void platform_set_dpi(float dpi);
/* The next frame shown saved whole as the player sees it, the touch
 * controls on it (--screen-shot), where the canvas's shots have the game
 * alone. */
void platform_shot_screen(const char *path);
/* The second screen (the 3DS's bottom one, issue #9): `draw` fills
 * SECOND_W x SECOND_H at a frame's end every few frames, false for black;
 * NULL for none. Elsewhere it is drawn only for --second-shot. */
#define SECOND_W 320
#define SECOND_H 240
typedef bool (*SecondScreen)(int w, int h);
void platform_second_screen(SecondScreen draw);
/* Draws it where it is shown (the 3DS's bottom screen, every few frames):
 * at a frame's update, before the game is read, where the core's own
 * thread still runs the GBA's frame. */
void platform_second_screen_draw(void);
/* The second screen's picture now, into a BMP (--second-shot). */
bool platform_save_second_screen(const char *path);
/* Inject buttons for scripted tests; merged with real input. */
void platform_inject(uint32_t buttons);
/* Whether a game controller is connected (a PC without one is told its keys). */
bool platform_pad_present(void);

static inline bool btn_pressed(uint32_t b) { return (P.pressed & b) != 0; }
static inline bool btn_held(uint32_t b) { return (P.held & b) != 0; }
static inline bool btn_released(uint32_t b) { return (P.released & b) != 0; }
/* Pressed, or held long enough to auto-repeat (menus). */
static inline bool btn_repeat(uint32_t b) { return (P.repeat & b) != 0; }

#endif
