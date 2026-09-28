#include "guardians.h"

#include <stddef.h>
#include <stdio.h>

#include "net.h"
#include "rivals.h"
#include "text.h"
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
	[18] = { "Colonel", "The Iron Strategist", 0x53, -1, 120, 168, 136 },
};
#define NGUARDIANS ((int)(sizeof guardians / sizeof *guardians))

const char *guardian_tip(int navi) {
	switch (navi) {
	case 1: return "HeatMan sends fire towers along the panels at us, and his flames come down the panels he lights. "
		"Keep moving, and step off the yellow panels!";
	case 2: return "ElecMan's current runs straight down our row, his lightning strikes the yellow panels, "
		"and he warps in close to slash. Keep moving!";
	case 3: return "SlashMan leaps in beside us to slash the lit panel, then spins across the whole field. "
		"Step off the yellow panel when he lands!";
	case 4: return "EraseMan's ghosts soak up our shots, and if our HP runs low he can erase us in one blow. "
		"Keep our HP up, and hit him with swords and bombs!";
	case 5: return "ChargeMan rams straight down our row like a train. Step aside, then hit back!";
	case 6: return "SpoutMan's bubbles burst over our panels, his hose sprays water down the lit ones, "
		"and he jumps onto our side to whirl his arms beside him. Step off the yellow panels!";
	case 7: return "TomahawkMan's eagle swoops down a lit row, and he steps in close to swing his axe wide. "
		"Step off the yellow panels, and keep our distance!";
	case 8: return "TenguMan dashes down a lit row, and his whirlwinds tear holes in our panels. "
		"Step off the yellow panels, and watch our footing!";
	case 9: return "GroundMan drills through the panels. Watch our footing!";
	case 10: return "DustMan drops scrap onto the lit panels and hurls our broken panels back at us, and their first hit stuns. "
		"When he breathes in, he pulls us up close for a big punch. Keep a Recover chip ready!";
	case 11: return "ProtoMan's shield stops our shots. Hit him when he swings his sword!";
	case 12: return "BlastMan's bombs roll down our row and burst, his flames dash along it, and a fire wall walks down a column. "
		"Step off the yellow panels!";
	case 13: return "DiveMan hides under the water, where nothing hits him: strike when he surfaces! "
		"When his wave lights our panels, stand in our back column. His torpedoes run in their shadows' row.";
	case 14: return "CircusMan keeps to the back of his area, claps down on a lit column and drops his tent to trap us. "
		"Keep moving, and bring chips that reach the back!";
	case 15: return "JudgeMan's whip cracks down a lit row, and his books slam across the field. Step off the yellow panels!";
	case 16: return "ElementMan changes his element as he fights. Hard hits work whatever he is!";
	case 18: return "Colonel sends his soldiers at us. Clear them out, then go for him!";
	default: return NULL;
	}
}

const Guardian *guardian(int navi) {
	static const Guardian unknown = { "???", "Guardian", GUARDIAN_NO_MUGSHOT, -1, 200, 200, 200 };
	return navi > 0 && navi < NGUARDIANS && guardians[navi].name ? &guardians[navi] : &unknown;
}

/* What each says: meeting MegaMan the first time, again after losing to
 * him, after beating him, as a stronger version, and when deleted. They
 * are the Nest's copies (docs/BOSSES.md), built from every battle the net
 * has seen: they remember. */
static const struct { const char *first, *rematch, *revenge, *stronger, *defeat; } lines[] = {
	[1] = { "So you're the one diving through this net. Let's see if you can take the heat!",
		"You put out my fire last time. Now I'll burn twice as hot!",
		"Back for more burns? You never learn, do you?",
		"The Nest stoked my flames even hotter. Burn to ash!",
		"Tch... My fire... went out..." },
	[2] = { "The current down here answers to me. One jolt is all it takes!",
		"Last time was a fluke. I'm fully recharged!",
		"You felt my voltage before. Feel it again!",
		"A million volts more than before! Brace yourself!",
		"Short circuit... Impossible..." },
	[3] = { "Heh. Fresh prey came crawling in. My claws will split you apart!",
		"You got lucky. The Swift Claw never misses twice!",
		"Still in one piece? Let me fix that.",
		"I'm faster than you can see now!",
		"Too... slow...? Not me..." },
	[4] = { "Target confirmed: MegaMan.EXE. Commencing deletion.",
		"Deletion failed last time. That will not repeat.",
		"I deleted you once. Doing it again is a formality.",
		"Deletion program upgraded. You will not escape.",
		"Error... Target... not... deleted..." },
	[5] = { "Full steam ahead! Who's on my tracks?|All aboard for your last ride!",
		"You derailed me once. Not again! Choo choo!",
		"Heh, got run over last time, huh? Next stop: you!",
		"Engine overhauled! Top speed! Out of my way!",
		"Engine... stalled... End of the line..." },
	[6] = { "Splash! Water is my element! I'll wash you right out!",
		"You made waves last time. Now I'll sink you!",
		"Glub glub! Back for another swim?",
		"The tide is rising! Nothing can stop it!",
		"Bloop... I'm all dried up..." },
	[7] = { "A warrior walks into my path. Face me with honor!",
		"Your spirit beat mine once. My axe remembers.",
		"My axe felled you before. It will again.",
		"My spirit burns stronger now!",
		"You fight... with true honor..." },
	[8] = { "Hohoho! A guest! Let my wind carry you off!",
		"You rode out my storm once. Not this time!",
		"Hohoho! Blown away before, blown away again!",
		"This gale could topple mountains!",
		"The wind... has turned..." },
	[9] = { "Rumble rumble! You're standing on my turf!|I'll drill you into the floor!",
		"You dug me up last time. I'm going deeper!",
		"Buried you once! I'll bury you again!",
		"New drill bit! Nothing is too hard now!",
		"Drill... jammed..." },
	[10] = { "Scrap! Junk! You'll make a fine addition to my heap!",
		"You slipped out of my heap. It won't happen again!",
		"You made such nice junk last time!",
		"My heap has grown! It'll crush you!",
		"Just... junk... after all..." },
	[11] = { "MegaMan. Show me your strength. Draw your weapon.",
		"You bested me once. My blade has been honed.",
		"You fell to my blade before. Rise higher.",
		"I have surpassed my limits. Come.",
		"...Well done. Go on ahead." },
	[12] = { "KABOOM! Came for the show?|Then watch me blow it all up!",
		"Your last trick was explosive. Mine are bigger!",
		"Haha! Want to go up in smoke again?",
		"Bigger booms! Hotter blasts! My best show yet!",
		"The show's... over...?" },
	[13] = { "Dive! Dive! Intruder in the deep!|Torpedoes ready... Fire!",
		"You sank my plans once. Full power this time!",
		"Surfaced again, did you? Back down you go!",
		"Hull reinforced! I can't be sunk!",
		"Taking on water... Abandon ship..." },
	[14] = { "Welcome, welcome! The show is about to begin...|...and you're the main act!",
		"The crowd wants a rematch! Let's not disappoint!",
		"Encore! Encore! Let's make you cry again!",
		"A brand new act, even scarier than the last!",
		"The curtain... falls..." },
	[15] = { "Order! The court is in session.|The defendant, MegaMan, stands accused!",
		"The verdict was overturned once. Not on appeal!",
		"Guilty then, guilty now! The sentence stands!",
		"The law has been rewritten in my favor!",
		"Court... is... adjourned..." },
	[16] = { "Fire, water, wood, lightning. All the elements obey me!",
		"You broke my harmony once. I have rebalanced.",
		"The elements rejected you. They still do.",
		"I have mastered every element!",
		"The elements... abandon me..." },
	[18] = { "Soldier. This position is held by me. Your advance stops here!",
		"You took this position once. I have revised my strategy.",
		"The last campaign was mine. So is this one.",
		"My forces have doubled. Your odds have not.",
		"A strategic... retreat..." },
};
#define NLINES ((int)(sizeof lines / sizeof *lines))

/* Rematches after many of MegaMan's wins: grudging respect. */
static const char *const respect[] = {
	"You again. You keep getting stronger...|This time I won't hold back!",
	"How many times must we fight?|Until one of us stops standing!",
	"I've studied every move you've made.|Let's see what you've learned!",
};

/* MegaMan's answer, for the lines that give him one. */
static const char *const replies[] = {
	"I won't lose!", "Let's go!", "Bring it on!", "I'm ready this time!",
};

/* How MegaMan and Lan take a guardian they have never met: BN6's Link Navis
 * and ProtoMan as friends in copied form, its villains, and the Navis of
 * the other Cybeast's version, whose copies came out in a HeelNavi's shape. */
static bool friendly(int navi) { return (navi >= 1 && navi <= 3) || navi == 5 || navi == 11; }
int guardian_sprite(int navi) {
	static const struct { uint8_t navi, sprite; } sprites[] = {
		{ 1, 0x47 }, { 2, 0x49 }, { 3, 0x4B }, { 4, 0x50 }, { 5, 0x4F },   /* Heat, Elec, Slash, Erase, Charge */
		{ 11, 0x3B }, { 13, 0x52 }, { 14, 0x54 }, { 15, 0x55 },            /* Proto, Dive, Circus, Judge */
		{ 18, 0x53 },                                                        /* Colonel */
	};
	for (unsigned i = 0; i < sizeof sprites / sizeof *sprites; ++i)
		if (sprites[i].navi == navi) return sprites[i].sprite;
	return GUARDIAN_HEEL_SPRITE;
}

/* the navis who stand in a HeelNavi's shape */
static bool misshapen(int navi) { return guardian_sprite(navi) == GUARDIAN_HEEL_SPRITE; }

int guardian_face(int navi) {
	const Guardian *g = guardian(navi);
	return g->mugshot == GUARDIAN_NO_MUGSHOT ? FACE_HEEL : g->mugshot;
}

const char *guardian_intro(int navi, int version, int biome) {
	static char buf[800];
	const Rival *r = rival(navi);
	const char *name = guardian(navi)->name;
	const char *s = NULL;
	if (navi > 0 && navi < NLINES && lines[navi].first) {
		/* (a meeting without a result, MegaMan gone or the run over, is no
		 * rematch) */
		bool fought = r->megaman_won || r->navi_won;
		if (!fought) s = lines[navi].first;
		else if (r->last == RIVAL_NAVI_WON) s = lines[navi].revenge;
		else if (version > 0 && r->megaman_won % 2) s = lines[navi].stronger;
		else if (r->megaman_won >= 3) s = respect[r->megaman_won % 3];
		else s = lines[navi].rematch;
	}
	if (!s) s = "The way on is through me. Prepare yourself!";
	int k = 0;
	#define ADD(...) (k += snprintf(buf + k, k < (int)sizeof buf ? sizeof buf - (size_t)k : 0, __VA_ARGS__))
	/* the first meeting: who MegaMan and Lan see */
	if (!r->megaman_won && !r->navi_won && !r->met) {
		if (friendly(navi)) ADD("@M %s?! ...No. You're one of the Nest's copies!|", name);
		else if (misshapen(navi)) ADD("@M That voice... it's %s! But that's a HeelNavi's body!|@L The Nest's copy didn't come out right!|", name);
		else ADD("@L That's %s! Or a copy the Nest made of him...|", name);
	}
	/* the Nest's own guardian knows what it is */
	if (biome == BIOME_NEST) ADD("The Nest built me from every battle you have fought.|");
	ADD("%s", s);
	/* MegaMan answers now and then, never over a first meeting */
	if (r->met && r->met % 3 != 1) ADD("|@M %s", replies[r->met % 4]);
	ADD("|@L Battle routine, set!|@M Execute!");
	#undef ADD
	return buf;
}

const char *guardian_defeat(int navi) {
	return navi > 0 && navi < NLINES && lines[navi].defeat ? lines[navi].defeat : "Ugh... You win...";
}

const char *guardian_area_name(int biome) {
	static const char *const names[BIOME_COUNT] = {
		[BIOME_CENTRAL] = "Central Area", [BIOME_SEASIDE] = "Seaside Area", [BIOME_SKY] = "Sky Area",
		[BIOME_GREEN] = "Green Area", [BIOME_GRAVEYARD] = "Graveyard", [BIOME_UNDERNET] = "Undernet",
		[BIOME_SECRET] = "Secret Area", [BIOME_NEST] = "Cybeast Nest", [BIOME_COMP] = "RoboDog Comp",
		[BIOME_HOMEPAGE] = "Aquarium HP", [BIOME_COMP_B] = "Lab Comps",
		[BIOME_ROBOT_COMP] = "Robot Control Comp", [BIOME_AQUARIUM_COMP] = "Aquarium Comp",
		[BIOME_JUDGE_COMP] = "Judge Tree Comp", [BIOME_WEATHER_COMP] = "Mr. Weather Comp",
		[BIOME_COPYBOT_COMP] = "CopyBot's Comp", [BIOME_ACDC_HP] = "ACDC HP", [BIOME_GREEN_HP] = "Green HP",
		[BIOME_SKY_HP] = "Sky HP",
	};
	return biome >= 0 && biome < BIOME_COUNT && names[biome] ? names[biome] : "the Net";
}

const char *guardian_area_in_text(int biome, int side) {
	static char buf[32];
	if (side == LAYER_UNDERNET) biome = BIOME_UNDERNET;
	else if (side == LAYER_SECRET) biome = BIOME_SECRET;
	bool the = biome == BIOME_GRAVEYARD || biome == BIOME_UNDERNET || biome == BIOME_SECRET || biome == BIOME_NEST;
	snprintf(buf, sizeof buf, "%s%s", the ? "the " : "", guardian_area_name(biome));
	return buf;
}

const char *guardian_area_motto(int biome) {
	static const char *const mottos[BIOME_COUNT] = {
		[BIOME_CENTRAL] = "Where every net path begins", [BIOME_SEASIDE] = "Currents of the aquarium net",
		[BIOME_SKY] = "Above the clouds of data", [BIOME_GREEN] = "Wild data, overgrown",
		[BIOME_GRAVEYARD] = "Where deleted data rests", [BIOME_UNDERNET] = "The lawless depths",
		[BIOME_SECRET] = "Where the strongest wait", [BIOME_NEST] = "Lair of the Cybeasts",
		[BIOME_COMP] = "Circuits of a home comp", [BIOME_HOMEPAGE] = "The aquarium's own homepage",
		[BIOME_COMP_B] = "Deep in the lab's machines",
		[BIOME_ROBOT_COMP] = "The city's robots run here", [BIOME_AQUARIUM_COMP] = "Mazes of water and light",
		[BIOME_JUDGE_COMP] = "Roots of the great tree", [BIOME_WEATHER_COMP] = "Where the forecast is made",
		[BIOME_COPYBOT_COMP] = "A copy of a copy", [BIOME_ACDC_HP] = "Home of ACDC Town",
		[BIOME_GREEN_HP] = "Home of Green Town", [BIOME_SKY_HP] = "Home of Sky Town",
	};
	return biome >= 0 && biome < BIOME_COUNT && mottos[biome] ? mottos[biome] : "";
}
