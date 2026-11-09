/* Save identities shared by the save writer and transfer preflight. */
#ifndef CW_SAVE_FORMAT_H
#define CW_SAVE_FORMAT_H

#include "chip_entry.h"

/* "CWEB": the extras switched off; "CWEC": Oran Isle's run.stop (the town's
 * work, which takes it first); both with their state's folders nine-bit,
 * as every build's before ten-bit chip ids. "CWED": the same Run, its
 * state's folders in ten bits (chip_entry.h): a run read from any older
 * magic is converted once (save_state_nine) */
#define RUN_MAGIC_NINE 0x43574542u
#define RUN_MAGIC_NINE_STOP 0x43574543u
#define RUN_MAGIC_TEN 0x43574544u
#define RUN_MAGIC (CHIP_ID_BITS == 9 ? RUN_MAGIC_NINE : RUN_MAGIC_TEN)
#define PROFILE_MAGIC 0x43575032u
#define WEEKLY_MAGIC 0x43575731u  /* "CWW1": weekly.sav, the week's net (weekly.h) */

#endif
