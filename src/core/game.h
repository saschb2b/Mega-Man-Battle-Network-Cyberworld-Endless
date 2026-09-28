/* Scene management and shared game entry points. */
#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	const char *name;
	void (*enter)(void);
	void (*update)(void);
	void (*draw)(void);
	void (*leave)(void);
} Scene;

void scene_set(const Scene *s);

extern const Scene scene_error;
extern const Scene scene_title;
extern const Scene scene_gallery;
extern const Scene scene_emu;     /* the game itself, on the embedded core */
extern bool emu_resume_requested; /* scene_emu continues the saved run */
extern bool emu_start_in_town;    /* scene_emu starts the run in the town (NEW GAME) */
extern bool title_summary;          /* the title opens on the finished run's summary */
extern char title_cause[48];       /* ... where MegaMan was deleted ("by DiveMan in Sky HP") */
extern bool title_new_best;         /* ... deeper than any run before */
extern bool title_won;              /* ... the run was won: the short net's Nest fell */
extern uint32_t title_seed;         /* NEW GAME's seed when not 0 (--seed on the title: a session replays) */


void error_show(const char *msg);

/* Seeded PRNG (xorshift). The run seed makes a run reproducible. */
uint32_t rng_next(void);
int rng_range(int lo, int hi); /* inclusive */
void rng_seed(uint32_t s);
uint32_t rng_state(void);    /* to put back after an aside, */
void rng_restore(uint32_t s);   /* ... with this */

extern char g_data_dir[512];

#endif
