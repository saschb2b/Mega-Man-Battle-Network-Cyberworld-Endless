/* extras_text.h. Menu words, short and plain: what each extra brings to a
 * run, as KeyItem's box says what an item is, two lines at most. */
#include "extras_text.h"

#include <stddef.h>

#include "run.h"

const HelperInfo *extra_words(int e) {
	/* (in EXTRA_*'s order; the tournament's "not in runs yet" goes once
	 * its bracket reaches runs, docs/game/tournament.md) */
	static const HelperInfo extras[EXTRAS] = {
		{ "Liberation", "BN5's Liberation Missions, on a dark layer past some forks", NULL },
		{ "Tournament", "Three Navis in a row in act 3's arena, in some runs", "Not in runs yet" },
		{ "Glitch", "A rare glitching warp to a layer that comes apart", NULL },
		{ "Virus Battler", "BN6's Virus Battler at a side path's end, in some acts", NULL },
		{ "DarkChips", "A flame of darkness offers a DarkChip in each act", NULL },
		{ "Older nets", "Other games' areas, Navis and viruses in BN6's net", NULL },
	};
	return &extras[e >= 0 && e < EXTRAS ? e : 0];
}
