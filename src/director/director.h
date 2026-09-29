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
/* The layer's map over the picture while SELECT is held (drawing only). */
void director_draw_map(void);
/* The rival's duel: its clock against ProtoMan's time, in battle (docs/RIVAL.md). */
void director_draw_duel(void);
/* Quitting on a layer's map, free to move: the run is saved there
 * (CONTINUE resumes it where MegaMan stood); false when it could not be. */
bool director_can_suspend(void);
bool director_suspend(void);
/* The PET's Save: a checkpoint where MegaMan stands, once he is free to
 * move on the map ("Run saved"). */
void director_save_here(void);
/* A Navi on the net has named the act's guardian this session. */
bool director_guardian_heard(void);
/* Where the run was last saved on this layer, for the quit prompt: its
 * start, or its Guardian Data. */
const char *director_saved_where(void);
/* Lan (or MegaMan) is on the map the run put him on: the picture can show. */
bool director_arrived(void);
/* What a player sees, in words, one fact a line (remote play). */
void director_describe(FILE *f);
/* Dev: MegaMan put at world (x, y) facing `face` (0-7, else unchanged). */
void director_dev_place(int x, int y, int face);
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
