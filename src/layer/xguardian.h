/* Another game's Navis as a territory's guardians (docs/BOSSES.md, BN5's
 * Navis): what the engine reads of them from their game's ROM, and the
 * guardians' traits either game's have (their element, its answer). The
 * navi numbers are guardians.h's: guardian_older for BN5's. */
#ifndef CW_XGUARDIAN_H
#define CW_XGUARDIAN_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"

/* Whether BN5's guardian `navi` is fought here: his Navi moved into BN6's
 * engine at every version a run fights him at (xnavi_install.h), which
 * BN5's ROM read brings (on the 3DS where its heap has room for it,
 * start_3ds.c) */
bool xguardian_fightable(int navi);
/* His battle in BN6's engine (docs/BOSSES.md, BN5's Navis): his moved id
 * at `version` where BN6's guardians stand, the boss theme, no running;
 * none (nfoes 0) where he is not fightable */
Encounter xguardian_encounter(int navi, int version);

/* The version (0 V1, 1 V2, 2 V3, 3 SP) BN5's guardian `navi` is fought at
 * on layer `depth` (pacing_xguardian_version, from his HP in his game) */
int xguardian_version(int navi, int depth);
/* His HP at that version in his game, -1 none; and as he is fought there,
 * held to the act's band (pacing_xguardian_hp) */
int xguardian_hp(int navi, int version);
int xguardian_hp_fought(int navi, int version, int depth);

/* A guardian's element (ELEM_*, 0 none) and the one that hits him twice as
 * hard (0 none), BN6's or BN5's: BN6's from its enemy table and traits,
 * BN5's from his game's stats (its first wheel's alone) */
int guardian_element(int navi);
int guardian_weakness(int navi);

/* His Navi chip for his Guardian Data, as BN6's guardians give theirs
 * (docs/BOSSES.md): BN5's V1 chip of him in the first three acts, his SP
 * from the fourth (where BN6's give EX or SP: BN5 has no EX) where it plays
 * in BN6's battles, else his V1; its code into *code (the folder's where
 * the chip has it, else its *, else its first); 0 none */
int xguardian_chip(int navi, int depth, int *code);
/* Whether BN6 chip `chip` is one of BN5's Navi chips of the six (V1, SP,
 * DS: a guardian's reward row holds his) */
bool xguardian_navi_chip(int chip);

/* What his Guardian Data says of his Soul (docs/META.md, Souls): that
 * MegaMan got it, that it is with them every battle, the chips that unite
 * with it, once a battle; all of it the first time a profile wins one
 * (profile.soul_taught), and once a profile that a Soul and a Cross are
 * not worn together (profile.soul_form_taught); that the run holds it
 * already where it does. Chat boxes split by '|'. */
const char *xguardian_soul_words(int navi);

/* The list-6 number he stands in and speaks with, his sprite and face
 * copied in from his game (xnavi_guardian); -1 where they cannot be */
int xguardian_slot(int navi);

#endif
