/* MegaMan and Lan on the older net's battles (docs/MULTIROM.md,
 * docs/VOICE.md): its battles on arriving, the chips that sit out, its
 * codes read its own way. */
#include "guest_words.h"

#include <stdio.h>
#include <string.h>

#include "cinema.h"
#include "data.h"
#include "director_folder.h"
#include "director_guest.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "guest.h"
#include "save.h"
#include "talk.h"

/* The folder's chips the older net never had (guest.c), named as MegaMan
 * says them: three or fewer by name (the Storm folder's "ElcPuls1,
 * DolThdr1 and Atk+10"), more as two and how many others ("ElcPuls1,
 * DolThdr1 and 2 more"), "" for none; how many chips sit out. A
 * playtester was told how many sat out and asked which (session 66). */
int out_names(char *s, size_t n) {
	uint16_t folder[BN6_FOLDER_ENTRIES], out[3];
	folder_now(folder);
	int k = guest_sitting_out(folder, out, 3);
	ChipInfo ci[3];
	memset(ci, 0, sizeof ci);
	for (int i = 0; i < k && i < 3; ++i) chip_info(out[i], &ci[i]);
	if (k > 3) snprintf(s, n, "%s,%s and %d more", ci[0].name, ci[1].name, k - 2);
	else if (k == 3) snprintf(s, n, "%s,%s and %s", ci[0].name, ci[1].name, ci[2].name);
	else if (k == 2) snprintf(s, n, "%s and %s", ci[0].name, ci[1].name);
	else snprintf(s, n, "%s", ci[0].name);
	return k;
}

/* Arriving where the layer's battles are an older net's, the first time a
 * profile does: that its battles run without the run's Crosses, said
 * before one (owner's call, 4 October 2026: a playtester's HeatCross
 * vanished in BN5's battle without a word, session 65; a Soul of BN5's
 * may stand in later, issue #69), and which of the folder's chips it never
 * had (they sit out); the first time a profile does holding BeastOut,
 * that the Cybeast can't come in either. After the arrival's own words. */
void older_net_words(void) {
	D.beat_cross = encounter_guest && !profile.cross_old_told;
	/* (and the Cybeast's, where the run holds BeastOut: a run brings it
	 * from its first act since issue #99, or has it from the first Net's
	 * Cybeast on (issue #109), where the older net dresses areas, and its
	 * emblem would be gone without a word, as a Cross was) */
	D.beat_beast = encounter_guest && flag_get(BN6_FLAG_BEAST_OUT) && !(profile.beast & BEAST_OLD_TOLD);
	char out[96];
	int n = out_names(out, sizeof out);
	size_t k = strlen(D.beat);
	/* (a profile told already: the run's own chips that sit out, named on
	 * its first such layer, as its folder is its own; a playtester's new
	 * run's CrakShot and Atk+10 went unnamed, session 68) */
	D.beat_out = encounter_guest && !D.beat_cross && n > 0 && !flag_get(RUN_OUT_NAMED_FLAG);
	if (D.beat_beast && !D.beat_cross) {
		k += (size_t)snprintf(D.beat + k, sizeof D.beat - k, "%s@M The older Net's battles,Lan...|@M The Cybeast can't come in there.|"
			"@L No CybeastButton back then,huh?", k ? "|" : "");
		if (k >= sizeof D.beat) return;
		if (D.beat_out)
			snprintf(D.beat + k, sizeof D.beat - k, "|@M Right. And %s didn't exist either.|@L Then %s out. Got it!", out, n == 1 ? "it sits" : "they sit");
		else snprintf(D.beat + k, sizeof D.beat - k, "|@M Right. We'll manage without it!");
		return;
	}
	if (D.beat_out) {
		snprintf(D.beat + k, sizeof D.beat - k, "%s@M The older Net's battles again,Lan.|@M %s didn't exist back then.|@L Then %s out. Got it!",
			k ? "|" : "", out, n == 1 ? "it sits" : "they sit");
		return;
	}
	if (!D.beat_cross) return;
	/* (a guardian of ours keeps his fight ours; one of the older net's own
	 * Navis fights the old way: docs/BOSSES.md, BN5's Navis) */
	k += (size_t)snprintf(D.beat + k, sizeof D.beat - k, "%s@M Wait... This Net's a copy of an older one!|@M Its battles run the old way.|"
		"@L The old way? There were no Crosses then!|@M Right. So our Cross can't come in...%s|@M Except against a guardian from our Net!", k ? "|" : "",
		D.beat_beast ? "|@M Or the Cybeast!" : "");
	if (n > 0 && k < sizeof D.beat)
		k += (size_t)snprintf(D.beat + k, sizeof D.beat - k, "|@M %s didn't exist then either.|@M So %s'll sit out.", out, n == 1 ? "it" : "they");
	if (k < sizeof D.beat)
		snprintf(D.beat + k, sizeof D.beat - k, "|@L Then it's you and %s,MegaMan!", n > 0 ? "the other chips" : "our chips");
}

/* ... and the words, after the older net's own if both are due */
static void recode_words(char *out, size_t size) {
	ChipInfo ci;
	chip_info(D.recode_chip, &ci);
	if (D.recode_due == 1)
		snprintf(out, size, "@L Huh? In there our %s %c was %s %c!|@M The old Net reads chip codes its own way,Lan.|"
			"@M A code it never knew becomes one it knows.|@M Out here,they're our codes again!",
			ci.name, code_letter(D.recode_from), ci.name, code_letter(D.recode_to));
	else
		snprintf(out, size, "@L Wait! It showed %s %c...|@L But our Pack got %s %c!|@M Our Net read the code its own way coming back,Lan.|"
			"@M Chips from the old Net come home in our codes!", ci.name, code_letter(D.recode_to), ci.name, code_letter(D.recode_from));
}

/* MegaMan's words after a profile's first battle in an older net's own
 * engine (docs/MULTIROM.md, Guest battles; issue #65): what it was, and
 * why chips sat out. A BN5 battle opened with no word of why the screen,
 * the chips' art and their rules changed. */
void guest_words(void) {
	if ((!D.guest_due && !D.recode_due) || talk_busy() || emu_read8(BN6_CHATBOX) || cinema_busy() || !on_map()) return;
	char words[720];
	int k = 0;
	if (D.guest_due) {
		int out = D.guest_due - 1;
		k = snprintf(words, sizeof words, "@M Lan,that battle ran on an older Net's system!|@L The Nest copied that Net too!?|"
			"@M Battles and all. Its viruses fight the old way.|@M And our chips work the way it knew them.");
		/* (unless the arrival's words named them on this run: said twice,
		 * as new, session 69) */
		if (out > 0 && !flag_get(RUN_OUT_NAMED_FLAG))
			k += snprintf(words + k, sizeof words - (size_t)k, "|@M %s didn't exist back then...|@M So %s had to sit out.", D.guest_out,
				out == 1 ? "it" : "they");
	}
	if (D.recode_due) {
		if (k) k += snprintf(words + k, sizeof words - (size_t)k, "|");
		recode_words(words + k, sizeof words - (size_t)k);
	}
	if (talk_start(words, FACE_MEGAMAN)) {
		if (D.guest_due) profile.guest_taught = 1;
		if (D.recode_due) profile.recode_taught |= (uint8_t)D.recode_due;
		D.guest_due = D.recode_due = 0;
		profile_save();
	}
}
