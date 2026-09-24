/* A layer's floor made drawable (legal.c): single floor cells added or
 * taken away until every panel has tiles seen with exactly its neighbours
 * in the original maps, where that can be done without changing how the
 * floor connects. */
#ifndef CW_LEGAL_H
#define CW_LEGAL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	int gw, gh;
	uint8_t *cell;           /* gw x gh, nonzero on floor: edited */
	const uint8_t *locked;   /* gw x gh, nonzero where the floor must stay as it is */
	/* whether the panel of grid cell (x, y) is drawn exactly (x, y may lie
	 * one outside the grid) */
	bool (*clean)(int x, int y, void *ctx);
	void *ctx;
} LegalGrid;

typedef struct { int edits, left; } LegalStats;

/* At most `budget` edits. `left`: the panels still not drawn exactly. */
LegalStats legal_fix(const LegalGrid *g, int budget);

#endif
