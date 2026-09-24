#include "guardians.h"

#include <stddef.h>

#include "rivals.h"
#include "run.h"

/* Mugshots share their index with the Navi's overworld sprite (list 6);
 * Gregar has none for Falzar's Navis. Pose 24 is the signature move the
 * Gregar Navis' overworld sprites carry. */
static const Guardian guardians[] = {
	[1] = { "HeatMan", "Flame of the Expo", 0x47, 24, 248, 120, 40 },
	[2] = { "ElecMan", "Master of Current", 0x49, 24, 248, 224, 64 },
	[3] = { "SlashMan", "The Swift Claw", 0x4B, 24, 120, 200, 96 },
	[4] = { "EraseMan", "The Deleter", 0x50, 24, 176, 96, 224 },
	[5] = { "ChargeMan", "Engine of Ruin", 0x4F, 24, 232, 72, 56 },
	[6] = { "SpoutMan", "Keeper of Tides", GUARDIAN_NO_MUGSHOT, -1, 96, 176, 248 },
	[7] = { "TomahawkMan", "Warrior of Green", GUARDIAN_NO_MUGSHOT, -1, 216, 168, 72 },
	[8] = { "TenguMan", "Lord of the Gale", GUARDIAN_NO_MUGSHOT, -1, 224, 88, 88 },
	[9] = { "GroundMan", "Driller of Depths", GUARDIAN_NO_MUGSHOT, -1, 232, 176, 48 },
	[10] = { "DustMan", "The Scrap Heap", GUARDIAN_NO_MUGSHOT, -1, 168, 168, 136 },
	[11] = { "ProtoMan", "Blade of Justice", 0x3B, -1, 232, 56, 72 },
	[12] = { "BlastMan", "The Living Blast", 0x51, -1, 248, 136, 48 },
	[13] = { "DiveMan", "Terror of the Deep", 0x52, -1, 72, 136, 232 },
	[14] = { "CircusMan", "Ringmaster of Fear", 0x54, -1, 232, 64, 96 },
	[15] = { "JudgeMan", "Voice of Verdict", 0x55, -1, 88, 104, 216 },
	[16] = { "ElementMan", "Lord of Elements", 0x56, -1, 176, 136, 232 },
	[17] = { "Colonel", "The Iron Strategist", 0x53, -1, 120, 168, 136 },
};
#define NGUARDIANS ((int)(sizeof guardians / sizeof *guardians))

const Guardian *guardian(int navi) {
	static const Guardian unknown = { "???", "Guardian", GUARDIAN_NO_MUGSHOT, -1, 200, 200, 200 };
	return navi > 0 && navi < NGUARDIANS && guardians[navi].name ? &guardians[navi] : &unknown;
}

/* What each says: meeting MegaMan the first time, again after losing to
 * him, after beating him, as a stronger version, and when deleted. */
static const struct { const char *first, *rematch, *revenge, *stronger, *defeat; } lines[] = {
	[1] = { "So you're the one\nclimbing the net.|Let's see if you\ncan take the heat!",
		"You again! This\ntime I'll burn\nhotter!",
		"Back for more\nburns? You never\nlearn!",
		"My flames have\nno limit now!\nBurn to ash!",
		"Tch... My fire\nwent out..." },
	[2] = { "The current here\nanswers to me.|One jolt is all\nit takes!",
		"Last time was a\nfluke. My power\nhas recharged!",
		"You felt my\nvoltage before.\nFeel it again!",
		"A million volts\nmore than before!\nPrepare yourself!",
		"Short circuit...\nImpossible..." },
	[3] = { "Heh. Fresh prey\ncame crawling in.|My claws will\nsplit you apart!",
		"You got lucky.\nThe Swift Claw\nnever misses twice",
		"Still in one\npiece? Let me fix\nthat.",
		"I'm faster than\nyou can see now!",
		"Too... slow...?\nNot me..." },
	[4] = { "Target confirmed.\nMegaMan.EXE.|Commencing\ndeletion.",
		"Deletion failed\nlast time. It\nwill not repeat.",
		"You were deleted\nonce. Again is\na formality.",
		"Deletion program\nupgraded. You\nwill not escape.",
		"Error... Target\nnot... deleted..." },
	[5] = { "Full steam ahead!\nWho's on my\ntracks?|All aboard for\nyour last ride!",
		"You derailed me\nonce. Not again!\nChoo choo!",
		"Heh, got run over\nlast time, huh?\nNext stop: you!",
		"Engine overhauled!\nTop speed! Out\nof my way!",
		"Engine... stall...\nEnd of the line..." },
	[6] = { "Splash! This\nwater is mine!|I'll wash you\nright out!",
		"You made waves\nlast time. Now\nI'll drown you!",
		"Glub glub! Back\nfor another\nswim?",
		"The tide rises!\nNo stopping it!",
		"Bloop... I'm all\ndried up..." },
	[7] = { "A warrior walks\ninto my forest.|Face me with\nhonor!",
		"Your spirit beat\nmine once. My axe\nremembers.",
		"The forest\nsleeps well since\nI felled you.",
		"My spirit burns\nstronger now!",
		"You fight with\ntrue honor..." },
	[8] = { "Hohoho! A guest\nin my sky!|Let the wind\ncarry you off!",
		"You rode out my\nstorm once. Not\nthis time!",
		"Hohoho! Blown\naway before!\nAgain!",
		"This gale could\ntopple mountains!",
		"The wind... has\nturned..." },
	[9] = { "Rumble rumble!\nYou're standing\non my turf!|I'll drill you\ninto the floor!",
		"You dug me up\nlast time. I'm\ngoing deeper!",
		"Buried you once!\nAnd I'll bury\nyou again!",
		"New drill bit!\nNothing is too\nhard now!",
		"Drill... jammed..." },
	[10] = { "Scrap! Junk!\nYou'll join my\ncollection!|Into the heap\nyou go!",
		"You slipped out\nof my heap. It\nwon't happen again",
		"You were such\nnice junk last\ntime!",
		"My heap has\ngrown! It'll\ncrush you!",
		"Just... junk...\nafter all..." },
	[11] = { "MegaMan. Show me\nyour strength.|Draw your\nweapon.",
		"You bested me\nonce. My blade\nhas been honed.",
		"You fell to my\nblade before.\nRise higher.",
		"...I have\nsurpassed my\nlimits. Come.",
		"...Well done.\nGo on ahead." },
	[12] = { "KABOOM! Did you\ncome for a show?|Then watch me\nblow it all up!",
		"Your last trick\nwas explosive.\nMine are bigger!",
		"Haha! Want to go\nup in smoke\nagain?",
		"Bigger booms!\nHotter blasts!\nMy best show!",
		"The show's...\nover...?" },
	[13] = { "Dive! Dive!\nIntruder in the\ndeep!|Torpedoes ready!\nFire!",
		"You sank my plans\nonce. Full\npower this time!",
		"Surface again,\ndid you? Back\ndown you go!",
		"Hull reinforced!\nI can't be sunk!",
		"Taking on water...\nAbandon ship..." },
	[14] = { "Welcome, welcome!\nThe show is about\nto begin!|And you are the\nmain act!",
		"The crowd wants\na rematch! Let's\nnot disappoint!",
		"Encore! Encore!\nLet's make you\ncry again!",
		"A brand new act!\nEven scarier than\nthe last!",
		"The curtain...\nfalls..." },
	[15] = { "Order! The court\nis in session.|The defendant,\nMegaMan, stands\naccused!",
		"The verdict was\noverturned once.\nNot on appeal!",
		"Guilty then,\nguilty now! The\nsentence stands!",
		"The law has been\nrewritten in my\nfavor!",
		"Court... is...\nadjourned..." },
	[16] = { "Fire, water,\nwood, lightning.|All the elements\nobey me!",
		"You broke my\nharmony once. I\nhave rebalanced.",
		"The elements\nrejected you.\nThey still do.",
		"I have mastered\nevery element!",
		"The elements...\nabandon me..." },
	[17] = { "Soldier. This\nposition is held\nby me.|Your advance\nstops here!",
		"You took this\nposition once. I\nhave re-planned.",
		"The last campaign\nwas mine. So is\nthis one.",
		"My forces have\ndoubled. Your\nodds have not.",
		"A strategic...\nretreat..." },
};
#define NLINES ((int)(sizeof lines / sizeof *lines))

/* Rematches after many of MegaMan's wins: grudging respect. */
static const char *const respect[] = {
	"You again. You\nkeep getting\nstronger...|This time I\nwon't hold back!",
	"How many times\nmust we fight?|Until one of us\nstops standing!",
	"I've studied\nevery move you\nmade.|Let's see what\nyou learned!",
};

/* MegaMan's answer, for the lines that give him one. */
static const char *const replies[] = {
	"I won't lose!", "Let's go!", "Bring it on!", "I'm ready!",
};

GuardianLine guardian_intro(int navi, int version) {
	static char buf[256], who[8];
	const Rival *r = rival(navi);
	const char *s = NULL;
	if (navi > 0 && navi < NLINES && lines[navi].first) {
		if (!r->met) s = lines[navi].first;
		else if (r->last == RIVAL_NAVI_WON) s = lines[navi].revenge;
		else if (version > 0 && r->met % 2) s = lines[navi].stronger;
		else if (r->megaman_won >= 3) s = respect[r->met % 3];
		else s = lines[navi].rematch;
	}
	if (!s) s = "The way on is\nthrough me.|Prepare yourself!";
	/* MegaMan answers now and then, never over a first meeting */
	int n = 0;
	const char *p = s;
	for (; *p; ++p) n += *p == '|';
	bool reply = r->met && r->met % 3 != 1;
	int k = 0;
	for (p = s; *p && k < (int)sizeof buf - 40; ++p) buf[k++] = *p;
	buf[k] = 0;
	for (int i = 0; i <= n && i < 6; ++i) who[i] = 'N';
	who[n + 1] = 0;
	if (reply && n < 5) {
		const char *a = replies[r->met % 4];
		buf[k++] = '|';
		while (*a && k < (int)sizeof buf - 1) buf[k++] = *a++;
		buf[k] = 0;
		who[n + 1] = 'M';
		who[n + 2] = 0;
	}
	return (GuardianLine){ buf, who };
}

GuardianLine guardian_defeat(int navi) {
	const char *s = navi > 0 && navi < NLINES && lines[navi].defeat ? lines[navi].defeat : "Ugh... You win...";
	return (GuardianLine){ s, "N" };
}

const char *guardian_area_name(int biome) {
	static const char *const names[BIOME_COUNT] = {
		[BIOME_CENTRAL] = "Central Area", [BIOME_SEASIDE] = "Seaside Area", [BIOME_SKY] = "Sky Area",
		[BIOME_GREEN] = "Green Area", [BIOME_GRAVEYARD] = "Graveyard", [BIOME_UNDERNET] = "Undernet",
		[BIOME_SECRET] = "Secret Area", [BIOME_NEST] = "Cybeast Nest", [BIOME_COMP] = "Comp Network",
		[BIOME_HOMEPAGE] = "Homepages", [BIOME_COMP_B] = "Lab Comps",
	};
	return biome >= 0 && biome < BIOME_COUNT && names[biome] ? names[biome] : "the Net";
}

const char *guardian_area_motto(int biome) {
	static const char *const mottos[BIOME_COUNT] = {
		[BIOME_CENTRAL] = "Where every net path begins", [BIOME_SEASIDE] = "Currents of the aquarium net",
		[BIOME_SKY] = "Above the clouds of data", [BIOME_GREEN] = "Wild data, overgrown",
		[BIOME_GRAVEYARD] = "Where deleted data rests", [BIOME_UNDERNET] = "The lawless depths",
		[BIOME_SECRET] = "Beyond the sealed gate", [BIOME_NEST] = "Lair of the Cybeasts",
		[BIOME_COMP] = "Circuits of a home comp", [BIOME_HOMEPAGE] = "Pages of the net's citizens",
		[BIOME_COMP_B] = "Deep in the lab's machines",
	};
	return biome >= 0 && biome < BIOME_COUNT && mottos[biome] ? mottos[biome] : "";
}
