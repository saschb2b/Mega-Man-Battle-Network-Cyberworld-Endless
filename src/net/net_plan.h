/* A layer's plan (docs/LEVEL_DESIGN.md, Identity and Arrivals; issues #98,
 * #106): the layout and the signature it is built in and the side MegaMan
 * arrives at, from the run's seed and the depth alone. */
#ifndef NET_PLAN_H
#define NET_PLAN_H

#include <stdint.h>

#include "net.h"

/* A layer's plan: its layout (LAYOUT_*), its signature (SIG_*) and the
 * side of the screen MegaMan arrives at (SIDE_*, issue #106). */
typedef struct { int layout, sig, side; } LayerPlan;

/* The plan of a layer: on the run's own layers, an act's three each in
 * another of its area's layouts and signatures, arriving at another side
 * of the screen, and an act opening in neither the layout, the signature
 * nor the side the act before closed in, where its area has others; a
 * side layer's (an Undernet detour, the Secret Area, a trip back) from the
 * generator's numbers and its seed. No signature where `kit`'s art draws
 * every room as its small hubs (LayerKit.hub_rooms). */
void layer_plan(uint32_t seed, int depth, int biome, int kind, const LayerKit *kit, LayerPlan *out);

#endif
