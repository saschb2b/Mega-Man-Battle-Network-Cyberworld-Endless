/* Chaud's lines (docs/RIVAL.md, docs/VOICE.md): his call on a duel layer
 * and his verdict after it. */
#include "duel_words.h"

#include <stdio.h>
#include <string.h>

#include "director_state.h"
#include "pacing.h"
#include "save.h"

/* Chaud's first words on a duel's end, with both times as the results
 * screen shows them: the netbattle, a win, a hit taken, a time too slow. */
int verdict_result(char *out, size_t n, bool won, bool beat, int rung, int mine, int his) {
	char a[16], b[16];
	snprintf(a, sizeof a, "%d:%02d.%02d", mine / 3600, mine / 60 % 60, (mine % 60) * 100 / 60);
	snprintf(b, sizeof b, "%d:%02d.%02d", his / 3600, his / 60 % 60, (his % 60) * 100 / 60);
	if (!won && rung == 2) return snprintf(out, n, "@C Hmph. You ran,Lan?|@C Better than deleted. But it's a loss.|@C ProtoMan will be back.|");
	if (!won) return snprintf(out, n, "@C Hmph. You ran from the duel?|@C That's a loss,Lan.|");
	if (rung == 2) return snprintf(out, n, "@C ...Jack out,ProtoMan.|@C You beat him,Lan.|");
	if (beat && rung == 1) return snprintf(out, n, "@C %s,and not a scratch.|@C ...Not bad,Lan.|@C ProtoMan,we train harder.|", a);
	if (beat) return snprintf(out, n, "@C %s. ProtoMan's was %s.|@C ...Not bad,Lan.|@C We'll be faster next time.|", a, b);
	if (mine < his) return snprintf(out, n, "@C %s,but MegaMan took a hit.|@C A clean bust or nothing,Lan.|", a);
	return snprintf(out, n, "@C %s. ProtoMan's was %s.|@C Too slow,Lan.|", a, b);
}

/* Chaud's call on a duel layer (docs/RIVAL.md): the record, ProtoMan's time
 * and the term, what a win opens, and Lan's answer. */
const char *duel_call_words(void) {
	int f = layer_objs_duel_frames, sec = f / 60;
	static char call[400];
	/* (the record said: Chaud remembers every duel) */
	char record[64];
	snprintf(record, sizeof record, "@C Lan,it's Chaud. It's %d-%d between us.|", profile.duel_won, profile.duel_lost);
	/* (where, as the net goes, and what for: "the third act" was the
	 * game's word, and a playtester asked what the netbattle would pay;
	 * the rung comes after two wins, whose clearance he holds: "every
	 * official gate" read as if he had none) */
	static const char *const full = "Beat him,and my full clearance is yours.|@C The official vaults open with it.";
	if (layer_objs_duel_later)
		snprintf(call, sizeof call, "@C Lan,it's Chaud. No more races.|@C ProtoMan wants a netbattle with MegaMan himself.|"
			"@C He'll be waiting past the next %s.|@C %s", pacing_act(run.depth) == 0 ? "two guardians" : "guardian",
			rival_clearance() < 2 ? full : "Get MegaMan ready.");
	else if (layer_objs_duel_rung == 2)
		snprintf(call, sizeof call, "@C Lan,it's Chaud. ProtoMan's on this layer.|@C This time it's no race. He'll face MegaMan himself.|"
			"@C %s%s", rival_clearance() < 2 ? full : "He hasn't forgotten the last time.",
			rival_clearance() < 2 && layer_objs_official_level >= 2 ? "|@C There's one beside him on this layer. Three Mega chips." : "");
	else {
		/* (what a win opens for one already cleared: the gate beside
		 * him, whose prize the duel is) */
		const char *stake = !layer_objs_official_level ? ""
			: rival_clearance() < layer_objs_official_level ? "@C The official vault beside him needs my full clearance.|@C Three wins. The last against ProtoMan himself.|"
			: layer_objs_official_level >= 2 ? "@C Beat it,and the official vault beside him opens.|@C Three Mega chips.|"
			: "@C Beat it,and the official gate beside him opens.|@C Inside,order one of three chips you've held.|";
		snprintf(call, sizeof call, "%s@C ProtoMan busted this layer's viruses in %d:%02d.%02d.|%s@C Think MegaMan can beat that%s?",
			profile.duel_won + profile.duel_lost ? record :
			"@C Lan. It's Chaud.|@C I hear you're diving the Endless Net.|@C The Nest copies Navis. ProtoMan's the real thing.|",
			sec / 60, sec % 60, (f % 60) * 100 / 60,
			/* (what a win earns, before the first: a playtester risked his
			 * run for pride alone) */
			/* (and what the gate holds: a playtester, five duels lost, took
			 * the gates for scenery, their prize never named) */
			profile.duel_won ? stake : layer_objs_official_level ? "@C Beat it,and you'll have my first clearance.|@C The official Chip Orders open with it.|"
				"@C One's on this layer,with three chips you've held.|"
			: "@C Beat it,and you'll have my first clearance.|@C The official Chip Orders open with it.|",
			layer_objs_duel_rung == 1 ? ",without a hit" : "");
	}
	/* (Lan answers: a call no one answered read as a message left, the
	 * netbattle's too) */
	size_t n = strlen(call);
	snprintf(call + n, sizeof call - n, "|@L %s", layer_objs_duel_later ? "We'll be ready,Chaud!"
		: profile.duel_won + profile.duel_lost ? "You're on,Chaud!" : "Chaud!? ...You're on!");
	return call;
}
