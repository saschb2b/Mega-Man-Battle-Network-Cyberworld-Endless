/* MegaMan on DarkChips (docs/META.md, docs/VOICE.md). */
#include "dark_words.h"

#include <stdio.h>

/* A DarkChip's price, `lost` max HP (dark_price): the first BN6 battle's
 * whole of it, BugFrags and bug and all; the session's first; one more */
const char *dark_price_words(int kind, int lost) {
	static char words[300];
	if (kind == DARK_PRICE_BN6)
		snprintf(words, sizeof words, "@M That DarkChip burned a BugFrag each time,Lan.|@M Its darkness bugged me till the battle ended.|"
			"@M That bug's gone now. But something stays...|@M My max HP fell by %d,for this whole dive.", lost);
	else if (kind == DARK_PRICE_FIRST)
		snprintf(words, sizeof words, "@M Ngh... That DarkChip took something from me,Lan.|@M My max HP fell by %d,for this whole dive.", lost);
	else snprintf(words, sizeof words, "@M Ngh... The DarkChip took %d more max HP.", lost);
	return words;
}

/* A DarkChip that ran as its base chip for want of a BugFrag, into `out` */
void dark_base_words(char *out, size_t n, const char *dark, const char *base) {
	snprintf(out, n, "@M We had no BugFrags,Lan...|@M So that %s was only a %s.", dark, base);
}
