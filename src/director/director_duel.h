/* The rival's duel (director_duel.c), for the director's other parts. */
#ifndef CW_DIRECTOR_DUEL_H
#define CW_DIRECTOR_DUEL_H

#include <stdbool.h>

void resume_duel(void);
void official_sync(bool resumed);
void duel_roll(void);
void duel_verdict(bool won);

#endif
