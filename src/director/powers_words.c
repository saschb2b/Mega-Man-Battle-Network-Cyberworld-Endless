/* What MegaMan says of the Crosses and the Cybeast (docs/META.md,
 * docs/VOICE.md): a Cross won from a guardian, how it feels and its
 * weakness; one the run cannot carry; the Graveyard's call; Dad's
 * CybeastButton after the Cybeast's fall. The Crosses themselves,
 * powers.c. */
#include <stdio.h>

#include "guardians.h"
#include "powers.h"
#include "save.h"

/* What MegaMan feels of each Cross, by its navi (HeatMan 1 .. ChargeMan 5) */
static const char *cross_feel(int navi) {
	static const char *const feel[6] = {
		[1] = "HeatMan's Cross data! I'm all fired up,Lan!",
		[2] = "ElecMan's Cross data! It's crackling through me!",
		[3] = "SlashMan's Cross data! I feel faster already!",
		[4] = "EraseMan's Cross data... It's cold,Lan. But strong.",
		[5] = "ChargeMan's Cross data! Choo,choo! Full steam ahead!",
	};
	return navi >= 1 && navi <= 5 ? feel[navi] : "";
}

/* The words after guardian `navi`'s battle: his Cross where it is new to
 * the run (`cross`), as the run holds Crosses; the Cybeast's waking (`beast`) */
const char *powers_reward_words(int navi, bool cross, bool beast) {
	static char text[512];
	int k = 0;
	#define ADD(...) (k += snprintf(text + k, k < (int)sizeof text ? sizeof text - (size_t)k : 0, __VA_ARGS__))
	const char *brought = powers_cross_name(run.cross);
	if (cross) {
		if (!brought) ADD("%sMegaMan got:\n\"%s\"!!|@M %s|@M But in it,%s attacks hit me twice as hard!", k ? "|" : "", powers_cross_name(navi), cross_feel(navi),
			powers_cross_weakness(navi));
		else if (run.cross == navi) ADD("%s@M %s's Cross data... We already brought his Cross!", k ? "|" : "", guardian(navi)->name);
		/* (said as the net's fact, not a rule's: "One Cross a run!" put
		 * the game's word in MegaMan's mouth; a first win opens its start
		 * for good, said here, where a playtester read "won't fit" as
		 * lost, and found the start in the setup; and said as settled,
		 * the brought one kept: "It won't fit beside our SlashCross" read
		 * to a playtester as an offer to swap, which never came, session
		 * 64) */
		else ADD("%s@M %s's Cross data...|@M We can only carry one Cross down here.|@M So we keep our %s,Lan.%s", k ? "|" : "",
			guardian(navi)->name, brought, profile.crosses_open >> navi & 1 ? "" : "|@M But next dive,we can start with his Cross!");
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
