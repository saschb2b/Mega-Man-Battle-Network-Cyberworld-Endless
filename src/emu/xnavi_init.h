/* What a Navi's own init does past BN6's actor init, made again by shims
 * of its home (the project's own Thumb; docs/multirom/bn4-navis.md,
 * Companions): its collision's own r3 (XNavi.coll_arg) and the rest of
 * its init (XNavi.init: its companions spawned, its +0x11, its sprite
 * updated, its update run), at its spawn's end (its table 7) and at its
 * companion start (BN6's actor init's call after its collision set up). */
#ifndef CW_XNAVI_INIT_H
#define CW_XNAVI_INIT_H

#include <stdint.h>

#include "bn6_navis.h"
#include "xnavi_recipe.h"

/* Its shims written (staged, xvirus_tx.h) in its home past its own shims (xvirus_shims_room): its spawn's end given to
 * table[7], which must be BN6's empty hook where it needs one; back the word its companion start takes (its .companion's
 * where it spawns none there), 0 where it failed (xv_fail says why). */
uint32_t xnavi_init_set(const XNavi *n, uint32_t table[BN6_NAVI_TABLE_COUNT]);

#endif
