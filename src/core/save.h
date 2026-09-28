/* Run checkpoints and the permanent profile, stored beside the game. */
#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>
#include <stddef.h>

#include "run.h"

typedef struct {
	int runs;
	int best_depth;
	int last_depth;   /* where the last run ended (was an unused score) */
	int bosses;
	int viruses;
	int secret_clears;
	int nest_clears;
	int music_volume; /* 0-10, stored +1 so a fresh profile reads as default */
	int sfx_volume;
	bool seen_intro;
	/* (fields after here are new since profiles began: an older, shorter
	 * profile reads them as 0) */
	uint8_t first_guardian;   /* the last new run's act 1 guardian, navi + 1 */
	uint8_t navicust_taught;  /* a Guardian Data's draft has said how the NaviCust's board works */
	uint8_t marks_taught;     /* the map's violet marks L has explained (MARK_*) */
	/* the meta layer (docs/META.md): the highest threat rung open to a new
	 * run, the starting folders unlocked (a bit each), short nets won */
	uint8_t threat_open;
	uint16_t folders_open;
	uint16_t short_wins;
	uint8_t last_net, last_folder, last_threat, last_helpers;   /* the setup screen starts where the last run's did */
	uint16_t marks;           /* the title's marks earned (MARK_*, BN6's own bits: meta.h) */
} Profile;

enum { MARK_SERVER = 1, MARK_WARP = 2, MARK_GATE = 4 };

extern Profile profile;

void save_init(void);
bool save_exists(void);
bool save_run(void);
bool load_run(void);
/* The saved run without loading it (none from before the current save format). */
bool peek_run(Run *out);
/* Deletes the run's save and its game state. */
void save_delete(void);
/* Where the run's checkpoint keeps the game's state. */
void save_state_path(char *out, size_t n);
void profile_save(void);
/* A new run from the title: the seed's, or the next seed's where its act 1
 * guardian is the last run's (BlastMan six runs running). */
void run_new_varied(uint32_t seed);
void profile_record_run(void);

#endif
