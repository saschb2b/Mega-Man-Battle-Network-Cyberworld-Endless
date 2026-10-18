/* A carried MegaMan in an Arena core (issue #120, docs/ONLINE_ARENA.md):
 * the OnlineLoadout a run's checkpoint gives (online_snapshot.h) written
 * into BN6's own RAM, where the game keeps them, through bn6.h's names:
 * the first folder, the pack, the NaviCust's board, HP and the buster.
 * Under BN6's rules the Arena's cores are plain BN6, so a MegaMan who
 * holds what plain BN6 cannot run (another game's chips, the Souls) is
 * refused, with the reason; under full rules they hold BN5's chips and
 * Souls too (arena_pair.h). The rules decide, never this game's ROMs, so both games
 * check a MegaMan alike. ROM-free: the core's memory through two calls. */
#ifndef CW_ARENA_LOADOUT_H
#define CW_ARENA_LOADOUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_snapshot.h"

/* BN6's own chip ids: 1 to this, below the pack's extension (BN6_PACK_IDS) */
#define ARENA_BN6_CHIP_LAST 0x13Fu

/* An Arena core's memory */
typedef struct {
	uint32_t (*read32)(void *context, uint32_t address);
	void (*write8)(void *context, uint32_t address, uint8_t value);
	void *context;
} ArenaMemory;

/* True where `format`'s cores (ONLINE_FORMAT_*, arena_format.h) can carry
 * `l`; else false and the reason in `why` */
bool arena_loadout_check(const OnlineLoadout *l, unsigned format, char *why, size_t n);
/* `l` into the core (checked first): false where it is refused, or the
 * core's folder and pack are not where the game keeps them yet */
bool arena_loadout_install(const ArenaMemory *m, const OnlineLoadout *l, unsigned format, char *why, size_t n);

#endif
