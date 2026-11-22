/* forms.h. The kinds are #774's table; a Soul's kind its game's (BN5's
 * Souls, docs/game/meta/souls.md; BN4's, docs/multirom/bn4-souls.md). */
#include "forms.h"

#include <string.h>

#include "bn6.h"
#include "crossselect.h"
#include "flags.h"
#include "guardians.h"
#include "meta.h"
#include "powers.h"
#include "rom.h"
#include "rivals.h"
#include "run.h"
#include "save.h"
#include "soulbn4.h"
#include "souls.h"
#include "soulunite.h"
#include "super_boss.h"

/* each form's powers: name and kind, by its number (1-based) */
typedef struct { const char *name; uint8_t kind; } Power;
static const Power cross_powers[] = {
	{ "HeatCross", KIND_FIRE }, { "ElecCross", KIND_ELEC }, { "SlashCross", KIND_SWORD }, { "EraseCross", KIND_CURSOR }, { "ChargeCross", KIND_FIRE },
	{ "SpoutCross", KIND_AQUA }, { "TomahawkCross", KIND_WOOD }, { "TenguCross", KIND_WIND }, { "GroundCross", KIND_BREAK }, { "DustCross", KIND_BREAK },
};
static const Power bn5_powers[] = {
	{ "ProtoSoul", KIND_SWORD }, { "GyroSoul", KIND_WIND }, { "SearchSoul", KIND_CURSOR }, { "NapalmSoul", KIND_FIRE }, { "MagnetSoul", KIND_ELEC },
	{ "MeddySoul", KIND_RECOVERY }, { "ColonelSoul", KIND_OBSTACLE }, { "ShadowSoul", KIND_INVISIBLE }, { "NumberSoul", KIND_PLUS },
	{ "TomahawkSoul", KIND_WOOD }, { "KnightSoul", KIND_BREAK }, { "ToadSoul", KIND_AQUA },
};
static const Power bn4_powers[] = {
	{ "RollSoul", KIND_RECOVERY }, { "GutsSoul", KIND_CRACK }, { "WindSoul", KIND_WIND }, { "SrchSoul", KIND_INVISIBLE }, { "FireSoul", KIND_FIRE },
	{ "ThunSoul", KIND_ELEC }, { "ProtSoul", KIND_SWORD }, { "NumSoul", KIND_PLUS }, { "MetlSoul", KIND_BREAK }, { "JunkSoul", KIND_OBSTACLE },
	{ "AquaSoul", KIND_AQUA }, { "WoodSoul", KIND_WOOD },
};

static const Power *powers_of(int form, int *n) {
	*n = form == RUN_FORM_CROSS ? (int)(sizeof cross_powers / sizeof *cross_powers) : form == RUN_FORM_SOUL_BN5 || form == RUN_FORM_SOUL_BN4 ? 12 : 0;
	return form == RUN_FORM_CROSS ? cross_powers : form == RUN_FORM_SOUL_BN5 ? bn5_powers : form == RUN_FORM_SOUL_BN4 ? bn4_powers : NULL;
}

const char *forms_power_name(int form, int p) {
	int n;
	const Power *pw = powers_of(form, &n);
	return pw && p >= 1 && p <= n ? pw[p - 1].name : NULL;
}

int forms_power_kind(int form, int p) {
	int n;
	const Power *pw = powers_of(form, &n);
	return pw && p >= 1 && p <= n ? pw[p - 1].kind : KIND_NONE;
}

/* (BN6's Navis: his Cross's kind, a Cross's element read against its
 * weakness, docs/game/meta/design.md; ProtoMan and Colonel, who have no
 * Cross, their Souls' in the older games; the rest none) */
static const uint8_t bn6_kinds[] = {
	[1] = KIND_FIRE, [2] = KIND_ELEC, [3] = KIND_SWORD, [4] = KIND_CURSOR, [5] = KIND_FIRE, [6] = KIND_AQUA, [7] = KIND_WOOD,
	[8] = KIND_WIND, [9] = KIND_BREAK, [10] = KIND_BREAK, [11] = KIND_SWORD, [18] = KIND_OBSTACLE,
};

/* the power navi `navi` has of his own in `form`: his Cross (BN6's ten),
 * his Soul (BN5's twelve; BN4's first twelve seats, Roll to WoodMan, in
 * its Souls' order) */
static int own_power(int form, int navi) {
	if (form == RUN_FORM_CROSS) return navi >= 1 && navi <= RUN_CROSS_NAVIS ? navi : 0;
	if (form == RUN_FORM_SOUL_BN5) { int ai = guardian_bn5_ai(navi); return ai >= 1 && ai <= SOULS_OF_BN5 ? ai : 0; }
	if (form == RUN_FORM_SOUL_BN4) return guardian_bn4(navi) && navi - GUARDIAN_BN4_FIRST < SOULS_OF_BN4 ? navi - GUARDIAN_BN4_FIRST + 1 : 0;
	return 0;
}

int forms_kind(int navi) {
	if (navi >= 0 && navi < (int)sizeof bn6_kinds && bn6_kinds[navi]) return bn6_kinds[navi];
	int p = own_power(RUN_FORM_SOUL_BN5, navi);
	if (p) return bn5_powers[p - 1].kind;
	p = own_power(RUN_FORM_SOUL_BN4, navi);
	return p ? bn4_powers[p - 1].kind : KIND_NONE;
}

/* (a BN4 Soul stands where BN4's Souls are moved in: soulbn4.h, each read
 * from BN4's ROM by its place among those moved) */
static bool bn4_ready(int p) {
	SoulBn4 s;
	for (int k = 0; k < SOULS_BN4; ++k)
		if (soulbn4_read(k, &s) && s.form == p) return true;
	return false;
}

static bool forms_ready(int form, int p) {
	if (form == RUN_FORM_CROSS) return p >= 1 && (p <= 5 || (p <= RUN_CROSS_NAVIS && crossselect_falzar()));
	/* (Team Colonel's six stand with BN5's Souls; Team ProtoMan's join as
	 * their own branch moves them, docs/game/team-protoman.md) */
	if (form == RUN_FORM_SOUL_BN5) return p > SOULS_TEAM && p <= SOULS_OF_BN5 && soulunite_ready();
	if (form == RUN_FORM_SOUL_BN4) return p >= 1 && p <= SOULS_OF_BN4 && bn4_ready(p);
	return false;
}

bool forms_offered(int form) {
	if (form == RUN_FORM_CROSS) return true;
	if (form == RUN_FORM_SOUL_BN5) return XR[XROM_BN5_COLONEL_US].data != NULL;
	return form == RUN_FORM_SOUL_BN4 && XR[XROM_BN4_RED_SUN_US].data != NULL && SOULS_BN4 > 0;
}

/* (a seeded pick that leaves the run's generator alone) */
static uint32_t mix(uint32_t h) {
	h ^= h >> 16;
	h *= 0x7FEB352Du;
	h ^= h >> 15;
	h *= 0x846CA68Bu;
	return h ^ h >> 16;
}

int forms_pay_how(int form, int navi, unsigned held, uint32_t seed, int *p) {
	int n;
	const Power *pw = powers_of(form, &n);
	*p = 0;
	if (!pw) return PAY_NONE;
	int own = own_power(form, navi);
	if (own && !(held >> (own - 1) & 1) && forms_ready(form, own)) { *p = own; return PAY_OWN; }
	int kind = forms_kind(navi);
	for (int k = 1; kind && k <= n; ++k)
		if (pw[k - 1].kind == kind && !(held >> (k - 1) & 1) && forms_ready(form, k)) { *p = k; return PAY_KIND; }
	int lack[FORM_POWERS], nlack = 0;
	for (int k = 1; k <= n; ++k)
		if (!(held >> (k - 1) & 1) && forms_ready(form, k)) lack[nlack++] = k;
	if (!nlack) return PAY_NONE;
	*p = lack[mix(seed ^ (uint32_t)navi * 0x9E3779B9u) % (uint32_t)nlack];
	return PAY_LACKED;
}

int forms_pay(int form, int navi, unsigned held, uint32_t seed) {
	int p;
	forms_pay_how(form, navi, held, seed, &p);
	return p;
}

static unsigned forms_held(void) {
	return run.form == RUN_FORM_CROSS ? run.cross_held : run.form == RUN_FORM_SOUL_BN5 ? souls_held_bn5() : run.form == RUN_FORM_SOUL_BN4 ? souls_held_bn4() : 0;
}

unsigned forms_held_layer(void) {
	return run.form == RUN_FORM_CROSS ? run.cross_layer : run.form == RUN_FORM_SOUL_BN5 ? souls_layer_bn5() : run.form == RUN_FORM_SOUL_BN4 ? souls_layer_bn4() : 0;
}

static void forms_give(int p) {
	if (run.form == RUN_FORM_CROSS && p >= 1 && p <= RUN_CROSS_NAVIS) { run.cross_held |= (uint16_t)(1u << (p - 1)); powers_cross_give(p); }
	if (run.form == RUN_FORM_SOUL_BN5) souls_give_bn5(p);
	if (run.form == RUN_FORM_SOUL_BN4) souls_give_bn4(p);
}

void forms_bring(void) {
	bool beast = run_beast_start() || run.depth > CYCLE_LAYERS;
	if (run.form == RUN_FORM_CROSS) {
		/* (the Cross brought, the run's from its start) */
		if (run.cross >= 1 && run.cross <= RUN_CROSS_NAVIS) {
			run.cross_held |= (uint16_t)(1u << (run.cross - 1));
			run.cross_layer |= (uint16_t)(1u << (run.cross - 1));
		}
		for (int p = 1; p <= RUN_CROSS_NAVIS; ++p)
			if (run_cross_in_hand(p)) powers_cross_give(p);
		if (beast) flag_set(BN6_FLAG_BEAST_OUT);
	} else if (beast) run.soul_turns = 1;
}

/* (BN5's or BN4's Souls given in any run, which a Soul form may bring
 * from its start: meta.h) */
static void soul_won(int p) {
	if (run.form == RUN_FORM_SOUL_BN5 || run.form == RUN_FORM_SOUL_BN4) meta_soul_won(run.form, p);
}

bool forms_after_boss(int navi) {
	/* (the Cybeast's fall, BeastOut for the rest of the run in CROSS, and
	 * in a Soul form a unison's turn more, BN5's SoulT+1) */
	if (navi == SUPER_CYBEAST) {
		if (run_form_crosses()) flag_set(BN6_FLAG_BEAST_OUT);
		else run.soul_turns = 1;
		return false;
	}
	if (super_boss(navi)) return false;
	/* (what the layer's data said, from the powers held as it was made; a
	 * power given since on the layer, another the run lacks) */
	int p = forms_pay(run.form, navi, forms_held_layer(), run.layer_seed);
	if (p && (forms_held() >> (p - 1) & 1)) p = forms_pay(run.form, navi, forms_held(), run.layer_seed);
	if (!p) return false;
	forms_give(p);
	soul_won(p);
	return run.form != RUN_FORM_CROSS;
}

const char *forms_reward_cross(int navi) {
	if (run.form != RUN_FORM_CROSS || super_boss(navi)) return NULL;
	return forms_power_name(RUN_FORM_CROSS, forms_pay(RUN_FORM_CROSS, navi, forms_held_layer(), run.layer_seed));
}

/* (ProtoMan, the rival: navi 11, guardians.h) */
#define RIVAL_NAVI 11

void forms_rival_pay(char *words, size_t n) {
	if (run.form == RUN_FORM_MIX) return;
	int p = forms_pay(run.form, RIVAL_NAVI, forms_held(), run.layer_seed ^ 0x52495641u);
	if (!p) return;
	forms_give(p);
	if (run.form != RUN_FORM_CROSS) meta_soul_won(run.form, p);
	size_t k = strlen(words);
	if (k < n) snprintf(words + k, n - k, "%s%s", k ? "|" : "", forms_rival_words(p));
}

/* (a Soul form opens once a Navi of its game falls as a guardian, in any
 * run, as a deleted Navi opens his Cross start: noted as he falls, and
 * from the rivals' records for the falls before forms) */
void forms_note_fall(int navi) {
	int f = guardian_older(navi) || guardian_team(navi) ? RUN_FORM_SOUL_BN5 : guardian_bn4(navi) ? RUN_FORM_SOUL_BN4 : RUN_FORM_MIX;
	if (f == RUN_FORM_MIX || (profile.forms_open >> f & 1)) return;
	profile.forms_open |= (uint8_t)(1u << f);
	profile_save();
}

void forms_note_falls(void) {
	for (int navi = 0; navi < RIVAL_NAVIS; ++navi)
		if (rival(navi)->megaman_won) forms_note_fall(navi);
}

void forms_describe(FILE *f) {
	fprintf(f, "runform %s\npowers", meta_form_name(run.form));
	unsigned held = forms_held();
	for (int p = 1; p <= FORM_POWERS; ++p)
		if (held >> (p - 1) & 1) fprintf(f, " %s", forms_power_name(run.form, p));
	fprintf(f, "%s\nsoulturns %d\n", held ? "" : " none", run.soul_turns);
}
