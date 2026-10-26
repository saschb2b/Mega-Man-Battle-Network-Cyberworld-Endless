/* The layers' make (generation, objects, loot and stock rolls): a run saved
 * by a build that makes them otherwise continues its layer afresh from its
 * start (director_save.c: the saved RAM's flags and Mystery Data would not match
 * this build's). Bump it with any change to what a layer seed makes.
 * LAYER_MAKE_HASH is layer_generate's output over the unit tests' seeds
 * (tests/test_core.c, test_layer_make): the tests fail when generation
 * changes, until both are updated together (issue #19). */
#ifndef LAYER_MAKE_H
#define LAYER_MAKE_H

#define LAYER_MAKE 106   /* (106: the NaviCust vendor on every act's middle layer, his list in BugFrags with a chip of the BugFrag Trader's pool last, docs/game/navicust.md 4; 105: BN5's End Area draws every room as its pads and keeps the way one wide, TILES_ONE_WIDE, issue #164; 104: a Virus Battler Mr.Prog at a side path's end on an act's first layer, in about half the acts from the second, net_pieces.c's layer_battler; 103: a dark layer's board gives every held Navi's command its spot, the Cross Navis' among them, docs/game/liberation/generator.md, The command rule; 102: a held Soul's kind in a share of the finds and on a dealer's list, loot_souls.h; 101: a dark layer's BonusPanels as BN5's: placed on boards without a sealed region too, its four chips, the planner's roulette of five; drawn in its own mission's map; a deleted guardian back as BN5 brings him, the planner's par after it; the board made for the run's team, its Pass BN5's fifth, docs/game/liberation/) */
#define LAYER_MAKE_HASH 0x149e3177u

#endif
