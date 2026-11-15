/* The shared dive's busting splits (issue #589; docs/online/shared-dive.md,
 * As built): a battle on a dive layer won is told to the server (DIVE
 * split) by its index among the layer's battles (the n-th random battle is
 * the same on every diver's game of the layer's seed), its time in tenths
 * of a second (BN6's DeleteTime), BN6's Busting Level and whether it was
 * the guardian's; the server's answer, its place among the week's
 * (DIVE_PLACE), shows in the feed's band. */
#ifndef CW_DIVE_SPLITS_H
#define CW_DIVE_SPLITS_H

#include "dive_wire.h"

/* Each frame on a dive layer, in battle or out. */
void dive_splits_frame(void);
/* A DIVE_PLACE come in. */
void dive_splits_place(const DivePlace *p);
/* What deleted MegaMan where his last battle was lost: its strongest
 * foe's NameID (BN6_T1_NAME_ID, data.h's enemy_name), 0 unknown. */
unsigned dive_splits_cause(void);

#endif
