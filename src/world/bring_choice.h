/* A Bring request's chip, chosen on BN6's own Chip Trader screen (issue
 * 842; docs/world/home/jobs.md, The chip given): the asker's script opens
 * it (ts_start_chip_trader BRING_OPEN), and while the run's request
 * for a chip of an element is up for settling, a screen for one lists the
 * Pack's chips of that element alone, asks in MegaMan's words, and its
 * Yes leaves it with nothing traded: the chip put in is told, and the
 * asker's script takes it as before (job_words.c). B leaves it untouched. */
#ifndef CW_BRING_CHOICE_H
#define CW_BRING_CHOICE_H

#include <stdbool.h>

/* The chips the asker's script opens the screen for, which no screen of
 * BN6's takes; the screen then takes one. */
#define BRING_OPEN 2

/* The visit's request up for settling asks a chip of `element` (0: none
 * does): the screen for one is the choice while it holds (its hooks put
 * in the first time one asks). */
void bring_choice_arm(int element);

/* The choice as the screen for one left it, once, after it closed. */
typedef enum { BRING_NONE, BRING_CHOSEN, BRING_KEPT } BringResult;
BringResult bring_choice_result(int *id, int *code);

#endif
