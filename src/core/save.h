/* Run checkpoints and the permanent profile, stored beside the game. */
#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>
#include <stddef.h>

#include "run.h"

typedef struct {
	int runs;
	int best_depth;
	int best_score;   /* unused; kept for the file layout */
	int bosses;
	int viruses;
	int secret_clears;
	int nest_clears;
	int music_volume; /* 0-10, stored +1 so a fresh profile reads as default */
	int sfx_volume;
	bool seen_intro;
} Profile;

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
void profile_record_run(void);

#endif
