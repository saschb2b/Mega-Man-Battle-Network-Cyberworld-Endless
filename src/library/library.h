/* The Library (docs/game/meta/library.md): the PET's Library row opens it
 * over the held game, where BN6's own Library opened: a page per entry
 * (BN6, BN5, BN4), each its game's standard chips, Megas and Gigas a run
 * can hand out, filled from the profile's Library. L and R turn the
 * entries, Left and Right the classes, Up and Down pick, B closes; a tap
 * does each. */
#ifndef CW_LIBRARY_H
#define CW_LIBRARY_H

#include <stdbool.h>

void library_open(void);
bool library_shown(void);
/* The frame's course while it is up, or its B still held after it closed:
 * true while it holds the game */
bool library_update(void);
/* ... drawn over the game's picture (P.core_x, P.core_y) while it is up */
void library_draw(void);
/* Put away at once: the game's scene left (a soft reset, a quit) */
void library_shut(void);
/* The slot under its cursor: the run's chip in it, 0 where it is not
 * filled (then `unseen`) or the page is empty; false while it is not up */
bool library_cursor(int *chip, bool *unseen);

#endif
