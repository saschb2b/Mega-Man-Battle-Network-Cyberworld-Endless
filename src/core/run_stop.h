/* Where the stop on the way stands (run_stop.c, docs/world/bn5-town.md):
 * once a run at most, in half the runs, before the first act from act 2
 * whose area the older Net dresses. From the run's seed alone. */
#ifndef CW_RUN_STOP_H
#define CW_RUN_STOP_H

#include <stdbool.h>
#include <stdint.h>

/* The run's coin: whether it holds the stop at all (half the runs). */
bool run_stop_coin(uint32_t seed);
/* Whether act `act` (0-based) of a net of `acts` acts before its Nest may
 * hold it: from act 2 (index 1), never the Nest's, where `older` (its way
 * on the area BN5 dresses). */
bool run_stop_fits(int act, int acts, bool older);

#endif
