/* The colours screen's picture (looks_screen.h), in BN6's PET screens' look
 * (pet_look.h; docs/engine/ui.md): the band with the screen's name and
 * L/R:PALETTE; STATUS's slot holding BN6's own MegaMan, his face over his
 * body, in the look's colours (looks_preview.h); the list in a window, the
 * look worn between Options' orange pointers and each channel with its
 * colour; the palette's window, its name between the Library's green page
 * arrows, its sixteen colours in a 4x4 grid, the plain cell under them and
 * a row of pips for the twelve palettes, EQUIP's orange corners on the
 * grid's cursor; ChipFolder's SELECT pop-up for what can be done with a
 * look; KeyItem's INFORMATION box under them. And where each part a tap
 * takes lies. */
#ifndef CW_LOOKS_DRAW_H
#define CW_LOOKS_DRAW_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "looks.h"
#include "looks_preview.h"

#define LOOKS_PLAIN LOOK_COLOURS   /* the grid's cell under its sixteen: the channel as BN6 draws it */
#define LOOKS_POPUP_ROWS LOOK_SLOTS   /* the pop-up's rows: a copy to each other look, all plain */
#define LOOKS_FLASH 18                /* frames a pick's swatch flashes */
enum { LOOKS_TOLD_NONE, LOOKS_TOLD_COPY, LOOKS_TOLD_PLAIN };

/* what the picture shows */
typedef struct {
	int t;
	int slot;                     /* the look worn, being set */
	Look look;                    /* ... as it stands, the grid's cursor on him while in the grid */
	Look before;                  /* ... as it was when the grid was entered */
	int chan[LOOK_CHANNELS];      /* the channels shown */
	int n_chan;
	int row;                      /* the list's cursor: 0 the looks, 1 on the channels */
	bool grid;                    /* the cursor in the grid */
	int page;                     /* the palette shown */
	int cell;                     /* the grid's cursor, 0-15 or LOOKS_PLAIN */
	int popup;                    /* the pop-up's cursor, -1 while it is shut */
	bool asking, yes;             /* ... its question (a look to be replaced) up, and Yes chosen */
	int told, told_from;          /* what the last pop-up did (LOOKS_TOLD_*), until the next press, and from which look */
	int flash, flash_row;         /* a pick's swatch flashing: frames left, its row */
	LooksPreview preview;         /* BN6's MegaMan in the look */
} LooksView;

void looks_draw_view(int x, int y, const LooksView *v);

/* where a tap takes each part, from the picture's top left */
SDL_Rect looks_row_rect(int row);      /* a list's row */
SDL_Rect looks_cell_rect(int cell);    /* a grid's cell, the plain one too */
SDL_Rect looks_page_rect(bool right);  /* the palette window's page arrows */
SDL_Rect looks_popup_rect(int row);    /* the pop-up's row */
SDL_Rect looks_popup_box(void);        /* ... the pop-up whole */
SDL_Rect looks_slot_value_rect(void);  /* the looks' row's letter between its pointers */

#endif
