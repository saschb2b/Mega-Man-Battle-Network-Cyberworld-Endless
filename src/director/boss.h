/* The layer guardian's sequence, from the arena to the open exit (boss.c). */
#ifndef CW_BOSS_H
#define CW_BOSS_H

#include <stdbool.h>
#include <stdint.h>

#include "guardian_objs.h"

/* A new layer: its guardian (navi 0 for none) with its scripts in `archive`. */
void boss_begin_layer(uint32_t archive, const GuardianStage *g);
/* Once a frame while MegaMan is on the map. */
void boss_update(void);
/* The guardian's battle was started and has not come back yet. */
bool boss_fighting(void);
/* Back from the guardian's battle. */
void boss_battle_over(bool won);
/* MegaMan was deleted: in the guardian's battle, it remembers. */
void boss_lost(void);
/* The exit pad works: no guardian, or it is beaten and its data taken. */
bool boss_exit_open(void);
bool boss_beaten(void);
/* The guardian's scene holds the stage (from its entrance to its logout). */
bool boss_cinematic(void);
/* Its Guardian Data has been taken. */
bool boss_done(void);
/* Nothing of the guardian under way: not met yet, or all done (a run can
 * be saved then). */
bool boss_idle(void);
/* Whether the guardian has just fallen and the run wants a checkpoint
 * (once: the call clears it). */
bool boss_take_checkpoint(void);
/* Whether MegaMan has just stepped into the arena and the run wants saving
 * at its door, this frame, before the staging takes him (once an approach:
 * the call clears it; the staging begins on the next frame). */
bool boss_take_door(void);
/* After a run saved mid-layer is loaded: the guardian's state from its
 * flags. */
void boss_resume(void);
/* Where the test autopilot heads while the guardian stands: into the
 * arena, then to its Guardian Data (to check, *talk). */
bool boss_goal(int *x, int *y, bool *talk);

#endif
