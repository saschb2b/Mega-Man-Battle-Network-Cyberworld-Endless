/* The run's story beats (docs/VOICE.md): what MegaMan, Lan and Dad say on
 * arriving somewhere new, and the cards an act begins and ends with. */
#include "story_words.h"

#include <stdio.h>
#include <string.h>

#include "boss.h"
#include "cinema.h"
#include "director_folder.h"
#include "director_layer.h"
#include "director_state.h"
#include "gfx.h"
#include "guardians.h"
#include "liberation.h"
#include "liberation_words.h"
#include "meta.h"
#include "nets_words.h"
#include "powers.h"
#include "rivals.h"
#include "save.h"
#include "visitors.h"

/* The net's version: the Nest rebuilds it, one stronger, each time its
 * guardian falls (1 for the first cycle). */
static int net_version(void) { return (run.depth - 1) / CYCLE_LAYERS + 1; }

/* The arrival's words begun: the Nest shakes, the guardian they name is
 * named, and the older net's word is said for the profile. */
void beat_said(void) {
	if (run.biome == BIOME_NEST) cinema_shake(30, 3);
	D.guardian_named = D.beat_guardian;
	D.beat[0] = 0;
	if (D.beat_ours) { profile.navis_ours_told = profile.native_taught = 1; profile_save(); }
	if (D.beat_native) { profile.native_taught = 1; profile_save(); }
	if (D.beat_deep) { profile.deep_taught = 1; profile_save(); }
}

/* Arriving where super boss `navi` waits (docs/BOSSES.md, Super bosses),
 * in his `version`: before a first battle MegaMan senses him, never names
 * him (docs/META.md, what MegaMan knows); the Cybeast a growl the beast in
 * him answers, Bass a dark signal he has felt somewhere before */
static const char *super_arrival(int navi, int version, bool known) {
	if (navi == SUPER_CYBEAST)
		return known ? "@M The bottom again,Lan...|@B GRRRRR...|@M The Cybeast's back. I can feel it.|@L Then we beat it again!"
			: "@M Lan... This is the bottom of the Net.|@B Grrrr...|@M That growl... The beast in me is answering!|"
			  "@L Whatever's down here,we're ready!";
	if (!known)
		return "@M Whoa... So this is the Secret Area.|@M There's a signal in here... Pure darkness.|@M I've felt it before... But where?|"
			"@L *gulp* Stay sharp,MegaMan!";
	if (version == SUPER_BASS_BX) return "@M Bass is here,Lan...|@M But his signal... It's like the Cybeast's!|@L What!? How!?";
	return "@M He's here,Lan. Bass.|@M I can feel him waiting.|@L Then let's show him how strong we got!";
}

/* The super boss waiting on this layer where one does (docs/BOSSES.md,
 * Super bosses): the endless Nest's Cybeast, the Secret Area's Bass; 0 for
 * none */
static int super_here(void) {
	int navi = run.side_kind == LAYER_SECRET && layer.boss_layer ? run.boss_order[BIOME_SECRET]
		: run.side_kind == LAYER_NORMAL && run.biome == BIOME_NEST && !run_short_nest(run.depth) ? run_guardian(BIOME_NEST) : 0;
	return super_boss(navi) ? navi : 0;
}

/* What MegaMan and Lan (and Dad) say on arriving somewhere new: the first
 * layer, a new cycle, the Undernet, the Graveyard, the Nest, the side
 * layers; where a super boss waits, what MegaMan senses of him. Empty for
 * the rest. */
void arrival_words(void) {
	const char *area = guardian_area_in_text(run.biome, LAYER_NORMAL);
	bool first_of_act = run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0;
	int super = super_here();
	D.beat[0] = 0;
	D.beat_guardian = D.guardian_named = false;
	if (super)
		snprintf(D.beat, sizeof D.beat, "%s", super_arrival(super, super_form(super, run.depth), guardian_known(super)));
	else if (run.side_kind == LAYER_UNDERNET)
		snprintf(D.beat, sizeof D.beat, "@M A copy of the Undernet...|@M The viruses in here are no joke,Lan.|@L Stay sharp! The exit pad leads back,right?|"
			"@M Right. Back to the main path!");
	else if (run.side_kind == LAYER_SECRET)
		snprintf(D.beat, sizeof D.beat, "@M Whoa... So this is the Secret Area.|@M Something strong is in here. I can feel it...|@L *gulp* Let's go,MegaMan!");
	/* (a trip back from home, docs/HOME.md: an area already won) */
	else if (run.side_kind == LAYER_BACK)
		snprintf(D.beat, sizeof D.beat, "@M We're back in %s,Lan!|@M It feels easier now...|@L 'Cause we got stronger! Let's stock up!", area);
	else if (run.depth == 1 && profile.runs >= 2)
		snprintf(D.beat, sizeof D.beat, "@M %s this time,Lan!|@L Alright! Let's find the exit pad!", area);
	else if (run.depth == 1)
		snprintf(D.beat, sizeof D.beat, "@M Whoa... It looks just like %s!|@M But it's all copied data,Lan.|@L Dad was right... Let's find the exit pad!", area);
	else if (first_of_act && (run.depth - 1) % CYCLE_LAYERS == 0)
		snprintf(D.beat, sizeof D.beat, "@D Lan,it's Dad! The Endless Net just rebuilt itself!|@D Same areas,but stronger data. It's Net V%d now.|"
			"@L Stronger,huh? Bring it on!", net_version());
	else if (run.biome == BIOME_NEST && run_short_nest(run.depth) && !run_short_last(run.depth))
		/* (threat 10: said before the first of the two, docs/META.md) */
		snprintf(D.beat, sizeof D.beat, "@M Lan... This is it. The Nest.|@M Something down here is copying everything...|@B Grrrr...|"
			"@M And it's not alone. A second guardian waits below!|@L Then we beat them both,MegaMan!");
	else if (run.biome == BIOME_NEST && run_short_nest(run.depth) && run.depth > SHORT_LAYERS)
		snprintf(D.beat, sizeof D.beat, "@M Below the Nest... The second guardian's here,Lan.|@B Grrrr...|@L The last one,MegaMan! Let's finish this!");
	else if (run.biome == BIOME_NEST)
		snprintf(D.beat, sizeof D.beat, "@M Lan... This is it. The Nest.|@M Something down here is copying everything...|@B Grrrr...|"
			"@L Hang on,MegaMan! Whatever it is,we'll find it!");
	else if (first_of_act && run.biome == BIOME_UNDERNET)
		snprintf(D.beat, sizeof D.beat, "@M Even the Undernet got copied...|@L Then stay sharp,MegaMan!");
	else if (first_of_act && run.biome == BIOME_GRAVEYARD)
		snprintf(D.beat, sizeof D.beat, "@M So much deleted data...|@L Creepy... Is the bottom close?|@M I think so,Lan.");
	else if (first_of_act && run.depth > 1) {
		/* a new act: where they are now, and whose copy waits at its end,
		 * named where they have battled him: else a signal MegaMan does
		 * not know (docs/META.md, what MegaMan knows) */
		int navi = run_guardian(run.biome);
		if (guardian_known(navi))
			snprintf(D.beat, sizeof D.beat, "@M We're through to %s,Lan!|@L %s's copy guards this one. Let's go!", area, guardian(navi)->name);
		else
			snprintf(D.beat, sizeof D.beat, "@M We're through to %s,Lan!|@M A strong Navi's signal waits deeper in.|@M ...I don't recognize it.|"
				"@L Then let's go find out who!", area);
		D.beat_guardian = true;
	}
	nets_arrival_words();
}

/* What the act's card says of the nets, where they meet (docs/MULTIROM.md,
 * One net: the telegraph): an older net's area, the deep's both nets, and
 * from the second act a BN6 area its visitors come to */
static const char *nets_card(int biome) {
	if (D.deep) return " - Both Nets";
	if (D.territory_native) return " - Older Net";
	return visitors_here(run.depth, biome) ? " - Nets meet" : "";
}

/* The act card's line under the area's name, its motto, and its colour;
 * an act whose way holds a dark layer says so in the motto's place, in
 * Nebula's purple (docs/LIBERATION.md) */
static const char *card_motto(int biome, SDL_Color *accent) {
	bool dark = run.side_kind == LAYER_NORMAL && lib_way_dark((run.depth - 1) % CYCLE_LAYERS / 3, biome);
	*accent = dark ? rgba(184, 120, 248, 255) : rgba(120, 200, 248, 255);
	return dark ? lib_card_line : guardian_area_motto(biome);
}

void area_card(void) {
	char act[48];
	int biome = run.biome, act_no = ((run.depth - 1) % CYCLE_LAYERS) / 3 + 1;
	if (run.side_kind == LAYER_UNDERNET) snprintf(act, sizeof act, "Through a dark warp");
	else if (run.side_kind == LAYER_SECRET) snprintf(act, sizeof act, "Beyond the sealed gate");
	else if (run.side_kind == LAYER_BACK) snprintf(act, sizeof act, "A trip back");
	else if (biome == BIOME_NEST && run_short_nest(run.depth) && run.depth > SHORT_LAYERS) snprintf(act, sizeof act, "Below the Nest");
	else if (biome == BIOME_NEST && net_version() > 1) snprintf(act, sizeof act, "The bottom of Net V%d", net_version());
	else if (biome == BIOME_NEST) snprintf(act, sizeof act, "The bottom of the Net");
	else if (net_version() > 1) snprintf(act, sizeof act, "Net V%d - Act %d", net_version(), act_no);
	else snprintf(act, sizeof act, "Act %d", act_no);
	/* the guardian ahead, from the start, so the folder can be set for it
	 * (as Slay the Spire shows each act's boss), where MegaMan has battled
	 * him, with a "?" where only a Navi on the net has named him (a
	 * playtester's card said "???" after a CONTINUE past the rumor,
	 * session 63): else "???", as nothing yet says who (docs/META.md, what
	 * MegaMan knows; his element alone gave SpoutMan away) */
	char ahead[48] = "";
	if (run.side_kind == LAYER_NORMAL || (run.side_kind == LAYER_SECRET && layer.boss_layer)) {
		int navi = run.side_kind == LAYER_NORMAL ? run_guardian(biome) : run.boss_order[biome];
		bool heard = run.side_kind == LAYER_NORMAL && guardian_heard();
		snprintf(ahead, sizeof ahead, "Guardian: %s%s", guardian_known(navi) || heard ? guardian(navi)->name : "???",
			!guardian_known(navi) && heard ? "?" : "");
	}
	/* (a CONTINUE by a guardian already deleted: it said he waited) */
	if (D.objs.guardian.navi && boss_beaten())
		snprintf(ahead, sizeof ahead, "%s deleted", guardian(D.objs.guardian.navi)->name);
	/* (a trip back: what it has cost, docs/HOME.md) */
	if (run.side_kind == LAYER_BACK) snprintf(ahead, sizeof ahead, "The Net's clock: %d", run.clock);
	size_t k = strlen(act);
	snprintf(act + k, sizeof act - k, "%s", nets_card(biome));
	SDL_Color accent;
	const char *motto = card_motto(biome, &accent);
	cinema_card(act, guardian_area_name(biome), motto, ahead[0] ? ahead : NULL, accent, 200);
}

/* Leaving an area past its beaten guardian. */
void clear_card(void) {
	char who[48], stats[48];
	int secs = D.act_frames / 60;
	snprintf(who, sizeof who, "%s deleted", D.act_guardian ? D.act_guardian : "Guardian");
	snprintf(stats, sizeof stats, "Viruses %d   Time %d:%02d", run.viruses_deleted - D.act_viruses, secs / 60, secs % 60);
	/* (an act continued from a checkpoint has no whole count) */
	cinema_card(guardian_area_name(run.biome), "AREA CLEAR", who, D.act_resumed ? NULL : stats, rgba(248, 208, 88, 255), 220);
}

/* The short net won and its exit open: its end said on the map, before
 * the pad (the win went from the pad straight to the title's summary),
 * the arrival's growl answered, Dad's voice, and the endless net's hook (a
 * playtester's first win ended on two lines). */
const char *final_words(void) {
	return "@M That was the Nest's last guardian,Lan...|@M The whole Net's gone quiet.|"
		"@B Grrrr......|"
		"@M ...Almost. Something deeper down is still awake.|@M The Nest was only its den...|"
		"@D Lan,MegaMan,it's Dad! I watched it all. You did it!|"
		"@D Whatever's growling down there,we'll be ready.|@D Now jack out and come home,you two.|"
		"@L We did it!! The exit's open. Let's jack out!";
}

/* Dad's mail with guardian `navi`'s battle data, said once the arrival's
 * card and words are done, or a guardian's data taken: one box, E-Mail
 * BN6's own */
const char *mail_words(int navi) {
	static char words[160];
	snprintf(words, sizeof words, "@M Lan,mail from Dad! Our battle data on %s!", guardian(navi)->name);
	return words;
}

/* Chaud's call once the Secret Area's guardian is done: after ProtoMan's
 * copy (and at its first clear, the signal that brings Bass there); after
 * Bass, who he is (docs/BOSSES.md, Super bosses) */
const char *secret_call_words(void) {
	if (run.boss_order[BIOME_SECRET] == SUPER_BASS)
		return rival(SUPER_BASS)->megaman_won == 1
			? "@C Lan,it's Chaud.|@C That was Bass. The real one.|@C Officials have chased him for years.|"
			  "@M He's hunting the Net's strongest copies...|@L Then we'll be the ones to stop him!"
			: "@C Bass again,Lan?|@C ...Hmph. Not bad.|@C Stay on your guard.";
	if (profile.marks & MARK_SECRET)
		return "@C Lan,it's Chaud.|@C That wasn't ProtoMan. He's been in my PET all day.|@C You beat a copy. Watch yourself.|"
			"@M The Nest can even copy ProtoMan...|@L Then we'd better stay on guard!";
	return "@C Lan,it's Chaud.|@C That wasn't ProtoMan. He's been in my PET all day.|@C You beat a copy. Watch yourself.|"
		"@C And Lan... Officials picked up another signal down there.|@C Not a copy's. Something real.|@M Something... real?";
}

/* Dad's call once the Cybeast's data is taken: it is deleted, and the Net
 * rebuilds itself (docs/BOSSES.md, Super bosses); where its fall gave the
 * run BeastOut, the CybeastButton unlocked (powers_den_text, issue #109) */
const char *den_call_words(void) {
	static char words[640];
	const char *call = rival(SUPER_CYBEAST)->megaman_won == 1
		? "@D Lan,MegaMan! You did it!|@D It's deleted... But the Net!|@D It's rebuilding itself!|@M It copied our battle too,Lan.|"
		  "@M Next time,it'll be stronger.|@L Then so will we!"
		: "@D Well done,you two!|@D The Net's rebuilding again...|@L We'll be ready for it!";
	const char *beast = powers_den_text(run.depth);
	snprintf(words, sizeof words, "%s%s%s", call, beast ? "|" : "", beast ? beast : "");
	return words;
}
