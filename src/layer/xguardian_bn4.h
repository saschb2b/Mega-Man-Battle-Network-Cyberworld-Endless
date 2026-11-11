/* BN4 Red Sun's Navis as a run meets them (docs/game/tournament.md, The entrants; docs/multirom/bn4-navis.md): those moved
 * into BN6's engine and proved against BN4's own battle, one line each in xguardian_bn4.c's list, each navi
 * GUARDIAN_BN4_FIRST + his seat (guardians.h): his name and look on a round's card, his own plate on BN4's bracket, his HP
 * at each version from BN4's records (his battle at his moved ids, xguardian.h), and the areas of BN4's that dress BN6's
 * name those its net's records set there as their guardians (docs/game/bosses/bn4-navis.md). */
#ifndef CW_XGUARDIAN_BN4_H
#define CW_XGUARDIAN_BN4_H

#include <stdbool.h>

#include "guardians.h"

/* The proved BN4 Navis: how many, and the k-th's navi number */
int xguardian_bn4_count(void);
int xguardian_bn4_navi(int k);
/* His BN4 V1 enemy id (his versions the next three), -1 none: whether he is fought here and his battle are xguardian.h's
 * (xguardian_bn4_fightable, xguardian_bn4_encounter) */
int xguardian_bn4_v1(int navi);
/* His name, epithet and colour on a card; NULL for one not proved */
const Guardian *xguardian_bn4_look(int navi);
/* His plate on BN4's bracket: BN4's own entrant id (BN4_ENTRANT_ID, GutsMan's 0x00); -1 none */
int xguardian_bn4_plate(int navi);
/* His HP at `version` (0 V1 .. 3 SP) in BN4's records, -1 none; his element there (ELEM_* 1-4, 0 none) */
int xguardian_bn4_hp(int navi, int version);
int xguardian_bn4_element(int navi);
/* His overworld sprite's number in BN4's list 6, his mugshot's the same in its list 8 (BN4_OW_*); 0 none */
int xguardian_bn4_ow(int navi);
/* The BN6 Navi (guardians.h's number) whose Navi chip his Guardian Data gives while his own sits out: his element's, else
 * his kind's (docs/game/bosses/bn4-navis.md, His Guardian Data); 0 none */
int xguardian_bn4_kin(int navi);

#endif
