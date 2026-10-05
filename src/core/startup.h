/* The start's data folder, ROMs and the player's files in it (startup.c),
 * for main.c and the no-ROM screens. */
#ifndef CW_STARTUP_H
#define CW_STARTUP_H

#include <stdbool.h>
#include <stddef.h>

/* The desktop builds (host, linux) open a window and keep their files in
 * the user's data folder; the handheld port fills the screen and keeps them
 * beside itself (its launcher passes --data-dir and --rom-dir). */
#ifdef CW_DESKTOP
#define DESKTOP true
#else
#define DESKTOP false
#endif
#ifdef CW_IOS
#define IOS true
#else
#define IOS false
#endif

/* The ROMs read, said at the start */
void say_roms(void);
void data_dir_setup(bool given);
/* (desktops) The ROM looked for again, and where front ends and downloads
 * keep theirs; `msg` where there is none */
bool desktop_rom_anywhere(char *msg, size_t msglen);
/* The ROM at the start (not the 3DS's): --rom-dir's, else the platform's */
bool start_rom(const char *rom_dir, char *msg, size_t msglen);
/* The player's files in the data folder, none headless (a test's input is
 * its own): the keys, the controllers' map, the settings, the touch
 * controls; --pad's virtual controller with pad.ini, headless too;
 * --smooth-motion's on or off (`smooth_arg`, -1 for neither) over
 * settings.ini */
void player_files(bool headless, int smooth_arg);

#endif
