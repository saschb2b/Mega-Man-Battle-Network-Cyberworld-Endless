/* online_chips.h: which chips go online (docs/online/netpass.md, What
 * crosses): a NetBattle's folder may hold any chip this game has a record
 * of, BN6's own and the chip registry's (BN5's, and BN4's from protocol
 * v17), a pending one sitting out of the deck on both cores as in a run;
 * (arena_loadout_check asks it of every folder chip, the sender's game
 * and the receiver's); a trade, its proposal, its answer and the spares list take only a chip
 * that plays here, never one whose code is not moved yet
 * (docs/multirom/bn4-chips.md, Online). */
#ifndef CW_ONLINE_CHIPS_H
#define CW_ONLINE_CHIPS_H

#include <stdbool.h>

/* a chip this game has a record of: one of BN6's own, or a row of the
 * chip registry's (BN5's ids, BN4's), to protocol.json's chip_last */
bool online_chip_known(unsigned id);
/* ... and one that plays (chips_sits_out: not a chip whose code waits) */
bool online_chip_plays(unsigned id);

#endif
