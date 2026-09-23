/* The run on the game's own engine: layers, exits, loot and encounters. */
#ifndef CW_DIRECTOR_H
#define CW_DIRECTOR_H

#include <stdbool.h>

/* Builds the run's current layer in its area's map and warps MegaMan in. */
bool director_start_layer(void);
/* Once a frame, after the game's frame: exits and encounters. */
void director_update(void);

#endif
