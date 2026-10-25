/* A plain-text log of every battle and run in the data folder
 * (runlog.txt): depth, area, the viruses and their HP, MegaMan's HP before
 * and after, and where each run ended; for a battle of another game's
 * its record there, and the Souls MegaMan united with. It shows where runs are
 * lost (docs/game/progression/). The same moments give the anonymous
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
/* ... a guardian's, `navi` the one fought, `e` his battle (BN5's by the
 * id moved in, "n" his family there), `hp` the HP he is fought at */
void runlog_guardian_start(int navi, const Encounter *e, int hp);
/* ... MegaMan united with Souls in it (souls.h: BN6_SOUL_USED's layout,
 * bit n Soul n, bit 16 + n its Chaos Unison): noted before its end */
void runlog_battle_souls(uint32_t used);
/* ... and ends, won or left (a deletion ends it as runlog_run_end), at
 * MegaMan's HP after, the battle's frames (BN6's DeleteTime) */
void runlog_battle_end(bool won, int frames);
/* The run is over, won, or MegaMan was deleted: the open battle and the
 * run end, its economy's counts (below) on its line and sent with the
 * statistics' run-end event. */
void runlog_run_end(bool won);

/* The run's economy's counts (analytics.h's AnalyticsEconomy, docs/
 * docs/game/economy/): begun with a new run (or where they were another run's),
 * a won battle's reward and a Guardian Data's chip counted as the battle
 * ends, each Mystery Data opened by its content (layer_objs.h's md_find:
 * kind 1 a chip, 3 zenny), kept beside the run's checkpoint (run.tally) */
void runlog_tally_begin(uint32_t seed);
void runlog_md_opened(uint32_t find);
void runlog_tally_save(void);
void runlog_tally_load(uint32_t seed);

#endif
