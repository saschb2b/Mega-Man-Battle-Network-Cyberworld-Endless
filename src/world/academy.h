/* The Cyber Academy (academy.c, docs/HOME.md piece 10): BN6's own maps of
 * Lan's school as a place of home, open for the clubs. */
#ifndef CW_ACADEMY_H
#define CW_ACADEMY_H

#include <stdbool.h>

#define ACADEMY_GROUP 0x02   /* bn6f constants/enums/GameAreas.inc */
#define ACADEMY_CLASS_6_1 0x00
#define ACADEMY_HALL_1F 0x04
#define ACADEMY_HALL_2F 0x05
#define ACADEMY_FOYER 0x06

/* The foyer, the 1F and 2F hallways and class 6-1 installed (indoors.c):
 * their doors between them and the gate out to where the planned town has
 * it open, the other rooms' shut, the run's people in them. False where
 * their doors cannot be read. */
bool academy_install(void);
/* Whether map (group, number) is one of the Academy's open to a run. */
bool academy_map(int group, int number);
/* The way out of Academy map `number` (its gate, the stairs down, the door
 * to the hallway): the middle of its cells; false none. */
bool academy_way_out(int number, int *x, int *y);
/* Each frame in Academy map `number`: its doors but the open ones shut
 * (indoors_shut). */
void academy_frame(int number);

#endif
