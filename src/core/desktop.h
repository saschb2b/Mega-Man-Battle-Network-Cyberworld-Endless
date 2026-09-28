/* The desktop builds' dialogs: the ROM on the first start, and the
 * AppImage's entry in the application menu. They use the desktop's own
 * dialogs (zenity or kdialog, else SDL's) and are called before the game's
 * window is made. */
#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdbool.h>
#include <stddef.h>

/* No usable ROM was found in rom_dir (msg says why). Asks the player for
 * one: a file chooser (zenity or kdialog) or, without one, the folder
 * opened in the file manager; then a new look. 1 once a ROM is loaded, 0
 * if they quit, -1 when no dialog could be shown (the game's own window
 * then asks). scan looks again and fills msg. */
int desktop_rom_dialog(const char *rom_dir, bool (*scan)(char *msg, size_t msglen), char *msg, size_t msglen);

/* Run as an AppImage: offers to add it to the application menu (until it
 * is added or the player says not to ask again), and keeps an entry it
 * made pointing at the AppImage when the file moved. */
void desktop_menu_entry(const char *data_dir);

/* --add-to-steam, --remove-from-steam: runs the add-to-steam.py this build
 * carries (python3, on the host), or in the Flatpak says the command to
 * run outside it. The process's exit status. */
int desktop_steam_command(bool remove);

/* A build outside a sandbox, started from the desktop with Steam installed
 * and the game not in it: offers once to add it, with its artwork. */
void desktop_steam_offer(const char *data_dir);

/* Started by Steam's Gaming Mode or Big Picture, or in gamescope (a Steam
 * Deck, SteamOS on a TV): the game fills the screen there, where a window
 * would be scaled to it by a fraction and blur. */
bool desktop_big_screen(void);

/* Looks for the ROM where emulator front ends and downloads keep them
 * (EmuDeck's and RetroDECK's gba folders, on the home or an SD card,
 * ~/ROMs, the Downloads folder) and, found, copies it into rom_dir. True
 * once it is loaded. */
bool desktop_rom_elsewhere(const char *rom_dir, char *msg, size_t msglen);

#endif
