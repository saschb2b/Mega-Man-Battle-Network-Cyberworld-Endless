/* The week's pack and split records as the server last said them (issue
 * #589; docs/online/shared-dive.md, As built): who dives now and on which
 * layer (DIVERS) and the week's fastest bust of each act's guardian, alone
 * and by party size (WEEK_SPLITS), heard in the Square and on a dive
 * alike, for the Board's posts (square_week.c). */
#ifndef CW_DIVE_PACK_H
#define CW_DIVE_PACK_H

#include "dive_wire.h"

void dive_pack_divers(const DiveDivers *d);
void dive_pack_splits(const WeekSplits *s);
/* As last said: none (zeroed) before the first. */
const DiveDivers *dive_pack_now(void);
const WeekSplits *dive_pack_records(void);

#endif
