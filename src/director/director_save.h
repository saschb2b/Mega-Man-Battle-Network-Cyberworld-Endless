/* Checkpoints (director_save.c), for the director's other parts. */
#ifndef CW_DIRECTOR_SAVE_H
#define CW_DIRECTOR_SAVE_H

#include <stdbool.h>

void save_checkpoint(void);
void arena_door_save(void);
/* Home's saves (docs/HOME.md), once a frame at home: the run saved in the
 * town between acts, a CONTINUE going on there: its checkpoint in Lan's HP
 * and the PET's Save, once MegaMan (or Lan) is free */
void home_saves(void);
/* The layer's checkpoint, once a frame on its map: due, and MegaMan free */
void checkpoint_update(void);
/* MegaMan into the layer's map again where he stands (or at its start) */
void layer_reenter(bool at_start);

#endif
