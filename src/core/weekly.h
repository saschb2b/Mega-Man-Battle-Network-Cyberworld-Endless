/* The week's net (issue #127, docs/online/weekly-net.md; docs/engine/saves.md,
 * The weekly slot): a dive on the seed a Net Square server gives for its
 * week, played in a save slot of its own beside the normal run, which
 * waits untouched in its files and is the title's CONTINUE again when the
 * dive ends. A dive starts from the standard setup and a standard profile
 * (docs/online/weekly-net.md, its design): the player's settings and what he has been
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
	uint16_t b_week, b_depth;   /* the deepest layer a dive of week b_week reached, posted or not (the summary's) */
	/* (grown at its end: an older weekly.sav reads with these 0) */
	uint8_t pulled;      /* the dive in the slot: its crew's pull-back spent (issue #718, crew_pull.h) */
	uint8_t r_pulled;    /* the end to post: its dive pulled back once, the Board's mark */
	uint8_t crew_fought, crew_won;   /* the dive in the slot: its crew's battles fought and won (issue #769; 0 in older files) */
	uint16_t intro_week; /* the week whose first layer's words and act card were shown (issue #779; 0 none: an older file's) */
	uint8_t spare[2];    /* (zero: the record's size a whole four) */
} WeeklyRecord;

/* The week's dive's first layer: its words and the act's card shown this
 * week already (once a week, issue #779), and marked shown. Both nothing
 * outside the week's dive. */
bool weekly_intro_shown(void);
void weekly_intro_mark(void);
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
/* The dive that ended last in this session, as it ended (not the week's
 * best, which the record keeps to post): told the server with its end, the
 * Board's from it (docs/online/shared-dive.md, The Board without a visit) */
typedef struct {
	bool ended, won, pulled;
	uint16_t depth, minutes;
	uint8_t crew_fought, crew_won;
	int place;           /* its place on the week's Board as the server answers its end: 0 asked, -1 no answer (issue #769) */
} WeeklyEnd;
const WeeklyEnd *weekly_last_end(void);
/* ... its place, as the server's Board says it after the end (0 still asked, -1 no answer) */
void weekly_end_placed(int place);
/* While the place is asked: a frame of the waiting link, the online part's
 * (dive_mail.c sets it, the dive's end screen pumps it), NULL none */
extern void (*weekly_end_pump)(void);
/* The dive in the slot fought a crew's battle (director_crew_over): kept at once */
void weekly_crew_battle(bool won);
/* An unfinished dive in the slot: its checkpoint's layer (1 where it has
 * none yet), 0 none. The title's CONTINUE takes it up (issue #759). */
int weekly_unfinished(void);
/* How often a dive was taken up again this session (weekly_continue): the
 * dive's link takes the Square up again for each one it finds itself
 * without (dive_link.c) */
unsigned weekly_resumes(void);
/* The dive just ended, won or deleted (the director, after its records):
 * its end kept to post (the better of two of one week), the slot left. */
void weekly_ended(bool won);
/* Back to the normal slot, its profile read again (the title's: its
 * CONTINUE is the normal run's); the record read again. */
void weekly_leave(void);
/* ... and whether that slot holds a run (save_exists): the title's CONTINUE */
bool weekly_normal_saved(void);
/* The dive's MegaMan deleted in a crew's battle and pulled back by the
 * crew: its one pull-back spent, kept at once (a CONTINUE from an earlier
 * checkpoint finds it spent) and its end marked for the Board. */
void weekly_pulled(void);
/* The kept end answered by the server (taken, or refused): forgotten. */
void weekly_posted(void);

#endif
