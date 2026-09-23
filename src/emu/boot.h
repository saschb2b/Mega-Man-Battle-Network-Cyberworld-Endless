/* Getting the game from power-on to MegaMan standing in the net. */
#ifndef CW_BOOT_H
#define CW_BOOT_H

#include <stdbool.h>
#include <stdint.h>

/* Warps MegaMan to map (group, number) at world (x, y) through the game's own
 * warp routine; `facing` is the overworld direction (0 up, clockwise). */
void emu_warp(int group, int number, int x, int y, int facing);

/* Powers on, starts a new game and lands in Central Area 1 without the
 * story, reusing a cached state in the data directory when there is one. */
bool emu_boot(void);

#endif
