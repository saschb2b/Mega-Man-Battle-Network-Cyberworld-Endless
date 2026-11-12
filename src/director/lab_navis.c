/* Navis in the Battle Lab (lab_navis.h). */
#include "lab_navis.h"

#include <stdio.h>

#include "data.h"
#include "guardians.h"
#include "lab_text.h"
#include "loot.h"
#include "rivals.h"
#include "run.h"
#include "save.h"
#include "super_boss.h"
#include "xguardian.h"

#define BN6_NAVI_VERSIONS 3     /* a guardian of BN6's: V1, EX, SP */
#define MOVED_NAVI_VERSIONS 4   /* another game's: V1, V2, V3, SP */
#define LAB_PROTOMAN 11          /* BN6's ProtoMan, the rival's Navi */
#define SUPER_FORMS 6           /* a super boss's forms, his enemy ids' six */

static int versions_most(int navi) { return guardian_moved(navi) ? MOVED_NAVI_VERSIONS : super_boss(navi) ? SUPER_FORMS : BN6_NAVI_VERSIONS; }

/* his enemy id in a version as the engine has it, -1 none */
static int navi_id(int navi, int version) {
	if (version < 0 || version >= versions_most(navi)) return -1;
	if (!guardian_moved(navi)) return enemy_id(1, navi, version);
	Encounter e = xguardian_encounter(navi, version);
	return e.nfoes ? e.foes[0].id : -1;
}

bool lab_navi_known(int navi) {
	if (navi < 1 || navi > LAB_NAVI_MOST || navi >= RIVAL_NAVIS) return false;
	if (guardian_moved(navi)) return xguardian_fightable(navi);
	const Guardian *g = guardian(navi);
	return (navi <= 18 || super_boss(navi)) && g && g->name && navi_id(navi, 0) >= 0;
}

/* (ProtoMan is the rival: his netbattle is every third duel won, docs/game/rival.md) */
int lab_navi_wins(int navi) { return navi == LAB_PROTOMAN ? profile.duel_won / 3 : rival(navi)->megaman_won; }

int lab_navi_need(int navi) { return super_boss(navi) || navi == LAB_PROTOMAN ? 1 : 2; }

unsigned lab_navi_versions(int navi, bool all) {
	if (!lab_navi_known(navi)) return 0;
	unsigned held = 0, first = 0;
	for (int v = versions_most(navi) - 1; v >= 0; --v)
		if (navi_id(navi, v) >= 0) { held |= 1u << v; first = 1u << v; }
	if (all) return held;
	if (lab_navi_wins(navi) < lab_navi_need(navi)) return 0;
	unsigned beaten = profile.navi_beaten[navi] & held;
	return beaten ? beaten : first;
}

void lab_navi_name(int navi, int version, char *out, size_t n) {
	int id = navi_id(navi, version);
	out[0] = 0;
	if (id >= 0) enemy_name(id, out, n);
	if (!out[0] && guardian(navi) && guardian(navi)->name) snprintf(out, n, "%s", guardian(navi)->name);
}

int lab_navi_hp(int navi, int version) { return guardian_moved(navi) ? xguardian_hp(navi, version) : navi_hp(navi, version); }

const char *lab_navi_version_word(int navi, int version) { return lab_navi_word(guardian_moved(navi) ? 1 : super_boss(navi) ? 2 : 0, version); }

bool lab_navi_encounter(int navi, int version, int col, int row, Encounter *e) {
	if (navi_id(navi, version) < 0) return false;
	*e = guardian_moved(navi) ? xguardian_encounter(navi, version) : make_boss(1, 0, navi);
	if (!e->nfoes) return false;
	e->foes[0].version = version;
	if (!guardian_moved(navi)) e->foes[0].id = -1;
	e->foes[0].col = col;
	e->foes[0].row = row;
	e->map_backdrop = false;
	e->held = false;
	return true;
}
