/* The battle floor: at least one random battle on every layer without a
 * guardian (docs/game/progression/structure.md, The battle floor). BN6's
 * step counter still decides when it comes, its odds raised on a layer
 * still without one; the few layers it misses hold their exit pad shut
 * until the battle due there is fought. */
#ifndef CW_BATTLE_FLOOR_H
#define CW_BATTLE_FLOOR_H

#include <stdbool.h>

/* Whether the layer still owes its battle (none fought on it yet) */
bool floor_owed(void);
/* Once a frame on the map: past the floor's share of the walk with no
 * battle yet, BN6's encounter roll reads its longest walk's odds */
void floor_walk(void);
/* MegaMan on the exit pad of a layer that owes its battle: it starts
 * there, the pad opening after it (true: it started, or is starting) */
bool floor_at_exit(void);

#endif
