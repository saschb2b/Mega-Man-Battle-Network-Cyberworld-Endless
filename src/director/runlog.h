/* A plain-text log of every battle and run in the data folder
 * (runlog.txt): depth, area, the viruses and their HP, MegaMan's HP before
 * and after, and where each run ended; for a battle in another game's
 * engine its record there and its reward too. It shows where runs are
 * lost (docs/PROGRESSION.md). The same moments give the anonymous
 * statistics their guardians' battles and runs' ends (analytics.h), sent
 * where the player said yes. */
#ifndef CW_RUNLOG_H
#define CW_RUNLOG_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"

/* A battle begins: `e` its foes, `kind` a word ("battle", "challenge",
 * "duel": ProtoMan's) ... */
void runlog_battle_start(const Encounter *e, const char *kind);
/* ... a guardian's, `navi` the one fought */
void runlog_guardian_start(int navi);
/* ... and ends, won or left (a deletion ends it as runlog_run_end), at
 * MegaMan's HP after, the battle's frames (BN6's DeleteTime) */
void runlog_battle_end(bool won, int frames);
/* A battle in another game's engine begins (guest.c): its record there,
 * its `n` enemies by that game's ids, their HP together ... */
void runlog_guest_start(uint32_t record, const int *ids, int n, int foehp);
/* ... a guardian's of that game (guest_boss_battle): the same, "guardian"
 * for "battle", `navi` his number here, his id there and the HP he is
 * fought at */
void runlog_guest_guardian_start(uint32_t record, int navi, int id, int foehp);
/* ... and ends: won or left, and what its results screen gave ("Cannon A",
 * "200 zenny", "HP+50", "none"), in `frames`; a deletion ends it as
 * runlog_run_end. */
void runlog_guest_end(bool won, const char *reward, int frames);
/* The run is over, won, or MegaMan was deleted: the open battle and the
 * run end. */
void runlog_run_end(bool won);

#endif
