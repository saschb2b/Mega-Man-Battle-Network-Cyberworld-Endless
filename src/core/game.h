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
extern bool title_summary;          /* the title opens on the finished run's summary */


void error_show(const char *msg);

/* Seeded PRNG (xorshift). The run seed makes a run reproducible. */
uint32_t rng_next(void);
int rng_range(int lo, int hi); /* inclusive */
void rng_seed(uint32_t s);
uint32_t rng_state(void);    /* to put back after an aside */

extern char g_data_dir[512];

#endif
