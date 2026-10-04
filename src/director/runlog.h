/* A plain-text log of every battle and run in the data folder
 * (runlog.txt): depth, area, the viruses and their HP, MegaMan's HP before
 * and after, and where each run ended; for a battle in another game's
 * engine its record there and its reward too. It shows where runs are
 * lost (docs/PROGRESSION.md). */
#ifndef CW_RUNLOG_H
#define CW_RUNLOG_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"

/* A battle begins: `e` its foes (NULL for a guardian), `kind` a word. */
void runlog_battle_start(const Encounter *e, const char *kind);
void runlog_battle_end(bool won);
/* A battle in another game's engine begins (guest.c): its record there,
 * its `n` enemies by that game's ids, their HP together ... */
void runlog_guest_start(uint32_t record, const int *ids, int n, int foehp);
/* ... and ends: won or left, and what its results screen gave ("Cannon A",
 * "200 zenny", "HP+50", "none"); a deletion ends it as runlog_run_end. */
void runlog_guest_end(bool won, const char *reward);
/* MegaMan was deleted: the open battle and the run end. */
void runlog_run_end(void);

#endif
