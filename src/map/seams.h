/* Which tiles the original maps set side by side (seams.c): a generated map
 * whose neighbouring tiles never touch in any of them shows a seam, a floor
 * edge that steps or breaks off. */
#ifndef CW_SEAMS_H
#define CW_SEAMS_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"

/* A tile as its neighbours see it: its front entry, or one of these. */
#define SEAM_VOID 0xFFFFFFFFu   /* draws nothing */
#define SEAM_ANY  0xFFFFFFFEu   /* not known, or anything may touch it */

typedef struct {
	uint64_t *slot;   /* hashes of (direction, left or top, right or bottom); 0 free */
	unsigned cap, n;
} TileSeams;

/* Adds the pairs of neighbours map `a` shows (its first layer where the map
 * holds the background too). */
void seams_add(TileSeams *s, const AreaSrc *a, bool bg_in_map);
void seams_free(TileSeams *s);
/* Whether `first` (left, or above with `vertical`) was seen beside `second`. */
bool seams_seen(const TileSeams *s, uint32_t first, uint32_t second, bool vertical);

/* A tile edge as its picture, a strip: the row or column of 8 pixels on
 * each side of it (BGR555, STRIP_NONE where nothing is drawn), each side
 * as a number (seams_side). The strips the maps show at their tiles' edges
 * are kept beside their pairs of tiles, as they show them and as their
 * first layer alone draws them: two tiles no map sets side by side still
 * meet cleanly where the strip across their edge is one a map shows, and
 * where none shows it the picture breaks there, a panel's line cut off or
 * a block of another shade. */
#define STRIP_NONE 0x8000
uint32_t seams_side(const uint16_t px[8]);
/* Whether a map shows the strip of sides `first` (the left tile's right
 * column, or with `vertical` the upper tile's bottom row) and `second`. */
bool seams_strip_seen(const TileSeams *s, uint32_t first, uint32_t second, bool vertical);

#endif
