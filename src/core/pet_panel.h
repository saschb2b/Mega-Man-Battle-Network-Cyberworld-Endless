/* The PET's panel the settings' screens share (the controls screen,
 * Options, the statistics' question): its colours, and the paused game
 * dimmed under BN6's screen frame (pet_look.h) with its title. */
#ifndef CW_PET_PANEL_H
#define CW_PET_PANEL_H

#include <SDL.h>

#include "gfx.h"
#include "minifont.h"
#include "pet_look.h"
#include "platform.h"

static const SDL_Color PET_GOLD = { 255, 230, 90, 255 }, PET_SKY = { 170, 200, 255, 255 }, PET_ORANGE = { 255, 170, 40, 255 },
	PET_DIM = { 120, 140, 170, 255 }, PET_NAVY = { 16, 60, 90, 255 }, PET_CYAN = { 66, 198, 231, 255 };

/* the paused game dimmed, the screen's frame over the picture at (x0, y0), `title` on its band */
static inline void pet_panel_draw(int x0, int y0, const char *title) {
	fill_rect(0, 0, P.w, P.h, rgba(0, 0, 0, 110));
	look_screen(x0, y0, CORE_W, CORE_H, title);
}

/* a line of small letters on the screen's body, its shade under it, centred on cx */
static inline void pet_panel_note(int cx, int y, const char *s, SDL_Color c) {
	int x = cx - minifont_width(s, 1) / 2;
	minifont_draw(x + 1, y + 1, s, LOOK_GREEN_LO, 1);
	minifont_draw(x, y, s, c, 1);
}

#endif
