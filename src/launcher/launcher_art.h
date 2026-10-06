/* The launcher's pictures (launcher_art.c), drawn here in the colours of
 * Gregar's PET (second_frame.h, tools/site_art.py): a cartridge's shell,
 * its label in its game's colours with the game's own face read from its
 * ROM as the screen is drawn (never kept in a file), the open spot a
 * cartridge goes in, the buttons and the small plates on them. */
#ifndef CW_LAUNCHER_ART_H
#define CW_LAUNCHER_ART_H

#include <SDL.h>
#include <stdbool.h>

/* A cartridge's two sizes, in canvas pixels */
#define CART_W 92
#define CART_H 58
#define CART_LARGE_W 112
#define CART_LARGE_H 72

/* Cartridge `slot` (launcher_text.h's SLOT_*) in `r`, CART_W x CART_H or
 * the large size; `flash` (0-255) a white over its label, just in */
void art_cartridge(int slot, SDL_Rect r, bool large, int flash);
/* An open spot of a cartridge's size: a recess with its outline in dashes,
 * marching by `phase`, gold where the cursor is (`lit`) */
void art_open_spot(SDL_Rect r, int phase, bool lit);
/* A plus 9 x 9 round (cx, cy) */
void art_plus(int cx, int cy, SDL_Color c);
/* The cursor's frame round `r`, its light coming and going with `t` */
void art_cursor(SDL_Rect r, int t);
/* A button: PLAY's gold plate (`gold`) or a slot; `lit` the cursor's,
 * `off` one that cannot be pressed yet, `icon` a play arrow after its word */
void art_button(SDL_Rect r, const char *label, bool gold, bool lit, bool off, bool icon);
/* A plate with a word on it (READY, NEEDED): `check` a tick before it;
 * its width */
int art_chip_width(const char *word, bool check);
void art_chip(int x, int y, const char *word, SDL_Color fill, SDL_Color ink, bool check);
#define CHIP_H 11

#endif
