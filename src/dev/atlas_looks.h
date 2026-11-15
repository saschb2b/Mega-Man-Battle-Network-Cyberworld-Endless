/* The atlas's Glitch looks (atlas_looks.c; docs/game/glitch-layer.md,
 * Checking an area's look). */
#ifndef CW_ATLAS_LOOKS_H
#define CW_ATLAS_LOOKS_H

#include <stdbool.h>

/* `spec` "intoAREA" (a layer of every other area drawn as a flip into
 * AREA shows it) or "fromAREA" (a layer of AREA drawn in every other
 * area's look), AREA as --net-biome names it (N, or xN another game's);
 * `seeds` layers each, their pictures and DIR/report.txt in `dir`. False
 * where `spec` is neither; *code its exit code. */
bool atlas_looks_run(const char *dir, const char *spec, int seeds, int *code);

#endif
