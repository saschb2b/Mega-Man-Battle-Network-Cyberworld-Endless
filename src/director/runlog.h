/* A plain-text log of every battle and run in the data folder
 * (runlog.txt): depth, area, the viruses and their HP, MegaMan's HP before
 * and after, and where each run ended. It shows where runs are lost
 * (docs/PROGRESSION.md). */
#ifndef CW_RUNLOG_H
#define CW_RUNLOG_H

#include <stdbool.h>

#include "foes.h"

/* A battle begins: `e` its foes (NULL for a guardian), `kind` a word. */
void runlog_battle_start(const Encounter *e, const char *kind);
void runlog_battle_end(bool won);
/* MegaMan was deleted: the open battle and the run end. */
void runlog_run_end(void);

#endif
