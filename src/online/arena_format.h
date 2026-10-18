/* The rules a NetBattle from the Net Square is fought by (docs/
 * ONLINE_ARENA.md, Formats): the pair's common ground, as a link battle
 * between Gregar and Falzar was by what both cartridges held. Two games
 * with BN5's ROM could fight by full rules (BN5's chips and Souls in both
 * pair cores), any other pair by BN6's: protocol.json's format byte, which
 * the challenge names and the server holds both to. Under BN6's rules the
 * MegaMan carried leaves home what plain BN6 has not (his Souls, BN5's
 * chips in the pack) and fights with a folder of BN6's chips alone, the
 * one his player picks, its BN5 chips stood in for (arena_standin.h). ROM-free: the carried record only. */
#ifndef CW_ARENA_FORMAT_H
#define CW_ARENA_FORMAT_H

#include <stdbool.h>
#include <stdint.h>

#include "online_snapshot.h"

/* Whether full rules are built: BN5 installed into both pair cores (the
 * next step of issue #120). Until then every pair fights by BN6's. */
#define ARENA_FULL_RULES 0

/* The rules for a NetBattle between a game whose ROMs (beyond BN6's) are
 * `mine` and one whose are `peer` (online_transport_roms, a peer's roms):
 * ONLINE_FORMAT_FULL where both have BN5's and full rules are built, else
 * ONLINE_FORMAT_BN6 */
unsigned arena_format_for(uint8_t mine, uint8_t peer);
/* ... whether a game of `mine` can fight by `format` */
bool arena_format_ok(unsigned format, uint8_t mine);

/* How folder `k` (0 Folder 1, 1 Folder 2, 2 the Extra folder) of `l` goes
 * under `format` */
typedef enum {
	ARENA_FOLDER_FITS,
	ARENA_FOLDER_EMPTY,   /* no folder there: not one of its 30 entries a chip */
	ARENA_FOLDER_BN5,     /* it holds BN5's chips, which sit out under BN6's rules, stood in for (arena_standin.h) */
} ArenaFolderFit;
ArenaFolderFit arena_folder_fit(const OnlineLoadout *l, unsigned format, int k);
/* ... the folders that can go, a bit each (bit k folder k): those holding
 * BN5's chips too under BN6's rules, by their stand-ins */
unsigned arena_folders_fitting(const OnlineLoadout *l, unsigned format);

/* The MegaMan carried into a NetBattle under `format`: `all` with folder
 * `k` (its Regular and TagChips) as the one he fights with; under BN6's
 * rules his Souls and BN5's chips in the pack left home */
void arena_format_carry(const OnlineLoadout *all, unsigned format, int k, OnlineLoadout *out);

#endif
