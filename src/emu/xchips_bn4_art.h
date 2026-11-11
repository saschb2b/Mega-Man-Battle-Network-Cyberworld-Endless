/* BN4's chip units' art in a room filled on demand (docs/multirom/bn4-chips.md, The art's room; docs/engine/emulation/memory.md):
 * a unit's sprites are about 13 KB and BN4 has some 110 units, where the core's copy has 127 KB for them (XC4_ART to
 * XC4_END, xchips_bn4.h), so no unit's art is installed. A sprite is put into the room as BN6 loads it for an object
 * (BN6_SPRITE_LOAD, the one routine every object's sprite comes through), read from the player's BN4 ROM and unpacked where
 * BN4 keeps it LZ77'd, and its list's entry pointed at it.
 *
 * The room keeps no record of what it holds that a state would have to save. A sprite's address names it: the room is
 * cut in slots (XC4_SLOT bytes), and the recipe's row k (XC4_ROOM_ROW(k), its number over every unit's rows) sits at a
 * slot's start + 4 x k. So the sprites in use are read from the objects in play, each one's sprite state's pointer (the
 * first, third and fourth kinds' objects), the same on both cores of a NetBattle and right again after a rollback or a
 * loaded state. A sprite an object in play holds is found at its place; a new one takes the lowest run of slots none in
 * play lies in, over whatever no object draws any more; where every slot is in play its entry stays BN6's placeholder
 * (BN6_SPRITE_PLACEHOLDER), as BN6 leaves an unloaded sprite.
 *
 * What the room's bytes hold is noted in the room itself (XC4_ROOM_TAGS: a u16 a slot, the row it holds a part of + 1), in
 * the core's copy as the bytes are (the Arena's cores share one copy on a 3DS): each battle frame a sprite an object in
 * play points at is copied again where its slots hold another's (xchips_bn4_art_frame's hook). */
#ifndef CW_XCHIPS_BN4_ART_H
#define CW_XCHIPS_BN4_ART_H

#include <stdbool.h>
#include <stdint.h>

#include "xchips_bn4.h"
#include "xvirus_recipe.h"

#define XC4_ROOM        XC4_ART                       /* the room: its slots' tags, then its slots, to XC4_END */
#define XC4_ROOM_TAGS   0x100u                        /* ... a u16 a slot */
#define XC4_SLOT        0x800u
#define XC4_SLOTS       ((XC4_END - XC4_ROOM - XC4_ROOM_TAGS) / XC4_SLOT)
#define XC4_ROOM_SLOT(i) (XC4_ROOM + XC4_ROOM_TAGS + XC4_SLOT * (uint32_t)(i))
_Static_assert(XC4_SLOTS <= 63 && 2u * XC4_SLOTS <= XC4_ROOM_TAGS, "the art's room: its slots a bit each of 64, a tag each");
#define XC4_ROWS        (XC4_SLOT / 4u)               /* the rows a slot's start can tell apart */
/* a row's place in a recipe (XvSprite.at): row k's address in the first slot, which names it whichever slot it is in */
#define XC4_ROOM_ROW(k) (XC4_ROOM_SLOT(0) + 4u * (uint32_t)(k))

/* The slots a row's sprite of `size` bytes takes (it starts 4 x row into its first) */
static inline int xchips_bn4_art_slots(int row, uint32_t size) { return (int)((4u * (uint32_t)row + size + XC4_SLOT - 1) / XC4_SLOT); }
/* The first of the lowest run of `n` slots free in `used` (bit i slot i in play) among `slots`, -1 none */
static inline int xchips_bn4_art_place(uint64_t used, int n, int slots) {
	if (n < 1 || n > slots || slots > 63) return -1;
	for (int s = 0; s + n <= slots; ++s)
		if (!(used & ((((uint64_t)1 << n) - 1) << s))) return s;
	return -1;
}

/* At a unit's install, staged (xvirus_tx.h): where `s` is a row of the room (its `at` an XC4_ROOM_ROW), checked to be a
 * sprite of the player's BN4 ROM (`rom`, its sprite lists at `lists`) that fits the room, its list's entry pointed at
 * BN6's placeholder until an object loads it; true. False for any other row. */
bool xchips_bn4_art_row(const XvSprite *s, const uint8_t *rom, uint32_t lists);
/* The room's hooks, once the units are installed: BN6's sprite load and the battle's frame; false where a hook found no
 * place. */
bool xchips_bn4_art_install(void);

#endif
