#include "arena_words.h"

#include <stdio.h>

const char *arena_battle_option(void) { return "Battle"; }

const char *arena_sent_words(const char *name) {
	static char s[128];
	snprintf(s, sizeof s, "OK! Let's see\nif %s\ntakes us on!", name);
	return s;
}

const char *arena_declined_words(void) { return "Aw... No NetBattle\nthis time."; }

const char *arena_gone_words(void) { return "Huh? They're gone..."; }

const char *arena_busy_words(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "Huh? %s's\nbusy right now...|Let's ask again\nlater,Lan.", name);
	return s;
}

const char *arena_not_here_words(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "Huh? %s\nisn't here anymore...", name);
	return s;
}

const char *arena_closed_words(void) { return "Aw... No NetBattles\non this Square."; }

const char *arena_no_stamp_words(void) { return "Hmm... The Arena\nwon't let that\nNetBattle through."; }

const char *arena_challenged_words(const char *name) {
	static char s[160];
	snprintf(s, sizeof s, "%s's\nMegaMan wants a\nPractice NetBattle!", name);
	return s;
}

const char *arena_folder_name(int k) {
	static const char *const names[3] = { "Folder1", "Folder2", "Extra Folder" };
	return names[k >= 0 && k < 3 ? k : 0];
}

const char *arena_folder_words(void) { return "Which folder goes,\nLan?"; }

const char *arena_no_folder_words(void) { return "Lan,we have no\nfolder for a\nNetBattle..."; }

const char *arena_challenged_question(void) { return "Take it,Lan?\n"; }

const char *arena_cant_words(void) { return "Lan,some chips in\nour Folder can't go\ninto a NetBattle..."; }

const char *arena_won_words(void) { return "Alright! We won,\nLan!!"; }

const char *arena_lost_words(void) { return "Ugh... They got us.|Next time,Lan!"; }

const char *arena_over_words(void) { return "Phew... Good battle!"; }

const char *arena_stopped_words(void) { return "Whoa! The NetBattle\nbroke off!"; }

const char *arena_installs_apart_words(void) {
	return "Huh? Our PETs don't\nmatch...|Lan,let's reboot\nthe PET and try\nagain!";
}
