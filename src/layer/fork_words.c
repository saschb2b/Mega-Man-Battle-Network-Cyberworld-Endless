/* fork_words.h. MegaMan's read of the next layer at a fork, as BN6's
 * MegaMan reads a signal ("A strong Navi's signal..."): what he senses, never
 * a place he has not seen (docs/voice/rules.md; Hades' doors, which show the
 * kind of reward behind them). At the pad he reads where it leads: "Past this
 * pad" read as this layer's far side, and a playtester walked onto the exit
 * for a shop they took to be on this layer (session 76). */
#include "fork_words.h"

#include <stdio.h>
#include <string.h>

#include "net.h"

/* What he can sense, strongest first: a call each (`first`, `too` when it
 * follows another) */
static const struct { unsigned holds; bool portal; const char *first, *too; } calls[] = {
	{ HOLDS_GUARDIAN, false, "A strong Navi's signal!", NULL },   /* (always first) */
	{ HOLDS_SERVER, true, "A strong virus signal!", "A strong virus signal,too!" },
	{ HOLDS_PROGRAMS, true, "Someone's selling programs!", "Someone's selling programs,too!" },
	{ HOLDS_TRADER | HOLDS_BUGTRADER, true, "A trader's calling out!", "A trader's calling out,too!" },
	{ HOLDS_DEALER, false, "Someone's selling chips!", "Someone's selling chips,too!" },
};

const char *fork_words(unsigned holds, bool dark, int where) {
	static char words[200];
	bool pad = where == FORK_PAD;
	if (dark) return pad ? "@M Lan... Where this pad leads...|@M It's gone dark! Dark panels,like Nebula's!" : NULL;
	int k = snprintf(words, sizeof words, "%s", pad ? "@M Where this pad leads... " : "@M Out there... ");
	int said = 0;
	for (size_t i = 0; i < sizeof calls / sizeof *calls && said < 2; ++i) {
		if (!(holds & calls[i].holds) || (!pad && !calls[i].portal)) continue;
		k += snprintf(words + k, sizeof words - (size_t)k, "%s%s", said ? "|@M " : "", said ? calls[i].too : calls[i].first);
		++said;
	}
	if (said) return words;
	return pad ? "@M Where this pad leads... It's quiet." : NULL;
}
