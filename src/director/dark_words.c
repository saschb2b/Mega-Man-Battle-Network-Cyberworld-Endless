/* MegaMan on DarkChips (docs/META.md, docs/VOICE.md). */
#include "dark_words.h"

#include <stdio.h>
#include <string.h>

#include "director_state.h"

/* MegaMan fell in an older net's battle after a DarkChip, and BN5 got him
 * up and fought with his body a while (session 68: twenty seconds out of a
 * playtester's hands, and no word of it): said before the price. */
void dark_rose_said(void) {
	char words[sizeof D.dark_words + 128];
	int n = snprintf(words, sizeof words, "@M Lan... I fell back there.|@M Something dark got me up... and fought with my body.|@M That wasn't us...%s%s",
		D.dark_words[0] ? "|" : "", D.dark_words);
	size_t len = n < 0 ? 0 : (size_t)n < sizeof D.dark_words ? (size_t)n : sizeof D.dark_words - 1;
	memcpy(D.dark_words, words, len);
	D.dark_words[len] = 0;
}

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
