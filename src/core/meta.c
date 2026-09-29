/* The meta layer (meta.h, docs/META.md). */
#include "meta.h"

#include <stdio.h>
#include <string.h>

#include "chip_pool.h"
#include "pacing.h"
#include "powers.h"
#include "rivals.h"
#include "run.h"
#include "save.h"

/* Each folder opens with a milestone that teaches its style and that a
 * first or second run reaches (not a count to grind, and not a guardian
 * the net may not bring for many runs: SlashMan and ElecMan, their first
 * keys, came in act 3 or the Nest). */
static const FolderInfo folders[FOLDER_COUNT] = {
	{ "Standard", "BN6's own starting folder", NULL },
	{ "Blade", "Swords in S for LifeSword. Nothing reaches the back", "delete any guardian" },
	{ "Storm", "Elec chips: double on Aqua, plain on the rest", "delete an Aqua guardian" },
};

/* (in any run, this build's or one before: rivals.sav keeps who fell) */
static bool earned(int folder) {
	switch (folder) {
	case FOLDER_BLADE:
		for (int n = 1; n < RIVAL_NAVIS; ++n)
			if (rival(n)->megaman_won > 0) return true;
		return false;
	case FOLDER_STORM:
		/* BN6's Aqua navis among the guardians: SpoutMan and DiveMan */
		return rival(6)->megaman_won > 0 || rival(13)->megaman_won > 0;
	default: return folder == FOLDER_STANDARD;
	}
}

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

/* (earned in any run; the profile's bit marks it as announced on a
 * summary, and keeps one opened under an older rule) */
bool meta_folder_open(int folder) {
	if (folder <= FOLDER_STANDARD || folder >= FOLDER_COUNT) return folder == FOLDER_STANDARD;
	return (profile.folders_open >> folder & 1) || earned(folder);
}

/* (the Cross navis, HeatMan 1 .. ChargeMan 5: rivals.sav keeps who fell) */
static bool cross_earned(int navi) { return navi >= 1 && navi <= 5 && rival(navi)->megaman_won > 0; }

bool meta_cross_open(int navi) { return navi >= 1 && navi <= 5 && ((profile.crosses_open >> navi & 1) || cross_earned(navi)); }

bool meta_endless_open(void) { return profile.short_wins > 0 || profile.nest_clears > 0; }

int meta_threat_open(void) { return profile.threat_open > THREAT_MAX ? THREAT_MAX : profile.threat_open; }

const char *meta_threat_rule(int rung) {
	static const char *const rules[THREAT_MAX] = {
		"Viruses a version stronger from act 2",
		"Heals only mid-act and before guardians",
		"Net Dealers charge half again",
		"Guardians at EX from act 2",
		"Servers may hold SP Navis from act 1",
		"Mystery Data holds chips, never zenny",
		"Chip Traders come half as often",
		"Guardian Data drafts two programs",
		"Guardian Data gives four HPMemory",
		"A second guardian waits below the short net's Nest",
	};
	return rung >= 1 && rung <= THREAT_MAX ? rules[rung - 1] : "";
}

bool meta_threat(int rung) { return run.threat >= rung; }

static const char *said[6];
static uint16_t marks_new;
static char lines[6][48];
static int nsaid;

static void say(const char *fmt, const char *what) {
	if (nsaid >= 6) return;
	snprintf(lines[nsaid], sizeof lines[nsaid], fmt, what);
	said[nsaid] = lines[nsaid];
	++nsaid;
}

bool meta_library_has(int id) { return id > 0 && id < 8 * (int)sizeof profile.library && (profile.library[id / 8] >> (id % 8) & 1); }

bool meta_library_add(int id) {
	if (id <= 0 || id >= 8 * (int)sizeof profile.library || meta_library_has(id)) return false;
	profile.library[id / 8] |= (uint8_t)(1u << (id % 8));
	return true;
}

int meta_library_count(int cls) {
	int n = 0;
	for (int id = 1; id < 8 * (int)sizeof profile.library; ++id)
		if (meta_library_has(id) && (cls < 0 ? chip_pool_class(id) >= 0 : chip_pool_class(id) == cls)) ++n;
	return n;
}

int meta_vault_need(int depth) {
	/* (a run holds its starting folder's dozen and about twenty more: act
	 * 2's opens after a run or two, act 4's for a collector) */
	static const int need[] = { 30, 30, 60, 90 };
	int act = pacing_act(depth);
	return need[act < 3 ? act : 3];
}

int meta_library_new(void) {
	int n = meta_library_count(-1) - profile.library_start;
	return n > 0 ? n : 0;
}

void meta_run_begun(void) {
	for (int f = 1; f < FOLDER_COUNT; ++f)
		if (earned(f)) profile.folders_open |= (uint16_t)(1u << f);
	for (int n = 1; n <= 5; ++n)
		if (cross_earned(n)) profile.crosses_open |= (uint8_t)(1u << n);
	/* (the Library as the run begins: the summary counts what it adds) */
	profile.library_start = (uint16_t)meta_library_count(-1);
	profile.library_run = run.seed;
	profile_save();
}

void meta_run_over(bool won) {
	nsaid = 0;
	marks_new = 0;
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
	/* the title's marks (meta.h; the summary's line "Unlocked: Gregar's mark") */
	static const struct { int bit; const char *what; } marks[] = {
		{ MARK_WIN, "Gregar's mark" }, { MARK_NEST, "Bass's mark" }, { MARK_SECRET, "the S mark" }, { MARK_THREAT, "the disc mark" },
		{ MARK_STD, "the STD COMP mark" }, { MARK_MEGA, "the MEGA COMP mark" }, { MARK_GIGA, "the GIGA COMP mark" },
	};
	/* (a Library class complete: every chip of it a run can hold) */
	bool comp[3];
	for (int c = 0; c < 3; ++c) comp[c] = chip_pool_class_count(c) > 0 && meta_library_count(c) >= chip_pool_class_count(c);
	for (unsigned i = 0; i < sizeof marks / sizeof *marks; ++i) {
		int b = marks[i].bit;
		bool earned = (b == MARK_WIN && won && run.mode == RUN_SHORT) ||
			(b == MARK_NEST && run.mode == RUN_ENDLESS && run.depth > CYCLE_LAYERS) ||
			(b == MARK_SECRET && run.secret_cleared) ||
			(b == MARK_THREAT && won && run.threat >= THREAT_MAX) ||
			(b == MARK_STD && comp[0]) || (b == MARK_MEGA && comp[1]) || (b == MARK_GIGA && comp[2]);
		if (!earned || (profile.marks & b)) continue;
		profile.marks |= (uint16_t)b;
		marks_new |= (uint16_t)b;
		say("%s", marks[i].what);
	}
	/* a folder, once its milestone is reached in any run */
	for (int f = 1; f < FOLDER_COUNT; ++f) {
		if ((profile.folders_open >> f & 1) || !earned(f)) continue;
		profile.folders_open |= (uint16_t)(1u << f);
		say("the %s folder", folders[f].name);
	}
	/* a Cross start, once its navi falls as a guardian */
	for (int n = 1; n <= 5; ++n) {
		if ((profile.crosses_open >> n & 1) || !cross_earned(n)) continue;
		profile.crosses_open |= (uint8_t)(1u << n);
		say("the %s start", powers_cross_name(n));
	}
	profile_save();
}

uint16_t meta_marks_new(void) { return marks_new; }

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
		if (!meta_folder_open(f) && folders[f].opens) {
			/* ("Delete an Aqua guardian: Storm") */
			snprintf(goal, sizeof goal, "%c%s: %s", folders[f].opens[0] - 'a' + 'A', folders[f].opens + 1, folders[f].name);
			return goal;
		}
	bool cross = false;
	for (int n = 1; n <= 5; ++n) cross |= meta_cross_open(n);
	if (!cross) return "Delete a Cross Navi: Cross start";
	if (meta_threat_open() < THREAT_MAX) {
		snprintf(goal, sizeof goal, "Win on threat %d for threat %d", meta_threat_open(), meta_threat_open() + 1);
		return goal;
	}
	return NULL;
}
