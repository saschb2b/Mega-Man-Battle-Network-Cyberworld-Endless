/* The Nintendo Switch's start (start_switch.c; switch/), before the game's. */
#ifndef CW_START_SWITCH_H
#define CW_START_SWITCH_H

#include <stdbool.h>
#include <stddef.h>

/* The sockets (online play, the statistics, nxlink), the SD card's folder
 * for the saves and the ROM (g_data_dir, where --data-dir gave none), and
 * stdout to the PC that sent the game over nxlink, else to log.txt beside
 * the saves */
void start_switch(bool data_dir_given);
/* The ROM: --rom-dir's, else the game's own rom folder, then where Switch
 * players keep GBA ROMs; `msg` where there is none. BN5's beside it, else
 * in those folders */
bool start_switch_rom(const char *rom_dir, char *msg, size_t n);

#endif
