/* The no-ROM screens (scene_norom.c), for main.c and the start. */
#ifndef CW_SCENE_NOROM_H
#define CW_SCENE_NOROM_H

#include <stdbool.h>
#include <stddef.h>

/* (desktops) the command line, which a ROM found restarts the game with;
 * the screen that says where to put the ROM */
extern char **g_argv;
void norom_show(void);
/* (iOS) the ROMs at the start: those kept, then the folder picked looked
 * in for what they lack */
bool ios_rom_start(char *msg, size_t msglen);
/* No ROM at the start: on iOS the screen that asks for it (but for a ROM
 * given by --rom-dir that is not one); elsewhere the plain error */
void rom_missing(const char *rom_dir, bool norom_scene, const char *msg);

#endif
