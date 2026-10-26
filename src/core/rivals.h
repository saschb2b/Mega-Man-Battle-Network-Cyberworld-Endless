/* What each guardian remembers of MegaMan across runs, as Hades' bosses
 * keep a tally of who won (docs/game/bosses/). Stored beside the profile. */
#ifndef RIVALS_H
#define RIVALS_H

#include <stdint.h>

#define RIVAL_NAVIS 32

enum { RIVAL_NONE, RIVAL_MEGAMAN_WON, RIVAL_NAVI_WON };

/* The winner's HP left as a battle ended, in parts of his max HP (1 to
 * this; 0 not known: a record from before they were kept) */
#define RIVAL_LEFT_FULL 64

typedef struct {
	int met, megaman_won, navi_won;
	int last;        /* RIVAL_*: how their last battle ended, */
	uint8_t form;    /* ... MegaMan's form in it (BN6_BATTLE_FORM: a Cross 1-5, Beast Out 11 and up), 0 none, */
	uint8_t left;    /* ... the winner's HP left (RIVAL_LEFT_FULL), */
	uint16_t run;    /* ... and the run it was fought in, profile.runs + 1; 0 not known */
} Rival;

/* How a battle ended, beyond who won */
typedef struct {
	int form;        /* MegaMan's form (BN6_BATTLE_FORM) */
	int hp, max;     /* the winner's HP left, and his max */
} RivalEnd;

void rivals_load(void);
/* Navi `navi`'s record (a blank one for an unknown navi). */
const Rival *rival(int navi);
void rival_met(int navi);
/* A battle with `navi` ended in `result` (RIVAL_*), as `how` (NULL: not
 * known), in the run `run` (profile.runs + 1). */
void rival_result(int navi, int result, const RivalEnd *how, int run);

#endif
