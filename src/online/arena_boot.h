/* The Arena's way in (issue #120, docs/online/arena/): from the ROM to
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

/* A way in for the ROM's bytes under full rules (BN5's installs in the
 * boot core and the pair, arena_pair.h); loadout[side] the MegaMan
 * written into that side's core (NULL: the boot's own, the game's start),
 * both checked first (arena_loadout_check); NULL where memory ran out */
ArenaBoot *arena_boot_new(const uint8_t *rom, size_t size, const OnlineLoadout *loadout[2]);
void arena_boot_free(ArenaBoot *boot);
/* Up to `frames` frames of the way (a frame of the boot core, or of the pair) */
ArenaBootStage arena_boot_run(ArenaBoot *boot, unsigned frames);
ArenaBootStage arena_boot_stage(const ArenaBoot *boot);
const char *arena_boot_error(const ArenaBoot *boot);
/* frames run so far, and roughly how many in all (for a progress bar) */
unsigned arena_boot_done(const ArenaBoot *boot);
unsigned arena_boot_total(void);
/* a frame of the pair through the menus, its mean or its worst, in ms:
 * what a frame costs this system now (the rollback window it affords) */
double arena_boot_frame_ms(const ArenaBoot *boot, bool worst);
/* the pair, once READY: the caller's from then on */
ArenaPair *arena_boot_take(ArenaBoot *boot);

/* The bout's seed, before the way in runs (docs/online/arena/from-square/rematch-in-a-second.md, A
 * rematch in a second): BN6's two random words written on both GBAs as
 * Practice's last confirm is pressed, before the battle keeps the first
 * and shuffles the folders from the second, so that each bout's hands and
 * rolls are its own, the same on every game and spectator that builds it
 * with that seed (a NetBattle's: its match); 0 none, the way in's own */
void arena_boot_seed(ArenaBoot *boot, uint32_t seed);
/* The pair kept where the seed goes in, for a rematch (arena_boot_take_kept
 * once READY: the caller's from then on), before the way in runs */
void arena_boot_keep(ArenaBoot *boot);
ArenaPairSnap *arena_boot_take_kept(ArenaBoot *boot);
/* A way in again from a pair kept there (`pair` the last bout's, `kept`
 * its snapshot; both the boot's from then on, freed where it fails): the
 * pair put back, the seed written, the press and the battle's opening run
 * (244 frames of the pair, not 1,817), ready as a whole way in would be
 * for the same two MegaMen; loadout[side] as arena_boot_new's, for a
 * carried HP */
ArenaBoot *arena_boot_again(const OnlineLoadout *loadout[2], ArenaPair *pair, ArenaPairSnap *kept);

#endif
