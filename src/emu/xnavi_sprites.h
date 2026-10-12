/* BN5's Navis' battle sprites in BN6's engine (issue #82): one numbering
 * for every track that draws a Navi of BN5's in BN6's battles (the Navis
 * themselves, their Navi chips' summons, a Soul's image, which is its
 * Navi's battle sprite): BN5's Navi AI n (Team Colonel's: 7 Colonel, 8
 * ShadowMan, 9 NumberMan, 10 TomahawkMan, 11 KnightMan, 12 ToadMan) at
 * BN6's sprite list 2 number 0x20 + (n - 7), the Dark Soul (BN5's AI 0x16)
 * at 0x26; others from 0x27 as they come. Each copied whole (BN6's battle
 * reads list 2 as it is) to a fixed place of its own in FREE_NAVI_SPRITES,
 * the same at every boot with the same ROMs, with its arm points' record:
 * BN6 finds an object's arm points by its struct1's list and number
 * (BN6_ARM_POINTS), so the Navis' table of records is grown to every
 * number of list 2 and pointed at, and each sprite's record is BN5's 30
 * points with BN6's four more (docs/ROM_DATA.md, The Navis' battle
 * sprites). */
#ifndef CW_XNAVI_SPRITES_H
#define CW_XNAVI_SPRITES_H

#include <stdbool.h>
#include <stdint.h>

#include "free_space.h"

#define XNAVI_SPRITE_FIRST 0x20                        /* list 2's number of BN5's AI 7 (Colonel), the six's first */
#define XNAVI_ARMS (FREE_NAVI_SPRITES + 0x7B000u)      /* the Navis' arm points' records grown, XNAVI_ARMS_COUNT of BN6_ARM_RECORD bytes */
#define XNAVI_ARMS_COUNT 256                           /* (every number of list 2, a byte) */
#define XNAVI_SPRITES_END (FREE_NAVI_SPRITES + 0x80000u)
/* Who the sprites' bytes are (claims.h) */
#define XNAVI_SPRITES_CLAIMS "the Navis' battle sprites (xnavi_sprites.c)"

/* The list 2 number BN5's Navi of AI index n takes in BN6, -1 where none
 * has a place (yet). */
int xnavi_sprite_number(int n);
/* BN5's Navi n's battle sprite (its struct1's: list 2's n for Team
 * Colonel's six, BN5's MegaMan's own for the Dark Soul, darkened by its
 * palette) copied into the core's copy at its place, its arm points'
 * record written and its number in BN6's grown list 2 pointed at it (the
 * families' room grown first: xvirus_grow.h): true once it is, now or
 * before; false, nothing written, where BN5's ROM is not read or a check
 * failed (the log says which). Not within a staged step (xvirus_tx.h). */
bool xnavi_sprite_install(int n);

#endif
