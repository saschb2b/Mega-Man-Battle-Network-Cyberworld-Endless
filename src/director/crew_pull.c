#include "crew_pull.h"

#include <string.h>

/* (the owner's calls of 21 November 2026, issue #718: a quarter of each
 * standing crewmate's HP, the sum half the fallen's max at most) */
#define GIFT_PART 4
#define BACK_PART 2

void crew_pull(const CrewPullIn *in, bool won, CrewPullOut *out) {
	memset(out, 0, sizeof *out);
	unsigned n = in->n > CREW_PULL_SEATS ? CREW_PULL_SEATS : in->n;
	for (unsigned k = 0; k < n; ++k) out->hp[k] = in->hp[k];
	/* (a win with nobody standing, every MegaMan deleted as the last foe fell: nobody to pull anyone back) */
	unsigned standing = 0;
	for (unsigned k = 0; k < n; ++k) standing += in->hp[k] > 0;
	if (!won || !standing) return;
	for (unsigned f = 0; f < n; ++f) {
		if (in->hp[f] || in->spent[f] || in->gone[f]) continue;
		unsigned cap = in->max_hp[f] / BACK_PART, got = 0;
		/* (the givers: those standing as the battle ended, never one just pulled back) */
		for (unsigned s = 0; s < n && got < cap; ++s) {
			if (!in->hp[s]) continue;
			unsigned give = out->hp[s] / GIFT_PART;
			if (give > cap - got) give = cap - got;
			out->hp[s] -= give;
			out->gave[s] += give;
			got += give;
		}
		out->hp[f] = got ? got : 1;
		out->pulled[f] = true;
	}
}
