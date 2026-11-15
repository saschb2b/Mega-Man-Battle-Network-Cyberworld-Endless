/* The shared dive's busting splits (issue #589; docs/online/shared-dive.md,
 * As built): a battle on a dive layer won is told to the server (DIVE
 * split) by its index among the layer's battles (each diver's own roll, so
 * the n-th is not always the same battle: a crew's alone is made alike,
 * director_crew.h), its time in tenths
 * of a second (BN6's DeleteTime), BN6's Busting Level and whether it was
 * the guardian's; the server's answer, its place among the week's
 * (DIVE_PLACE), shows in the feed's band. */
#ifndef CW_DIVE_SPLITS_H
#define CW_DIVE_SPLITS_H

#include "dive_wire.h"

/* Each frame on a dive layer, in battle or out. */
void dive_splits_frame(void);
/* A battle of the layer fought on the party's core (a crew's, dive_join.h)
 * counted, so that the next one's index is the layer's next. */
void dive_splits_fought(void);
/* A DIVE_PLACE come in. */
void dive_splits_place(const DivePlace *p);
/* What deleted MegaMan where his last battle was lost: its strongest
 * foe's NameID (BN6_T1_NAME_ID, data.h's enemy_name), 0 unknown. */
unsigned dive_splits_cause(void);

#endif
