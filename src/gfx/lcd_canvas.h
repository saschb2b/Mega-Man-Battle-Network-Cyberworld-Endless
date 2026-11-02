/* A GBA's colours on what the engine draws on the game's canvas (issue
 * #265): with Options' Colour correction on (any of its colours), every
 * colour the engine draws over the core's picture (its PET look's screens,
 * the map, the counts, the title) through the same table as the picture's
 * (lcd_frame.h), so the screen reads as one and Options shows what it
 * does. Not on a screen of its own (platform_own_taps: the ROMs screen,
 * SAVES, the text field), nor the touch controls or the second screen,
 * which are no GBA's and draw on their own. docs/engine/lcd.md */
#ifndef CW_LCD_CANVAS_H
#define CW_LCD_CANVAS_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

/* A count that steps each time the colour the engine's drawing takes
 * changes (the line, or a screen of its own): a texture made from colours
 * at another count is made again */
unsigned lcd_canvas_gen(void);
/* A colour as drawn on the canvas now, its alpha kept */
SDL_Color lcd_canvas_colour(SDL_Color c);
/* ... an ARGB8888 colour (R at bit 16), and `n` pixels of them in place */
uint32_t lcd_canvas_argb(uint32_t c);
void lcd_canvas_pixels(uint32_t *px, int n);

#endif
