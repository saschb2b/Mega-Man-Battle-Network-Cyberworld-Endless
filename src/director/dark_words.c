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
