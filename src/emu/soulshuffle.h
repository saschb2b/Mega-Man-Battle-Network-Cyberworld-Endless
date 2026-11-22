/* SearchSoul's SHUFFLE on BN6's Custom screen (docs/multirom/souls.md, Team ProtoMan's Souls): BN6 keeps BN5's button
 * (element 8 and 9 of types 4 and 5, A on it reshuffling the chips not picked) behind a test of its own; SearchSoul passes it,
 * with BN5's three shuffles a turn where BN6 gives one. BN6's own screen, art and shuffle do the rest. */
#ifndef CW_SOULSHUFFLE_H
#define CW_SOULSHUFFLE_H

#include <stdbool.h>

/* the two hooks, once a core (after the Souls' code: soulown.c); false where a byte they stand on is not Gregar's */
bool soulshuffle_install(int breaks);

#endif
