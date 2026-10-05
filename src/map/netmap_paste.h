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
void nm_paste_pads(const Learned *L, uint16_t *map, int tw, int th);
void nm_paste_emblems(const Learned *L, uint16_t *map, int tw, int th);
void nm_paste_arrows(const Learned *L, uint16_t *map, int tw, int th);
void nm_paste_bushes(const Learned *L, uint16_t *map, int tw, int th);
void nm_rebank_map(const Learned *L, uint16_t *map, size_t cells);

#endif
