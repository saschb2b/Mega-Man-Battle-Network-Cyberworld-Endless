/* MegaMan on DarkChips (docs/META.md, docs/VOICE.md). */
#include "dark_words.h"

#include <stdio.h>

/* A DarkChip's price, `lost` max HP (dark_price): the first BN6 battle's,
 * its bug gone (what it burned, its flame has said); the session's first;
 * one more */
const char *dark_price_words(int kind, int lost) {
	static char words[300];
	if (kind == DARK_PRICE_BN6)
		snprintf(words, sizeof words, "@M That bug's gone now. But something stays...|@M My max HP fell by %d,for this whole dive.", lost);
	else if (kind == DARK_PRICE_FIRST)
		snprintf(words, sizeof words, "@M Ngh... That DarkChip took something from me,Lan.|@M My max HP fell by %d,for this whole dive.", lost);
	else snprintf(words, sizeof words, "@M Ngh... The DarkChip took %d more max HP.", lost);
	return words;
}

const char *dark_bn5_does(int plus, bool invs) {
	static char words[96];
	if (plus > 0) snprintf(words, sizeof words, "My next attack chip hits %d harder!", plus);
	else if (invs) snprintf(words, sizeof words, "For ten seconds,the darkness takes over!|It fights for us. Nothing can touch me!");
	else words[0] = 0;
	return words;
}

/* A DarkChip that ran as its base chip for want of a BugFrag, into `out` */
void dark_base_words(char *out, size_t n, const char *dark, const char *base) {
	snprintf(out, n, "@M We had no BugFrags,Lan...|@M So that %s was only a %s.", dark, base);
}
