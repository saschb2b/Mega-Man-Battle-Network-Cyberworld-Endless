/* What the guardians say, and MegaMan and Lan of them (docs/VOICE.md,
 * docs/BOSSES.md): each one's lines meeting MegaMan, again, after a win or
 * a loss, stronger, deleted; the net's rumor of him and MegaMan's battle
 * data on him; ProtoMan's netbattle terms; the split's question. Who they
 * are, guardians.c. */
#include <stdio.h>
#include <string.h>

#include "guardians.h"
#include "rivals.h"
#include "save.h"
#include "super_lines.h"
#include "text.h"

const char *guardian_tip(int navi) {
	/* (the super bosses' own: super_lines.c) */
	if (super_boss(navi)) return super_tip(navi);
	switch (navi) {
	/* (the last box, "|@M ", for when a sword lands, where a guardian's
	 * warps made swords miss:
	 * watched in god mode, HeatMan stands at the back through his tower
	 * and his flame, SpoutMan in his front column through his bubbles,
	 * SlashMan beside MegaMan some 40 frames after his slash, EraseMan at
	 * the back while his ghosts drift, and so on for each below; Colonel's
	 * dark-screen slash, unannounced, deleted a standing MegaMan) */
	case 1: return "His fire tower gives no yellow warning!|@M It turns into our row. Sidestep it late!|@M His flamethrower sweeps the lit row.|"
		"@M A shadow under us? He's leaping there. Move!|@M He stays back while his fire burns. Strike!";
	case 2: return "His current runs straight down our row!|@M His lightning strikes the yellow panels.|@M He warps in close to slash. Keep moving!|"
		"@M Strike while his lightning falls!";
	case 3: return "He leaps beside us to slash the lit panel!|@M Step off it when he lands!|@M Then he spins across the whole field.|"
		"@M Blades left on our side fly back down their rows!|@M After his slash,he stays close. Swing!";
	case 4: return "His ghosts soak up our shots!|@M They drift across the rows. Dodge up and down!|@M Low on HP? He erases us in one blow!|"
		"@M Heal up before we face him!|@M He stays back as his ghosts drift. Shoot!";
	/* (watched: the cars come with him down the other two rows, a column
	 * or two behind; a playtester stepped into another row as the train
	 * passed and ran into a car, twice deleted by him on layer 9) */
	case 5: return "He rams down our row like a train!|@M His coal bombs burst on the lit panels.|@M His freight cars follow in the other rows,blocking chips!|"
		"@M Once he passes,step into his row. Cars never follow!|@M Hit him as he pulls back in!";
	/* (watched: landed beside us, he lights the panels at his sides alone, and
	 * his arms sweep the rows above and below them too: a playtester a
	 * panel up and over, off every lit one, was deleted there, session 57) */
	case 6: return "His bubbles burst over our panels!|@M His hose sprays the lit ones. Step off!|@M When he whirls on our side,get two panels away!|"
		"@M His arms reach his corners too. Those don't light!|@M He blows bubbles up close. Swing!";
	case 7: return "His eagle swoops down a lit row!|@M He steps in close to swing his axe wide.|@M Step off the yellow and keep our distance!|"
		"@M His eagle and axe leave him open. Strike!";
	case 8: return "He dashes down a lit row!|@M His whirlwinds tear holes in our panels.|@M Step off the yellow and watch our footing!|"
		"@M Between dashes,he hovers close. Swing!";
	case 9: return "He drills up under the lit panel!|@M His drill missiles run down their shadows' rows.|@M Panels he drills through crack. Keep moving!|"
		"@M When he bursts up on our side,hit him!";
	case 10: return "He drops scrap on the lit panels!|@M He throws our broken panels back. The first hit stuns!|"
		"@M When he breathes in,he pulls us close to punch!|@M Keep a Recover chip ready!|@M Between throws,he stands close. Swing!";
	/* (every shape his panels light, and the one that tells less than it
	 * hits: in a playtester's netbattle the cross, a diagonal and a row
	 * lit where he slashed, and one lit panel, his, stood for a WideSword
	 * down its whole column; shots met his shield, a lock-on chip and Navi
	 * chips got through) */
	case 11: return "His shield stops our shots.|@M When panels light,he slashes them. Step off every one!|"
		"@M If only ours lights,his WideSword hits that column!|@M Get out of the column,not just off the panel!|"
		"@M Lock-on and Navi chips get through!";
	case 12: return "His bombs roll down our row and burst!|@M His flames dash along it too.|@M His fire wall sweeps our side,column by column!|"
		"@M Step off the yellow panels!|@M He hovers still to throw bombs. Strike!";
	case 13: return "He moves unseen under the water!|@M His wave lights our panels? Get to our back column!|@M His torpedoes run down their shadows' rows.|"
		"@M Wait till he surfaces. Then strike!";
	/* (watched: the panel under MegaMan lights for a few frames, which his
	 * feet hide, then CircusMan fades from his panel and the tent drops
	 * there; his fade came 6 frames before MegaMan was held, so the panel,
	 * marked over MegaMan by the director, is the tell. A chip used while
	 * he is away finds no target, and a playtester's HeatDrgn rose beside
	 * the tent as it deleted him; another's: the panel lit about half a
	 * second, the tent fell in 10 frames and hit 5 every 9-10 frames for
	 * 220-250, 120 in all, mashing no shorter, and one cage fell while his
	 * MachGun2 still fired, session 65) */
	case 14: return "He claps down on a lit column!|@M His lion leaps down the burning hoop's row.|@M The panel under us lights? His tent's coming. Move!|"
		"@M It falls in half a second. No escape after!|@M No chips that keep firing while he crackles!|"
		"@M He stays back. Bring chips that reach!|@M Never attack while he's gone!";
	case 15: return "His whip cracks down a lit row!|@M His books slam across the field.|@M Step off the yellow panels!|"
		"@M He whips right in front of us. Swing!";
	case 16: return "His element changes as he fights!|@M His whirlwinds run down our rows.|@M In green,logs burst up under us as grass spreads!|"
		"@M Hard hits work whatever he is!|@M He calls whirlwinds up close. Swing!";
	/* (what a playtester's first fight against him met unwarned: his
	 * Cannons turned aside while he readied a slash, and his cape's sweep
	 * along the row he landed in, with only his own panel lit) */
	case 18: return "He sends soldiers at us. Clear them out!|@M Zigzag panels light? He warps in to slash. Step off!|"
		"@M Our hits glance off while he readies a slash.|@M He stays beside us after it. Swing back then!|"
		"@M In the dark,his cape sweeps the row he's in!|@M Leave his row before the dark comes!";
	/* BN5's, as their own battles were watched in its engine (romlab, a
	 * frame every 20 over a minute of each, MegaMan standing in his
	 * middle panel; issue #69) */
	case 24: return "A slant of panels lights? He warps in to slash!|@M Step off the yellow!|@M He hefts his cannon? Get out of his row!|"
		"@M He stays close after his slash. Swing!";
	case 25: return "He floats and splits into copies!|@M His fire pillars rush down our row. Get out!|@M His shuriken rain on the lit panels. Step off!|"
		"@M He lands to throw shuriken. Strike!";
	case 26: return "His numbered balls roll down every row!|@M Each number is its HP. Break ours with a shot!|"
		"@M His dice blow up around where they land. Get clear!|@M He stays back while his balls roll. Strike!";
	case 27: return "His axe flies down our row,then swings back!|@M His totem pole drops fire on the lit panels.|@M Step off the yellow!|"
		"@M He holds still as his totem burns. Strike!";
	case 28: return "Our hits glance off his stone armor!|@M His wrecking ball drops on the lit panel!|"
		"@M Rocks fall where shadows open under us!|@M He leaps and crashes down,cracking our panels.|@M He's open while his ball swings. Strike!";
	case 29: return "He hops between his lily pads.|@M His music notes stun! Keep out of their way!|@M His frogs leap over to our side!|"
		"@M He sits still while he plays. Strike!";
	default: return NULL;
	}
}

int guardian_netbattle_terms(char *out, size_t n) {
	return snprintf(out, n, "Enough,MegaMan. No more racing.|Chaud says you're ready. Face me.|"
		"@M %s|@M If he deletes us,our dive's over!|@L We can still run if it goes bad.", guardian_tip(11));
}

/* (a first meeting's hint: the net's gossip, which lives in that world and
 * could know something, as the Net Dealer's "word is" does; a name's
 * worth of insight, the moves left for the fight to show, and EraseMan's
 * danger said as a danger: an erase at low HP had come with no word) */
const char *guardian_rumor(int navi) {
	switch (navi) {
	case 1: return "his fire never goes out!";
	case 2: return "his lightning strikes out of nowhere!";
	case 3: return "you never see him coming. Just his claws!";
	case 4: return "he deletes Navis outright. Don't face him weak!";
	case 5: return "he flattens anything on his tracks!";
	case 6: return "he floods the whole field!";
	case 7: return "he never fights alone. Watch the sky!";
	case 8: return "he's as fast as the wind!";
	case 9: return "he comes up from below!";
	case 10: return "he turns your own panels against you!";
	case 11: return "his shield stops everything,and his sword's a blur!";
	case 12: return "he leaves nothing but craters!";
	case 13: return "he hunts from under the water!";
	case 14: return "he runs his show from the back!";
	case 15: return "he passes sentence with a whip!";
	case 16: return "he's never the same element twice!";
	case 18: return "his army is huge. One stroke ends it!";
	/* (BN5's: the older net's Navis, as the net tells of them) */
	case 24: return "the old Net's Colonel runs it like a war!";
	case 25: return "he strikes from the dark,then he's gone!";
	case 26: return "he lets the dice decide,and they like him!";
	case 27: return "the old Net's spirits fight at his side!";
	case 28: return "no attack has ever dented his armor!";
	case 29: return "his music freezes Navis in place!";
	default: return NULL;
	}
}

/* What each says: meeting MegaMan the first time, again after losing to
 * him, after beating him, as a stronger version, and when deleted. They
 * are the Nest's copies (docs/BOSSES.md), built from every battle the net
 * has seen: they remember. */
static const struct { const char *first, *rematch, *revenge, *stronger, *defeat; } lines[] = {
	/* (each in his habit of speech, docs/VOICE.md; DiveMan's "Awooga!",
	 * JudgeMan's "thou" and ElementMan's beeps as BN6's script gives them
	 * in their own boxes, faces 0x52, 0x55 and 0x56: verified there, where
	 * the guide's table has Capt'n Blackbeard's "matey" for DiveMan) */
	[1] = { "Hoo! I'm all fired up!|C'mon,MegaMan! Take the heat!",
		"You put out my fire once!|Now I burn twice as hot! C'mon!",
		"Back for more burns,huh?|C'mon! I'll toast you again!",
		"The Nest stoked my flames!|I'm hotter than ever! Burn!",
		"Ngh... My fire... went out..." },
	[2] = { "...tzz. I have been waiting.|One jolt will be enough.",
		"That loss was a fluke...tzz.|I am fully recharged.",
		"You felt my voltage before.|...tzz. Shall we repeat it?",
		"...tzz,tzz. My voltage has doubled.|Brace yourself.",
		"Tzz...zz... A short circuit...?" },
	[3] = { "Slaaash! Fresh prey!|My claws will cut you to ribbons!",
		"You got lucky last time.|Slash! I won't miss twice!",
		"Still in one piece?|Slaaash! Let me fix that!",
		"I'm faster than ever now!|Slash! You won't even see me!",
		"Too... slow...? Me...?" },
	[4] = { "Hyahaha! There you are!|My scythe wants to delete you!",
		"You slipped away last time...|My scythe won't miss! Hyahaha!",
		"Hyahaha! I deleted you once!|Let's do it again!",
		"My scythe got sharper!|One swing,and you're gone! Hyahaha!",
		"Hya...ha...? Me... deleted...?" },
	[5] = { "Choo,choo!! Departure time!|Next stop,your deletion!",
		"You derailed me last time!|Not again! Choo,choooo!!",
		"Choo,choo! Back on my tracks?|Please stand clear of the train!",
		"Engine overhauled! Top speed!|Choo,choo! Clear the line!",
		"Choo...oo... End of the line..." },
	[6] = { "D-Don't come closer,drip!|I-I'll wash you away!",
		"Y-You beat me last time...|N-Not again,drip!",
		"Drip! You came back!?|I'll wash you out again!",
		"The tide is rising,drip!|I-I'm not scared anymore!",
		"Driiip...! I'm all dried up..." },
	[7] = { "Yo! A warrior,huh?|There's a saying... \"The bold win!\"",
		"You beat me fair and square.|But they say,\"Fall seven,rise eight!\"",
		"My axe got you before!|As they say,\"Old habits die hard!\"",
		"They say spirit grows in battle!|Mine's burning bright now!",
		"Ngh... You fight with honor..." },
	[8] = { "Hmph! A guest on my wind...|Show me your skill,grasshopper!",
		"Hmph. You rode out my storm once.|Do not expect it twice!",
		"Blown away last time,grasshopper!|Have you trained at all?",
		"Hmmmph!! My gale has grown!|It could topple mountains!",
		"Nngh... The wind... has turned..." },
	[9] = { "Whiiiir! Yo,this is my turf!|I'll drill ya right under!",
		"Ya dug me up last time.|Whiiir! Now I'm goin' deeper!",
		"Buried ya once,didn't I?|Whiiiir! Let's do it again!",
		"Got me a new drill bit!|Whiiiir! Nothin's too hard now!",
		"Whii...rrr... Drill's jammed..." },
	[10] = { "Gahaha! Fresh scrap!|Into the trash with ya!",
		"Ya slipped outta my heap!|Gahaha! I'll suck ya right up!",
		"Gahahaha! Ya made fine junk!|Back for the scrap heap?",
		"My vacuum's twice as strong!|Gahahaha! Nothin' escapes!",
		"Gah... I'm the junk... now..." },
	[11] = { "...MegaMan.|Draw your weapon. Show me.",
		"You bested me once.|My blade is sharper now.",
		"You fell to my blade before.|...Rise higher.",
		"I have surpassed my limits.|...Come.",
		"...Well done. Go on ahead." },
	[12] = { "Kwohohoho... A visitor!|I'll burn you to cinders!",
		"You got lucky,brat.|Kwohohoho... Not twice!",
		"Kwohohoho! Back for more?|You'll go up in smoke again!",
		"The Nest fed my flames...|Kwohohoho! Nothing will be left!",
		"Gwaah! Me... in ashes...!?" },
	[13] = { "Awooga! Awooga! Intruder!|Dive! Dive! Torpedoes ready!",
		"You sank me last time.|Full power now! Awooga!",
		"Surfaced again,did you?|Back to the depths! Awooga!",
		"Hull reinforced! Awooga!|Nothing can sink me now!",
		"Taking on water...! Awoo...ga..." },
	[14] = { "Ahoo,hoo,hoo! Welcome,welcome!|You're the star of my show!",
		"The crowd wants a rematch!|Ahoo,hoo,hoo! Showtime!",
		"Encore! Encore!|Let's make you cry again! Ahoo,hoo!",
		"A brand new act!|Even scarier! Ahoo,hoo,hoo!",
		"Ahoo...hoo... Curtain call..." },
	[15] = { "Order! Order!|MegaMan,thou art accused!",
		"Thou hast won the last trial.|This appeal shall go my way!",
		"Guilty then,guilty now!|Thy sentence stands!",
		"The law now favors me!|Thou hast no defense!",
		"Court... is adjourned...!?" },
	[16] = { "PIKIRARA... PIKIRI!|Fire,water,wood... All obey me.",
		"PIRIRA... You broke my harmony once.|It is whole again.",
		"The elements rejected you.|PIKIRI... They still do.",
		"KIRAPIRA!! I have mastered every element!",
		"PIKIRAAAA!! The elements... leave me..." },
	[18] = { "Halt,MegaMan.|Your advance ends here.",
		"You took this ground once.|I have revised my strategy.",
		"The last battle was mine.|So is this one.",
		"My forces have doubled.|Your odds have not.",
		"Ngh... A strategic... retreat..." },
	/* BN5's: the Nest's copies of the older net's Navis (our words; BN5's
	 * own text is never copied) */
	[24] = { "This sector is under my command.|Stand down,or be removed!",
		"You broke my line once.|I have drawn a new one.",
		"Retreat was wise last time.|It still is.",
		"Reinforcements have arrived.|The odds are mine now.",
		"Outmaneuvered... The sector is yours..." },
	[25] = { "...A careless target.|My blades are already moving.",
		"You slipped my shadow once.|Not twice.",
		"The shadows took you before.|They hunger again.",
		"My shadow has grown.|You cannot see all of me.",
		"The shadow... fades..." },
	[26] = { "Calculating... Your odds are zero!|Let's roll the numbers!",
		"My last calculation had an error!|Now it's corrected!",
		"My numbers said you'd lose!|Let's check them again!",
		"Bigger numbers,better odds!|It all adds up to your defeat!",
		"Does not... compute..." },
	[27] = { "The spirits watch this ground.|Show them your courage!",
		"You stood your ground once.|Stand again!",
		"The spirits chose me last time.|They choose me still.",
		"The totem burns brighter now!",
		"The spirits... have spoken..." },
	[28] = { "Halt,young knight!|None pass while I stand guard!",
		"You dented my armor once.|It has been forged anew.",
		"You fell to my iron ball.|Kneel once more,if you please!",
		"Thicker armor,a heavier swing!|Despair,young knight!",
		"My armor... broken... Well fought..." },
	[29] = { "Ribbit! A new audience!|Let the music play! Now dance!",
		"You stopped my song last time.|Encore,ribbit!",
		"Ribbit ribbit! My song stunned you!|Shall I play it again?",
		"A whole new symphony,ribbit!|Louder than ever!",
		"The concert... is over... ribbit..." },
};
#define NLINES ((int)(sizeof lines / sizeof *lines))

/* Rematches after many of MegaMan's wins: grudging respect. */
static const char *const respect[] = {
	"You again... Stronger every time.|This time,I won't hold back!",
	"How many times must we fight!?|Until one of us falls!",
	"I've studied all your moves.|Show me what you've learned!",
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

/* How MegaMan and Lan take guardian `navi`, `name`, at the first meeting:
 * a friend's copy, a foe's, or a Navi of the older net (BN5's, copied with
 * it) */
static const char *first_sight(int navi, const char *name) {
	static char s[128];
	if (guardian_older(navi)) snprintf(s, sizeof s, "@L No way! That's %s!|@M The Nest copied the older Net's Navis too!|", name);
	else if (friendly(navi)) snprintf(s, sizeof s, "@M %s!? ...No,wait.|@M You're one of the Nest's copies!|", name);
	else snprintf(s, sizeof s, "@L That's %s!|@M Or a copy the Nest made of him...|", name);
	return s;
}

/* What the guardian says on meeting MegaMan: a first meeting's line, his
 * revenge after a win, a stronger version's, respect after three losses,
 * else a rematch's. */
static const char *greeting(int navi, int version, const Rival *r) {
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
	return s ? s : "You want through? Beat me first!";
}

/* MegaMan answers now and then, never over a first meeting (after a loss
 * the record in it: "I'm ready this time!" before every rematch read as a
 * loop to a playtester two down against CircusMan). */
static const char *answer(const Rival *r, const char *name) {
	static char said[160];
	static const char *const times[] = { "", "", "twice", "three times", "four times" };
	static const char *const next[] = { "", "", "a third", "a fourth", "a fifth" };
	if (!r->met) return "";
	if (r->last == RIVAL_NAVI_WON && r->navi_won >= 2 && r->navi_won <= 4)
		snprintf(said, sizeof said, "|@M %s beat us %s... Not %s time!", name, times[r->navi_won], next[r->navi_won]);
	else if (r->last == RIVAL_NAVI_WON && r->navi_won > 4) snprintf(said, sizeof said, "|@M %d losses to %s... Not this time!", r->navi_won, name);
	else if (r->last == RIVAL_NAVI_WON) return "|@M I'm ready this time!";
	else if (r->met % 3 != 1) snprintf(said, sizeof said, "|@M %s", replies[r->met % 3]);
	else return "";
	return said;
}

/* From battle data, when to strike: the last of MegaMan's words in the
 * guardian's tip (NULL for none). */
static const char *strike_when(int navi) {
	const char *tip = guardian_known(navi) ? guardian_tip(navi) : NULL, *when = NULL;
	for (const char *p = tip; p && (p = strstr(p, "|@M ")) != NULL; p += 4) when = p + 4;
	return when;
}

const char *guardian_intro(int navi, int version, int biome) {
	static char buf[800];
	const Rival *r = rival(navi);
	if (super_boss(navi)) return super_intro(navi, version, r);
	const char *name = guardian(navi)->name;
	int k = 0;
	#define ADD(...) (k += snprintf(buf + k, k < (int)sizeof buf ? sizeof buf - (size_t)k : 0, __VA_ARGS__))
	/* the first meeting: who MegaMan and Lan see */
	if (!r->megaman_won && !r->navi_won && !r->met) ADD("%s", first_sight(navi, name));
	/* and at every meeting after, who speaks without a face (a playtester
	 * who had met SpoutMan three times asked who was talking) */
	else if (guardian_face(navi) == FACE_NONE) ADD("@L It's %s's copy again!|", name);
	/* the Nest's own guardian knows what it is */
	if (biome == BIOME_NEST) ADD("The Nest built me from all your battles!|");
	ADD("%s", greeting(navi, version, r));
	ADD("%s", answer(r, name));
	/* and when to strike, just before the fight: the briefing on his layer
	 * came ten minutes before a playtester's fight, who asked for it again
	 * at the arena */
	const char *when = strike_when(navi);
	if (when && *when) ADD("|@M Lan,remember... %c%s", *when >= 'A' && *when <= 'Z' ? *when - 'A' + 'a' : *when, when + 1);
	ADD("|@L Battle routine,set!|@M Execute!!");
	#undef ADD
	return buf;
}

const char *guardian_defeat(int navi) {
	return navi > 0 && navi < NLINES && lines[navi].defeat ? lines[navi].defeat : "Ugh... You win...";
}

/* A first battle's Guardian Data: its battle data comes with it, and the
 * next briefing reads it; after `power`, what else it gave (or NULL) */
const char *guardian_data_words(const char *power) {
	static char with_data[640];
	snprintf(with_data, sizeof with_data, "%s%s@M And his battle data! Next time,we'll know his moves!", power ? power : "", power ? "|" : "");
	return with_data;
}

/* MegaMan on an older net guardian's Soul (docs/META.md, Souls in BN5
 * territory), won from `navi`, of chip kind `kind` (BN5_CHIP_KIND, 0-11):
 * one held already (`held`); else what it does, the whole of it the first
 * time, `chip` one of BN6's chips of its kind ("" none), `dark` the
 * DarkChip that unites with it (NULL none) */
const char *guardian_soul_words(int navi, int kind, bool held, const char *chip, const char *dark) {
	static char s[720];
	/* (the kinds by BN5_CHIP_KIND, as MegaMan says them) */
	static const char *const kinds[12] = { "Fire", "Aqua", "Elec", "Wood", "Recovery", "Plus", "Sword", "Invisible", "Cursor", "obstacle",
		"Wind", "Break" };
	const char *name = guardian(navi)->name;
	if (held) {
		snprintf(s, sizeof s, "@M %s's Soul data... We already have his Soul!", name);
		return s;
	}
	int k = snprintf(s, sizeof s, "MegaMan got:\n%s's\nSoul!!", name);
	/* (where it wakes, and the run's chips that unite with it: the run's
	 * own; how a Soul Unison goes is BN5's, its players know it) */
	if (profile.soul_taught)
		snprintf(s + k, sizeof s - (size_t)k, "|@M His Soul,Lan! It wakes in the older Net's battles.");
	else {
		k += snprintf(s + k, sizeof s - (size_t)k, "|@M %s's Soul is in me,Lan!|@M It wakes in the older Net's battles.|@M Our %s chips unite us there%s%s!", name,
			kinds[kind], chip[0] ? ",like " : "", chip);
		if (dark && k < (int)sizeof s) snprintf(s + k, sizeof s - (size_t)k, "|@M The DarkChip %s unites us too...", dark);
	}
	return s;
}
