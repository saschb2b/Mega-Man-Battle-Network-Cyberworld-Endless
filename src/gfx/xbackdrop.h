/* Another Battle Network game's battle backgrounds in BN6
 * (docs/MULTIROM.md): a background's record, tiles, map, palette and
 * animation scripts copied from that game's ROM into BN6's free space, and
 * entered after BN6's 22 in copies of BN6's three background tables, which
 * the game's loader is pointed at; a battle's record names it by its
 * number there. */
#ifndef CW_XBACKDROP_H
#define CW_XBACKDROP_H

/* Background `bg` (its number in extra ROM `xrom`'s own tables) in BN6:
 * the number a battle's record gives it, or `fallback` where it cannot be.
 * The first call copies in every background the other games' areas bring
 * (NetAreaDef.xbg), in their order; again where the core's ROM copy was
 * made anew. */
int xbackdrop_install(int xrom, int bg, int fallback);

#endif
