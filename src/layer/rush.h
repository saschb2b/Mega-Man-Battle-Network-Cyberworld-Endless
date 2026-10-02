/* Rush's gaps on a layer (issue #14): BN6's own Rush, called with RushFood
 * at a gap's edge, lying in it for good once called. */
#ifndef CW_RUSH_H
#define CW_RUSH_H

#include <stdint.h>

/* Points the game's gap table at the layer's gaps on host map (group,
 * number), writes their records and Rush's, and readies the flags: Rush
 * known, each gap not yet bridged, its bones shown. */
void rush_install(int group, int number);

/* Appends the gaps' map objects (Rush lying in each panel, shown once
 * called, and the bones before) to `recs`, 20-byte spawn records, from
 * record n, `max` at most; the new count. */
int rush_objects(uint8_t *recs, int n, int max);

#endif
