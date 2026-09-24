/* What each guardian remembers of MegaMan across runs, as Hades' bosses
 * keep a tally of who won (docs/BOSSES.md). Stored beside the profile. */
#ifndef RIVALS_H
#define RIVALS_H

#define RIVAL_NAVIS 32

enum { RIVAL_NONE, RIVAL_MEGAMAN_WON, RIVAL_NAVI_WON };

typedef struct {
	int met, megaman_won, navi_won;
	int last;   /* RIVAL_*: how their last battle ended */
} Rival;

void rivals_load(void);
/* Navi `navi`'s record (a blank one for an unknown navi). */
const Rival *rival(int navi);
void rival_met(int navi);
void rival_result(int navi, int result);

#endif
