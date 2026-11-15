/* Checkpoints (director_save.c), for the director's other parts. */
#ifndef CW_DIRECTOR_SAVE_H
#define CW_DIRECTOR_SAVE_H

#include <stdbool.h>

void save_checkpoint(void);
void arena_door_save(void);
/* Home's saves (docs/world/home/), once a frame at home: the run saved in the
 * town between acts, a CONTINUE going on there: its checkpoint in Lan's HP
 * and the PET's Save, once MegaMan (or Lan) is free */
void home_saves(void);
/* The layer's checkpoint, once a frame on its map: due, and MegaMan free */
void checkpoint_update(void);
/* MegaMan into the layer's map again where he stands (or at its start) */
void layer_reenter(bool at_start);
/* The run as it stands, held back for a quit before MegaMan is free again
 * (director_save.c, a quit mid-talk): as the director begins a talk, and
 * once a frame on a layer, which lets it go */
void director_before_talk(void);
void held_watch(void);
/* Whether a talk due on the map may begin: MegaMan free, the run held back
 * before it as it begins */
bool talk_due_now(void);

#endif
