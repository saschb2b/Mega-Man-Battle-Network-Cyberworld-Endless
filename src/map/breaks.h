/* The breaks in a generated map's picture (breaks.c), for the tile test
 * (build.py tiles): where it shows, across a tile's edge, what no original
 * map of its area shows there. */
#ifndef CW_BREAKS_H
#define CW_BREAKS_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"

/* The strips an area's maps show at their tiles' edges: the row or column
 * of 8 pixels on each side of an edge, as the map shows it and as its
 * first layer alone draws it (a generated map sets most tiles so). */
typedef struct Breaks Breaks;

/* Kept only while this is set (the dev atlas sets it before an area is
 * learned): a game never asks after them. */
extern bool breaks_measure;

/* Adds map `a`'s strips to *b (made at the first; NULL stays NULL where
 * breaks_measure is off or memory ran out). With no map, lets them go and
 * leaves *b NULL. */
void breaks_add(Breaks **b, const AreaSrc *a);

/* A picture of tw x th tiles (ARGB, alpha 0 where empty) against them. An
 * edge's strip is held to the nearest strip the maps show with the same
 * pixels on one of its sides: the pixels of the other side that differ by
 * more than a shade are what the picture breaks there. Half the edge or
 * more (BREAK_PIXELS of 8) is a break, a block of another floor or shade
 * set against this one; fewer a nick, a panel's line or light cut short.
 * Per tile into `out`: bit 0 a break across its right edge, bit 1 across
 * its lower edge, bits 2 and 3 a nick there. *breaks and *nicks: how many. */
#define BREAK_PIXELS 4
void breaks_count(Breaks *b, const uint32_t *px, int tw, int th, uint8_t *out, int *breaks, int *nicks);

#endif
