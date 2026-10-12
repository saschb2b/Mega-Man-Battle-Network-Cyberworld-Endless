/* The families' code mover (xvirus.c) for other content that brings its
 * game's code into BN6's engine the same way: the Navis (xnavi_install.c,
 * issue #82). A family's recipe (xvirus_recipe.h) says what is copied
 * where; these stage it within a step (xvirus_tx.h): its blocks and clones
 * copied and walked, their pools and tables moved, their immediates
 * changed and their calls out through veneers in its home, then what it
 * draws (its sprites, effects and object types for the second-level
 * dispatch). One at a time. */
#ifndef CW_XVIRUS_MOVE_H
#define CW_XVIRUS_MOVE_H

#include <stdbool.h>
#include <stdint.h>

#include "xvirus_recipe.h"

/* Recipe f's move begun: false (the step failed) where its game's ROM is
 * not read. Its veneers go to its home's XV_HOME_VENEERS on. */
bool xv_move_begin(const XVirusFamily *f);
/* Its blocks and clones moved, and their veneers written */
void xv_move_code(void);
/* A state's or hook's address in BN6 (its own code moved, a clone's, or one
 * of BN6's routines checked), 0 (the step failed) for none */
uint32_t xv_move_ref(XvRef r);
/* Its sprites, effects and object types (the dispatch's) */
void xv_move_art(void);
/* Its game's ROM, the move's veneers made so far */
const uint8_t *xv_move_rom(void);
int xv_move_veneers(void);

#endif
