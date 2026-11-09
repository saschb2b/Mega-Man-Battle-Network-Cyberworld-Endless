/* The layers' sizes against their regions (layer_sizes.c,
 * docs/engine/emulation/free-space.md). */
#ifndef CW_LAYER_SIZES_H
#define CW_LAYER_SIZES_H

/* Every area's layers over runs of seeds 1 to `seeds` (`areas` "" all, or
 * "=A,B,..." BN6's numbers and xN another game's), the largest tile map and
 * coordinate data written to DIR/sizes<areas>.txt; 0 where both fit their
 * regions. */
int layer_sizes_run(const char *dir, const char *areas, int seeds);

#endif
