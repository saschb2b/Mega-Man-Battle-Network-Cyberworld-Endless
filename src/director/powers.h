/* What MegaMan keeps from the Navis he deletes: the game's own Crosses and
 * BeastOut, switched on through the event flags the story sets. */
#ifndef CW_POWERS_H
#define CW_POWERS_H

#include <stdbool.h>

/* After a won boss battle against `navi` (navi index): his Cross, BN5's
 * guardian's Soul, and BeastOut from the Cybeast (issue #109). */
void powers_after_boss(int navi);
/* The Cross of `navi` (HeatMan 1 .. ChargeMan 5), NULL for a navi with none. */
const char *powers_cross_name(int navi);
/* The attacks that hit MegaMan twice as hard in that Cross ("Aqua"). */
const char *powers_cross_weakness(int navi);
/* What the Cross does beyond its buster, where BN6 gives it more, or NULL. */
const char *powers_cross_strength(int navi);
/* What it does to a Navi, where that differs from a virus, or NULL. */
const char *powers_cross_on_navis(int navi);
/* Whether MegaMan has `navi`'s Cross: its CROSSSELECT entry on. */
bool powers_cross_owned(int navi);
/* A run's start: what it brought of MegaMan's powers (docs/META.md), in
 * the Custom screen from the first battle: the Cross of run.cross, and
 * BeastOut with the BeastOut helper (issue #99). */
void powers_bring(void);
/* What the player is told after that battle at `depth` (ta_talk's boxes:
 * a Cross MegaMan did not have yet, the beast stirred in the first
 * Graveyard), or NULL. */
const char *powers_reward_text(int navi, int biome, int depth);
/* Dad's word after the Cybeast's fall at `depth`, said on in his call
 * (ta_talk's boxes): the PET's CybeastButton unlocked, where the fall
 * gave the run BeastOut (the first Net's, a run that did not bring it),
 * or NULL. */
const char *powers_den_text(int depth);

/* (powers_words.c) MegaMan's words after guardian `navi`'s battle: his
 * Cross, where it is new to the run (`cross`), and the Nest's call to the
 * Cybeast in him (`beast`), which unlocks nothing; NULL for none */
const char *powers_reward_words(int navi, bool cross, bool beast);
/* (powers_words.c) Dad's word that he unlocks the CybeastButton, MegaMan
 * having beaten the beast */
const char *powers_beast_words(void);

#endif
