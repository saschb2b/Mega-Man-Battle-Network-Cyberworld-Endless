/* BN4's Navis' attacks' art and pool songs in one room the battles share (docs/multirom/bn4-navis.md, Art and songs by
 * battle), as their battle sprites share theirs (xnavi_bn4_room.h): not eighteen Navis' sprites and songs resident, at
 * about 40 KB a Navi. The generator lays each Navi's out from the room's start (tools/xnavirecipe_bn4_art.py), the room
 * (XN_BN4_ART to XN_BN4_END, xnavi_recipe.h) the size of the largest. His install lists his sprites at their numbers,
 * pointed at his places, and his songs' headers at his pool's slots (xnavi_bn4_sounds.h), and writes no byte of the room;
 * his seat's byte of XN_BN4_ARTS names his own seat. When a battle's record is written (xnavi_bn4_room_for), before BN6 sets
 * the Navi up, the room is filled from the player's BN4 ROM with the art and songs of the seat that byte names, unpacked
 * where BN4 keeps a sprite LZ77'd, and each of that Navi's sprites' list entries pointed at its place (a number two Navis
 * list, each at a place of his own). What a family copied stays in the families' art, resident. */
#ifndef CW_XNAVI_BN4_ART_H
#define CW_XNAVI_BN4_ART_H

#include <stdbool.h>
#include <stdint.h>

#include "xvirus_recipe.h"

/* At a BN4 Navi's install, staged (xvirus_tx.h): where `s` is a row of the art's room, checked to fit its room in the
 * player's BN4 ROM (`rom`, its sprite lists at `lists`) and its list's entry pointed at its place where the entry is free
 * (one a Navi before him pointed into the room kept: the fill points it in each battle); true. False for any other row. */
bool xnavi_bn4_art_row(const XvSprite *s, const uint8_t *rom, uint32_t lists);
/* ... and his seat's byte of XN_BN4_ARTS his own (the step failed, xv_fail, where it is listed already) */
void xnavi_bn4_art_list(const XVirusFamily *f);
/* A battle's BN4 Navi at `seat`: the room filled with the art and songs of the seat its byte of XN_BN4_ARTS names, that
 * Navi's sprites' entries pointed at them (the core's copy alone); true where it was. Between frames. */
bool xnavi_bn4_art_for(int seat);

#endif
