/* The 3DS's start (start_3ds.c; issue #9, 3ds/), before the game's. */
#ifndef CW_START_3DS_H
#define CW_START_3DS_H

#include <stdbool.h>
#include <stddef.h>

/* The New 3DS's faster clock and cache, the SD card's folder for the saves
 * and the ROM (g_data_dir, where --data-dir gave none), and stdout to the
 * PC that sent the game over 3dslink, else to log.txt beside the saves */
void start_3ds(bool data_dir_given);
/* The ROM: --rom-dir's, else the game's own rom folder, then where 3DS
 * players keep GBA ROMs; `msg` where there is none */
bool start_3ds_rom(const char *rom_dir, char *msg, size_t n);

#endif
