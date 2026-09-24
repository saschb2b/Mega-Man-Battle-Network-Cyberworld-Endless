/* The original random battles of each area's maps (docs/ROM_DATA.md): which
 * viruses come together, where they stand, rocks and cubes on the field,
 * and the battlefield's panels. */
#ifndef CW_FORMATIONS_H
#define CW_FORMATIONS_H

#include <stdbool.h>
#include <stdint.h>

#define FORMATION_MAX_ENTS 8

typedef struct {
	uint8_t battlefield;
	uint8_t n;
	struct { uint8_t panel; uint16_t id; } ent[FORMATION_MAX_ENTS];   /* panel: row << 4 | column, from 1 */
	uint8_t weight;      /* how many of the original tables list it */
	bool navi;           /* holds a navi (the SP navis the areas hide) */
} Formation;

/* The formations of area `biome`, weighted; 0 without a ROM. */
int formations_of(int biome, const Formation **out);

#endif
