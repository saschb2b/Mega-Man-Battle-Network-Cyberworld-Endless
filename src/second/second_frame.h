/* The PET's frame (second_frame.c) that every panel of the second screen
 * sits in, and the PET's colours for the panels. */
#ifndef CW_SECOND_FRAME_H
#define CW_SECOND_FRAME_H

#include "gfx.h"

/* Gregar's green PET, as tools/site_art.py measured it from BN6's screens */
#define PET_GREEN     rgba(8, 189, 115, 255)
#define PET_LINE      rgba(74, 231, 115, 255)
#define PET_DARK      rgba(0, 123, 74, 255)
#define PET_CYAN      rgba(66, 198, 231, 255)
#define PET_CYAN_HI   rgba(107, 222, 255, 255)
#define PET_NAVY      rgba(16, 82, 107, 255)
#define PET_SLOT      rgba(16, 99, 107, 255)
#define PET_SLOT_EDGE rgba(33, 74, 82, 255)
#define PET_EXIT      rgba(0, 49, 74, 255)
#define PET_GOLD      rgba(255, 214, 16, 255)
#define PET_WHITE     rgba(247, 255, 247, 255)
#define PET_DIM       rgba(107, 156, 173, 255)   /* what has passed: a chip used */

/* Text in lines no wider than `w` (a '|' breaks one too), at most `most`
 * of them, each SECOND_WRAP long at most: how many it made */
#define SECOND_WRAP   96
#define SECOND_LINE_H 13
int second_wrap(const char *text, int w, char lines[][SECOND_WRAP], int most);
/* ... drawn from (x, y), at most `most` lines (8): the y under them */
int second_wrapped(const char *text, int x, int y, int w, int most, SDL_Color c);
/* A row of the PET's on a slot from (x, y), w wide: `name` in gold, `value` in white */
void second_row(int x, int y, int w, const char *name, const char *value);
#define SECOND_ROW_H 18
/* A dark slot, BN6's for a value, w x h from (x, y) */
void second_slot(int x, int y, int w, int h);
/* BN6's three light stripes after a screen's name, slanting up, from
 * (x, y): SECOND_STRIPES_W wide, SECOND_STRIPES_H tall */
#define SECOND_STRIPES_W 19
#define SECOND_STRIPES_H 12
void second_stripes(int x, int y);
/* The frame in w x h: the header with `title` on its left (`slide` pixels
 * short of its place, as it slides in) and the place on its right, the
 * strip of HP, Zenny and BugFrags under it; the body left for the panel */
SDL_Rect second_frame(int w, int h, const char *title, int slide);
/* ... its body alone, the frame drawn before and left as it was */
SDL_Rect second_frame_body(int w, int h);
/* ... without a run: the header with `title` and `where`, no strip */
SDL_Rect second_frame_rest(int w, int h, const char *title, const char *where);

#endif
