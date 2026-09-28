/* The meta layer (meta.h, docs/META.md). */
#include "meta.h"

#include <stdio.h>
#include <string.h>

#include "guardians.h"
#include "rivals.h"
#include "run.h"
#include "save.h"

/* Each folder opens as a run deletes the guardian whose style it takes
 * (the skill's milestones that teach, not counts to grind). */
static const FolderInfo folders[FOLDER_COUNT] = {
	{ "Standard", "BN6's own starting folder", 0 },
	{ "Blade", "Swords in S for LifeSword. Nothing reaches the back", 3 },   /* SlashMan */
	{ "Storm", "Elec chips: double on Aqua, plain on the rest", 2 },       /* ElecMan */
};

const FolderInfo *meta_folder(int folder) { return &folders[folder >= 0 && folder < FOLDER_COUNT ? folder : 0]; }

/* The folders' chips by BN6's record numbers (docs/ROM_DATA.md, chip
 * records): no more than five of a chip, as the game's folders hold; codes
 * gathered so a hand can be picked together. */
#define C(id, code) (uint16_t)((id) | (((code) == '*' ? 26 : (code) - 'A') << 9))
enum { SWORD = 71, WIDESWRD = 72, LONGSWRD = 73, AREAGRAB = 163, PANLGRAB = 162, RECOV10 = 154, ATK10 = 192, BARRIER = 178,
	THUNDER = 30, ELCPULS1 = 34, DOLTHDR1 = 31, CANNON = 1, VULCAN1 = 5, MINIBOMB = 54, AIRSHOT = 4, BUSTERUP = 175 };
static const uint16_t blade[30] = {
	/* LifeSword's three in S, a spare in L; AreaGrab to close in, nothing
	 * that reaches the back */
	C(SWORD, 'S'), C(SWORD, 'S'), C(SWORD, 'S'), C(SWORD, 'S'), C(SWORD, 'L'),
	C(WIDESWRD, 'S'), C(WIDESWRD, 'S'), C(WIDESWRD, 'S'), C(WIDESWRD, 'S'), C(WIDESWRD, 'L'),
	C(LONGSWRD, 'S'), C(LONGSWRD, 'S'), C(LONGSWRD, 'S'),
	C(AREAGRAB, 'S'), C(AREAGRAB, 'S'), C(AREAGRAB, 'S'), C(PANLGRAB, '*'), C(PANLGRAB, '*'),
	C(RECOV10, 'L'), C(RECOV10, 'L'), C(RECOV10, 'L'), C(RECOV10, '*'), C(RECOV10, '*'),
	C(ATK10, '*'), C(ATK10, '*'), C(ATK10, '*'), C(ATK10, '*'),
	C(BARRIER, '*'), C(BARRIER, '*'), C(BARRIER, '*'),
};
static const uint16_t storm[30] = {
	/* Elec in S and A: Thunder, ElcPuls1, DolThdr1; Cannons and bombs for
	 * the rest */
	C(THUNDER, 'S'), C(THUNDER, 'S'), C(THUNDER, 'S'), C(THUNDER, 'S'),
	C(ELCPULS1, 'S'), C(ELCPULS1, 'S'), C(DOLTHDR1, 'A'),
	C(CANNON, 'A'), C(CANNON, 'A'), C(CANNON, 'A'), C(CANNON, '*'),
	C(VULCAN1, 'S'), C(VULCAN1, 'S'), C(VULCAN1, 'S'),
	C(MINIBOMB, '*'), C(MINIBOMB, '*'), C(AIRSHOT, '*'), C(AIRSHOT, '*'), C(AIRSHOT, '*'),
	C(RECOV10, 'A'), C(RECOV10, 'A'), C(RECOV10, '*'), C(RECOV10, '*'),
	C(AREAGRAB, 'S'), C(AREAGRAB, 'S'), C(BARRIER, 'A'), C(BARRIER, 'A'),
	C(ATK10, '*'), C(ATK10, '*'), C(BUSTERUP, '*'),
};
#undef C

const uint16_t *meta_folder_chips(int folder) {
	switch (folder) {
	case FOLDER_BLADE: return blade;
	case FOLDER_STORM: return storm;
	default: return NULL;
	}
}

/* (its guardian deleted in any run, this build's or one before; the
 * profile's bit marks it as announced on a summary) */
bool meta_folder_open(int folder) {
	if (folder <= FOLDER_STANDARD || folder >= FOLDER_COUNT) return folder == FOLDER_STANDARD;
	return (profile.folders_open >> folder & 1) || (folders[folder].navi && rival(folders[folder].navi)->megaman_won > 0);
}

bool meta_endless_open(void) { return profile.short_wins > 0 || profile.nest_clears > 0; }

int meta_threat_open(void) { return profile.threat_open > THREAT_MAX ? THREAT_MAX : profile.threat_open; }

const char *meta_threat_rule(int rung) {
	static const char *const rules[THREAT_MAX] = {
		"Viruses a version stronger from act 2",
		"Heals only mid-act and before guardians",
		"Net Dealers charge half again",
		"Guardians at EX from act 2",
		"Servers may hold SP Navis from act 1",
	};
	return rung >= 1 && rung <= THREAT_MAX ? rules[rung - 1] : "";
}

bool meta_threat(int rung) { return run.threat >= rung; }

static const char *said[6];
static char lines[6][48];
static int nsaid;

static void say(const char *fmt, const char *what) {
	if (nsaid >= 6) return;
	snprintf(lines[nsaid], sizeof lines[nsaid], fmt, what);
	said[nsaid] = lines[nsaid];
	++nsaid;
}

void meta_run_over(bool won) {
	nsaid = 0;
	/* the endless net, once a short one is won */
	if (won && run.mode == RUN_SHORT && !meta_endless_open()) say("%s", "the endless net");
	if (won) profile.short_wins += run.mode == RUN_SHORT;
	/* the next threat rung, once the Nest falls on this one */
	if (won && run.threat >= meta_threat_open() && run.threat < THREAT_MAX) {
		profile.threat_open = (uint8_t)(run.threat + 1);
		static char rung[24];
		snprintf(rung, sizeof rung, "threat %d", run.threat + 1);
		say("%s", rung);
	}
	/* a folder, once its guardian has been deleted in any run */
	for (int f = 1; f < FOLDER_COUNT; ++f) {
		if ((profile.folders_open >> f & 1) || !folders[f].navi || rival(folders[f].navi)->megaman_won <= 0) continue;
		profile.folders_open |= (uint16_t)(1u << f);
		say("the %s folder", folders[f].name);
	}
	profile_save();
}

int meta_unlocked(const char **out, int max) {
	int n = nsaid < max ? nsaid : max;
	for (int i = 0; i < n; ++i) out[i] = said[i];
	return n;
}

const char *meta_next_goal(void) {
	static char goal[64];
	/* (a line of the summary: 32 letters at most) */
	if (!meta_endless_open()) return "Win the net for the endless net";
	for (int f = 1; f < FOLDER_COUNT; ++f)
		if (!meta_folder_open(f) && folders[f].navi) {
			snprintf(goal, sizeof goal, "Delete %s: the %s folder", guardian(folders[f].navi)->name, folders[f].name);
			if (strlen(goal) > 32) snprintf(goal, sizeof goal, "Delete %s: %s folder", guardian(folders[f].navi)->name, folders[f].name);
			return goal;
		}
	if (meta_threat_open() < THREAT_MAX) {
		snprintf(goal, sizeof goal, "Win on threat %d for threat %d", meta_threat_open(), meta_threat_open() + 1);
		return goal;
	}
	return NULL;
}
