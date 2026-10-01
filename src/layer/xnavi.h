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
 * areas bring (NetAreaDef.xnavi), in their order, so each has the same
 * place in every session; again where the core's ROM copy was made anew. */
int xnavi_slot(int xrom, int navi, int fallback);

#endif
