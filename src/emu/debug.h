/* CYBERWORLD_EMU_DEBUG: the game's state on stderr while it runs;
 * CYBERWORLD_WATCH: bytes of its memory, printed as they change. */
#ifndef CW_DEBUG_H
#define CW_DEBUG_H

#include <stdbool.h>
#include <stdio.h>

bool emu_debug_on(void);
/* Once a frame: mode, map, position and music every 30-60 frames; the
 * watched bytes where they changed. */
void emu_debug_frame(void);
/* A file in the data directory for dumps ("wb"); NULL when it cannot be made. */
FILE *emu_debug_file(const char *name);

#endif
