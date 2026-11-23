/* MegaMan's side's aim between a Surrounded field's two sides (issue
 * #838; docs/rom/chips-code.md, A chip used turned round).
 *
 * BN6 never turns MegaMan; a liberation's Surrounded battle (BN5's field,
 * BN6_SIDES_SURROUNDED: the enemy's columns 1-2 and 5-6, his 3-4) does, on
 * L or R. Two of BN6's routines aim a chip by its side alone, and here
 * miss:
 * - its search of a row (BN6_ROW_SEARCH) walks from the field's edge
 *   behind the searcher and gives the first panel its filter takes:
 *   GolmHit's fist the first of the enemy's. Behind MegaMan between the
 *   two sides lies the enemy's, so the fist came down on column 6 turned
 *   left and on column 1 facing right. A hook where it sets its start
 *   starts it at the object's own column, walking the same way;
 * - the way toward the enemy (BN6_ENEMY_DIRECTION) by the alliance alone,
 *   where object_getFrontDirection takes the flip too: a chip of DiveMan's
 *   and BN4's moved ones (their code calls it in BN4's place) flew right
 *   from MegaMan turned left. A hook answers the way the object faces.
 * Both act for an object of MegaMan's side on a field none of BN6's three
 * plain ones, so every plain battle, and every foe's aim, runs as BN6's;
 * the way toward the enemy is unchanged for him unturned. */
#ifndef CW_TURNED_AIM_H
#define CW_TURNED_AIM_H

#include <stdbool.h>

#include "bn6_panels.h"

/* Whether an object of `alliance` on a field of column pattern `sides`
 * (BattleSettings +6) aims as it faces, from its own column */
static inline bool turned_aim_mends(int sides, int alliance) {
	return alliance == 0 && sides != BN6_SIDES_PLAIN && sides != BN6_SIDES_WIDE && sides != BN6_SIDES_NARROW;
}

/* The hooks, once (with the liberation battle's: liberation_battle.c) */
void turned_aim_install(void);

#endif
