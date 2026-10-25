/* MegaMan and Lan where the nets meet (docs/multirom/one-net.md: the
 * telegraph; docs/voice/rules.md): an older net's area fought our way, the first
 * visitor of the other net, the deep's both nets; and, a run continued
 * without BN5's ROM, the chips and Souls of BN5's that wait for it. */
#include "nets_words.h"

#include <stdio.h>
#include <string.h>

#include "chips.h"
#include "cinema.h"
#include "data.h"
#include "director_folder.h"
#include "director_state.h"
#include "emu.h"
#include "save.h"
#include "souls.h"
#include "talk.h"
#include "visitor_families.h"

/* Where the nets meet, the first time a profile comes there, after the
 * arrival's own words: an older net's area whose battles are ours (a
 * profile once told its battles ran the old way, on the guest core that
 * has left runs, hears that its Cross and its Navis are ours now), and the
 * deep, where both nets' viruses come */
void nets_arrival_words(void) {
	/* (an older net's area: a territory's, or the deep's dressed in one) */
	bool older = D.territory_native || (D.deep && D.layer_tiles >= NET_AREAS);
	D.beat_ours = older && (profile.cross_old_told || profile.guest_taught) && !profile.navis_ours_told;
	D.beat_native = older && !profile.native_taught && !D.beat_ours;
	D.beat_deep = D.deep && !profile.deep_taught;
	size_t k = strlen(D.beat);
	if (D.beat_ours)
		k += (size_t)snprintf(D.beat + k, sizeof D.beat - k, "%s@M Lan! Our Cross works in the older Net now!|@M Whoa... Even its Navis fight like ours!|"
			"@L Alright! Cross and all!", k ? "|" : "");
	else if (D.beat_native)
		k += (size_t)snprintf(D.beat + k, sizeof D.beat - k, "%s@M Wait... This Net's a copy of an older one!|@L The Nest copied that too!?|"
			"@M Its viruses are the old ones,Lan.|@M But we fight them our way. Cross and all!", k ? "|" : "");
	if (D.beat_deep && k < sizeof D.beat)
		snprintf(D.beat + k, sizeof D.beat - k, "%s@M Lan... The two Nets run together down here.|@L The old viruses and ours? Side by side?|"
			"@M Yeah. Anything could come at us!", k ? "|" : "");
}

void nets_battle_note(const Encounter *e) {
	if (!e->visitor || profile.visitor_taught || D.visitor_due) return;
	D.visitor_due = e->xrecord ? 2 : 1;
	D.visitor_family = e->foes[e->visitor - 1].family;
}

/* (MegaMan free on the map, nothing said or shown: a word can start) */
static bool quiet(void) { return !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && on_map(); }

/* MegaMan on the first visitor a profile met: one of the older net's in
 * a battle of ours, or one of ours in the older net's (docs/multirom/one-net.md,
 * One net: the telegraph), named by its family */
static void visitor_words(void) {
	if (!D.visitor_due || !quiet()) return;
	char name[24], words[240];
	enemy_name(visitor_id(D.visitor_due == 1 ? VISIT_BN5 : VISIT_BN6, D.visitor_family, 0), name, sizeof name);
	if (D.visitor_due == 1)
		snprintf(words, sizeof words, "@M Lan,that %s back there...|@M It came from the older Net!|@L The two Nets are mixing!?|"
			"@M Where they meet,yeah. Stay sharp!", name);
	else snprintf(words, sizeof words, "@M Huh? That %s was one of ours,Lan!|@L In the older Net!?|@M The Nets meet here. Stay sharp!", name);
	if (!talk_start(words, FACE_MEGAMAN)) return;
	D.visitor_due = 0;
	profile.visitor_taught = 1;
	profile_save();
}

/* (a run continued: the folder's chips and the run's Souls of BN5's that
 * wait for its ROM, looked for once) */
static void resumed(void) {
	if (!D.resumed) return;
	D.resumed = false;
	D.old_chips_due = folder_old_chips_out();
	D.old_souls_due = souls_held() && !chips_from_bn5();
}

/* After a CONTINUE without BN5's ROM (docs/multirom/one-net.md, One net: the
 * design, call 4): the run's folder holding chips of BN5's, they sit out of
 * battles until it is back; the run holding Souls, they sleep till then.
 * MegaMan says so, on the map, once. */
static void old_net_words(void) {
	if ((!D.old_chips_due && !D.old_souls_due) || !quiet()) return;
	char words[320];
	int k = 0;
	if (D.old_chips_due)
		k = snprintf(words, sizeof words, "@M Huh? Some of our chips came from the older Net...|@L And its cartridge isn't here!|"
			"@M Right. They'll sit out until it's back!");
	if (D.old_souls_due && k < (int)sizeof words)
		snprintf(words + k, sizeof words - (size_t)k, "%s@M I can't feel the old Souls,Lan...|@M Not till the older Net's back.", k ? "|" : "");
	if (talk_start(words, FACE_MEGAMAN)) D.old_chips_due = D.old_souls_due = false;
}

void nets_words_due(void) {
	resumed();
	visitor_words();
	old_net_words();
}
