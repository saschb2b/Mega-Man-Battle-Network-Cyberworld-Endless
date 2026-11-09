/* The stop on the way into an act (director_stop.c, docs/world/bn5-town.md):
 * Oran Isle, asked on the act's pink pad in Lan's HP. */
#ifndef CW_DIRECTOR_STOP_H
#define CW_DIRECTOR_STOP_H

#include <stdbool.h>

/* Whether the pink pad asks before it warps: the stop due at this visit. */
bool stop_due(void);
/* MegaMan's words and question on the pad (the cursor on No), its answer
 * in HOME_LINK_FLAG; false where the talk could not start. */
bool stop_ask(void);
/* The answer: Yes the trip to the island, No the pad's warp; once a run. */
void stop_answer(bool yes);
/* Each frame at home: the island's arrival words, its save, the job
 * taken, and Lan's leaving it by its jack-in. */
void stop_frame(void);
/* Whether Lan stands on the island now. */
bool stop_here(void);
/* A CONTINUE there: the island installed again after home (true where
 * Lan is not there). */
bool stop_rebuild(void);

#endif
