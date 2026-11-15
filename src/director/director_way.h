/* Where things lie and the way on (director_way.c), for the director's other parts. */
#ifndef CW_DIRECTOR_WAY_H
#define CW_DIRECTOR_WAY_H

#include <stdbool.h>

int way_last(void);
extern const char *const ways[8];
const char *way_to(int tx, int ty, int *far);
bool duel_waiting(int *wx, int *wy);
const char *town_way(int *far);
void route_floor(void);
const char *route_to(int tx, int ty, int *far);
const char *lie_and_walk(int wx, int wy, int *far, bool *winds);
const char *lies_at(int wx, int wy);
bool hurt_now(void);
bool heal_spent(void);
void hp_full_watch(void);
/* The Recovery Mr. Prog's panel while the way on leads to him first. */
bool heal_first(int *x, int *y);
/* MegaMan walking away from that heal (the way on the exit's again), and
 * L asking again (the heal first again) */
void heal_turn_watch(void);
void heal_turn_reset(void);
/* The walk in panels from MegaMan on panel (cx, cy) to the guardian's
 * arena, as the arrow leads, on its way in; -1 elsewhere or for none. */
int arena_walk(int cx, int cy);
void goal_way(void);
void push_arrow(void);
/* MegaMan standing still on the exit pad as drawn: its warp (pad_art.h). */
void exit_pad_step(void);
void arrow_update(void);
/* MegaMan on panel (cx, cy) of a fork's layer: his read of the layer past
 * its exit pad, once he comes near it */
void fork_pad_read(int cx, int cy);

#endif
