/* The PET's panel the settings' screens share (the controls screen,
 * Options, the statistics' question): its colours, and the paused game
 * dimmed under its frame. */
#ifndef CW_PET_PANEL_H
#define CW_PET_PANEL_H

#include <SDL.h>

#include "gfx.h"
#include "platform.h"

static const SDL_Color PET_GOLD = { 255, 230, 90, 255 }, PET_SKY = { 170, 200, 255, 255 }, PET_ORANGE = { 255, 170, 40, 255 },
	PET_DIM = { 120, 140, 170, 255 }, PET_NAVY = { 16, 60, 90, 255 }, PET_CYAN = { 66, 198, 231, 255 };

/* the paused game dimmed, the panel's frame over the picture at (x0, y0) */
static inline void pet_panel_draw(int x0, int y0) {
	fill_rect(0, 0, P.w, P.h, rgba(0, 0, 0, 110));
	fill_rect(x0 + 4, y0 + 4, CORE_W - 8, CORE_H - 8, PET_CYAN);
	fill_rect(x0 + 6, y0 + 6, CORE_W - 12, CORE_H - 12, rgba(16, 60, 90, 250));
}

#endif
