/* The emote strip drawn (emote_strip_draw.c; emote_strip.h): a PET screen
 * of the project's own in Gregar's PET colours (second_frame.h), over
 * the bottom of the game's picture, and the preview's window at its top
 * left. Drawing only: it reads the view it is given and changes nothing;
 * where its parts lie is the course's too (its taps). */
#ifndef CW_EMOTE_STRIP_DRAW_H
#define CW_EMOTE_STRIP_DRAW_H

#include <stdbool.h>

#include "gfx.h"
#include "online_types.h"

typedef struct {
	int page;                          /* 0 the emotes, 1 the phrases */
	int cursor;                        /* the item under it (square_emote.h's items) */
	int anim, frame;                   /* the preview: MegaMan's animation (BN6_OW_MEGAMAN's) and its frame */
	int favourites[4];                 /* the items on L and up, right, down, left */
	int kept;                          /* a favourite just kept: its way, -1 none */
	int t;                             /* frames it has been up */
	bool aims;                         /* the item under the cursor can be aimed */
	char aim[ONLINE_NAME_BYTES + 1];   /* ... whom at, "" no one faced */
} StripView;

/* The pages' items: the emotes in two rows of six, the phrases in two
 * columns of four */
#define STRIP_COLS(page) ((page) ? 2 : 6)
#define STRIP_ROWS(page) ((page) ? 4 : 2)
/* Where its parts lie, in the game picture's pixels: item `i` of page
 * `page`, the preview's window, a page's tab, the favourite on way `way` */
SDL_Rect strip_cell(int page, int i);
SDL_Rect strip_preview(void);
SDL_Rect strip_tab(int page);
SDL_Rect strip_favourite(int way);
/* An arrow 5 x 5 pointing way `way` (0 up, 1 right, 2 down, 3 left) from
 * (x, y): a favourite's way of L, here and on the palette */
void strip_arrow(int x, int y, int way, SDL_Color c);
/* The strip, its picture at (x, y) */
void strip_draw(int x, int y, const StripView *v);

#endif
