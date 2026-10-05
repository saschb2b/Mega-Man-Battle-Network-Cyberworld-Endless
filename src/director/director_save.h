/* Checkpoints (director_save.c), for the director's other parts. */
#ifndef CW_DIRECTOR_SAVE_H
#define CW_DIRECTOR_SAVE_H

void save_checkpoint(void);
void arena_door_save(void);
/* Home's checkpoint (docs/HOME.md): the run saved in the town between
 * acts, a CONTINUE going on there. */
void home_save(void);

#endif
