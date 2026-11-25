/* MegaMan's side's aim between a Surrounded field's two sides (issue
 * #838; docs/rom/chips-code.md, A chip used turned round).
 *
 * BN6 never turns MegaMan; a liberation's Surrounded battle (BN5's field,
 * BN6_SIDES_SURROUNDED: the enemy's columns 1-2 and 5-6, his 3-4) does, on
 * L or R. BN6's routines that aim a chip by its side alone here miss:
 * - its search of a row (BN6_ROW_SEARCH) walks from the field's edge
 *   behind the searcher and gives the first panel its filter takes:
 *   GolmHit's fist the first of the enemy's. Behind MegaMan between the
 *   two sides lies the enemy's, so the fist came down on column 6 turned
 *   left and on column 1 facing right. A hook where it sets its start
 *   starts it at the object's own column, walking the same way;
 * - the way toward the enemy (BN6_ENEMY_DIRECTION) by the alliance alone,
 *   where object_getFrontDirection takes the flip too: a chip of DiveMan's
 *   and BN4's moved ones (their code calls it in BN4's place) flew right
 *   from MegaMan turned left. A hook answers the way the object faces,
 *   while MegaMan stands turned round (unturned, his battle's every way
 *   toward the enemy as BN6's).
 * The rest (issue #846) take their side alone in places of their own:
 * - BN4's moved code calls the way toward the enemy with objects whose flip
 *   BN4 left 0 (it never turned MegaMan): answered from there by the way
 *   he faces (PanlSht, RollAro, GutPnch, DrkVulcn, DarkBomb, Boomer);
 * - BN6's search of the row across (BN6_ROW_PANEL_ACROSS), which BN4's
 *   Meteors calls, by the alliance: its way by the object's flip;
 * - BN6's chips and BN4's that read the alliance into a start column, a
 *   table's index or a flip (SandWrm, BurnSqr, TenguMan, AirRaid, Magnum;
 *   BN4's SeekBom, PitRng, GutsMan): the register turned round with him;
 * - BlastMan's flames started at the field's edge behind him, by the
 *   alliance: from his own column the way he faces, turned or not, as the
 *   row's search.
 * All act for an object of MegaMan's side on a field none of BN6's three
 * plain ones, so every plain battle, and every foe's aim, runs as BN6's. */
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
