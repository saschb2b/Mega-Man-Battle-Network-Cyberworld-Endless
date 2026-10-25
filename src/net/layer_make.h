/* The layers' make (generation, objects, loot and stock rolls): a run saved
 * by a build that makes them otherwise continues its layer afresh from its
 * start (director_save.c: the saved RAM's flags and Mystery Data would not match
 * this build's). Bump it with any change to what a layer seed makes.
 * LAYER_MAKE_HASH is layer_generate's output over the unit tests' seeds
 * (tests/test_core.c, test_layer_make): the tests fail when generation
 * changes, until both are updated together (issue #19). */
#ifndef LAYER_MAKE_H
#define LAYER_MAKE_H

#define LAYER_MAKE 102   /* (102: a held Soul's kind in a share of the finds and on a dealer's list, loot_souls.h; 101: a dark layer's BonusPanels as BN5's: placed on boards without a sealed region too, its four chips, the planner's roulette of five; drawn in its own mission's map; a deleted guardian back as BN5 brings him, the planner's par after it; the board made for the run's team, its Pass BN5's fifth, docs/LIBERATION.md) */
#define LAYER_MAKE_HASH 0xdfa0d42au

#endif
