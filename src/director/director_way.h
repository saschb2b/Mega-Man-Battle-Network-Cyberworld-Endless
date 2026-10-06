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
void goal_way(void);
void push_arrow(void);
void arrow_update(void);

#endif
