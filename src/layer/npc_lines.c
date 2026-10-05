/* What bystander navis say on the Endless Net's layers: the navis of other
 * operators who jacked in at the statue too. Each line is true of this
 * game; the chat box wraps them to its width ('|' starts a new box). */
#include "npc_lines.h"

#include "run.h"
#include "save.h"

#define N(a) (int)(sizeof a / sizeof *a)

/* the first two areas */
static const char *const early[] = {
	"Hey! You jacked in at the statue too?|These paths weren't here yesterday!",
	"It's all copied data down here. Even the viruses!|They still hurt,though... Ow.",
	"The exit pads only go down...|Nobody's found a way back up yet!",
	"Did you hear? Every area ends with a guardian!|Its exit pad stays shut until it's deleted!",
	"Green Mystery Data has chips and Zenny inside!|The blue ones? Even rarer stuff!",
	"Net Dealers sell on each area's first layers!|There's one by every guardian! Save up!",
	"My operator holds B to charge my Buster!|A charged shot makes most viruses flinch!",
	"Careful! Mystery Data in battle breaks at any hit!|Even your own! Win with it whole,and it's yours!",
};

/* the middle areas */
static const char *const mid[] = {
	"Ugh... I fought a guardian just like HeatMan...|It knew every move I had!",
	"Did you hear? Dark warps show up from here on!|A side trip to an Undernet copy,and back!",
	"There's a sealed gate in the Undernet copy...|They say three ScrtData open it!",
	"ScrtData hide in blue Mystery Data down deep!|Keep your eyes open!",
	"A strong virus signal? Tough fight...|But the chip it leaves is worth it!",
	"My new program did nothing until I installed it!|In the PET,pick MegaMan,then NaviCust!",
	"A deleted guardian drops an HPMemory!|More HP,more chances!",
};

/* the Undernet, the Graveyard and the Nest */
static const char *const deep[] = {
	"Something at the very bottom copies the whole Net...|My operator calls it the Nest.",
	"I heard a Cybeast roar down below...|My operator says it's just data. R-Right?",
	"Some Navis down here just say the same thing...|Over and over. Copies,I guess...",
	"The BugFrag Trader wants BugFrags,not chips!|And the Undernet's full of 'em!",
	"Phew... I always heal up before a guardian.|There's a Mr.Prog by every arena!",
	"My operator wants to jack me out...|But I've gotta see what's at the bottom!",
};

/* a net the Nest has rebuilt */
static const char *const again[] = {
	"Wait... Haven't we met before?|The Nest rebuilt everything. Maybe even me...",
	"The Nest fell,and the Net came right back!|Same areas,but stronger data. Yikes...",
	"Ugh,the guardians came back tougher!|Like they learned from last time...",
	"Every time the Nest falls,it builds the Net again.|How deep does this go...?",
};

/* (the first areas' lines a profile's first runs need, which ring false
 * after: on a playtester's tenth, a bystander's word on green and blue
 * Mystery Data, session 63) */
static const bool early_basic[N(early)] = { [4] = true, [6] = true, [7] = true };
#define VETERAN_RUNS 3   /* runs played (profile.runs), from which the basics are left out */

/* anywhere */
static const char *const tips[] = {
	"Did you know?|Chips of one code can be sent together!|Stack 'em in your Folder!",
	"Got junk chips? Try a Chip Trader!|Three chips in,one chip out!",
	"Custom Gauge full? My operator hits L or R!|New chips right away! Neat,huh?",
	"AreaGrab steals the enemy's front column!|More room to move,less room to hide!",
	"My operator pulls me out of bad fights!|L on the Custom Screen! It doesn't always work!",
	"Something coming at you while picking chips?|Press SELECT! The Custom Screen hides for a peek!",
};

const char *npc_line(int depth, int i) {
	int place = (depth - 1) % CYCLE_LAYERS;
	const char *const *tier = place < 6 ? early : place < 12 ? mid : deep;
	int n = place < 6 ? N(early) : place < 12 ? N(mid) : N(deep);
	/* in a rebuilt net every other bystander remembers */
	if (depth > CYCLE_LAYERS && i % 2 == 0) return again[((i / 2) % N(again) + N(again)) % N(again)];
	bool veteran = profile.runs >= VETERAN_RUNS;
	/* (the beginner's tips in a newcomer's first act only: deeper, a
	 * bystander telling a playtester how the Custom Gauge works rang
	 * false) */
	int nt = place < 3 && !veteran ? N(tips) : 0;
	if (veteran && tier == early) {
		const char *kept[N(early)];
		int m = 0;
		for (int j = 0; j < N(early); ++j) if (!early_basic[j]) kept[m++] = early[j];
		return kept[((i % m) + m) % m];
	}
	int k = ((i % (n + nt)) + n + nt) % (n + nt);
	return k < n ? tier[k] : tips[k - n];
}
