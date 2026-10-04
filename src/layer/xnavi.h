/* Another Battle Network game's Navis on its areas' layers
 * (docs/MULTIROM.md): a Navi's overworld sprite and its mugshot, copied
 * from that game's ROM into BN6's free space and listed at a number Gregar
 * leaves empty in both its list 6 and its mugshots (Falzar's Navis'), so
 * the sprite's number is its face's, as BN6 has it. */
#ifndef CW_XNAVI_H
#define CW_XNAVI_H

#include <stdint.h>

/* A sprite's length from its header (`hdr`, the 4 bytes before its
 * animation table, `n` bytes readable from it): the furthest its
 * animations, frames, tiles, palettes and object lists reach, 0 where
 * they reach past `n`. */
uint32_t xnavi_sprite_len(const uint8_t *hdr, uint32_t n);

/* Navi `navi` (its number in list 6 of extra ROM `xrom`) in BN6: the
 * list-6 number that shows it, its mugshot the same, or `fallback` where
 * it cannot be. The first call copies in every Navi the other games'
 * areas bring (NetAreaDef.xnavi and xnavi2), in their order, so each has
 * the same place in every session; again where the core's ROM copy was
 * made anew. */
int xnavi_slot(int xrom, int navi, int fallback);

/* A guardian of another game (docs/BOSSES.md, BN5's Navis): Navi `navi`
 * of ROM `xrom` (list 6 and its mugshot) copied into a place of its own,
 * past the other Navis and objects, and listed at a number Gregar leaves
 * empty that no area's Navi takes; that number, -1 where it cannot be.
 * Each call copies him anew: one guardian at a time. */
int xnavi_guardian(int xrom, int navi);

/* Another game's map objects in BN6 (docs/MULTIROM.md): sprites of its list
 * 7, each listed at one of Gregar's list-7 numbers that point at a
 * placeholder no layer lists (docs/ROM_DATA.md). */
enum {
	XOBJ_DARK_FLAME,   /* BN5's flame of darkness, its palette turned purple: the DarkChips' (docs/META.md) */
	XOBJ_CUBE,         /* BN5's Security Cube, its P-Codes' lock: standing (animation 0), opening (1) */
	XOBJ_DARK_WALL,    /* BN5's wall of dark flames, across a walkway along world Y or X (0, 1), burning out (2, 3) */
	XOBJ_DARK_HOLE,    /* BN5's dark hole, a vortex standing past a platform's edge */
	XOBJ_COUNT
};

/* Object `which` (XOBJ_*) in BN6: the list-7 number that shows it, -1
 * where it cannot be (its game's ROM not read, no room). The first call
 * copies in every one whose game's ROM is read, in their order after the
 * Navis; again where the core's ROM copy was made anew. */
int xnavi_object(int which);

/* BN6's own flame of darkness (docs/META.md, BN6's own DarkChips): its blue
 * flame of sprite list 7 copied into a place of its own, its palette turned
 * purple as BN5's flame is, and listed at a list-7 number Gregar leaves on
 * its placeholder; that number, -1 where it cannot be. No other game's ROM
 * is needed (the 3DS's runs meet it too). */
int xnavi_bn6_flame(void);

#endif
