/* The NaviCust as a run's second build axis (docs/NAVICUST.md): the
 * programs a run can be offered, by tier and build, the guardian's draft,
 * the ExpMemry milestones, and MegaMan's words for the bugs a layout
 * causes. Programs are BN6's: a program n is given as id n * 4 (EF 1B)
 * in one of its colours (1 White, 2 Yellow, 3 Pink, 4 Red, 5 Blue,
 * 6 Green). */
#ifndef CW_NAVICUST_H
#define CW_NAVICUST_H

#include <stdbool.h>
#include <stdint.h>

enum { BUILD_BUSTER, BUILD_HAND, BUILD_GUARD, BUILD_FIELD, BUILD_HP, BUILD_COUNT };

#define NAVICUST_DRAFT 3      /* programs a guardian's draft offers */
#define NAVICUST_BUGS  16     /* bug types the game counts */

typedef struct {
	uint8_t program;   /* BN6's program number, 1-46 (given as program * 4) */
	uint8_t color;     /* 1-6; 0 until navicust_color picks one */
} NaviProgram;

/* The draft of a guardian at `depth`: three programs of three different
 * builds, each of a tier the act has reached (ROM-free; the rng decides). */
int navicust_draft(int depth, NaviProgram out[NAVICUST_DRAFT]);
/* One of the program's colours, as the ROM's program records have them
 * (the rng picks among a plus part's three); 0 when the ROM has none. */
int navicust_color(int program);
/* What the program does, in MegaMan's words ("Custom1: one more chip each
 * turn."); NULL for a program outside the pool. */
const char *navicust_about(int program);
/* Whether `program` may ever be offered (the inert and the harmful are not). */
bool navicust_in_pool(int program);
/* The build a program belongs to (BUILD_*), -1 outside the pool. */
int navicust_build(int program);
/* BugFrags a guardian's draft pays when none is taken, at `depth`. */
int navicust_skip_frags(int depth);
/* ExpMemry a guardian at `depth` gives (the board grows 4x4, 5x4, 5x5):
 * the second and fourth acts' guardians of the first cycle. */
bool navicust_expmemry(int depth);
/* MegaMan's words for the bug counts (one per type, as the game keeps
 * them): speaker-marked boxes for talk_start; "" for none. */
const char *navicust_bug_words(const uint8_t counts[NAVICUST_BUGS]);

#endif
