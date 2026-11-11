/* The layers' make (generation, objects, loot and stock rolls): a run saved
 * by a build that makes them otherwise continues its layer afresh from its
 * start (director_save.c: the saved RAM's flags and Mystery Data would not match
 * this build's). Bump it with any change to what a layer seed makes: add a
 * file to src/net/layer_make.d/, NNN-WORDS.txt (the number it makes, a few
 * words), holding a clause on what changed. The build counts the files
 * (the Makefile's and Android's CMake's layer_make_count.h), so two
 * branches that each bump add two files and the merge counts both: no line
 * both would edit (docs/project/merging.md, Counters).
 *
 * The hash that guards it, layer_generate's output over the unit tests'
 * seeds (tests/test_core.c, test_layer_make; issue #19), is recorded with the
 * make it was taken at in tests/layer_make_hash.txt, written on dev by its
 * workflow after each merge (tools/layer_make.py): the tests fail where
 * generation changed and no bump's file was added. */
#ifndef LAYER_MAKE_H
#define LAYER_MAKE_H

#include "layer_make_count.h"   /* LAYER_MAKE_BUMPS, from the build */

#define LAYER_MAKE_BASE 100     /* (the make before the first file of layer_make.d: its bumps were a comment here) */
#define LAYER_MAKE (LAYER_MAKE_BASE + LAYER_MAKE_BUMPS)

#endif
