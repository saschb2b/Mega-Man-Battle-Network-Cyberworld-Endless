/* What MegaMan and Dad say of a form run's powers (forms.h; docs/game/meta/forms.md,
 * docs/voice/rules.md): a guardian's data taken in the shape of a power
 * MegaMan knows, a Soul he feels in a Navi's data where the run's form
 * has none, a Soul's first teaching, and SoulT+1's turn after the
 * Cybeast's fall. */
#include <stdio.h>

#include "data.h"
#include "forms.h"
#include "guardians.h"
#include "powers.h"
#include "run.h"
#include "save.h"
#include "souls.h"
#include "xchips.h"

/* (a kind as MegaMan says it, with its article: BN5's kinds, then BN4's
 * panel-cracking) */
static const char *kind_words(int kind) {
	static const char *const words[KINDS] = { "", "a Fire", "an Aqua", "an Elec", "a Wood", "a Recovery", "a Plus", "a Sword", "an Invisible",
		"a Cursor", "an obstacle", "a Wind", "a Break", "a panel-cracking" };
	return kind > KIND_NONE && kind < KINDS ? words[kind] : "";
}

static char text[640];
#define ADD(...) (k += snprintf(text + k, k < (int)sizeof text ? sizeof text - (size_t)k : 0, __VA_ARGS__))

/* (a Soul new to the run: BN6's box for it, then, the first time a
 * profile wins one, what it asks and gives, as guardian_soul_words says
 * it for BN5's own) */
static int soul_got(int k, int p) {
	int kind = forms_power_kind(run.form, p);
	ADD("%sMegaMan got:\n%s!!", k ? "|" : "", forms_power_name(run.form, p));
	if (profile.soul_taught) return k;
	ADD("|@M It's with us every battle now,Lan.");
	uint16_t ids[8];
	ChipInfo ci = { .name = "" };
	if (kind != KIND_CRACK && xchips_kind(kind - 1, ids, 8) > 0) chip_info(ids[0], &ci);
	if (ci.name[0]) ADD("|@M Pick %s chip last,like %s.", kind_words(kind), ci.name);
	else ADD("|@M Pick %s chip last.", kind_words(kind));
	ADD("|@M Then UNITE! Once a battle.");
	return k;
}

const char *forms_reward_words(int navi, bool beast) {
	int k = 0, p, how = forms_pay_how(run.form, navi, forms_held_layer(), run.layer_seed, &p);
	const char *name = guardian(navi)->name;
	bool soul_form = run.form != RUN_FORM_CROSS;
	/* (a Navi of an older game in a CROSS run: his Soul felt, never taken;
	 * it opens his game's form for a later dive, the setup's to show) */
	bool felt = !soul_form && (guardian_older(navi) || guardian_team(navi) || (guardian_bn4(navi) && navi - GUARDIAN_BN4_FIRST < SOULS_OF_BN4));
	if (felt) ADD("@M Lan... There's a Soul in his data.");
	if (how == PAY_OWN && soul_form) k = soul_got(k, p);
	/* (another game's Navi, or one whose own the run has: his data in the
	 * shape of a power MegaMan knows, the Cross's name BN6's box's) */
	if (how == PAY_KIND || how == PAY_LACKED) {
		ADD("%s@M Whoa... %s's data is changing!", k ? "|" : "", name);
		if (soul_form) {
			ADD("|@M It's taking a Soul's shape!");
			k = soul_got(k, p);
		} else ADD("|@M It's taking a Cross's shape!");
	}
	const char *call = beast ? powers_reward_words(navi, false, true) : NULL;
	if (call) ADD("%s%s", k ? "|" : "", call);
	return k ? text : NULL;
}

/* (Dad's call after the Cybeast's fall, story_words.c, in a Soul form: the
 * beast's power in the Souls, a unison's turn more) */
const char *forms_beast_words(void) {
	return "@D One more thing,Lan.|@D MegaMan beat the beast. Its power is in his Souls now.|@D Each unison lasts a turn longer.|"
		"@M Leave it to me,Dad!|@N A unison now lasts four turns!";
}

/* (the rival beaten in his netbattle: what MegaMan took from the fight,
 * BN6's box for it) */
const char *forms_rival_words(int p) {
	static char said[160];
	snprintf(said, sizeof said, "@M Lan! I learned something from ProtoMan!|@N MegaMan got:\n%s!!", forms_power_name(run.form, p));
	return said;
}
