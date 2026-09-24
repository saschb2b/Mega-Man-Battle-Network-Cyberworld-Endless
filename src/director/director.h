/* The run on the game's own engine: layers, exits, loot and encounters. */
#ifndef CW_DIRECTOR_H
#define CW_DIRECTOR_H

#include <stdbool.h>

/* Builds the run's current layer in its area's map and warps MegaMan in. */
bool director_start_layer(void);
/* Rebuilds the saved run's layer and restores the game at its checkpoint. */
bool director_resume(void);
/* Once a frame, after the game's frame: exits and encounters. */
void director_update(void);
/* Where the test autopilot heads (grid panel): the guardian, to talk to
 * (*talk), or the exit pad. */
bool director_goal_panel(int *x, int *y, bool *talk);
/* Dev tools: MegaMan walks a layer's map (not a battle, a menu, a warp). */
bool director_on_map(void);
/* ... on to the next layer, or the next guardian's (arriving in the room
 * before its arena), or to grid panel (x, y) of this one. */
bool director_dev_next_layer(void);
bool director_dev_guardian(void);
bool director_dev_warp_cell(int x, int y);
/* Test hook (--net-biome): every layer in this biome. */
extern int director_debug_biome;

#endif
