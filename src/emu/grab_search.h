/* AreaGrab's search for its column, ended where BN6's never does
 * (docs/rom/battle.md, AreaGrab's column; found 12 November 2026 on a
 * liberation's Surrounded field). BN6 looks for the grabber's side's
 * hindmost column that is wholly its own (bn6f sub_800D5BA, from
 * sub_800D58C: AreaGrab and four more callers): from the grabber's column
 * backward, and where that walks off the field, from its column forward;
 * off the field again it turns round once more, for good. BN6's own
 * battles always leave such a column (MegaMan's first is never taken). A
 * Surrounded field gives him two, 3 and 4, and a Catack that drives
 * through a row takes a panel of each: AreaGrab then stopped the game,
 * the chip's dimmed screen left standing.
 *
 * A hook where the walk turns round: turned round already, the search
 * ends on the grabber's own column, which holds a panel of the other
 * side's, so AreaGrab takes that column's panels back. On BN6's own core
 * alone (the Arena's battles are never Surrounded). */
#ifndef CW_GRAB_SEARCH_H
#define CW_GRAB_SEARCH_H

#include <stdbool.h>

/* Whether a walk that left the field with step `step` (r4: a column a
 * time) has turned round already: it walks the way the grabber faces,
 * BN6's 0x0800E2CA of its alliance and its flip (+1 MegaMan's side
 * unturned) */
static inline bool grab_search_lost(int step, int alliance, int flip) { return step == 1 - 2 * ((alliance ^ flip) & 1); }

/* The hook, once (with the liberation battle's: liberation_battle.c) */
void grab_search_install(void);

#endif
