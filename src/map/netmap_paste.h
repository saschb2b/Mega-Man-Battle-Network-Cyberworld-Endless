/* The originals' scenery pasted (netmap_paste.c), for the map's other files. */
#ifndef CW_NETMAP_PASTE_H
#define CW_NETMAP_PASTE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "netmap_parts.h"

void nm_stair_origin(const Stair *st, int *X0, int *Y0);
void nm_paste_stairs(const Learned *L, uint16_t *map, int tw, int th);

/* ---- props ---- */

/* The last layer's props as drawn: their anchors (world units, props.h). */
typedef struct { bool ok; int X, Y; const PropStamp *st; } PropAt;
extern PropAt nm_prop_at[MAX_PROPS];

void nm_props_place(const Learned *L);
void nm_paste_props(uint16_t *map, int tw, int th);
void nm_paste_ornaments(const Learned *L, uint16_t *map, int tw, int th);
/* The area's pad, cut whole from its maps, on the layer's pads; `beside`:
 * both layers of the floor round the pads that touch a platform, picked
 * with the pads taken for void, or NULL (netmap.c) */
void nm_paste_pads(const Learned *L, uint16_t *map, const uint16_t *beside, int tw, int th);
/* Both layers of the floor round the pads the stamp goes on that touch a
 * platform's panel (8 ways), picked from `led` with those pads taken for
 * void, `floor` the layer's as the classes read it, for the stamp to stand
 * flush with them (freed by the caller); NULL where no pad touches one. */
uint16_t *nm_pick_beside_pads(const Learned *L, const TileGrid *grid, const TileBook *led, int nled, TileFloor floor);
void nm_paste_emblems(const Learned *L, uint16_t *map, int tw, int th);
void nm_paste_arrows(const Learned *L, uint16_t *map, int tw, int th);
void nm_paste_bushes(const Learned *L, uint16_t *map, int tw, int th);
/* The area's side pieces (sides.h) on the layer's free ground panels of
 * their neighbourhoods, some by the layer's seed. */
void nm_paste_sides(const Learned *L, uint16_t *map, int tw, int th);
/* The side pieces the last nm_paste_sides set: how many, and piece i with
 * the world corner of the panel it grips. */
int nm_sides_set(int i, const SidePiece **p, int *X, int *Y);
void nm_rebank_map(const Learned *L, uint16_t *map, size_t cells);

#endif
