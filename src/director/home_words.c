/* home_words.h. Coming home after an act: MegaMan's relief and Lan's
 * answer, one beat a box (docs/VOICE.md), by how far the run has come. */
#include "home_words.h"

#include <stdio.h>

#include "run.h"

const char *home_words(const char *beaten) {
	static char words[240];
	const char *who = beaten ? beaten : "The guardian";
	int act = (run.depth - 1) % CYCLE_LAYERS / 3;   /* (acts cleared in this cycle, the Nest's 0) */
	if (act == 0)
		snprintf(words, sizeof words, "@M Lan... The Net's rebuilding.|@L Then we dive again! Ready,MegaMan?");
	else if (run_short_nest(run.depth))
		snprintf(words, sizeof words, "@M Phew... %s's deleted,Lan.|@M The Nest is next.|@L Then we get ready. Together!", who);
	else if (act == 1)
		snprintf(words, sizeof words, "@M Phew! We're home,Lan!|@M %s's deleted!|@L Nice one,MegaMan! Let's catch our breath.", who);
	else if (act == 2)
		snprintf(words, sizeof words, "@M Whew... %s was tough.|@L But we won! Home sweet home!", who);
	else
		snprintf(words, sizeof words, "@M Home again,Lan!|@L Alright! Let's get set for the next dive!");
	return words;
}
