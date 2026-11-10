/* The Library's picture (library.h), in BN6's PET screens' look
 * (pet_look.h), laid out as BN6's own Library to the pixel: the chip
 * under the cursor as BN6's card on the left (its face in its class's
 * colour, a Mega's cyan and a Giga's pink, its description in the game's
 * own three lines), the page's slots in the window on the right, seven
 * rows, its class between the page arrows and its count in a pill; the
 * entries as tags on the band, the one shown lit with its count; and
 * where each part a tap takes lies. */
#ifndef CW_LIBRARY_DRAW_H
#define CW_LIBRARY_DRAW_H

#include <SDL.h>
#include <stdbool.h>

#include "library_pages.h"

#define LIBRARY_ROWS 7   /* slots shown at once, as BN6's */

/* what the picture shows, made as the page turns */
typedef struct {
	int t;
	int entry, cls;                   /* the page: LIB_BN6 .., LIB_STD .. */
	LibSlot rows[LIB_SLOTS_MOST];     /* its slots listed: every one of a StdChip page, the filled of the others (as BN6's) */
	bool seen[LIB_SLOTS_MOST];        /* ... each filled or not */
	int n, at, top;                   /* their count, the cursor, the first shown */
	int held, of;                     /* the page's count */
	int entry_held, entry_of;         /* the entry's, its three classes together */
} LibraryView;

void library_draw_view(int x, int y, const LibraryView *v);

/* BN6's Library card, 80 x 134 from (x, y), its face in class `cls`'s
 * colour (LIB_STD ..): chip `chip`'s picture, element, power and
 * description, or (0) a blank one, its head dark (the reward pick's,
 * reward_pick_draw.c, holds a program or an item there) */
#define LIBRARY_CARD_W 80
#define LIBRARY_CARD_H 134
void library_card(int x, int y, int chip, int cls);
void library_card_blank(int x, int y, int cls);
/* ... its head alone, chip `chip`'s picture, element and power over the blank card */
void library_card_face(int x, int y, int chip, int cls);
/* ... up to three lines (apart by newlines) where the card's description stands */
void library_card_lines(int x, int y, const char *lines);

/* where a tap takes each part, from the picture's top left */
SDL_Rect library_row_rect(int i);         /* row i shown */
SDL_Rect library_turn_rect(int right);    /* the class's arrows */
SDL_Rect library_tab_rect(const LibraryView *v, int entry);   /* an entry's tag on the band */

#endif
