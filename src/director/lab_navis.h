/* Navis in the Battle Lab (docs/game/battle-lab.md, Navis): which a profile
 * can set, by the records it keeps already, and their battles. A guardian
 * is open once his NaviCode is held (deleted twice as a guardian), Bass
 * and the Cybeast after a first win, each in the versions MegaMan has
 * beaten (Profile.navi_beaten; his first where none was noted, a win of
 * before the note was kept). One Navi a battle, alone. */
#ifndef CW_LAB_NAVIS_H
#define CW_LAB_NAVIS_H

#include <stdbool.h>
#include <stddef.h>

#include "foes.h"

#define LAB_NAVI 0x80        /* a LabFoe's version with this bit: a Navi, its number in `family`, its version the rest */
#define LAB_NAVI_MOST 64     /* navi numbers looked through: 1 to this */

/* Whether navi `navi` is one the Lab lists at all here: a guardian of
 * BN6's, a super boss, or another game's guardian whose ROM is read */
bool lab_navi_known(int navi);
/* MegaMan's wins over him, and how many open him */
int lab_navi_wins(int navi);
int lab_navi_need(int navi);
/* The versions that can be set, a bit each: every one the engine holds
 * where `all`, else those beaten once he is open; 0 while he is not */
unsigned lab_navi_versions(int navi, bool all);
/* His name and HP in a version, and what a row calls the version ("V1",
 * "EX", "SP"; another game's "V2", "V3") */
void lab_navi_name(int navi, int version, char *out, size_t n);
int lab_navi_hp(int navi, int version);
const char *lab_navi_version_word(int navi, int version);
/* His battle, at panel (col, row) from 0; false where he cannot be fought here */
bool lab_navi_encounter(int navi, int version, int col, int row, Encounter *e);

#endif
