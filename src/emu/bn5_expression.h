/* Battle Network 5: Team Colonel (USA): what it has to show a character's
 * mood where BN6 has the like or lacks it, beside bn5.h's addresses (it
 * includes this; bn6_expression.h has BN6's). Its sprite lists are found as
 * BN6's are (docs/rom/bn5-world.md, Another game's Navis); read as data only. */
#ifndef CW_BN5_EXPRESSION_H
#define CW_BN5_EXPRESSION_H

#define BN5_OW_MEGAMAN          0x30   /* sprite list 6's MegaMan, plain: its first 0x2E animations timed as BN6_OW_MEGAMAN's */
#define BN5_OW_MEGAMAN_ANIMS    0x59   /* ... its animations, past 0x2D its own: the buster aimed and fired, falls, getting up */
#define BN5_OW_ANIM_GOT         0x18   /* BN5_OW_MEGAMAN: the item-get pose, a hand raised (3 frames, 35 long), as BN6's */
#define BN5_OW_ANIM_ARRIVE      0x2D   /* BN5_OW_MEGAMAN: a jack-in's arrival, blank then turning round (10 frames, 50 long), as BN6's */
#define BN5_FX_HEART            0x26   /* sprite list 4's heart, plain, 1 frame (BN6's only heart is Roll's, BN6_BATTLE_ROLL's) */
#define BN5_FX_NOTE             0x44   /* sprite list 4's music note, plain, 4 frames looping (12 long), which BN6's lists lack */

#endif
