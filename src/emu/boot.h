/* Getting the game from power-on to MegaMan standing in the net. */
#ifndef CW_BOOT_H
#define CW_BOOT_H

#include <stdbool.h>
#include <stdint.h>

/* Powers on, starts a new game and lands in Central Area 1 without the
 * story, reusing a cached state in the data directory when there is one. */
bool emu_boot(void);

#endif
