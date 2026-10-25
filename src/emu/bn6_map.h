/* BN6's map as it is drawn on the overworld: its tile map in RAM and the
 * screen maps its BGs are copied from each frame (docs/rom/net-maps.md, BN6's
 * map drawing). Read and written for a dark layer's board in BN5's look
 * (src/director/liberation_look.c). */
#ifndef CW_BN6_MAP_H
#define CW_BN6_MAP_H

/* The map's tile map, decompressed at map entry and read in place: a
 * 12-byte header (tw, th, 0, 0, the two layers' offsets), then tw x th
 * entries of layer 0 (BG1), then layer 1's (BG2) */
#define BN6_MAP_TILEMAP       0x02013A00u
#define BN6_MAP_TILEMAP_HEAD  12
/* The BGs' screen maps in IWRAM, 32 x 32 entries each, copied to VRAM
 * every frame: what scrolls in is written here from the tile map, the map
 * tile t at t plus a constant (the scroll's: BG1 and BG2 scroll by
 * x + y + 256 and (y - x) / 2 - 256 of the world at the picture's middle,
 * measured), mod 32 */
#define BN6_BG0_SCREEN        0x030025C0u /* the chat box, the map's name */
#define BN6_BG1_SCREEN        0x03002DC0u /* the map's layer 0 */
#define BN6_BG2_SCREEN        0x030035C0u /* the map's layer 1 */
#define BN6_BG_SCREEN_SIZE    0x800
/* The map's coordinate data, decompressed at map entry and read in place
 * (docs/rom/net-maps.md, Map buffers): its walls' section first, a u32 count n,
 * then n entries of a u16 cell key ((cy + 127) * 254 + cx + 127, 8-unit
 * world cells, ascending) and a u16 offset, then n shapes of 4 bytes (z,
 * value, height, type; src/map/coords.c). A wall whose value has bit 7 set
 * is skipped while event 0x1640 + (value & 0x7F) is set */
#define BN6_MAP_COORDS        0x02027A00u
#define BN6_COORD_ENTRIES     (BN6_MAP_COORDS + 4) /* the walls' section's entries, after its count */
#define BN6_COORD_ENTRY       4           /* an entry's bytes, and a shape's */
#define BN6_COORD_KEY_SPAN    254
#define BN6_COORD_VALUE       0x01        /* in a shape */

#endif
