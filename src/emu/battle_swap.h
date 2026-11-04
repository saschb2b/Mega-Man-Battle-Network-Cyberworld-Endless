/* A Navi swapped for another inside one BN6 battle (the Glitch layer's
 * copy, its design's The true swap): the battle's enemy, a
 * Navi of BN6's, deleted at its HP's end is taken off uncounted and
 * without its explosion, glitching out; the plan's next Navi arrives on
 * its panel through BN6's own mid-fight spawn, glitching in, and fights as
 * itself; the last form's deletion wins the battle as BN6 wins any
 * (docs/rom/navis.md, A Navi swapped mid-battle). Its hooks are set at the
 * first arm and read nothing until armed: a battle never armed meets none
 * of them, and battle_swap_disarm takes them off again. */
#ifndef CW_BATTLE_SWAP_H
#define CW_BATTLE_SWAP_H

#include <stdbool.h>

#define SWAP_FORMS 3   /* the forms a plan holds at most */

typedef struct {
	int id;   /* the enemy id: a Navi of BN6's at its version (0x101 HeatMan V1; docs/rom/navis.md) */
	int hp;   /* the HP it fights with (HP and MaxHP), 0 its own */
} SwapForm;

/* The next battle's plan, between frames before it starts: the first
 * form the battle's own enemy (the director's record spawns it), the
 * others in turn. False, and nothing armed, where a form is no Navi whose
 * deletion is BN6's generic Navi's (BN6_NAVI_DYING), or n is out of 2 to
 * SWAP_FORMS. */
bool battle_swap_arm(const SwapForm *forms, int n);
/* Its hooks off and its plan gone, between frames (after the battle). */
void battle_swap_disarm(void);

typedef struct {
	bool armed;    /* a plan stands, its battle not over */
	int forms;     /* the plan's forms */
	int form;      /* the form fighting now, 0 first; -1 before the first is found on the field */
	int gone;      /* forms taken off so far */
	int fails;     /* spawns that found no room (each retried the next frame) */
	int result;    /* the battle loop's phase as the battle ended (BN6_LOOP: 0x0C won, 0x10 lost), 0 while it goes on */
} SwapState;

/* What the plan came to, between frames */
SwapState battle_swap_state(void);

#endif
