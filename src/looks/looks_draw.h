/* The colours screen's picture (looks_screen.h), in BN6's PET screens' look
 * (pet_look.h; docs/engine/ui.md): the band with the screen's name; STATUS's
 * slot holding BN6's own MegaMan in the look's colours; the list in a
 * window, the look worn between Options' orange pointers and each channel
 * with its colour; the palette's window, its name between the Library's
 * green page arrows, its sixteen colours in a 4x4 grid and the plain cell
 * under them, EQUIP's orange corners on the grid's cursor; KeyItem's
 * INFORMATION box under them. And where each part a tap takes lies. */
#ifndef CW_LOOKS_DRAW_H
#define CW_LOOKS_DRAW_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "looks.h"

#define LOOKS_PLAIN LOOK_COLOURS   /* the grid's cell under its sixteen: the channel as BN6 draws it */
#define LOOKS_ROWS (1 + LOOK_CHANNELS)

/* what the picture shows */
typedef struct {
	int t;
	int slot;                     /* the look worn, being set */
	Look look;                    /* ... as it stands, the grid's cursor on him while in the grid */
	Look before;                  /* ... as it was when the grid was entered */
	int chan[LOOK_CHANNELS];      /* the channels shown (an inert one left out) */
	int n_chan;
	int row;                      /* the list's cursor: 0 the looks, 1 on the channels */
	bool grid;                    /* the cursor in the grid */
	int page;                     /* the palette shown */
	int cell;                     /* the grid's cursor, 0-15 or LOOKS_PLAIN */
	uint8_t preview[32];          /* his battle palette in the look's colours (GBA BGR555) */
} LooksView;

void looks_draw_view(int x, int y, const LooksView *v);

/* where a tap takes each part, from the picture's top left */
SDL_Rect looks_row_rect(int row);      /* a list's row */
SDL_Rect looks_cell_rect(int cell);    /* a grid's cell, the plain one too */
SDL_Rect looks_page_rect(bool right);  /* the palette window's page arrows */

#endif
