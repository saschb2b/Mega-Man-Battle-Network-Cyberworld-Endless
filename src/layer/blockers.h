/* The Link Navi obstacles on a layer (issue #42): BN6's own map objects
 * (handler 3), which the run's Crosses clear as Gregar's Link Navis do. */
#ifndef CW_BLOCKERS_H
#define CW_BLOCKERS_H

#include <stdbool.h>
#include <stdint.h>

#include "text.h"

/* The obstacles' present flags, one a slot (layer.block[]): set, the
 * obstacle stands; cleared, it opens and stays open (docs/ROM_DATA.md). */
#define BLOCK_PRESENT_FLAG 0x1458

/* Readies the layer's obstacles: their records (once a core) and their
 * present flags (set, never cleared here: an obstacle of the map being left
 * would open). */
void blockers_install(void);

/* Obstacle b's sprite (list category, index), which the map must load. */
void blocker_sprite(int b, int *category, int *index);

/* Each obstacle's talk, MegaMan's and its Link Navi's, in `t`; and the
 * map's checks taking them (the layer's archive, compressed, the map's own:
 * call once `t` is whole). */
void blockers_talks(TextArchive *t, int scripts[2]);
void blockers_checks(int group, int number, const TextArchive *t, const int scripts[2]);

/* Appends the obstacles' spawn records to `recs` from record n, `max` at
 * most; the new count. */
int blockers_objects(uint8_t *recs, int n, int max);

#endif
