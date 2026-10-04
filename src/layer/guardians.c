#include "guardians.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "net.h"
#include "rivals.h"
#include "rom.h"
#include "run.h"
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
	/* BN5's (docs/BOSSES.md, BN5's Navis): their faces and sprites copied in
	 * from its ROM as they are met (guardian_set_face), their pose its own
	 * animation 24, as Gregar's Navis' */
	[24] = { "Colonel", "Commander of the Old Net", GUARDIAN_NO_MUGSHOT, 24, 96, 184, 176 },
	[25] = { "ShadowMan", "Blade of the Shadows", GUARDIAN_NO_MUGSHOT, 24, 160, 96, 224 },
	[26] = { "NumberMan", "Master of the Odds", GUARDIAN_NO_MUGSHOT, 24, 176, 216, 80 },
	[27] = { "TomahawkMan", "Spirit of the Totem", GUARDIAN_NO_MUGSHOT, 24, 232, 88, 72 },
	[28] = { "KnightMan", "The Iron Fortress", GUARDIAN_NO_MUGSHOT, 24, 152, 136, 200 },
	[29] = { "ToadMan", "Maestro of the Marsh", GUARDIAN_NO_MUGSHOT, 24, 104, 200, 104 },
};
#define NGUARDIANS ((int)(sizeof guardians / sizeof *guardians))

const char *guardian_tip(int navi) {
	switch (navi) {
	/* (a second box, "|@M ", for when a sword lands, where a guardian's
	 * warps made swords miss:
	 * watched in god mode, HeatMan stands at the back through his tower
	 * and his flame, SpoutMan in his front column through his bubbles,
	 * SlashMan beside MegaMan some 40 frames after his slash, EraseMan at
	 * the back while his ghosts drift, and so on for each below; Colonel's
	 * dark-screen slash, unannounced, deleted a standing MegaMan) */
	case 1: return "HeatMan's fire tower crawls at us unlit and turns into our row: sidestep it late. His flamethrower sweeps the lit row. "
		"When a shadow opens under us, he's leaping there: clear the yellow!|@M He stands still at the back while his tower and flame play out: strike then!";
	case 2: return "ElecMan's current runs straight down our row, his lightning strikes the yellow panels, "
		"and he warps in close to slash. Keep moving!|@M He stands still while his lightning comes down: strike then!";
	case 3: return "SlashMan leaps in beside us to slash the lit panel, then spins across the whole field. "
		"Step off the yellow panel when he lands! Blades he leaves stuck in our side fly back across their rows.|@M He stays beside us a moment after his slash: swing back then!";
	case 4: return "EraseMan's ghosts soak up our shots and drift across the rows: dodge up and down. "
		"If our HP runs low he erases us in one blow, so heal before we face him!|@M He holds still at the back while his ghosts drift at us: strike then, with something that reaches him!";
	/* (watched: the cars come with him down the other two rows, a column
	 * or two behind; a playtester stepped into another row as the train
	 * passed and ran into a car, twice deleted by him on layer 9) */
	case 5: return "ChargeMan rams down our row like a train, and his coal bombs burst on the lit panels.|@M When his freight cars come "
		"too, they roll down the other two rows a column or two behind him and block our chips.|@M Once he's passed, step into his "
		"row behind him: the cars never follow there. Hit him as he pulls back in at the back!";
	/* (watched: landed beside us, he lights the panels at his sides alone, and
	 * his arms sweep the rows above and below them too: a playtester a
	 * panel up and over, off every lit one, was deleted there, session 57) */
	case 6: return "SpoutMan's bubbles burst over our panels, and his hose sprays water down the lit ones. Step off the yellow panels!|"
		"@M When he jumps onto our side and whirls, his arms reach the panels at his corners too, though only those at his sides light up: "
		"get two panels away from him!|@M He stands still in front of us while he blows bubbles: swing then!";
	case 7: return "TomahawkMan's eagle swoops down a lit row, and he steps in close to swing his axe wide. "
		"Step off the yellow panels, and keep our distance!|@M He stands still while his eagle swoops, and stays close a moment after his axe: strike then!";
	case 8: return "TenguMan dashes down a lit row, and his whirlwinds tear holes in our panels. "
		"Step off the yellow panels, and watch our footing!|@M He hovers right in front of us between his dashes: swing then!";
	case 9: return "GroundMan bursts up from under the lit panel with his drill, and his drill missiles run down the rows in their shadows. "
		"The panels he drills through crack: keep moving!|@M When he bursts up on our side, he stays there a moment: hit him then!";
	case 10: return "DustMan drops scrap onto the lit panels and hurls our broken panels back at us, and their first hit stuns. "
		"When he breathes in, he pulls us up close for a big punch. Keep a Recover chip ready!|@M He stands a while right in front of us between throws: swing then!";
	/* (every shape his panels light, and the one that tells less than it
	 * hits: in a playtester's netbattle the cross, a diagonal and a row
	 * lit where he slashed, and one lit panel, his, stood for a WideSword
	 * down its whole column; shots met his shield, a lock-on chip and Navi
	 * chips got through) */
	case 11: return "ProtoMan's shield stops our shots. When panels light up, he dashes in to slash them: step off every lit one.|"
		"@M If only ours lights, his WideSword takes that whole column: get out of the column, not just off the panel!|"
		"@M Chips that lock on to him, and Navi chips, get past his shield!";
	case 12: return "BlastMan's bombs roll down our row and burst, his flames dash along it, and a wall of fire sweeps across our side, a column at a time. "
		"Step off the yellow panels!|@M He hovers still while he throws his bombs: strike then!";
	case 13: return "DiveMan moves unseen under the water: hold our chips until he surfaces, then strike! "
		"When his wave lights our panels, stand in our back column. His torpedoes run in their shadows' row.";
	/* (watched: the panel under MegaMan lights for a few frames, which his
	 * feet hide, then CircusMan fades from his panel and the tent drops
	 * there; his fade came 6 frames before MegaMan was held, so the panel,
	 * marked over MegaMan by the director, is the tell. A chip used while
	 * he is away finds no target, and a playtester's HeatDrgn rose beside
	 * the tent as it deleted him; another's: the panel lit about half a
	 * second, the tent fell in 10 frames and hit 5 every 9-10 frames for
	 * 220-250, 120 in all, mashing no shorter, and one cage fell while his
	 * MachGun2 still fired, session 65) */
	case 14: return "CircusMan claps down on a lit column, and his lion leaps through a burning hoop down its row. "
		"When the panel under us lights up, his tent is about to drop on it: step off at once!|@M It drops in half a second, "
		"with no way out once it's down: no long chip while he crackles on his panel!|@M He keeps to the back: "
		"bring chips that reach it, and hold them while he's gone from the field!";
	case 15: return "JudgeMan's whip cracks down a lit row, and his books slam across the field. Step off the yellow panels!|@M He stands right in front of us while he cracks his whip: swing then!";
	case 16: return "ElementMan changes his element as he fights: whirlwinds run down our rows, and in green, logs burst up under us as grass spreads. "
		"Hard hits work whatever he is!|@M He stands right in front of us while he calls his whirlwinds: swing then!";
	/* (what a playtester's first fight against him met unwarned: his
	 * Cannons turned aside while he readied a slash, and his cape's sweep
	 * along the row he landed in, with only his own panel lit) */
	case 18: return "Colonel sends his soldiers at us: clear them out. When our panels light in a zigzag, he warps in and slashes across them. "
		"Step off the yellow panels!|@M Our hits glance off him while he readies a slash, and he stays beside us a moment after it: swing back then!|"
		"@M When he lands in our row and the screen goes dark, his cape sweeps the row: get out of it before the dark comes!";
	/* BN5's, as their own battles were watched in its engine (romlab, a
	 * frame every 20 over a minute of each, MegaMan standing in his
	 * middle panel; issue #69) */
	case 24: return "The old net's Colonel lights a slant of our panels, then warps in and slashes across them: step off the yellow! "
		"When he hefts his cannon, its blast runs down his row: get out of it.|@M Right after his slash he stands close: swing then!";
	case 25: return "ShadowMan floats over his panels and splits into copies. His pillars of fire rush down our row: get out of it! "
		"His shuriken rain down on the lit panels: step off the yellow!|@M He lands to throw them: strike then!";
	case 26: return "NumberMan's numbered balls roll at us down every row, and each one's number is its HP: break the one in our row with a shot or two!|"
		"@M His dice land on our side and blow up around where they fall: get clear of them!|@M He stands still at the back while his balls roll: strike then!";
	case 27: return "The old net's TomahawkMan throws his axe across our row, and it swings back. The totem pole at his back drops fire "
		"on the lit panels: step off the yellow!|@M He holds still while his totem calls the fire: strike then!";
	case 28: return "KnightMan stands in his stone armor, and our hits glance off it. When he swings, his wrecking ball drops on the lit panel.|"
		"@M Rocks fall where shadows open under us, and he leaps and crashes down, cracking our panels.|@M He's open while his ball swings: strike then!";
	case 29: return "ToadMan hops between his lily pads. His music notes drift at us, and their shock stuns: keep out of their way! "
		"His frogs leap over to our side.|@M He sits still on his pad while he plays: strike then!";
	default: return NULL;
	}
}

int guardian_netbattle_terms(char *out, size_t n) {
	return snprintf(out, n, "Enough racing, MegaMan.|Chaud says you're ready. This time, you face me.|"
		"@M He won't hold back, Lan. %s|@M If ProtoMan deletes us, the dive's over. We can run if it goes bad.", guardian_tip(11));
}

/* (a first meeting's hint: the net's gossip, which lives in that world and
 * could know something, as the Net Dealer's "word is" does; a name's
 * worth of insight, the moves left for the fight to show, and EraseMan's
 * danger said as a danger: an erase at low HP had come with no word) */
const char *guardian_rumor(int navi) {
	switch (navi) {
	case 1: return "his fire never runs out.";
	case 2: return "his lightning comes out of a clear sky.";
	case 3: return "you never see him coming, only his claws.";
	case 4: return "he deletes Navis outright. Don't face him weak!";
	case 5: return "he runs down anything in his path.";
	case 6: return "he floods the whole field.";
	case 7: return "he never fights alone: something circles overhead.";
	case 8: return "he moves like the wind itself.";
	case 9: return "he comes up from below.";
	case 10: return "he turns your own panels against you.";
	case 11: return "his shield stops everything, and his sword is faster than sight.";
	case 12: return "he leaves nothing but craters.";
	case 13: return "he hunts from under the water.";
	case 14: return "he runs his show from the back of the ring.";
	case 15: return "he passes sentence with a whip.";
	case 16: return "he's never the same element twice.";
	case 18: return "he commands an army, and ends fights with a single stroke.";
	/* (BN5's: the older net's Navis, as the net tells of them) */
	case 24: return "the old net's Colonel runs his ground like a battlefield.";
	case 25: return "he strikes from the dark and is gone before you turn.";
	case 26: return "he lets the dice decide, and the dice like him.";
	case 27: return "the old net's spirits fight at his side.";
	case 28: return "no attack has ever dented his armor.";
	case 29: return "his music leaves Navis unable to move.";
	default: return NULL;
	}
}

bool guardian_known(int navi) {
	const Rival *rv = rival(navi);
	return rv->megaman_won + rv->navi_won > 0;
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
	/* BN5's: the Nest's copies of the older net's Navis (our words; BN5's
	 * own text is never copied) */
	[24] = { "This sector of the old net is under my command.|Stand down, or be removed by force!",
		"You broke my line once. I have drawn a new one.",
		"Retreat was your wisest move last time. It still is.",
		"Reinforcements have arrived. The odds are mine now.",
		"Outmaneuvered... The sector... is yours..." },
	[25] = { "A target walks into the dark. How careless.|My blades are already moving!",
		"You slipped my shadow once. Not twice.",
		"The shadows took you last time. They are hungry again.",
		"My shadow has grown. You cannot see all of me now.",
		"The shadow... fades..." },
	[26] = { "Calculating... Your odds of winning: zero point zero!|Let's roll the numbers!",
		"An error in my last calculation. Corrected!",
		"My numbers said you'd lose, and you did! Let's check them again!",
		"Bigger numbers, better odds! It all adds up to your defeat!",
		"Does not... compute..." },
	[27] = { "The old net's spirits watch this ground.|Show them your courage, stranger!",
		"You stood your ground once. Stand again!",
		"The spirits chose me last time. They choose me still.",
		"The totem burns brighter now!",
		"The spirits... have spoken..." },
	[28] = { "Halt! None pass this gate while I stand!|No blow has ever broken my armor!",
		"You dented my armor once. It has been forged anew.",
		"You fell before my iron ball. Kneel again!",
		"Thicker armor, a heavier swing. Despair!",
		"My armor... broken..." },
	[29] = { "Ribbit! A new audience for my concert!|Let the music play, and you dance!",
		"You stopped my song last time. Encore, ribbit!",
		"Ribbit ribbit! My song put you to sleep last time!",
		"A whole new symphony, ribbit! Louder than ever!",
		"The concert... is over... ribbit..." },
};
#define NLINES ((int)(sizeof lines / sizeof *lines))

/* Rematches after many of MegaMan's wins: grudging respect. */
static const char *const respect[] = {
	"You again. You keep getting stronger...|This time I won't hold back!",
	"How many times must we fight?|Until one of us stops standing!",
	"I've studied every move you've made.|Let's see what you've learned!",
};

/* MegaMan's answer, for the lines that give him one ("I'm ready this
 * time!" only after a loss: a playtester 3-0 against SpoutMan heard it) */
static const char *const replies[] = {
	"I won't lose!", "Let's go!", "Bring it on!",
};

/* How MegaMan and Lan take a guardian they have never met: BN6's Link Navis
 * and ProtoMan as friends in copied form, and the rest as the Nest's
 * copies of foes. */
static bool friendly(int navi) { return (navi >= 1 && navi <= 3) || navi == 5 || navi == 11; }

/* Falzar's Navis, whose faces Gregar lacks, speak with the portrait set
 * for them where the layer made one, else without a face: the HeelNavi's
 * is the bystanders' (a playtester read SpoutMan's lines as a
 * bystander's); BN5's with their own, copied in (xnavi_guardian) */
static int set_face[NGUARDIANS];   /* the face + 1, 0 for none */

int guardian_sprite(int navi) {
	static const struct { uint8_t navi, sprite; } sprites[] = {
		{ 1, 0x47 }, { 2, 0x49 }, { 3, 0x4B }, { 4, 0x50 }, { 5, 0x4F },   /* Heat, Elec, Slash, Erase, Charge */
		{ 11, 0x3B }, { 12, 0x51 }, { 13, 0x52 }, { 14, 0x54 }, { 15, 0x55 }, /* Proto, Blast, Dive, Circus, Judge */
		{ 16, 0x56 }, { 18, 0x53 },                                          /* Element, Colonel */
	};
	for (unsigned i = 0; i < sizeof sprites / sizeof *sprites; ++i)
		if (sprites[i].navi == navi) return sprites[i].sprite;
	/* (BN5's: his own, copied in at the face's number, guardian_set_face) */
	if (guardian_older(navi) && set_face[navi]) return set_face[navi] - 1;
	return GUARDIAN_HEEL_SPRITE;
}

int guardian_older_sprite(int navi) {
	/* (BN5's lists 6 and 8: docs/ROM_DATA.md, BN5's Navis) */
	static const uint8_t numbers[GUARDIAN_OLDER_LAST - GUARDIAN_OLDER_FIRST + 1] = { 69, 75, 71, 72, 73, 74 };
	return guardian_older(navi) ? numbers[navi - GUARDIAN_OLDER_FIRST] : 0;
}

/* Every guardian in his own shape (they had stood in a HeelNavi's body,
 * seven of seventeen, the copies "that didn't come out right"; the owner
 * asked for their personality back):
 *   Gregar's own overworld sprite where it has one, which faces every way;
 *   BlastMan's and ElementMan's, which Gregar draws only facing right
 *   through down-left (its story never turns them away from the camera):
 *   turned down-right or down-left where the arena would face them into
 *   the screen, as they stood invisible there;
 *   Falzar's Navis, whom Gregar has no overworld sprite of, in their
 *   battle sprites (list 0, 0x2E + the navi, each uncompressed), which face
 *   left as they fight, mirrored to face right, standing in animation 0
 *   and logging in by BN6's warp-in, their animation 3 (docs/ROM_DATA.md). */
NpcBody guardian_body(int navi, int face) {
	NpcBody b = { 6, guardian_sprite(navi), face, false, NPC_ANIM_LOG_IN, guardian(navi)->pose };
	if (navi == 12 || navi == 16) b.anim = face == 1 ? 3 : face == 7 ? 5 : face;
	if (navi >= 6 && navi <= 10) b = (NpcBody){ 0, 0x2E + navi, 0, face >= 1 && face <= 3, 3, -1 };
	return b;
}

void guardian_set_face(int navi, int face) {
	if (navi > 0 && navi < NGUARDIANS) set_face[navi] = face < 0 ? 0 : face + 1;
}

int guardian_face(int navi) {
	const Guardian *g = guardian(navi);
	if (g->mugshot != GUARDIAN_NO_MUGSHOT) return g->mugshot;
	return navi > 0 && navi < NGUARDIANS && set_face[navi] ? set_face[navi] - 1 : FACE_NONE;
}

/* How MegaMan and Lan take guardian `navi`, `name`, at the first meeting:
 * a friend's copy, a foe's, or a Navi of the older net (BN5's, copied with
 * it) */
static const char *first_sight(int navi, const char *name) {
	static char s[128];
	if (guardian_older(navi)) snprintf(s, sizeof s, "@L That's %s, from the older net!|@M The Nest copied its Navis too, Lan!|", name);
	else if (friendly(navi)) snprintf(s, sizeof s, "@M %s?! ...No. You're one of the Nest's copies!|", name);
	else snprintf(s, sizeof s, "@L That's %s! Or a copy the Nest made of him...|", name);
	return s;
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
	if (!r->megaman_won && !r->navi_won && !r->met) ADD("%s", first_sight(navi, name));
	/* and at every meeting after, who speaks without a face (a playtester
	 * who had met SpoutMan three times asked who was talking) */
	else if (guardian_face(navi) == FACE_NONE) ADD("@L %s's copy again!|", name);
	/* the Nest's own guardian knows what it is */
	if (biome == BIOME_NEST) ADD("The Nest built me from every battle you have fought.|");
	ADD("%s", s);
	/* MegaMan answers now and then, never over a first meeting */
	/* (after a loss the record in it: "I'm ready this time!" before every
	 * rematch read as a loop to a playtester two down against CircusMan) */
	static const char *const times[] = { "", "", "twice", "three times", "four times" };
	static const char *const next[] = { "", "", "a third", "a fourth", "a fifth" };
	if (r->met && r->last == RIVAL_NAVI_WON && r->navi_won >= 2 && r->navi_won <= 4)
		ADD("|@M %s has beaten us %s. Not %s time!", name, times[r->navi_won], next[r->navi_won]);
	else if (r->met && r->last == RIVAL_NAVI_WON && r->navi_won > 4) ADD("|@M %d times %s has beaten us. Not this time!", r->navi_won, name);
	else if (r->met && r->last == RIVAL_NAVI_WON) ADD("|@M I'm ready this time!");
	else if (r->met && r->met % 3 != 1) ADD("|@M %s", replies[r->met % 3]);
	/* and, from battle data, when to strike, just before the fight: the
	 * briefing on his layer came ten minutes before a playtester's fight,
	 * who asked for it again at the arena */
	const char *tip = guardian_known(navi) ? guardian_tip(navi) : NULL, *when = NULL;
	for (const char *p = tip; p && (p = strstr(p, "|@M ")) != NULL; p += 4) when = p + 4;
	if (when && *when) ADD("|@M Remember our battle data, Lan: %c%s", *when >= 'A' && *when <= 'Z' ? *when - 'A' + 'a' : *when, when + 1);
	ADD("|@L Battle routine, set!|@M Execute!");
	#undef ADD
	return buf;
}

const char *guardian_defeat(int navi) {
	return navi > 0 && navi < NLINES && lines[navi].defeat ? lines[navi].defeat : "Ugh... You win...";
}

/* Another game's area where it draws `biome` in this run (run_dress). */
static const NetAreaDef *dressed(int biome) {
	int a = run_dress(biome);
	return a >= NET_AREAS ? net_area_def(a) : NULL;
}

/* BN6's own name for the area */
static const char *own_name(int biome) {
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

const char *guardian_area_name(int biome) {
	const NetAreaDef *x = dressed(biome);
	return x ? x->name : own_name(biome);
}

const char *guardian_area_short(int biome) {
	const NetAreaDef *x = dressed(biome);
	if (x) return x->short_name;
	/* (nine letters at most: the PET's PLACE holds twelve, with the layer) */
	static const char *const names[BIOME_COUNT] = {
		[BIOME_CENTRAL] = "Central", [BIOME_SEASIDE] = "Seaside", [BIOME_SKY] = "Sky Area", [BIOME_GREEN] = "Green",
		[BIOME_GRAVEYARD] = "Graveyard", [BIOME_UNDERNET] = "Undernet", [BIOME_SECRET] = "Secret", [BIOME_NEST] = "Nest",
		[BIOME_COMP] = "RoboDog", [BIOME_HOMEPAGE] = "Aquarium", [BIOME_COMP_B] = "Lab Comp", [BIOME_ROBOT_COMP] = "RobotComp",
		[BIOME_AQUARIUM_COMP] = "Aquarium", [BIOME_JUDGE_COMP] = "JudgeTree", [BIOME_WEATHER_COMP] = "Weather",
		[BIOME_COPYBOT_COMP] = "CopyBot", [BIOME_ACDC_HP] = "ACDC HP", [BIOME_GREEN_HP] = "Green HP", [BIOME_SKY_HP] = "Sky HP",
	};
	return biome >= 0 && biome < BIOME_COUNT && names[biome] ? names[biome] : "Net";
}

const char *guardian_area_in_text(int biome, int side) {
	static char buf[32];
	if (side == LAYER_UNDERNET) biome = BIOME_UNDERNET;
	else if (side == LAYER_SECRET) biome = BIOME_SECRET;
	/* (another game's area in its place goes by its own name, "Nebula
	 * Area", but by BN6's as BN6 says it: BN5's Undernet is the Undernet) */
	const char *name = guardian_area_name(biome);
	bool the = !strcmp(name, own_name(biome)) && (biome == BIOME_GRAVEYARD || biome == BIOME_UNDERNET || biome == BIOME_SECRET || biome == BIOME_NEST);
	snprintf(buf, sizeof buf, "%s%s", the ? "the " : "", name);
	return buf;
}

/* ... as the split names a way: another game's area as the older net's,
 * which two playtesters could not tell from BN6's own (session 65) */
const char *guardian_way_area(int biome) {
	static char buf[48];
	const char *name = guardian_area_in_text(biome, LAYER_NORMAL);
	if (!dressed(biome)) return name;
	snprintf(buf, sizeof buf, "the older net's %s", strncmp(name, "the ", 4) ? name : name + 4);
	return buf;
}

const char *guardian_area_motto(int biome) {
	const NetAreaDef *x = dressed(biome);
	if (x) return x->motto;
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

/* (where no way's guardian has been battled, it is said once: "A Navi
 * we've never battled guards Aquarium HP, and a Navi we've never battled
 * guards Judge Tree Comp." read to a playtester as the sentence repeating
 * itself, session 64; a second of three is "another") */
void guardian_way_question(char *out, size_t n, const char *const who[3], const char *const area[3], int dark) {
	static const char sealed[] = "|@M A dark way leads down into the Undernet itself too, but it's sealed. Clearing the Secret Area, past the "
		"golden gate, would open it. Which way?";
	int ways = dark == 2 ? 3 : 2, unknown = 0;
	for (int k = 0; k < ways; ++k) unknown += !who[k];
	if (unknown == ways && ways == 3) {
		snprintf(out, n, "@M The net splits below us, Lan! Navis we've never battled guard all three ways: %s, %s, and a dark way down "
			"into the Undernet. Which way?", area[0], area[1]);
		return;
	}
	if (unknown == ways) {
		snprintf(out, n, "@M The net splits below us, Lan! Navis we've never battled guard both ways: %s and %s.%s", area[0], area[1],
			dark ? sealed : " Which way?");
		return;
	}
	char said[3][48];
	for (int k = 0, told = 0; k < ways; ++k)
		snprintf(said[k], sizeof said[k], "%s", who[k] ? who[k] : told++ ? "another we've never battled" : "a Navi we've never battled");
	/* (the first names a sentence's start) */
	if ('a' <= said[0][0] && said[0][0] <= 'z') said[0][0] = (char)(said[0][0] - 32);
	if (ways == 3)
		snprintf(out, n, "@M The net splits below us, Lan! %s guards %s,|@M %s guards %s, and a dark way leads down into the Undernet, "
			"where %s waits. Which way?", said[0], area[0], said[1], area[1], said[2]);
	else
		snprintf(out, n, "@M The net splits below us, Lan! %s guards %s,|@M and %s guards %s.%s", said[0], area[0], said[1], area[1],
			dark ? sealed : " Which way?");
}
