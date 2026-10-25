/* BN6's wait for the next frame, halted instead of spun (docs/engine/emulation/hooks.md,
 * Hooks; issue #35). */
#ifndef CW_IDLE_H
#define CW_IDLE_H

#include <stdbool.h>

struct mCore;

/* Its hook, once the core is up. */
void idle_install(void);
/* ... on another core's own table (hook_core): the Arena's pair, whose two
 * cores halt there as the game's does (docs/online/arena/rollback/overview.md) */
bool idle_on(struct mCore *core);

#endif
