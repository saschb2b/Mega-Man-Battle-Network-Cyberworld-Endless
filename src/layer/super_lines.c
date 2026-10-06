/* super_lines.h. Bass is BN6's legend, real where everything else in the
 * Endless Net is a copy: few words, cold, "Hmph.", MegaMan by his name,
 * the strong his prey; beaten, a vow. The Cybeast never speaks: it roars,
 * with its own face, while MegaMan, Lan and Dad speak (docs/VOICE.md, the
 * cast). Unmarked boxes are the boss's own. */
#include "super_lines.h"

#include <stdio.h>
#include <string.h>

#include "run.h"
#include "super_boss.h"

/* (a meeting without a result, the run left or lost elsewhere, is no
 * rematch, as the guardians' greeting has it) */
static bool fought(const Rival *r) { return r->megaman_won || r->navi_won; }

static const char *bass_meeting(int version, const Rival *r) {
	if (!fought(r))
		return r->met ? "...Hmph. You again.|Copy or not... I delete the strong." :
			"@L W-Whoa! Someone was in that stone!?|@M That's... Bass!!|...Hmph.|Another copy,come to be deleted?|"
			"@M I'm no copy! I'm MegaMan!|Copy or not... I delete the strong.";
	if (r->last == RIVAL_NAVI_WON) return "Hmph. Back again?|Then be deleted again.";
	/* (his form the first time he comes as Bass BX: the beast's data) */
	if (version == SUPER_BASS_BX && r->megaman_won == 2)
		return "Feel it,MegaMan? The beast's power.|Its data lay scattered. I took it.|@M Lan... Bass absorbed the Cybeast!|@L N-No way...!";
	if (r->megaman_won >= 3) return "You keep getting stronger,MegaMan.|Good. Then deleting you means something.";
	return "MegaMan... I've been waiting.|Every copy down here made me stronger.|@M His signal's darker than before,Lan!";
}

static const char *beast_meeting(const Rival *r) {
	if (!fought(r))
		return "@B GRRRAAAAH!!|@M L-Lan... It's a Cybeast!|@L Gregar!? But the beast's in you!|@M The Net copied it... from the beast in me!|"
			"@D Lan,MegaMan! It's Dad!|@D That beast's what's been copying everything!|@D Delete it,you two!";
	if (r->last == RIVAL_NAVI_WON) return "@B GRRRRR...|@M It beat us last time,Lan.|@L Not this time! We're ready!";
	if (r->megaman_won >= 3) return "@B GRRRAAAH!!|@M The Net keeps copying it,Lan...|@L And we keep winning!";
	return "@B GRRRRAAAAH!!|@M It's back,Lan... and stronger.|@M It copied our last battle!|@L Then we'll beat it again!";
}

/* From battle data, how to fight him: the last of MegaMan's words in the
 * tip, said again just before the fight (as a guardian's) */
static const char *strike_when(int navi) {
	const char *tip = super_tip(navi), *when = NULL;
	for (const char *p = tip; p && (p = strstr(p, "|@M ")) != NULL; p += 4) when = p + 4;
	return when;
}

const char *super_intro(int navi, int version, const Rival *r) {
	static char buf[640];
	int k = snprintf(buf, sizeof buf, "%s", navi == SUPER_BASS ? bass_meeting(version, r) : beast_meeting(r));
	const char *when = fought(r) ? strike_when(navi) : NULL;
	if (when && k < (int)sizeof buf)
		k += snprintf(buf + k, sizeof buf - (size_t)k, "|@M Lan,remember... %c%s", *when >= 'A' && *when <= 'Z' ? *when - 'A' + 'a' : *when, when + 1);
	if (k < (int)sizeof buf) snprintf(buf + k, sizeof buf - (size_t)k, "|@L Battle routine,set!|@M Execute!!");
	return buf;
}

const char *super_defeat(int navi, int version) {
	if (navi == SUPER_CYBEAST) return "@B Grrr...rrrr......|@M It's... going quiet,Lan.";
	if (version == SUPER_BASS_BX) return "Even the beast's power...!?|...Remember this,MegaMan.";
	if (version == SUPER_BASS_SP) return "...Again? Grr...|This isn't over,MegaMan.";
	return "Hmph... I underestimated you.|Next time,there won't be a next time.";
}

/* (BN6's own moves for them, watched in god mode, Bass's 1800 and
 * Gregar's 2500: his cape takes no damage, and comes off as he steps up
 * to his front column to blast the panels he lights; his wheels roll in
 * along two rows and turn into ours; his arm cannon fires down his row
 * with no panel lit. Every attack of the Cybeast's lights the panels it
 * lands on: its rocks, its lightning's path, its fireball, its claws'
 * swipe, its charge down a row) */
const char *super_tip(int navi) {
	if (navi == SUPER_BASS)
		return "His cape stops our shots cold!|@M Up front,he blasts the lit panels.|@M Dark wheels roll in,then turn into our row!|"
			"@M His arm cannon fires down his row. No yellow!|@M His cape comes off as he attacks. Strike then!";
	if (navi == SUPER_CYBEAST)
		return "It lights our panels before every attack!|@M Rocks,lightning and fire come down on them.|@M Its claws and its charge,too!|"
			"@M Keep off the lit panels,and keep shooting!";
	return NULL;
}

const char *super_rumor(int navi) {
	return navi == SUPER_CYBEAST ? "Did you hear?|The floor down here keeps shaking...|Something huge is waking up!" : NULL;
}

const char *super_reward_head(int navi) {
	return navi == SUPER_BASS ? "MegaMan picked up the data Bass dropped!" : "MegaMan downloaded the Cybeast's data!";
}

void super_card(int navi, int version, int depth, const char **top, const char **sub) {
	static char bottom[40];
	if (navi == SUPER_CYBEAST) {
		int net = (depth - 1) / CYCLE_LAYERS + 1;
		if (net > 1) snprintf(bottom, sizeof bottom, "The bottom of Net V%d", net);
		else snprintf(bottom, sizeof bottom, "The bottom of the Net");
		*top = bottom;
		*sub = "The Cybeast";
		return;
	}
	*top = "Master of the Secret Area";
	*sub = version == SUPER_BASS_BX ? "The beast's power is his" : version == SUPER_BASS_SP ? "Stronger with every copy" : "Not a copy";
}

const char *super_reward_words(int navi, int version, bool first) {
	static char buf[320];
	const char *what = navi == SUPER_CYBEAST ? "@M The Cybeast's data,Lan!|@M There's a GigaChip in it!"
		: version == SUPER_BASS_BX ? "@M Some of the beast's power...|@M He left it behind,Lan!"
		: version == SUPER_BASS_SP ? "@M He dropped data again,Lan!|@M Another GigaChip!"
		: "@M Bass dropped this as he went...|@M Lan,it's GigaChip data!";
	snprintf(buf, sizeof buf, "%s%s", what, !first ? "" : navi == SUPER_CYBEAST ? "|@M And its battle data!|@M Next time,we'll know how it fights!"
		: "|@M And his battle data,Lan!|@M Next time,we'll know how he fights!");
	return buf;
}
