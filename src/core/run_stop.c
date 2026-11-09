/* The stop on the way's rule (docs/world/bn5-town.md, Where it stands): a
 * coin of the run's seed, as the Glitch layer's and the Virus Battler's
 * are, and the acts it may stand before. */
#include "run_stop.h"

bool run_stop_coin(uint32_t seed) {
	uint32_t h = seed ^ 0x0A7A15E5u;
	h ^= h >> 16; h *= 0x7FEB352Du; h ^= h >> 15; h *= 0x846CA68Bu; h ^= h >> 16;
	return (h & 1) != 0;
}

bool run_stop_fits(int act, int acts, bool older) { return older && act >= 1 && act < acts; }
