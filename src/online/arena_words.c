#include "arena_words.h"

#include <stdio.h>

#include "protocol.h"

const char *arena_battle_option(void) { return "Battle"; }

/* the rules, as MegaMan names them */
static const char *rules_words(unsigned format) { return format == ONLINE_FORMAT_FULL ? "Full rules" : "BN6 rules"; }

const char *arena_sent_words(const char *name, unsigned format, bool souls_home) {
	static char s[128];
	snprintf(s, sizeof s, "%s%s! Let's see\nif %s\ntakes us on!", souls_home ? "Our Souls stay home:\nBN6 rules!|" : "",
		souls_home ? "OK" : rules_words(format), name);
	return s;
}

const char *arena_declined_words(void) { return "Aw... No NetBattle\nthis time."; }

const char *arena_gone_words(void) { return "Huh? They're gone..."; }

const char *arena_challenged_words(const char *name, unsigned format) {
	static char s[160];
	snprintf(s, sizeof s, "%s's\nMegaMan wants a\nPractice NetBattle!|%s:\n%s", name, rules_words(format),
		format == ONLINE_FORMAT_FULL ? "Souls and BN5 chips\ntoo!" : "no Souls,and BN6\nchips only.");
	return s;
}

const char *arena_folder_name(int k) {
	static const char *const names[3] = { "Folder1", "Folder2", "Extra Folder" };
	return names[k >= 0 && k < 3 ? k : 0];
}

const char *arena_folder_words(unsigned format, const char *standins, bool souls_home) {
	static char s[400];
	int n = snprintf(s, sizeof s, "%s", standins ? standins : "");
	if (souls_home) n += snprintf(s + n, sizeof s - (size_t)n, "Our Souls stay home\ntoo.|");
	snprintf(s + n, sizeof s - (size_t)n, "%s,Lan.\nWhich folder goes?", rules_words(format));
	return s;
}

const char *arena_standin_words(const char *folder, int n, const char *gone, const char *in) {
	static char s[160];
	if (n == 1) snprintf(s, sizeof s, "%s's %s\nsits out,Lan.|Our %s\nstands in for it!|", folder, gone, in);
	else snprintf(s, sizeof s, "%s: %d BN5\nchips sit out,Lan.|Chips from our Pack\nstand in for them,|like %s\nfor %s!|", folder, n, in, gone);
	return s;
}

const char *arena_no_folder_words(bool bn5) {
	return bn5 ? "Lan,every folder of\nours has BN5 chips...|BN6 rules won't\ntake them.|Let's swap them out\nat home first!"
		: "Lan,we have no folder\nfor a NetBattle...";
}

const char *arena_challenged_question(void) { return "Take it,Lan?\n"; }

const char *arena_cant_words(bool souls) {
	return souls ? "Lan,our Souls can't\ngo in a NetBattle..." : "Lan,some chips in\nour Folder can't go\ninto a NetBattle...";
}

const char *arena_won_words(void) { return "Alright! We won,\nLan!!"; }

const char *arena_lost_words(void) { return "Ugh... They got us.|Next time,Lan!"; }

const char *arena_over_words(void) { return "Phew... Good battle!"; }

const char *arena_stopped_words(void) { return "Whoa! The NetBattle\nbroke off!"; }

const char *arena_installs_apart_words(void) {
	return "Huh? Our PETs don't\nmatch on BN5...|Lan,let's reboot\nthe PET and try\nagain!";
}
