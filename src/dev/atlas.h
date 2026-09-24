/* The dev atlas: every area's generated layers drawn without the game
 * (atlas.c, docs/DEVTOOLS.md). */
#ifndef CW_ATLAS_H
#define CW_ATLAS_H

/* `spec` DIR[:BIOMES[:SEEDS]]; 0 when written. */
int atlas_run(const char *spec);

#endif
