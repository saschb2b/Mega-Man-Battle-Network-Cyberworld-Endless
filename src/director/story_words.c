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
#include "encounter.h"
#include "gfx.h"
#include "guardians.h"
#include "guest_words.h"
#include "save.h"

/* The net's version: the Nest rebuilds it, one stronger, each time its
 * guardian falls (1 for the first cycle). */
static int net_version(void) { return (run.depth - 1) / CYCLE_LAYERS + 1; }

/* The arrival's words begun: the Nest shakes, the guardian they name is
 * named, and the older net's word is said for the profile. */
void beat_said(void) {
	if (run.biome == BIOME_NEST) cinema_shake(30, 3);
	D.guardian_named = D.beat_guardian;
	D.beat[0] = 0;
	if (D.beat_cross) { profile.cross_old_told = 1; profile_save(); }
	if (D.beat_cross || D.beat_out) flag_set(RUN_OUT_NAMED_FLAG);
}

/* What MegaMan and Lan (and Dad) say on arriving somewhere new: the first
 * layer, a new cycle, the Undernet, the Graveyard, the Nest, the side
 * layers. Empty for the rest. */
void arrival_words(void) {
	const char *area = guardian_area_in_text(run.biome, LAYER_NORMAL);
	bool first_of_act = run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0;
	D.beat[0] = 0;
	D.beat_guardian = D.guardian_named = false;
	if (run.side_kind == LAYER_UNDERNET)
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
	older_net_words();
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
	/* (and where its battles are another game's, so: the owner's call for
	 * issue #65, the card and L saying it, no card of its own at the
	 * switch) */
	if (encounter_guest) {
		size_t k = strlen(act);
		snprintf(act + k, sizeof act - k, " - older Net battles");
	}
	cinema_card(act, guardian_area_name(biome), guardian_area_motto(biome), ahead[0] ? ahead : NULL, rgba(120, 200, 248, 255), 200);
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
 * card and words are done */
const char *mail_words(int navi) {
	static char words[160];
	snprintf(words, sizeof words, "@M Lan,you've got mail from Dad!|@M Our battle data on %s! It's in the PET's E-Mail.",
		guardian(navi)->name);
	return words;
}

/* Chaud's call once the Secret Area's guardian is done */
const char *secret_call_words(void) {
	return "@C Lan,it's Chaud.|@C That wasn't ProtoMan. He's been in my PET all day.|@C You beat a copy. Watch yourself.|"
		"@M The Nest can even copy ProtoMan...|@L Then we'd better stay on guard!";
}
