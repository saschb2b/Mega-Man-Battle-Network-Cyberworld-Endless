/* Visitors: where the nets meet (issue #82, docs/MULTIROM.md, One net: the
 * design). From the run's second act on, where BN5's ROM is read and its
 * families are in BN6's engine, one random battle in four takes one virus
 * of the other net in place of one of its own: BN6's areas meet BN5's
 * viruses, BN5's areas BN6's. The visitor stands on the panel of the one
 * it replaces, of the same role (visitor_families.h), a family the battle
 * does not hold, at the highest version that keeps the battle in the
 * act's band. Never in an opening battle, a Server's, the duel's, one the
 * Gentle helper eases, nor in the deep, whose acts mix the nets already. */
#ifndef CW_VISITORS_H
#define CW_VISITORS_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"

/* A family's role, where its own game's records set it: the back where
 * three of four of its placements are in the last column, the front where
 * most are in the first enemy column, else either */
enum { VISITOR_FREE, VISITOR_BACK, VISITOR_FRONT };
/* ... from its placements in the enemy's columns, first to last */
int visitor_role(int front, int middle, int back);

/* Whether the layer's battle `index` (its random battles fought so far,
 * from 0) is one of its visitors' by the gate alone: one in each four,
 * the one of the four from the layer's seed, so a reroll keeps it and a
 * checkpoint's layer has the same. No random numbers drawn. */
bool visitor_gate(uint32_t layer_seed, int index);
/* ... and by every rule: a plain random battle (kind loot.h's
 * ENC_NORMAL), from the run's second act (depth 4 on), not with the
 * Gentle helper, not in the deep (the Undernet, the Graveyard, the Nest:
 * `biome`), where the nets' families are in BN6's engine */
bool visitor_due(uint32_t layer_seed, int index, int depth, int biome, int kind);
/* (dev: every battle the rules allow takes one where loot.h's
 * loot_visitors_always is set, --dev visitors) */
/* Whether the random battles at `depth` in area `biome` can bring
 * visitors (visitor_due's rules but the gate's and the battle's kind):
 * the act's card says the nets meet */
bool visitors_here(int depth, int biome);

/* Battle `e` (a random battle at `depth`) with one of its viruses taken
 * by a visitor of the other net: the replaced one's panel (plain or
 * cracked) and role, a family `e` does not hold that its net's own random
 * battles reach at `depth` (its stage), at the highest version up to the
 * act's (BN5's by tier) that keeps the battle's HP to the band's top and
 * its hit to the band's cap; Encounter.visitor names it (1 + its index).
 * Its picks from the run's random numbers. False, `e` unchanged, where
 * none fits. */
bool visitor_apply(Encounter *e, int depth);

/* The deep converges (docs/MULTIROM.md, One net): the older net's area
 * whose records a random battle takes, -1 BN6's area's own: a territory's
 * (`territory`, -1 none) always; in the deep (`deep`, loot.h's loot_deep:
 * its older net's areas, -1 none), a coin between the nets, and between
 * the two areas where it names both (the Nest's), from the run's random
 * numbers, reproducible from the layer's seed as BN6's picks are; nothing
 * drawn outside the deep */
int nets_battle_area(const int deep[2], int territory);

#endif
