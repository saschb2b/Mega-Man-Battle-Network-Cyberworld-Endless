/* The week's net (issue #127, docs/ONLINE.md, The week's net; docs/SAVES.md,
 * The weekly slot): a dive on the seed a Net Square server gives for its
 * week, played in a save slot of its own beside the normal run, which
 * waits untouched in its files and is the title's CONTINUE again when the
 * dive ends. A dive starts from the standard setup and a standard profile
 * (docs/ONLINE.md, its design): the player's settings and what he has been
 * told carry over, nothing a run is stronger or different for. Its end is
 * kept (weekly.sav) until a visit to the Square posts it. */
#ifndef CW_WEEKLY_H
#define CW_WEEKLY_H

#include <stdbool.h>
#include <stdint.h>

/* savedata/weekly.sav: the dive in the slot, and a dive's end to post */
typedef struct {
	uint16_t week;       /* the dive in the slot: its week's number (0 none) */
	uint8_t running;     /* ... unfinished: its checkpoint in the slot */
	uint8_t pending;     /* a dive's end waits to be posted (r_*) */
	uint32_t seed;       /* the dive's seed */
	uint16_t r_week, r_depth;
	uint32_t r_seed;
	uint8_t r_act, r_won;
	uint16_t r_minutes;
	uint32_t r_hash;     /* its summary's hash (weekly_hash), never 0 */
} WeeklyRecord;

/* The record as weekly.sav holds it (read once, again after the title). */
const WeeklyRecord *weekly_record(void);
/* The run in play is the week's dive. */
bool weekly_on(void);
/* A new dive of week `week` on `seed`: the slot's files gone, its standard
 * profile made from the player's, the run made with the standard setup,
 * and the game asked to start it in the town as NEW GAME does. */
bool weekly_dive(uint16_t week, uint32_t seed);
/* The unfinished dive again, from its checkpoint as CONTINUE takes a run
 * (from its start where it has none); false where there is none. */
bool weekly_continue(void);
/* The dive just ended, won or deleted (the director, after its records):
 * its end kept to post (the better of two of one week), the slot left. */
void weekly_ended(bool won);
/* Back to the normal slot, its profile read again (the title's: its
 * CONTINUE is the normal run's); the record read again. */
void weekly_leave(void);
/* ... and whether that slot holds a run (save_exists): the title's CONTINUE */
bool weekly_normal_saved(void);
/* The kept end answered by the server (taken, or refused): forgotten. */
void weekly_posted(void);

#endif
