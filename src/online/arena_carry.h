/* The run's MegaMan for the Arena (issue #120): the carried record read
 * from the data folder's checkpoint, its state loaded into a core of the
 * plain ROM that never runs (docs/ONLINE_LINK.md: a checkpoint's state is
 * read, not run, without its run's map rebuilt), its Souls the run's. */
#ifndef CW_ARENA_CARRY_H
#define CW_ARENA_CARRY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_snapshot.h"

/* False where there is no checkpoint, or its state cannot be read */
bool arena_carry_read(OnlineLoadout *out);
/* The checkpoint's state, its bytes (the caller frees them) and their
 * count; NULL where there is none */
uint8_t *arena_carry_state(size_t *size);

#endif
