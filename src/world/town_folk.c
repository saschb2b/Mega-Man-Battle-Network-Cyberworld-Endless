/* town_folk.h. At each visit home the town's people stand elsewhere and a
 * different few are out, as a home's people would be (docs/HOME.md, crowds
 * that move); their places stay the town's, each with its facing and what
 * is said there. From the run's depth, so a CONTINUE finds them as they
 * were. */
#include "town_folk.h"

#include "run.h"

/* 0 to n - 1, from the high bits (the low bits of the first draws follow
 * the seed's) */
static int roll(uint32_t *h, int n) {
	*h ^= *h << 13; *h ^= *h >> 17; *h ^= *h << 5;
	return (int)(((uint64_t)*h * (uint32_t)n) >> 32);
}

void town_folk_visit(const TownLines *lines, uint32_t seed, bool home, FolkVisit *v) {
	int visit = (run_reached() - 1) / 3, stand[MAX_FOLK], shuffled[MAX_FOLK], n = 0;
	uint32_t h = (seed ^ (uint32_t)(visit + 1) * 0x9E3779B9u) | 1u;
	for (int i = 0; i < lines->nfolk && i < MAX_FOLK; ++i) {
		const Folk *f = &lines->folk[i];
		v->who[i] = i;
		/* (one in four people out; the Mr.Prog and the robot dog never) */
		v->out[i] = f->cat == 5 && roll(&h, 4) == 0;
		if (home && f->cat == 5 && !f->walk) {
			stand[n] = shuffled[n] = i;
			++n;
		}
	}
	for (int k = n - 1; k > 0; --k) {
		int j = roll(&h, k + 1), t = shuffled[k];
		shuffled[k] = shuffled[j];
		shuffled[j] = t;
	}
	for (int k = 0; k < n; ++k) v->who[stand[k]] = shuffled[k];
}
