/* Where a layer's set pieces stand (net_set_pieces.c), for the generator's other files. */
#ifndef CW_NET_SET_PIECES_H
#define CW_NET_SET_PIECES_H

#include <stdbool.h>

#include "net_shapes.h"

/* ---- Rush's gaps (issue #14) ----
 * As BN6 sets them: a walkway aimed across one to three void panels at a
 * pad of its own, the pad holding one thing (Green Area 2's HPMemory, Sky
 * Area 1's ColArmy B), where the act's plan calls for one (net_pieces.c).
 * The stand is a ground floor panel off the way with floor behind it; the
 * gap's panels and the island's 3x3 are void with void a panel round them
 * and no prop, inside the camera's window. */
typedef struct { int x, y, d, len, score; } GapSite;

GapSite ng_plan_gap(int want);
void ng_carve_gap(const GapSite *g, int rise);
void ng_place_hidden(int rise);
void ng_plan_teleport(void);
void ng_carve_teleport_island(int rise);
void ng_plan_obstacle(int kind);
int ng_cube_kind(int depth, int biome, int kind);
int ng_pcode_cube(void);
bool ng_apart(int x, int y);
void ng_place_teller(const int *order, int n);
void ng_place_block_rewards(void);
void ng_place_lane(const LayerKit *kit);

#endif
