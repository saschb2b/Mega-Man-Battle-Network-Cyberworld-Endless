/* The second-level dispatch (issue #82; docs/engine/emulation/memory.md, its free space
 * and the RAM plan; docs/multirom/families.md): BN6's battle
 * runs an object by the routine of its type, a byte, from its kind's table
 * (the first, third and fourth kinds'), and BN5's moved families, Navis and
 * chips bring more types than those tables have room for. So one type of
 * each kind, dead in BN6 (XV_T1_DISPATCH, XV_T3_DISPATCH, XV_T4_DISPATCH),
 * runs whatever moved routine its object was spawned as:
 *
 *  - a moved family's code calls its game's spawners with its game's type
 *    numbers, as it did there; each such call goes to one of ours
 *    (XV_SPAWN_*: the recipe's calls name them), which reads the number's
 *    BN6 twin from TWIN (one table a kind, by its game's number) and spawns
 *    that through BN6's spawner of the same kind and place in the run
 *    order, or, where the number is moved (XV_TYPE_MOVED), the kind's
 *    dispatch type; either way its game's number goes into the kind's
 *    side table in RAM by the object's slot (BN6_DISPATCH_SIDE), which
 *    only the dispatch type's routine reads;
 *  - each frame the dispatch type's routine (XV_STUB_DISPATCH) reads that
 *    number back by the object's slot and goes on to MOVED's routine for
 *    it (one table a kind, the moved copy of its game's routine), as BN5's
 *    runner would: r0 the routine, r1 the number times four, r5 the object,
 *    lr the runner's way back.
 *
 * Its code is the project's own Thumb, made here (xvirus_dispatch_code)
 * and written past EMU_FREE as BN6's tables are grown (xvirus_grow.c), with
 * MOVED (every entry the routine that lets a stray object go) and TWIN
 * (every entry XV_TYPE_MOVED) blank until a family names its types. */
#ifndef CW_XVIRUS_DISPATCH_H
#define CW_XVIRUS_DISPATCH_H

#include <stdbool.h>
#include <stdint.h>

#include "xvirus_recipe.h"

#define XV_DISPATCH_KINDS 3           /* the kinds it serves: the first, third and fourth */
#define XV_STUBS_SIZE (XV_STUBS_END - XV_STUBS)

/* The kind's place among the three (0, 1, 2 for the first, third and
 * fourth kinds), -1 for another. */
int xvirus_dispatch_slot(int kind);
/* The kind's dispatch type (XV_T*_DISPATCH), -1 for another kind. */
int xvirus_dispatch_type(int kind);
/* Where the kind's tables and its stub are (bus addresses): its MOVED
 * table's entry for its game's type n, its TWIN table's, its side table in
 * BN6's RAM (32 bytes, by slot) and the routine its dispatch type runs. 0
 * for a kind it does not serve. */
uint32_t xvirus_dispatch_moved_at(int kind, int n);
uint32_t xvirus_dispatch_twin_at(int kind, int n);
uint32_t xvirus_dispatch_side(int kind);
uint32_t xvirus_dispatch_routine(int kind);
/* The dispatch's spawner (XV_SPAWN_*) a moved family's call to its game's
 * spawner `from` goes to (`game` XV_GAME_BN6 or an XRomId), 0 where `from`
 * is none of that game's spawners (BN6's own are none: its families' code is
 * BN6's types already). */
uint32_t xvirus_dispatch_spawner(int game, uint32_t from);
/* Whether `to` is one of the dispatch's spawners (XV_SPAWN_*), and its kind
 * and BN6's spawner it calls (where not NULL). */
bool xvirus_dispatch_is_spawner(uint32_t to, int *kind, uint32_t *bn6);
/* The dispatch's code as it stands at XV_STUBS, XV_STUBS_SIZE bytes: the
 * project's own Thumb (its literals the addresses above). */
void xvirus_dispatch_code(uint8_t out[XV_STUBS_SIZE]);

#endif
