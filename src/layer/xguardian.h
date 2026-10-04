/* Another game's Navis as a territory's guardians (docs/BOSSES.md, BN5's
 * Navis): what the engine reads of them from their game's ROM, and the
 * guardians' traits either game's have (their element, its answer). The
 * navi numbers are guardians.h's: guardian_older for BN5's. */
#ifndef CW_XGUARDIAN_H
#define CW_XGUARDIAN_H

#include <stdint.h>

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

/* A chip of his Soul's kind (the chips his Soul unites with in his game,
 * BN5_CHIP_KIND) for his Guardian Data, in place of the Navi chip
 * BN6's guardians give (docs/BOSSES.md): one of BN6's standard chips whose
 * namesake in his game is of it, rolled from `seed` (the rarest left out
 * where there are others); its code into *code, as a guardian's Navi chip
 * comes (the folder's where the chip has it, else its *, else its first);
 * 0 none */
int xguardian_chip(int navi, uint32_t seed, int *code);

/* The list-6 number he stands in and speaks with, his sprite and face
 * copied in from his game (xnavi_guardian); -1 where they cannot be */
int xguardian_slot(int navi);

#endif
