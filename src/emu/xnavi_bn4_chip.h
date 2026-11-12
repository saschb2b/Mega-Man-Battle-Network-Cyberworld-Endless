/* A BN4 Navi chip's Navi in rooms of the chips' own (docs/multirom/bn4-chips.md,
 * The Navi chips; docs/multirom/bn4-navis.md, Battle sprites by battle): a
 * BN4 Navi's battle sprite, his attacks' art and his pool's songs are in the
 * core's copy for the battle he fights in alone (xnavi_bn4_room.h,
 * xnavi_bn4_art.h), one room each that the battles share. His chip
 * (src/emu/xchips_bn4.c's units of kind XCHIP_NAVI: the chip's own Navi,
 * who runs the moved Navi's attack objects and draws his sprite) may be
 * used in any battle, so the chips have a room each of their own
 * (FREE_BN4_CHIP_SPRITE, FREE_BN4_CHIP_ART, the art's laid out as the
 * battle's room is), filled from the player's BN4 ROM as a chip's Navi
 * object begins its own states, thirty frames before BN6 spawns the Navi:
 * his list 2 entry and his art's entries pointed there, and the pool's
 * slots at his songs while the object lives (xnavi_bn4_sounds.h). Where the
 * battle's rooms hold that Navi already (his chip used against him, or
 * after him), nothing is copied and the entries name the battle's rooms.
 * What the rooms hold is kept in the core's copy (XN_BN4_HELD), so a core
 * knows its own. */
#ifndef CW_XNAVI_BN4_CHIP_H
#define CW_XNAVI_BN4_CHIP_H

#include <stdbool.h>
#include <stdint.h>

/* Before each frame of BN6's core (xinstall.c's step, before the pool's
 * pointing): the rooms filled for the BN4 Navi chip whose Navi object runs
 * its own states, the pool's override set while it does and ended after. */
void xnavi_bn4_chip_frame(void);

#endif
