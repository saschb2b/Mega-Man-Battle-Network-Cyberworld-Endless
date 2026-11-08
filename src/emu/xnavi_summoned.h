/* xnavi_summoned.h: an enemy a BN4 Navi's summon spawns arrives by BN4's
 * summoned appear, whatever its family (docs/multirom/bn4-navis.md, Roll's
 * versions). BN4's spawn of an enemy into a fight under way marks it +5 2
 * (BN4_SUMMON_MARK), which BN4's generic state 0, every BN4 family's, sends
 * on to its summoned appear (BN4_SUMMONED_APPEAR); the summon's shim marks
 * BN6's spawn so (xvirus_shim_summon). A family moved for a summon alone
 * runs BN4's generic state 0 as its own (XVirusFamily.summoned: Mettaur's),
 * a visitor BN6's (Billy's), which takes BN6's own appear by its AI data's
 * +2. So BN6's virus dispatcher's call of its state runner
 * (BN6_VIRUS_STATE_CALL, r0 the state table by AI index) is hooked: a moved
 * family's enemy so marked, in its state 0, runs from the table the
 * summon's word names, the summoned family's, whose state 0 is BN4's with
 * its summoned appear. One copy of that appear serves every family a summon
 * spawns, and a visitor's own state 0, which brings it into a battle, stays
 * BN6's. */
#ifndef CW_XNAVI_SUMMONED_H
#define CW_XNAVI_SUMMONED_H

#include <stdbool.h>
#include <stdint.h>

#include "xnavi_recipe.h"

/* The word at `at` (in the summon's room, staged: xvirus_tx.h) made the state table of the family moved for summon `m`
 * (XVirusFamily.summoned, installed, among its ids), and the hook set once a core. True where none of its ids is such a
 * family (no word: every enemy it spawns arrives by its own state 0) */
bool xnavi_summoned_set(const XnSummon *m, uint32_t at);

#endif
