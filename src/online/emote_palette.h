/* The Net Square's palette on the second screen (issue #178; docs/ONLINE.md,
 * Emotes): on the 3DS's bottom screen and Android's second display, every
 * emote and phrase as a button of the PET's, one tap each, as the DS games
 * kept their touch screen for the PET; the favourites on L marked with
 * their way, the aimed emotes' target named. Its tab beside it shows the
 * NetPass, the panel the Square had before. The top screen stays BN6's.
 * This is its course and where its buttons lie; second_palette.c draws it. */
#ifndef CW_EMOTE_PALETTE_H
#define CW_EMOTE_PALETTE_H

#include <stdbool.h>

#include "gfx.h"
#include "online_types.h"

typedef struct {
	int tab;                           /* 0 the palette, 1 the NetPass */
	int flash;                         /* an item just tapped (square_emote.h's items), -1 none */
	bool spent;                        /* ... the burst spent: it went nowhere */
	int favourites[4];                 /* the items on L and up, right, down, left */
	char aim[ONLINE_NAME_BYTES + 1];   /* the MegaMan faced, "" none: Wave, Cheer and Bow go to him */
} PaletteView;

/* As MegaMan arrives in the Square: the palette's tab, nothing lit. */
void emote_palette_reset(void);
/* Once a frame in the Square, `s` as shown: a tap on the second screen
 * taken (a tab, or an item sent as the strip sends it). */
void emote_palette_frame(const OnlineSnapshot *s);
/* What the second screen draws (second.c reads it once a frame). */
void emote_palette_view(PaletteView *v);

/* Where its parts lie in a second screen of w x h: item `item`, a tab */
SDL_Rect emote_palette_item(int w, int h, int item);
SDL_Rect emote_palette_tab(int w, int h, int tab);

#endif
