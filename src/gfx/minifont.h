/* A 3x5 pixel font of capitals, digits and punctuation, drawn for this
 * engine, for the screens shown before a ROM is found (the game's own font
 * comes from the ROM). Lowercase letters draw as capitals. */
#ifndef CW_MINIFONT_H
#define CW_MINIFONT_H

#include <SDL.h>

/* A character's 3x5 pixels: bit (4 - row) * 3 + (2 - column) set where
 * one is drawn (0 for a character it has none of). */
unsigned short minifont_glyph(char ch);
/* Pixels a string takes across at `scale` (4 a character, less the last gap). */
int minifont_width(const char *s, int scale);
/* Draws `s` with its top-left at (x, y) on the canvas, `scale` canvas
 * pixels to a font pixel. */
void minifont_draw(int x, int y, const char *s, SDL_Color c, int scale);
/* The same, centred on x. */
void minifont_draw_centered(int x, int y, const char *s, SDL_Color c, int scale);

#endif
