/* What MegaMan says of the Crosses and the Cybeast (docs/game/meta/,
 * docs/voice/rules.md): a Cross won from a guardian and how it feels; one the
 * run cannot carry; the Graveyard's call; Dad's
 * CybeastButton after the Cybeast's fall. The Crosses themselves,
 * powers.c. */
#include <stdio.h>

#include "guardians.h"
#include "powers.h"
#include "save.h"

/* The words after guardian `navi`'s battle: his Cross where it is new to
 * the run (`cross`) and one was brought (one the run takes is BN6's own
 * "MegaMan got:" box, beside his chip's: powers_reward_cross, reward_words.c;
 * what MegaMan felt of each went with the main path's words cut,
 * docs/game/core-loop.md); the Cybeast's waking (`beast`) */
const char *powers_reward_words(int navi, bool cross, bool beast) {
	static char text[512];
	int k = 0;
	#define ADD(...) (k += snprintf(text + k, k < (int)sizeof text ? sizeof text - (size_t)k : 0, __VA_ARGS__))
	const char *brought = powers_cross_name(run.cross);
	if (cross && brought) {
		if (run.cross == navi) ADD("%s@M %s's Cross data... We already brought his Cross!", k ? "|" : "", guardian(navi)->name);
		/* (said as the net's fact, not a rule's: "One Cross a run!" put
		 * the game's word in MegaMan's mouth; and said as settled, the
		 * brought one kept: "It won't fit beside our SlashCross" read to a
		 * playtester as an offer to swap, which never came, session 64.
		 * That a first win opens its start for the next dive is the
		 * setup's to show, never MegaMan's: the voice sweep,
		 * docs/voice/sweep/net.md; one box since the main path's words cut) */
		else ADD("%s@M %s's Cross data... We keep our %s.", k ? "|" : "", guardian(navi)->name, brought);
	}
	/* (the Nest's call, and Lan's word, no unlocking: the beast stirs
	 * here, and its power comes once MegaMan has beaten it at the Nest,
	 * issue #109; a run that brought BeastOut hears the same) */
	if (beast)
		ADD("%s@B Grrrr...!|@M L-Lan... The Nest is calling to the Cybeast in me!|"
			"@L Easy,MegaMan! You've kept it in check all along!|@M ...Right. I won't let it take over,Lan.", k ? "|" : "");
	#undef ADD
	return k ? text : NULL;
}

/* Dad's word, said on in his call after the Cybeast's fall (story_words.c):
 * beat the beast, then its power (issue #109) */
const char *powers_beast_words(void) {
	return "@D One more thing,Lan.|@D MegaMan beat the beast. Its power is his now.|@D I'm unlocking your PET's CybeastButton.|"
		"@D Just don't let it take over,OK?|@M Leave it to me,Dad!|@N MegaMan can now BeastOut!";
}
