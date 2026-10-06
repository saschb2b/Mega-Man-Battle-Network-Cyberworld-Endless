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
/* The scene the frames run (main.c), NULL before the first */
const Scene *scene_current(void);

extern const Scene scene_error;
extern const Scene scene_title;
extern const Scene scene_intro;   /* the start: the boot screen and the GitHub notice, then the title */
extern const Scene scene_gallery;
extern const Scene scene_emu;     /* the game itself, on the embedded core */
extern const Scene scene_launcher; /* the ROMs before the game: two cartridges, PLAY (src/launcher/) */

/* The launcher: at a start, shown where it is wanted (LAUNCHER_AUTO: no
 * BN6, the first start, BN5 gone since the last), always (LAUNCHER_OPEN) or
 * never; true when it shows, `begin` starting the game at its PLAY.
 * `rom_dir` where copies are kept (NULL: the data folder's rom/) */
enum { LAUNCHER_OFF, LAUNCHER_AUTO, LAUNCHER_OPEN };
bool launcher_start(int mode, const char *rom_dir, void (*begin)(void));
/* ... whether this build has it: the desktops and the phones, not the 3DS,
 * a PortMaster handheld or the browser (whose page chooses the ROMs) */
bool launcher_here(void);
/* ... over the title, to add BN5 or see the ROMs (its R); false where there is none */
bool launcher_open(void);
extern bool emu_resume_requested; /* scene_emu continues the saved run */
extern bool emu_start_in_town;    /* scene_emu starts the run in the town (NEW GAME) */
extern bool emu_start_at_home;    /* ... or at home before its first layer's act (--scene home) */
/* The saved run (peek_run's seed and depth) was saved at home, between
 * acts: the title's CONTINUE says "Home" (scene_emu.c, by the director's
 * act note). */
bool emu_saved_home(uint32_t seed, int depth);
extern bool title_summary;          /* the title opens on the finished run's summary */
extern bool title_setup;            /* ... or on the setup after NEW GAME (--scene setup, for a capture) */
extern char title_cause[48];       /* ... where MegaMan was deleted ("by DiveMan in Sky HP") */
extern char title_learned[24];     /* ... the guardian whose battle data that first fight gave, "" for none */
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
