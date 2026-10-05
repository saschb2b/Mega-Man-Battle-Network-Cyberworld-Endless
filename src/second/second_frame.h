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

/* The frame in w x h: the header with `title` on its left (`slide` pixels
 * short of its place, as it slides in) and the place on its right, the
 * strip of HP, Zenny and BugFrags under it; the body left for the panel */
SDL_Rect second_frame(int w, int h, const char *title, int slide);

#endif
