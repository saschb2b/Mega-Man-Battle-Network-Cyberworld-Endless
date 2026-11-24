/* The colours screen (issue #854; docs/game/looks.md): MegaMan's look,
 * opened from the PET's start menu outside battle. A list of the saved
 * look worn (A, B or C, Left and Right between them) and his colour
 * channels; A on a channel goes into the palette's 4x4 grid, where the
 * cursor recolours him live, A picks and B puts the colour back. L and R
 * page through the palettes, a locked one shown with where it comes from.
 * The preview is BN6's own battle sprite in the look's colours; each
 * change is applied to the game at once (look_apply), and B on the list
 * leaves with the look kept. */
#ifndef CW_LOOKS_SCREEN_H
#define CW_LOOKS_SCREEN_H

#include <stdbool.h>
#include <stdio.h>

void looks_screen_open(void);
bool looks_screen_shown(void);
/* The frame's course while it is up, or its key still held after it
 * closed: true while it holds the game */
bool looks_screen_update(void);
/* ... drawn over the game's picture (P.core_x, P.core_y) while it is up */
void looks_screen_draw(void);
/* Put away at once: the game's scene left */
void looks_screen_shut(void);
/* The state in words: `look SLOT armour PALETTE/N suit plain ...` always
 * (the look worn), and while it is up `looks at ROW grid|list page PALETTE
 * cell N`; reads only */
void looks_screen_describe(FILE *f);

#endif
