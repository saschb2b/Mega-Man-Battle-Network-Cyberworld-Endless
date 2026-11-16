/* The PortMaster port's update where its launcher is older than 0.22.0's
 * (update_install.c, docs/engine/updates.md's A PortMaster handheld): that
 * launcher gives the game no update folder and runs nothing after it, so
 * the game finds what port/update.py needs itself, the folder beside the
 * game's, the launcher that started it and python3, and runs update.py
 * as it ends. Plain C over the file system, so the unit tests run it on
 * the host (tests/test_update.c). */
#ifndef CW_UPDATE_PORT_H
#define CW_UPDATE_PORT_H

#include <stdbool.h>
#include <stddef.h>

/* The update folder beside the game's, as port/update.py and the 0.22.0
 * launcher name it: "/roms/ports/cyberworld" gives
 * "/roms/ports/.cyberworld-update". False where `game` is no absolute
 * path of a folder below the root, or the name does not fit */
bool update_port_dir(const char *game, char *dir, size_t n);
/* The launcher that started the game, from its shell's arguments (the
 * parent's /proc cmdline: `len` bytes, each argument ended by a NUL, and
 * a NUL at args[len]): the last that is an absolute path ending in ".sh",
 * a file in the folder that holds `game` (links followed: ROCKNIX's /roms
 * is /storage/roms). False where none is */
bool update_port_launcher(const char *args, size_t len, const char *game, char *launcher, size_t n);
/* `name`, a file that may be run, in one of `path`'s folders (PATH's
 * form, ':' between them; only absolute ones): false where none holds it */
bool update_port_find(const char *path, const char *name, char *found, size_t n);

#endif
