/* The run on the game's own engine: layers, exits, loot and encounters. */
#ifndef CW_DIRECTOR_H
#define CW_DIRECTOR_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

/* Builds the run's current layer in its area's map and warps MegaMan in. */
bool director_start_layer(void);
/* A new run: Lan in the town, the first layer built for the port's jack-in. */
bool director_start_run(void);
/* Lan is still in the town. */
bool director_in_town(void);
/* A run is under way on the layers (it has been saved). */
bool director_on_layer(void);
/* Lan (or MegaMan) is on the map the run put him on: the picture can show. */
bool director_arrived(void);
/* What a player sees, in words, one fact a line (remote play). */
void director_describe(FILE *f);
/* The pad's keys on their way to the game: on the map L is MegaMan's
 * word on where they are. */
uint32_t director_keys(uint32_t keys);
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
/* The director lets go of the game (the real world's tour). */
void director_stop(void);
/* Test hook (--net-biome): every layer in this biome. */
extern int director_debug_biome;
/* --talk: chats to open at given frames (director.c) */
extern const char *director_dev_talks;

#endif
