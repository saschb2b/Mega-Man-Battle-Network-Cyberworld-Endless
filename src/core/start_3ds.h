/* The 3DS's start (start_3ds.c; issue #9, 3ds/), before the game's. */
#ifndef CW_START_3DS_H
#define CW_START_3DS_H

#include <stdbool.h>
#include <stddef.h>

/* (its console_start and console_rom, start_console.h: the New 3DS's
 * faster clock and cache, the SD card's folder, stdout to 3dslink or
 * log.txt; the extra ROMs, BN5's and BN4's, read beside BN6's) */
/* The heap's use and its high-water mark, for log.txt: "heap N KB in use,
 * M KB at most of H KB" */
void start_3ds_heap(char *out, size_t n);

#endif
