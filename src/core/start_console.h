/* A console's own start, before the game's (main.c): the New 3DS's
 * (start_3ds.c), the Switch's (start_switch.c) and the PlayStation
 * Vita's (start_vita.c), each built where it runs; CW_CONSOLE says this
 * build is one. */
#ifndef CW_START_CONSOLE_H
#define CW_START_CONSOLE_H

#include <stdbool.h>
#include <stddef.h>

#if defined(__3DS__) || defined(__SWITCH__) || defined(__vita__)
#define CW_CONSOLE 1
#endif

/* The console's start: its folder for the saves and the ROM (g_data_dir,
 * where --data-dir gave none), its network and clocks, and stdout to the
 * PC that sent the game over the network, else to log.txt beside the
 * saves */
void console_start(bool data_dir_given);
/* The ROM: --rom-dir's, else the game's own rom folder, then where the
 * console's players keep GBA ROMs; `msg` where there is none. BN5's
 * beside it, or in those folders */
bool console_rom(const char *rom_dir, char *msg, size_t n);

#endif
