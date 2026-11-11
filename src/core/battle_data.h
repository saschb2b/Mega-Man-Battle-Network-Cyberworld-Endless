/* The battle data on virus families (docs/game/battle-lab.md, The gauge): how
 * often MegaMan has deleted each family, and which of its versions, in any
 * run. A family's data is complete at DATA_COMPLETE deletions; complete, the
 * Battle Lab can set every version of it deleted at least once. A run's own
 * deletions wait in its tally beside the checkpoint (run.data) and join the
 * profile as the run ends, so a CONTINUE from an older save counts no battle
 * twice. */
#ifndef CW_BATTLE_DATA_H
#define CW_BATTLE_DATA_H

#include <stdbool.h>
#include <stdint.h>

#define DATA_FAMILIES 256    /* the engine's AI indexes: BN6's 1-31, the moved families' from 32 */
#define DATA_VERSIONS 6      /* a family's versions, a bit each: 0-3, the Rares 4 and 5 */
#define DATA_COMPLETE 8      /* deletions of a family, any version, for its data to be complete */
#define DATA_LAB_OPENS 3     /* families complete for Dad's lab to send the Battle Lab */

/* A run's deletions, kept beside its checkpoint */
typedef struct {
	uint32_t seed;
	uint8_t count[DATA_FAMILIES];      /* to 255 */
	uint8_t versions[DATA_FAMILIES];   /* bit v: version v deleted */
} DataTally;

/* A new run: its tally begun. A tally another run left on disk without
 * its end (a run abandoned) joins the profile first. */
void battle_data_begin(uint32_t seed);
/* A virus of `family` (its AI index) in `version` deleted in the run's own
 * battle */
void battle_data_deleted(int family, int version);
/* The run's tally with its checkpoint, and back on a CONTINUE */
void battle_data_save(void);
void battle_data_load(uint32_t seed);
/* The run is over: its tally into the profile (which the caller saves),
 * and gone. How many families' data it completed. */
int battle_data_run_end(void);

/* What MegaMan holds on a family, the profile's and the run's together:
 * its deletions (to 255), the versions deleted (a bit each), whether its
 * data is complete; and how many families' are */
int battle_data_count(int family);
unsigned battle_data_versions(int family);
bool battle_data_complete(int family);
int battle_data_families_complete(void);

#endif
