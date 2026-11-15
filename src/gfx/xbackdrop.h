/* Another Battle Network game's backgrounds in BN6 (docs/multirom/): a
 * battle background's record, tiles, map, palette and animation scripts
 * copied from that game's ROM into BN6's free space, and entered after
 * BN6's 22 in copies of BN6's three background tables, which the game's
 * loader is pointed at; a battle's record names it by its number there.
 * And a net map's: its backdrop and its animations (palette banks
 * cycling, the backdrop's tiles turning), in the map its area's layers
 * take over. */
#ifndef CW_XBACKDROP_H
#define CW_XBACKDROP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

/* Where BN6 loads the backdrop of map (group, number) as set now, its
 * tiles' VRAM [*lo, *hi) (bus addresses); false where it has none. */
bool xbackdrop_map_vram(int group, int number, uint32_t *lo, uint32_t *hi);

/* The BGAnimData record (a bus address) BN6 loads map (group, number)'s
 * backdrop from as set now; and the one net area `area`'s own map has:
 * BN6's own as its ROM has it, or another game's copied in by
 * xbackdrop_map. 0 where none: a Glitch layer's look flips the backdrop
 * with the floor through BN6's LoadBGAnimData (BN6_LOAD_BG_ANIM_DATA). */
uint32_t xbackdrop_map_record(int group, int number);
uint32_t xbackdrop_area_record(int area);

/* The colours another game's net area `area` shows from its map's entry:
 * the first frame of each palette script its learned map runs (its
 * GFXAnim list, read from that game's ROM), laid over `pal` (BN6's BG
 * palette buffer's layout, `npal` bytes), as a Glitch layer's look of it
 * takes them (docs/game/glitch-layer.md): a bank its map stores blank
 * takes its colours there (BN5's Undernet's skull totems). How many
 * scripts; 0 for BN6's own areas. */
int xbackdrop_palette_first(int area, uint8_t *pal, size_t npal);

#endif
