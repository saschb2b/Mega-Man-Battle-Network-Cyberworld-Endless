/* The state's lines for a developer reproducing a playtest (the state in
 * words with CYBERWORLD_STATE_POS set; director_describe calls it): where
 * Lan or MegaMan stands and what holds him, the floor around him, the
 * arrow's way and heal, the layer drawn in letters with
 * CYBERWORLD_STATE_POS=map, the game's NPC objects near him. Reads alone. */
#ifndef CW_DIRECTOR_DESCRIBE_POS_H
#define CW_DIRECTOR_DESCRIBE_POS_H

#include <stdio.h>

void director_describe_pos(FILE *f);

#endif
