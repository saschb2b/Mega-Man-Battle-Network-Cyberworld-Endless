/* home_words.h. Coming home after an act: MegaMan's relief and Lan's
 * answer, one beat a box (docs/VOICE.md), by how far the run has come. */
#include "home_words.h"

#include <stdio.h>
#include <string.h>

#include "guardians.h"
#include "run.h"

const char *home_words(const char *beaten, int ways, const char *const ports[3], const char *sealed) {
	static char words[400];
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
	/* (the ports and where they are: docs/HOME.md) */
	size_t k = strlen(words);
	if (ways == 2)
		snprintf(words + k, sizeof words - k, "|@M Two ports are open this time!|@M The %s,and the %s!", ports[0], ports[1]);
	else if (ways == 3)
		snprintf(words + k, sizeof words - k, "|@M Three ports are open!|@M The %s,the %s...|@M And something dark at the %s.", ports[0],
			ports[1], ports[2]);
	k = strlen(words);
	if (sealed) snprintf(words + k, sizeof words - k, "|@M Something dark waits at the %s too...", sealed);
	return words;
}

const char *home_port_words(int biome, int navi, bool dark, bool sealed) {
	static char words[200];
	const char *area = guardian_way_area(biome);
	if (navi && !guardian_known(navi)) navi = 0;
	if (sealed)
		snprintf(words, sizeof words, "@M A dark way leads down here...|@M It's sealed. Clearing the Secret Area would open it.");
	else if (dark)
		snprintf(words, sizeof words, "@M Lan... This one leads into the Undernet.|@M %s waits down there!",
			navi ? guardian(navi)->name : "A Navi we've never battled");
	else if (navi)
		snprintf(words, sizeof words, "@M This port goes to %s,Lan.|@M %s waits there!", area, guardian(navi)->name);
	else
		snprintf(words, sizeof words, "@M This port goes to %s,Lan.|@M A Navi we've never battled waits there...", area);
	return words;
}
