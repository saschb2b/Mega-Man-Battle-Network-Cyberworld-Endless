/* The layers' make (generation, objects, loot and stock rolls): a run saved
 * by a build that makes them otherwise continues its layer afresh from its
 * start (director_save.c: the saved RAM's flags and Mystery Data would not match
 * this build's). Bump it with any change to what a layer seed makes.
 * LAYER_MAKE_HASH is layer_generate's output over the unit tests' seeds
 * (tests/test_core.c, test_layer_make): the tests fail when generation
 * changes, until both are updated together (issue #19). */
#ifndef LAYER_MAKE_H
#define LAYER_MAKE_H

#define LAYER_MAKE 91
#define LAYER_MAKE_HASH 0x9f0624e1u

#endif
