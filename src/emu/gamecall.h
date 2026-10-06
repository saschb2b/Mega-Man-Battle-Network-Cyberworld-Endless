/* Calls into the game's own code from the engine. */
#ifndef CW_GAMECALL_H
#define CW_GAMECALL_H

#include <stdbool.h>
#include <stdint.h>

/* The hooks the calls run through, once the core is up. */
void gamecall_install(void);

/* Runs the game's Thumb routine `fn` with r0, r1 on the next frame of the
 * game mode, in place of its state update (up to 120 frames); false when
 * it never ran. */
bool game_call(uint32_t fn, uint32_t r0, uint32_t r1);

/* game_call with r2 too; `out` (may be NULL) gets the routine's r0 and r1
 * as it returned. */
bool game_call_ret(uint32_t fn, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t out[2]);

/* Warps MegaMan to map (group, number) at world (x, y) at once, through the
 * game's own warp routine; `facing` is the overworld direction. */
void emu_warp(int group, int number, int x, int y, int facing);

/* Starts the game's warp to entry 1 of the current map's warp list with its
 * departure: MegaMan jacks out, and in at the entry's place. */
void emu_warp_out(void);
/* ... to entry `entry` of it, as its trigger cells would. */
void emu_warp_link(int entry);

#endif
