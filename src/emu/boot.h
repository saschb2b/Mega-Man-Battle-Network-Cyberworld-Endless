/* Getting the game from power-on to MegaMan standing in the net. */
#ifndef CW_BOOT_H
#define CW_BOOT_H

#include <stdbool.h>
#include <stdint.h>

/* Powers on, starts a new game and lands in Central Area 1 without the
 * story, reusing the state kept in the data directory where there is one
 * and it holds a new game (a played one's made again, issue #688).
 * `from`: that state read from another file (NULL its own), a test's. */
bool emu_boot(const char *from);

#endif
