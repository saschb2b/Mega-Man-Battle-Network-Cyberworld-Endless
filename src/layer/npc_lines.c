/* What bystander navis say on the Endless Net's layers: the navis of other
 * operators who jacked in at the statue too. Each line is true of this
 * game; the chat box wraps them to its width ('|' starts a new box). They
 * gossip of what the run adds, never teach BN6 (its Mystery Data, Buster,
 * NaviCust, Chip Traders, Custom Screen: the beginners' tips and the
 * basics a newcomer heard went in the voice sweep of 19 October 2026,
 * docs/voice/sweep/net.md). */
#include "npc_lines.h"

#include "run.h"

#define N(a) (int)(sizeof a / sizeof *a)

/* the first two areas */
static const char *const early[] = {
	"Hey! You jacked in at the statue too?|These paths weren't here yesterday!",
	"It's all copied data down here. Even the viruses!|They still hurt,though... Ow.",
	"The exit pads only go down...|Nobody's found a way back up yet!",
	"Did you hear? Every area ends with a guardian!|Its exit pad stays shut until it's deleted!",
	"Net Dealers sell on each area's first layers!|There's one by every guardian! Save up!",
};

/* the middle areas */
static const char *const mid[] = {
	"Ugh... I fought a guardian just like HeatMan...|It knew every move I had!",
	"Did you hear? Dark warps show up from here on!|A side trip to an Undernet copy,and back!",
	"There's a sealed gate in the Undernet copy...|They say three ScrtData open it!",
	"ScrtData hide in blue Mystery Data down deep!|Keep your eyes open!",
	"A strong virus signal? Tough fight...|But the chip it leaves is worth it!",
	"A deleted guardian drops an HPMemory!|More HP,more chances!",
};

/* the Undernet, the Graveyard and the Nest */
static const char *const deep[] = {
	"Something at the very bottom copies the whole Net...|My operator calls it the Nest.",
	"I heard a Cybeast roar down below...|My operator says it's just data. R-Right?",
	"Some Navis down here just say the same thing...|Over and over. Copies,I guess...",
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

const char *npc_line(int depth, int i) {
	int place = (depth - 1) % CYCLE_LAYERS;
	const char *const *tier = place < 6 ? early : place < 12 ? mid : deep;
	int n = place < 6 ? N(early) : place < 12 ? N(mid) : N(deep);
	/* in a rebuilt net every other bystander remembers */
	if (depth > CYCLE_LAYERS && i % 2 == 0) return again[((i / 2) % N(again) + N(again)) % N(again)];
	return tier[((i % n) + n) % n];
}
