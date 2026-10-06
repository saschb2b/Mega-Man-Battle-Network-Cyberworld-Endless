/* A 5 x 7 pixel font of the engine's own (pixfont.c), drawn for this
 * project, nothing from the ROM: capitals, lowercase with descenders,
 * digits and punctuation, each letter as wide as it needs, and a dark
 * shade under it as the PET's labels have. The screens before a game draw
 * with it, where there may be no ROM yet and so no game font (the
 * launcher, src/launcher/). */
#ifndef CW_PIXFONT_H
#define CW_PIXFONT_H

#include <SDL.h>
#include <stdbool.h>

/* Lines from the top of a capital to the bottom of a descender: 7 and 2 */
#define PIXFONT_CAP 7
#define PIXFONT_H 9
/* A line's height with the gap under it */
#define PIXFONT_LINE 11

/* Its icons, as characters in a string */
#define PIXFONT_CHECK "\x01"   /* a tick */
#define PIXFONT_PLAY "\x02"    /* a right-pointing triangle */
#define PIXFONT_DOT "\x03"     /* a middle dot */

enum { PIXFONT_LEFT, PIXFONT_CENTER, PIXFONT_RIGHT };

/* Pixels `s` takes across at `scale` */
int pixfont_width(const char *s, int scale);
/* Draws `s` from (x, y), the top of its capitals, aligned on x, `scale`
 * canvas pixels to a font pixel, in `c` with its shade one pixel to the
 * right and below (shade.a 0: none) */
void pixfont_draw(int x, int y, const char *s, SDL_Color c, SDL_Color shade, int align, int scale);
/* Lines of `text` (newlines break one too) no wider than `w` pixels at
 * `scale`, at most `most`, each at most 95 characters: how many */
int pixfont_wrap(const char *text, int w, int scale, char lines[][96], int most);

#endif
