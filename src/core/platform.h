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
	float fill;         /* (settings.ini's screen) the canvas shown at this scale to fill the screen, not a whole one; 0 whole */
	SDL_Texture *sharp; /* ... drawn whole-scaled into this first, then smoothly to its size */
	int core_x, core_y; /* top-left of the centered 240x160 core */
	float dp;          /* screen pixels to a dp (a 160th of an inch): the touch controls' sizes */
	uint32_t held, pressed, released, repeat;
	int repeat_timer[16];
	/* The same for a screen that must never lock (the controls screen):
	 * the pads' own buttons (pads_menu_held), never their map, with the
	 * keys, the touch controls and scripted input as above. */
	uint32_t menu_held, menu_pressed, menu_repeat;
	int menu_timer[16];
	bool quit;
	bool headless;
	bool forced;       /* a fixed size (--size, headless): the window never relays out */
	bool fullscreen;
	uint64_t frame;
	bool keyboard_last; /* last input came from the keyboard */
	bool skip_present;  /* this frame is played, not shown: the loop catching up (main.c) */
	bool blend;         /* smooth motion (settings.ini): each refresh a mix of the last two frames */
	bool paused;        /* the browser page's menu open over the game: no frames, no sound (cw_set_paused) */
	int quit_prompt;    /* frames left of "Esc again to quit" after one Escape */
	bool quit_pad;      /* ... opened by a controller's SELECT+START, held */
	bool background;    /* the app sent to the background (SDL_APP_*): on iOS the loop draws nothing then (main.c) */
	int textures_lost;  /* the renderer's resets (SDL_RENDER_DEVICE_RESET): a texture made before the last is gone */
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
/* The keyboard's map for the controls screen: per key (scancode) its GBA
 * buttons (BTN_*). Got, the defaults, a key made a GBA button's only one
 * (padmap_bind's swap: a button left with none takes the keys `gba` had),
 * put in play and written to keys.ini; a GBA button's keys in words ("J,
 * X"; NULL: the map in play). Escape and F11 are no button's. */
typedef struct { uint32_t bits[SDL_NUM_SCANCODES]; } KeyMap;
void platform_keys_get(KeyMap *k);
void platform_keys_default(KeyMap *k);
void platform_keys_bind(KeyMap *k, int gba, int scancode);
/* `scancode` added to GBA button `gba`'s keys (the rest kept: a direction's
 * WASD and arrows), taken from any other button */
void platform_keys_add(KeyMap *k, int gba, int scancode);
void platform_keys_set(const KeyMap *k);
void platform_keys_label(const KeyMap *k, int gba, char *out, size_t n);
bool platform_key_free(int scancode);
/* This frame's keys pressed (scancodes, at most `most`): "press a key"
 * on the controls screen. */
int platform_keys_pressed(int *out, int most);
/* A finger at screen pixel (x, y): to the controls screen while it is
 * open, else the touch controls (platform_poll's, and --taps' in tests). */
void platform_finger(uint32_t type, SDL_FingerID id, float x, float y);
void platform_begin_frame(void);
void platform_end_frame(void);
/* Apply fx_mosaic and fx_fade to the canvas (called once the scene has drawn). */
void platform_apply_effects(void);
bool platform_save_canvas(const char *path);
/* Files were written: in a browser, keep them (IndexedDB); elsewhere nothing. */
void platform_persist(void);
/* settings.ini in the data folder (made with the defaults when missing):
 * smooth_motion = on or off (P.blend); screen = auto, whole or fill (P.fill). */
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
/* A screen that takes the pointer itself (the launcher, src/launcher/): the
 * touch controls put away while it does (and back as they were after),
 * each finger's lift and each left click kept at its canvas pixel for
 * platform_tap, which hands out the last one once, and the canvas laid out
 * as a menu's: the largest whole scale that leaves it 240 x 160 or more, or
 * 160 x 240 on a screen held upright. */
void platform_own_taps(bool on);
bool platform_tap(int *x, int *y);
/* ... where the mouse is over it, or a finger on it: its canvas pixel;
 * false where neither is */
bool platform_pointer(int *x, int *y);
/* ... a file dropped on the window since the last call (a desktop's): its
 * path, once */
bool platform_dropped(char *path, size_t n);
/* Escape or Android's Back asks `back` first while it is set (true: it
 * took the press); NULL for none */
void platform_on_back(bool (*back)(void));
/* What an iPhone's notch, rounded corners and home bar cover of the
 * canvas's edges, in its pixels; 0 elsewhere. */
void platform_safe_edges(int *top, int *left, int *bottom, int *right);
/* The next frame shown saved whole as the player sees it, the touch
 * controls on it (--screen-shot), where the canvas's shots have the game
 * alone. */
void platform_shot_screen(const char *path);
/* The second screen (the 3DS's bottom one, issue #9; on Android a display
 * beside the game's, the AYN Thor's lower screen): `draw` fills w x h every
 * few frames, SECOND_W x SECOND_H on the 3DS and at least that on Android,
 * false for black; NULL for none, which blacks the screen. Elsewhere it is
 * drawn only for --second-shot. */
#define SECOND_W 320
#define SECOND_H 240
typedef bool (*SecondScreen)(int w, int h);
/* ... and `changed`, where given, whether its picture would differ from
 * the last one drawn: where not, none is drawn (but once a second) */
typedef bool (*SecondChanged)(void);
void platform_second_screen(SecondScreen draw, SecondChanged changed);
/* Whether the memory the second screen draws into now still holds the
 * last picture drawn there (the 3DS's bottom screen's): what did not
 * change need not be drawn again */
bool platform_second_screen_kept(void);
/* Draws it where it is shown (the 3DS's bottom screen, Android's second
 * display), every few frames: at a frame's update, before the game is
 * read, where the core's own thread still runs the GBA's frame. */
void platform_second_screen_draw(void);
/* ... at the next frame too, whatever its pace (a new panel, its title's
 * slide). */
void platform_second_screen_soon(void);
/* The second screen's picture now, into a BMP (--second-shot), at the
 * 3DS's size or the one set (--second-size WxH: an Android display's, to
 * 640 x 480) */
bool platform_save_second_screen(const char *path);
void platform_second_shot_size(int w, int h);
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
