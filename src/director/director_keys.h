/* The pad on its way to the game (director_keys.c), for the director's other parts. */
#ifndef CW_DIRECTOR_KEYS_H
#define CW_DIRECTOR_KEYS_H

#include <stdbool.h>

void unwedge(void);
/* MegaMan where an A of his may be meant for someone (an NPC in the
 * probes' reach, A held, the walk up to one): words of his own wait. */
bool talk_in_reach(void);

#endif
