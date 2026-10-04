/* Another Battle Network game's backgrounds in BN6 (docs/MULTIROM.md): a
 * battle background's record, tiles, map, palette and animation scripts
 * copied from that game's ROM into BN6's free space, and entered after
 * BN6's 22 in copies of BN6's three background tables, which the game's
 * loader is pointed at; a battle's record names it by its number there.
 * And a net map's: its backdrop and its animations (palette banks
 * cycling, the backdrop's tiles turning), in the map its area's layers
 * take over. */
#ifndef CW_XBACKDROP_H
#define CW_XBACKDROP_H

/* Background `bg` (its number in extra ROM `xrom`'s own tables) in BN6:
 * the number a battle's record gives it, or `fallback` where it cannot be.
 * The first call copies in every background the other games' areas bring
 * (NetAreaDef.xbg), in their order; again where the core's ROM copy was
 * made anew. */
int xbackdrop_install(int xrom, int bg, int fallback);

/* The map net area `area`'s layers take over (net_area_def) set to move as
 * the area's own: for another game's area, its learned map's backdrop
 * (record, scroll callbacks) and GFXAnim scripts, copied; for BN6's, and
 * where the copy could not be made, BN6's own, every map another game's
 * areas take over set back. The first call copies in every other game's
 * area's, in their order; again where the core's ROM copy was made anew. */
void xbackdrop_map(int area);

#endif
