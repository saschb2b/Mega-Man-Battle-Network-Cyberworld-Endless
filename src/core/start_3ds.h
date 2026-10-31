/* The 3DS's start (start_3ds.c; issue #9, 3ds/), before the game's. */
#ifndef CW_START_3DS_H
#define CW_START_3DS_H

#include <stdbool.h>
#include <stddef.h>

/* (its console_start and console_rom, start_console.h: the New 3DS's
 * faster clock and cache, the SD card's folder, stdout to 3dslink or
 * log.txt; BN5's ROM read where the heap has room for it) */
/* Where BN5's ROM lies, found but left unread for want of room in the
 * heap; NULL where it was read or is not there */
const char *start_3ds_bn5_unread(void);
/* The heap's use and its high-water mark, for log.txt: "heap N KB in use,
 * M KB at most of H KB" */
void start_3ds_heap(char *out, size_t n);

#endif
