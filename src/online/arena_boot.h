/* The Arena's way in (issue #120, docs/ONLINE_ARENA.md): from the ROM to
 * both cores at a Practice NetBattle's Custom screen, with no state kept
 * anywhere. One core boots as the game's own boot does (src/emu/boot.c:
 * NEW GAME, then the game's warp into Seaside Area 1); each player's
 * MegaMan is written into a copy of it; then the pair, on the cable, goes
 * through BN6's own menus (Comm, Link Cable, NetBattle, Single Battle,
 * Practice) with scripted keys. Emulation is deterministic: every client
 * makes the same pair. Run in slices, as fast as the machine allows. */
#ifndef CW_ARENA_BOOT_H
#define CW_ARENA_BOOT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arena_pair.h"
#include "online_snapshot.h"

typedef enum {
	ARENA_BOOT_POWER,     /* the boot core from power-on to the overworld */
	ARENA_BOOT_ENTRY,     /* the pair through BN6's menus to Practice */
	ARENA_BOOT_READY,     /* both cores at the Custom screen: arena_boot_take */
	ARENA_BOOT_FAILED,    /* (the reason in `error`) */
} ArenaBootStage;

typedef struct ArenaBoot ArenaBoot;

/* A way in for the ROM's bytes; loadout[side] the MegaMan written into
 * that side's core (NULL: the boot's own, the game's start), both checked
 * first (arena_loadout_check); NULL where memory ran out */
ArenaBoot *arena_boot_new(const uint8_t *rom, size_t size, const OnlineLoadout *loadout[2]);
void arena_boot_free(ArenaBoot *boot);
/* Up to `frames` frames of the way (a frame of the boot core, or of the pair) */
ArenaBootStage arena_boot_run(ArenaBoot *boot, unsigned frames);
ArenaBootStage arena_boot_stage(const ArenaBoot *boot);
const char *arena_boot_error(const ArenaBoot *boot);
/* frames run so far, and roughly how many in all (for a progress bar) */
unsigned arena_boot_done(const ArenaBoot *boot);
unsigned arena_boot_total(void);
/* the pair, once READY: the caller's from then on */
ArenaPair *arena_boot_take(ArenaBoot *boot);

#endif
