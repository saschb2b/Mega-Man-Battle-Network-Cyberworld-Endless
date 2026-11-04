/* BN6's map as it is drawn on the overworld: its tile map in RAM and the
 * screen maps its BGs are copied from each frame (docs/rom/net-maps.md, BN6's
 * map drawing). Read and written for a dark layer's board in BN5's look
 * (src/director/liberation_look.c). */
#ifndef CW_BN6_MAP_H
#define CW_BN6_MAP_H

/* BN6_MAP_GROUP (bn6.h): the internet's groups from here, the real
 * world's under it (bn6f's map groups; Lan's HP 0x88, a layer 0x8C) */
#define BN6_GROUP_INTERNET    0x80

/* The map's tile map, decompressed at map entry and read in place: a
 * 12-byte header (tw, th, 0, 0, the two layers' offsets), then tw x th
 * entries of layer 0 (BG1), then layer 1's (BG2) */
#define BN6_MAP_TILEMAP       0x02013A00u
#define BN6_MAP_TILEMAP_HEAD  12
#define BN6_MAP_TILEMAP_W     (BN6_MAP_TILEMAP + 0)   /* its header's width and height in tiles, a byte each */
#define BN6_MAP_TILEMAP_H     (BN6_MAP_TILEMAP + 1)
#define BN6_TILEMAP_COLOURS   0x02        /* a tile map's header +2 (bn6f's Unk_2, which no code of BN6's reads): 1 where its tiles are
                                           * 256-colour, the real world's (BN6's and BN5's) and every map of BN4's; 0 BN6's and BN5's net maps */
/* The BGs' screen maps in IWRAM, 32 x 32 entries each, copied to VRAM
 * every frame: what scrolls in is written here from the tile map, the map
 * tile t at t plus a constant (the scroll's: BG1 and BG2 scroll by
 * x + y + 256 and (y - x) / 2 - 256 of the world at the picture's middle,
 * measured), mod 32 */
#define BN6_BG0_SCREEN        0x030025C0u /* the chat box, the map's name */
#define BN6_BG1_SCREEN        0x03002DC0u /* the map's layer 0 */
#define BN6_BG2_SCREEN        0x030035C0u /* the map's layer 1 */
#define BN6_BG_SCREEN_SIZE    0x800
/* The BGs' controls, copied to their registers each frame (bn6f
 * render_800172C) from RenderInfo, which eToolkit names; the map's BG1 and
 * BG2 are 16-colour on the net, as BN6 sets them entering a map, 256-colour
 * on a layer in BN4's tiles (src/map/netmap.c, docs/rom/bn4.md, Net maps
 * in 256 colours) */
#define BN6_TOOLKIT_RENDER    0x08        /* eToolkit RenderInfoPtr: RenderInfo (bn6f eRenderInfo, 0x0200AC40) */
#define BN6_RENDER_BGCNT      0x04        /* RenderInfo +4: BG0CNT to BG3CNT, a u16 each */
#define BN6_BGCNT_256         0x0080      /* a BG control's 256-colour bit (64 bytes a tile, its pixels colour indices) */
#define BN6_BGCNT_NET_BG1     0x3D03      /* the net's map in RenderInfo, BG1: priority 3, tiles from 0x06000000, its map at 0x0600E800, bit 13
                                           * set (which BG1's register drops: it reads 0x1D03); BG2 the same tiles, priority 2, its map at
                                           * 0x0600F000 (seen on BN6's, BN5's and BN4's layers alike) */
#define BN6_BGCNT_NET_BG2     0x3E02
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

/* BN6's encounter roll (bn6f sub_80AA4C0, bn6.h's BN6_ENCOUNTER_*) checks once each 0x40 of BN6_STEPS_WALKED:
 * odds out of 32 from a row by that walk and a column by the map's class (src/director/battle_floor.c) */
#define BN6_ENCOUNTER_ODDS    0x08020C5Cu /* bn6f byte_8020C5C: 17 rows of 8, a row each 0x40 walked (the last from 0x400), a column a class */
#define BN6_ENCOUNTER_CLASS   0x08020CE4u /* bn6f byte_8020CE4: each net map's class, at (MapGroup - 0x80) * 16 + MapNumber: 0 the most battles, 7 none */

#endif
