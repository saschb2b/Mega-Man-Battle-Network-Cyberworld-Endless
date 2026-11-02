/* A plain BN6 core of its own from power-on to the net, as the game's own
 * boot takes BN6's (src/emu/boot.c): the title, NEW GAME, the intro's
 * first line, the game's warp into Seaside Area 1, the intro ended as its
 * own end commands end it. Two hooks on the core's own table make the
 * game's call (gamecall.c's): at cbGameState the warp routine runs in the
 * update's place and returns into the free space past the game, where the
 * second goes back to the main loop. The state it ends in holds RAM alone,
 * so a core of the same ROM that no hook of the boot's touched loads it.
 * The Arena's way in (arena_boot.c) and Options' scenes (showcase.c)
 * begin with it. */
#ifndef CW_CORE_BOOT_H
#define CW_CORE_BOOT_H

#include <stdbool.h>
#include <stdint.h>

#include "hook.h"

struct mCore;

typedef struct {
	struct mCore *core;
	HookCore *hooks;
	unsigned step, in_step;
	unsigned ran;   /* frames run */
	struct { bool queued, called; uint32_t lr, kept[8]; } call;
} CoreBoot;

/* The boot of `core` (reset, not yet run) begun: its hooks set; false
 * where they cannot be */
bool core_boot_begin(CoreBoot *b, struct mCore *core);
/* One frame of it, its sound dropped: 1 while it runs on, 0 once MegaMan
 * stands on the net, -1 where the game's warp did not run */
int core_boot_frame(CoreBoot *b);
/* Its hooks off the core again (before a state of it is kept) */
void core_boot_end(CoreBoot *b);
/* Its frames in all (for a progress bar) */
unsigned core_boot_total(void);

#endif
