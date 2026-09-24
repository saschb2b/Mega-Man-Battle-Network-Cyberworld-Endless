/* Calls into the game's own code from the engine. */
#ifndef CW_GAMECALL_H
#define CW_GAMECALL_H

#include <stdbool.h>
#include <stdint.h>

/* Runs the game's Thumb routine `fn` with r0, r1 on the next frame of the
 * game mode (up to 120 frames); false when it never ran. */
bool game_call(uint32_t fn, uint32_t r0, uint32_t r1);

/* Warps MegaMan to map (group, number) at world (x, y) at once, through the
 * game's own warp routine; `facing` is the overworld direction. */
void emu_warp(int group, int number, int x, int y, int facing);

/* Starts the game's warp to entry 1 of the current map's warp list with its
 * departure: MegaMan jacks out, and in at the entry's place. */
void emu_warp_out(void);

#endif
