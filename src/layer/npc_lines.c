/* What bystander navis say on the Endless Net's layers: the navis of other
 * operators who jacked in at the statue too. Each line is true of this
 * game; the chat box wraps them to its width ('|' starts a new box). */
#include "npc_lines.h"

#include "run.h"

#define N(a) (int)(sizeof a / sizeof *a)

/* the first two areas */
static const char *const early[] = {
	"My operator jacked me in at the statue too. These paths weren't here yesterday!",
	"Everything down here is copied data. Even the viruses!|They still hurt, though.",
	"The exit pad only goes down. Nobody's found a way back up yet!",
	"Each area's last layer has a guardian. The exit pad stays shut until it's deleted.",
	"Green Mystery Data holds chips and Zenny. The blue ones hold rarer things!",
	"Net Dealers set up shop on an area's first layers, and one waits by every guardian. Save some Zenny!",
	"Hold B to charge your buster. A charged shot makes most viruses flinch.",
};

/* the middle areas */
static const char *const mid[] = {
	"I fought a guardian that looked just like HeatMan...|It knew every move I had!",
	"From here on, dark warps show up on some layers: a side trip into a copy of the Undernet, and back.",
	"There's a sealed gate in the Undernet copy. They say three ScrtData open it.",
	"ScrtData hide in blue Mystery Data on the deeper layers. Keep your eyes open!",
	"A strong virus signal is a tough fight, but the chip it leaves is worth it.",
	"A NaviCust program does nothing until it's installed. In the PET: MegaMan, then NaviCust!",
	"A deleted guardian leaves its HPMemory behind. More HP, more chances!",
};

/* the Undernet, the Graveyard and the Nest */
static const char *const deep[] = {
	"Something at the very bottom is copying the whole net. My operator calls it the Nest.",
	"I heard a Cybeast roar from below...|My operator says it's just data. Right?",
	"Some navis down here don't answer. They just say the same thing over and over.|Copies, I guess...",
	"The BugFrag Trader takes BugFrags, not chips. The Undernet's full of them.",
	"Heal up before a guardian. There's always a Mr. Prog by the arena.",
	"My operator wants to jack me out. But I have to see what's at the bottom!",
};

/* a net the Nest has rebuilt */
static const char *const again[] = {
	"Wait... Haven't we met? The Nest rebuilt everything. Maybe even me.",
	"The Nest went down, and the net came right back. Same areas, stronger data.",
	"The guardians came back tougher. It's like they learned from last time.",
	"Every time the Nest falls, it builds the net again. How deep does it go?",
};

/* anywhere */
static const char *const tips[] = {
	"Chips that share a code can be sent together. Stack them in your folder!",
	"Chip Traders swap three of your chips for one. Good for clearing out junk.",
	"When the Custom Gauge fills, press L or R to pick new chips right away.",
	"AreaGrab steals the enemy's front column. More room to move, less room to hide!",
	"A fight going badly? An operator can try to pull their Navi out: L on the Custom screen. It doesn't always work!",
	"Something coming at you while you pick chips? SELECT on the Custom screen hides it for a look at the field!",
};

const char *npc_line(int depth, int i) {
	int place = (depth - 1) % CYCLE_LAYERS;
	const char *const *tier = place < 6 ? early : place < 12 ? mid : deep;
	int n = place < 6 ? N(early) : place < 12 ? N(mid) : N(deep);
	/* in a rebuilt net every other bystander remembers */
	if (depth > CYCLE_LAYERS && i % 2 == 0) return again[((i / 2) % N(again) + N(again)) % N(again)];
	/* (the beginner's tips in the first act only: deeper, a bystander
	 * telling a playtester how the Custom Gauge works rang false) */
	int nt = place < 3 ? N(tips) : 0;
	int k = ((i % (n + nt)) + n + nt) % (n + nt);
	return k < n ? tier[k] : tips[k - n];
}
