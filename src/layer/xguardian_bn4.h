/* BN4 Red Sun's Navis as a run meets them (docs/game/tournament.md, The entrants; docs/multirom/bn4-navis.md): those moved
 * into BN6's engine and proved against BN4's own battle, one line each in xguardian_bn4.c's list, each navi
 * GUARDIAN_BN4_FIRST + his seat (guardians.h): his name and look on a round's card, his own plate on BN4's bracket, his HP
 * at each version from BN4's records, his battle at his moved ids. Rounds alone: the final is the act's guardian, and BN4's
 * guard no area yet. */
#ifndef CW_XGUARDIAN_BN4_H
#define CW_XGUARDIAN_BN4_H

#include <stdbool.h>

#include "foes.h"
#include "guardians.h"

/* The proved BN4 Navis: how many, and the k-th's navi number */
int xguardian_bn4_count(void);
int xguardian_bn4_navi(int k);
/* Whether BN4's Navi `navi` is fought here: one of the proved, installed at every version (V1, V2, V3, SP), which BN4's ROM
 * read brings */
bool xguardian_bn4_fightable(int navi);
/* His name, epithet and colour on a card; NULL for one not proved */
const Guardian *xguardian_bn4_look(int navi);
/* His plate on BN4's bracket: BN4's own entrant id (BN4_ENTRANT_ID, GutsMan's 0x00); -1 none */
int xguardian_bn4_plate(int navi);
/* His HP at `version` (0 V1 .. 3 SP) in BN4's records, -1 none */
int xguardian_bn4_hp(int navi, int version);
/* His battle in BN6's engine: his moved id at `version` where BN6's guardians stand, the boss theme, no running; none
 * (nfoes 0) where he is not fightable */
Encounter xguardian_bn4_encounter(int navi, int version);

#endif
