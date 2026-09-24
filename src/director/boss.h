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
/* Where the test autopilot heads while the guardian stands: into the
 * arena, then to its Guardian Data (to check, *talk). */
bool boss_goal(int *x, int *y, bool *talk);

#endif
