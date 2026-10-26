/* A carried MegaMan in an Arena core (issue #120, docs/online/arena/):
 * the OnlineLoadout a run's checkpoint gives (online_snapshot.h) written
 * into BN6's own RAM, where the game keeps them, through bn6.h's names:
 * the first folder, the pack, the NaviCust's board, HP and the buster.
 * The Arena's cores hold BN5's chips and Souls too (full rules, every
 * NetBattle's: arena_pair.h); a MegaMan whose record holds what no core
 * can (a chip id past the registry, a code past '*', HP past BN6's) is
 * refused, with the reason, both games alike. ROM-free: the core's memory
 * through two calls. */
#ifndef CW_ARENA_LOADOUT_H
#define CW_ARENA_LOADOUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_snapshot.h"


/* An Arena core's memory */
typedef struct {
	uint32_t (*read32)(void *context, uint32_t address);
	void (*write8)(void *context, uint32_t address, uint8_t value);
	void *context;
} ArenaMemory;

/* True where the pair's cores can carry `l`; else false and the reason
 * in `why` */
bool arena_loadout_check(const OnlineLoadout *l, char *why, size_t n);
/* `l` into the core (checked first): false where it is refused, or the
 * core's folder and pack are not where the game keeps them yet */
bool arena_loadout_install(const ArenaMemory *m, const OnlineLoadout *l, char *why, size_t n);

#endif
