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

/* Where the game's own output goes: on Android, whose apps' stdout and
 * stderr go nowhere, to logcat under "Cyberworld" (the online lines, the
 * Arena's hashes and stalls, frame_log's); elsewhere where it went */
void startup_output(void);
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
 * --smooth-motion's on or off (`smooth_arg`, -1 for neither) and
 * --sound's gba or soft (`sound_arg`, a SoundFilter, headless too) over
 * settings.ini; then the anonymous statistics' session (analytics_net.h),
 * a player's own where `plain` (no test's or developer's option) */
void player_files(bool headless, int smooth_arg, int sound_arg, bool plain);

#endif
