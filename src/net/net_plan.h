/* A layer's plan (docs/LEVEL_DESIGN.md, Identity; issue #98): the layout
 * and the signature it is built in, from the run's seed and the depth
 * alone. */
#ifndef NET_PLAN_H
#define NET_PLAN_H

#include <stdint.h>

#include "net.h"

/* The layout (LAYOUT_*) and signature (SIG_*) of a layer: on the run's own
 * layers, an act's three each in another of its area's layouts and
 * signatures, and an act opening in neither the layout nor the signature
 * the act before closed in, where its area has others; a side layer's (an
 * Undernet detour, the Secret Area, a trip back) from the generator's
 * numbers and its seed. None where `kit`'s art draws every room as its
 * small hubs (LayerKit.hub_rooms). */
void layer_plan(uint32_t seed, int depth, int biome, int kind, const LayerKit *kit, int *layout, int *sig);

#endif
