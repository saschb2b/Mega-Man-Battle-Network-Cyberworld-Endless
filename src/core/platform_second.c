/* The second screen's part of platform.h (issue #9; its panels, src/second/):
 * drawn into memory where it is shown, the 3DS's bottom screen and
 * Android's second display, and for --second-shot on any build. */
#include "platform_second.h"

#include <string.h>

#include "gfx.h"
#include "platform.h"
#include "present_3ds.h"
#include "second_android.h"

/* (its draws' time for the frame log: platform_second_parts) */
static uint64_t part_second;
static int part_seconds;

int platform_second_parts(uint64_t *ticks) {
	int n = part_seconds;
	*ticks = part_second;
	part_second = 0;
	part_seconds = 0;
	return n;
}

static SecondScreen second;
/* (the 3DS's bottom screen draws into the same memory each time, which
 * the GPU copies from and leaves as it was; Android's is turned to Java's
 * byte order in place, a shot's is new) */
static bool drawing_kept;

bool platform_second_screen_kept(void) { return drawing_kept; }

#if defined(__3DS__) || defined(__ANDROID__)
static SecondChanged second_changed;
static bool second_soon;   /* a draw at the next frame (platform_second_screen_soon) */
static uint64_t drawn_at;  /* the frame of the last draw */

void platform_second_screen_soon(void) { second_soon = true; }

/* Whether a draw is due at this frame: asked for, or the pace's frame and
 * a picture that changed since the last (or a second old: Android makes
 * its display's window again after the screen was off). A battle's or the
 * folder's panel changes as rarely as the game's state does, and its draw
 * then costs nothing, nor the copy to the screen. */
static bool second_due(unsigned pace) {
	if (second_soon) return true;
	if (P.frame % pace) return false;
	return !second_changed || second_changed() || P.frame - drawn_at >= 60;
}
#else
void platform_second_screen_soon(void) {}
#endif

void platform_second_screen(SecondScreen draw, SecondChanged changed) {
	second = draw;
#if defined(__3DS__) || defined(__ANDROID__)
	second_changed = changed;
#else
	(void)changed;
#endif
	/* (none: black at once, the scene that drew it gone; the title keeps
	 * it dark, where the 3DS had kept the run's last map) */
	if (draw) return;
#if defined(__3DS__)
	present3ds_bottom_show(false);
#elif defined(__ANDROID__)
	second_android_dark();
#endif
}

/* The second screen drawn into memory, w x h (RGBA8888, `pitch` bytes a
 * row): whether it holds a picture. (Through the software renderer, and
 * read back, the map took 20 ms on a New 3DS, a frame lost every redraw.) */
static bool draw_second(uint32_t *px, int w, int h, int pitch) {
	if (!second) return false;
	gfx_draw_into(px, w, h, pitch);
	bool drew = second(w, h);
	gfx_draw_into(NULL, 0, 0, 0);
	return drew;
}

/* (a shot's size: the 3DS's, or an Android display's, --second-size) */
#define SHOT_MAX_W (SECOND_W * 2)
#define SHOT_MAX_H (SECOND_H * 2)
static int shot_w = SECOND_W, shot_h = SECOND_H;

void platform_second_shot_size(int w, int h) {
	shot_w = w < SECOND_W ? SECOND_W : w > SHOT_MAX_W ? SHOT_MAX_W : w;
	shot_h = h < SECOND_H ? SECOND_H : h > SHOT_MAX_H ? SHOT_MAX_H : h;
}

bool platform_save_second_screen(const char *path) {
	static uint32_t px[SHOT_MAX_W * SHOT_MAX_H];
	memset(px, 0, sizeof px);
	if (!draw_second(px, shot_w, shot_h, shot_w * 4)) return false;
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(px, shot_w, shot_h, 32, shot_w * 4, SDL_PIXELFORMAT_RGBA8888);
	bool ok = s && SDL_SaveBMP(s, path) == 0;
	if (s) SDL_FreeSurface(s);
	return ok;
}

/* The bottom screen's picture, every tenth frame (the pace MegaMan's mark
 * on it pulses), drawn straight into the memory the GPU copies from; the
 * scene calls it as its update begins, where the GBA's frame still runs on
 * its own core and this one would wait for it anyway (drawn with the
 * present, it made that frame late, 2 ms of 3DS time six times a second) */
void platform_second_screen_draw(void) {
#if defined(__3DS__)
	if (!second_due(10)) return;
	second_soon = false;
	drawn_at = P.frame;
	int pitch;
	static uint32_t *kept;
	uint32_t *px = present3ds_bottom(&pitch);
	uint64_t t0 = SDL_GetPerformanceCounter();
	drawing_kept = px && px == kept;
	bool on = px && draw_second(px, SECOND_W, SECOND_H, pitch);
	kept = on ? px : NULL;
	drawing_kept = false;
	part_second += SDL_GetPerformanceCounter() - t0;
	++part_seconds;
	present3ds_bottom_show(on);
#elif defined(__ANDROID__)
	/* (a display beside the game's, the AYN Thor's lower screen: every
	 * fifth frame, so MegaMan's mark keeps up with his walk, at the size
	 * picked for the display, handed to Java to show; drawn in 0.15 ms and
	 * handed over in 0.1 in the emulator, on a desktop's core) */
	if (!second_due(5)) return;
	second_soon = false;
	drawn_at = P.frame;
	int w, h;
	uint32_t *px = second_android_begin(&w, &h);
	if (!px) return;
	uint64_t t0 = SDL_GetPerformanceCounter();
	second_android_end(draw_second(px, w, h, w * 4));
	part_second += SDL_GetPerformanceCounter() - t0;
	++part_seconds;
#endif
}
