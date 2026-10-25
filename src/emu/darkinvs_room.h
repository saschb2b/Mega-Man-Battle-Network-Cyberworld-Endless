/* BN5's chip AI's room for a MegaMan in a NetBattle (issue #182;
 * docs/online/arena/two-a-side/full-rules.md): BN5's
 * chip AI keeps its state past BN6's 0x100 bytes of AI data, so a
 * MegaMan under it needs BN5's 0x140 (BN5_AI_DATA_SIZE). Side 0's player
 * has RAM_PLAYER_AI from his set-up (darkinvs.c). The others of a
 * NetBattle (by seat: side x 2 + 0 a side's player, + 1 its partner,
 * BN6_BATTLE_PLAYERS' words) each have a room of their own at the end
 * of BN6's AI data's pool (bn6_ai.h), its entries taken from the pool as
 * he first needs it: never under another's AI data, and never let go
 * nor laid out again by BN6 until the next battle sets the pool up.
 * Side 1's player's (a 1v1 NetBattle's other MegaMan) the last two
 * entries'; side 0's partner's and side 1's partner's below it. */
#ifndef CW_DARKINVS_ROOM_H
#define CW_DARKINVS_ROOM_H

#include <stdbool.h>
#include <stdint.h>

#include "bn5.h"
#include "bn6.h"

#define DARKINVS_SEATS 4   /* side 0's player and partner, side 1's player and partner */

/* how far down from the pool's end seat's room ends: 0 side 1's player's, 1 side 0's partner's, 2 side 1's partner's; -1
 * side 0's player, whose block is RAM_PLAYER_AI, or none */
static inline int darkinvs_room_rank(unsigned seat) { return seat == 2 ? 0 : seat == 1 ? 1 : seat == 3 ? 2 : -1; }

/* seat's room: BN5_AI_DATA_SIZE bytes in the pool's last entries; 0 for side 0's player or none */
static inline uint32_t darkinvs_room(unsigned seat) {
	int rank = darkinvs_room_rank(seat);
	return rank < 0 ? 0 : BN6_AI_POOL + BN6_AI_POOL_ENTRIES * BN6_AI_DATA_SIZE - (uint32_t)(rank + 1) * BN5_AI_DATA_SIZE;
}

/* the pool's entries seat's room lies on, as BN6_AI_POOL_USED's bits (entry i bit 31 - i) */
static inline uint32_t darkinvs_room_entries(unsigned seat) {
	uint32_t at = darkinvs_room(seat), bits = 0;
	if (!at) return 0;
	for (uint32_t i = (at - BN6_AI_POOL) / BN6_AI_DATA_SIZE; i <= (at + BN5_AI_DATA_SIZE - 1 - BN6_AI_POOL) / BN6_AI_DATA_SIZE; ++i)
		bits |= 1u << (31 - i);
	return bits;
}

/* seat's room taken from the pool whose taken entries are `used`, of which `rooms` the rooms took before: false where an
 * entry it lies on is another actor's (the pool full of actors); else the pool's bits and the rooms' after it */
static inline bool darkinvs_room_take(unsigned seat, uint32_t used, uint32_t rooms, uint32_t *used_after, uint32_t *rooms_after) {
	uint32_t need = darkinvs_room_entries(seat);
	if (!need || (used & need & ~rooms)) return false;
	*used_after = used | need;
	*rooms_after = rooms | need;
	return true;
}

#endif
