/* The Library's picture (library.h), in BN6's PET screens' look
 * (pet_look.h), laid out as BN6's own Library: the entries as tabs under
 * the band, the chip under the cursor as its card on the left (its face
 * in its class's colour, as BN6 tints a Mega's and a Giga's), the page's
 * slots in the screen's one window on the right, its class between the
 * page arrows and its count in a pill; and where each part a tap takes
 * lies. */
#ifndef CW_LIBRARY_DRAW_H
#define CW_LIBRARY_DRAW_H

#include <SDL.h>
#include <stdbool.h>

#include "library_pages.h"

#define LIBRARY_ROWS 6   /* slots shown at once */

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

/* where a tap takes each part, from the picture's top left */
SDL_Rect library_row_rect(int i);         /* row i shown */
SDL_Rect library_turn_rect(int right);    /* the class's arrows */
SDL_Rect library_tab_rect(int entry);     /* an entry's tab */

#endif
