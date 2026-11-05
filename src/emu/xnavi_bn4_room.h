/* BN4's Navis' battle sprites in one room the battles share (docs/multirom/bn4-navis.md, Battle sprites by battle): the
 * same rule as their sounds' pool (xnavi_bn4_sounds.h), not eighteen sprites resident. Each BN4 Navi's list 2 number
 * (XNAVI_SPRITE_BN4 + his seat) points at the room (XN_BN4_SPRITE, xnavi_recipe.h) from his install, and his BN4 sprite's
 * number is listed in the core's copy (XN_BN4_SPRITES, a byte a seat). When a battle's record is written
 * (emu_encounter_set), before BN6 sets the Navi up at his spawn (his init's sprite_load), the room is filled with the
 * battle's BN4 Navi's battle sprite, read from the player's BN4 ROM and unpacked where BN4 keeps it LZ77'd: BN6 draws a list
 * 2 sprite whose pointer has no bit 31 straight from it. One battle holds one BN4 Navi. */
#ifndef CW_XNAVI_BN4_ROOM_H
#define CW_XNAVI_BN4_ROOM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "xvirus_recipe.h"

/* Whether a recipe's sprite is a BN4 Navi's battle sprite in the room: then his install points his list 2 entry at it and
 * lists his sprite (xnavi_bn4_room_list), and copies nothing */
bool xnavi_bn4_room_sprite(const XvSprite *s);
/* At a BN4 Navi's install, staged (xvirus_tx.h): his battle sprite (`f`'s first, BN4's list 2) checked to fit the room in
 * the player's BN4 ROM (`rom`, its sprite lists at `lists`), his BN4 number listed at his seat; the step failed (xv_fail)
 * where it does not fit or his seat is listed already */
void xnavi_bn4_room_list(const XVirusFamily *f, const uint8_t *rom, uint32_t lists);
/* A battle's record written: its entities (4 bytes each, BN6's BattleSettings' from +0x20: byte 0 0x11 an enemy, bytes 2-3
 * its id; 0xF0 the end), `len` bytes. Where one is a BN4 Navi's id, the room filled with his battle sprite (the core's copy
 * alone); true where it was. Between frames. */
bool xnavi_bn4_room_for(const uint8_t *entities, size_t len);

#endif
