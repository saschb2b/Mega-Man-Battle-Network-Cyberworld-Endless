/* A liberation board's planner (docs/game/liberation/quality-bar.md): it
 * plays a board made (liberation_board.h) many times by BN5's rules,
 * abstracted, as a fair player would (MegaMan alone for now: Liberation,
 * LongSwrd, Pass, fights), counts each phase's sensible moves, and sets
 * the par from the phases it took. ROM-free, its own random numbers. */
#ifndef CW_LIBERATION_PLAN_H
#define CW_LIBERATION_PLAN_H

#include <stdint.h>

#include "liberation_board.h"

/* Board b played from `seed`: its measures into b->m and its par into
 * b->par; LIB_WHY_NONE where it keeps the quality bar, else why not
 * (unwon, dead phases, a bare board with nothing to decide, LongSwrd
 * useless or dominant). */
int lib_plan(LibBoard *b, uint32_t seed);
/* (the measures' own: every board played kept, its measures taken) */
extern bool lib_plan_lax;

#endif
