/* A liberation battle's field, as BN5 picks it (docs/game/liberation/building/stage-2-battle.md, Stage
 * 2): the dark round the faced panel, weighed by where MegaMan stands, sets
 * a level, and the level whose each column is: MegaMan's four, the usual
 * three, his two, or Surrounded, foes on both sides of him and L or R to
 * turn round (bn5_liberation.h's BN5_LIB_LEVEL, bn6_panels.h's
 * BN6_SIDES_*). */
#ifndef CW_LIBERATION_FIELD_H
#define CW_LIBERATION_FIELD_H

#include <stdbool.h>

#include "foes.h"

/* BN5's levels of the dark round a panel (BN5_LIB_STATE +0x0D) */
#define LIB_LEVELS 5

/* The level of the dark round panel (x, y), faced from the panel before it
 * in the grid way `face` (0 +x, 1 +y, 2 -x, 3 -y): 0-4 */
int lib_field_level(int x, int y, int face);
/* The column pattern a battle takes at a level (BN6_SIDES_*): a Darkloid's
 * never Surrounded */
int lib_field_sides(int level, bool darkloid);
/* The battle set out on that pattern as BN5's records set theirs: MegaMan
 * at column 3 and his foes on both sides of him where Surrounded, in
 * columns 5-6 where his side is four wide; objects in their way left out */
void lib_field_apply(Encounter *e, int sides);

#endif
