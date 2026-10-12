/* BN5's Navis' battle sprites in BN6's sprite list 2 (issue #82; one rule
 * for every track that needs them: a Navi fought in BN6's engine, a Navi
 * chip, a Soul's image as MegaMan unites): BN5's list-2 sprite of Navi
 * AI index n (Team Colonel's, 7 Colonel to 12 ToadMan) copied from BN5's
 * ROM to a fixed place of its own in FREE_NAVI_SPRITES and listed at BN6's
 * list-2 number XNAVI_SPRITE_FIRST + (n - 7), in the families' room's grown
 * list 2 (xvirus_grow.h), once a core. */
#ifndef CW_XNAVI_SPRITES_H
#define CW_XNAVI_SPRITES_H

#include <stdbool.h>

#include "free_space.h"

#define XNAVI_SPRITE_FIRST_AI 7      /* the first Navi AI index of BN5's taken (Colonel) */
#define XNAVI_SPRITE_LAST_AI  12     /* ... the last (ToadMan) */
#define XNAVI_SPRITE_FIRST    0x20   /* BN6's list-2 number of the first */
#define XNAVI_SPRITE_ROOM     0x15000u   /* each one's room in FREE_NAVI_SPRITES (KnightMan's, the largest, 0x14D94 bytes) */
#define XNAVI_SPRITE_CLAIMS "BN5's Navis' battle sprites (xnavi_sprites.c)"   /* whose bytes they are (claims.h) */

/* BN6's list-2 number of BN5 Navi AI index n, -1 for none of them */
int xnavi_sprite_number(int n);
/* BN5 Navi n's battle sprite in BN6's list 2 (BN6's tables grown first
 * where they are not): true once it is there (now, or before by any
 * caller); false, nothing written, where BN5's ROM is not read, its sprite
 * cannot be read or does not fit, or the list's number holds another's. */
bool xnavi_sprite_install(int n);

#endif
