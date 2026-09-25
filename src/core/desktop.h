/* The desktop builds' dialogs: the ROM on the first start, and the
 * AppImage's entry in the application menu. */
#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdbool.h>
#include <stddef.h>

/* No usable ROM was found in rom_dir (msg says why). Asks the player for
 * one: a file chooser (zenity or kdialog), the folder opened in the file
 * manager, or a new look. True once a ROM is loaded; false if they quit.
 * scan looks again and fills msg. */
bool desktop_rom_dialog(const char *rom_dir, bool (*scan)(char *msg, size_t msglen), char *msg, size_t msglen);

/* Run as an AppImage: offers once to add it to the application menu, and
 * keeps an entry it made pointing at the AppImage when the file moved. */
void desktop_menu_entry(const char *data_dir);

#endif
