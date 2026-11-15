/* A homebrew file put in its own place (update_install.c's Switch and
 * 3DS ways, docs/engine/updates.md's The consoles): the .nro or .3dsx the
 * game runs from, as its loader's argv[0] names it; the new one
 * downloaded beside it under another name, which the Homebrew Menu and
 * Launcher do not list, and swapped in only once the game has ended, the
 * running file never written. Plain C over stdio's files, so the unit
 * tests run it on the host (tests/test_update.c). */
#ifndef CW_UPDATE_SWAP_H
#define CW_UPDATE_SWAP_H

#include <stdbool.h>
#include <stddef.h>

/* The file the game runs from, from its loader's argv[0], where the game
 * can put a new one in its place: a path from the SD card's root
 * ("sdmc:/switch/cyberworld-endless.nro", or "/switch/..." on the card
 * the process stands on) ending in `ext` (".nro", ".3dsx", in any case),
 * a file there. False otherwise: a copy sent over the network (3dslink's
 * "3dslink:/"), no argv[0], another name */
bool update_swap_self(const char *arg0, const char *ext, char *self, size_t n);
/* The new one's place while it downloads and waits for the game's end:
 * `self` and ".update" */
void update_swap_staged(const char *self, char *staged, size_t n);
/* A start's tidying: a download an earlier start left, and an old file a
 * swap set aside ("self.old") where `self` is there again */
void update_swap_tidy(const char *self);
/* As the game ends: the old file set aside as "self.old", the staged one
 * renamed to its name (a FAT card renames nothing over a file that is
 * there), the old one removed; where the rename fails, the old one put
 * back. True where the new one is in place */
bool update_swap(const char *self);

#endif
