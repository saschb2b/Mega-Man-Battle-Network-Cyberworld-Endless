/* RetroArch's lcd3x shader over the picture on the screen (issue #265):
 * libretro/glsl-shaders' handheld/lcd3x.glslp (shaders/lcd3x.glsl,
 * Gigaherz's, in the public domain), drawn at the viewport's own pixels as
 * its preset draws it. Each screen pixel is the picture's times its
 * column's red, green and blue factors (each picture pixel's three stripes)
 * and its row's (a dark line through each):
 *   xfactors = (4 + sin(2 pi x + offsets)) / 5
 *   yfactor  = (16 + sin(2 pi y)) / 17
 * x and y counted in the picture's pixels at the screen pixel's centre,
 * offsets pi (1/2, 1/2 - 2/3, 1/2 - 4/3). The factors are a column's times
 * a row's, so two strips multiplied over the picture draw them. Not on the
 * 3DS, whose picture its own GPU puts on the screen (present_3ds.c).
 * docs/engine/lcd.md */
#ifndef CW_LCD_GRID_H
#define CW_LCD_GRID_H

#include <SDL.h>
#include <stdbool.h>

bool lcd_grid(void);
void lcd_set_grid(bool on);
/* Over the target's `dst`, where a picture `w` x `h` pixels is drawn */
void lcd_grid_draw(SDL_Renderer *r, const SDL_Rect *dst, int w, int h);

#endif
