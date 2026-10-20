/* The MegaMan a NetBattle from the Net Square carries (docs/ONLINE_ARENA.md,
 * The folder). Every NetBattle is fought by full rules since protocol v9
 * (issue #131): both games hold the full NetPass (netpass.h), so BN5's
 * chips and Souls go in both pair cores (arena_pair.h). The player picks
 * the folder carried where more than one holds chips. ROM-free: the
 * carried record only. */
#ifndef CW_ARENA_FORMAT_H
#define CW_ARENA_FORMAT_H

#include <stdbool.h>
#include <stdint.h>

#include "online_snapshot.h"

/* The folders of `l` that can go, a bit each (bit k folder k: 0 Folder 1,
 * 1 Folder 2, 2 the Extra folder): those that hold a chip in one of
 * their 30 entries */
unsigned arena_folders_fitting(const OnlineLoadout *l);

/* The MegaMan carried into a NetBattle: `all` with folder `k` (its Regular
 * and TagChips) as the one he fights with */
void arena_format_carry(const OnlineLoadout *all, int k, OnlineLoadout *out);

#endif
