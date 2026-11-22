/* Save identities shared by the save writer and transfer preflight. */
#ifndef CW_SAVE_FORMAT_H
#define CW_SAVE_FORMAT_H

#include "chip_entry.h"

/* "CWEB": the extras switched off; "CWEC": Oran Isle's run.stop (the town's
 * work, which takes it first); both with their state's folders nine-bit,
 * as every build's before ten-bit chip ids. "CWED": the same Run, its
 * state's folders in ten bits (chip_entry.h): a run read from a magic
 * older than it is converted once (save_state_nine). "CWEE": MegaMan's
 * form for the run and what goes with it (issue #774), read from "CWED"
 * as the mix */
#define RUN_MAGIC_NINE 0x43574542u
#define RUN_MAGIC_NINE_STOP 0x43574543u
#define RUN_MAGIC_TEN 0x43574544u
#define RUN_MAGIC_FORMS 0x43574545u
#define RUN_MAGIC RUN_MAGIC_FORMS
#define PROFILE_MAGIC 0x43575032u
#define WEEKLY_MAGIC 0x43575731u  /* "CWW1": weekly.sav, the week's net (weekly.h) */

#endif
