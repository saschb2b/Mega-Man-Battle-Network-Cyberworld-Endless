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

/* Adds the pairs of neighbours map `a` shows (its front layer where the map
 * holds the background too). */
void seams_add(TileSeams *s, const AreaSrc *a, bool bg_in_map);
void seams_free(TileSeams *s);
/* Whether `first` (left, or above with `vertical`) was seen beside `second`. */
bool seams_seen(const TileSeams *s, uint32_t first, uint32_t second, bool vertical);
/* The unseen pairs among the four neighbours of a tile: nb[] left, above,
 * right, below. */
int seams_unseen(const TileSeams *s, uint32_t look, const uint32_t nb[4]);

#endif
