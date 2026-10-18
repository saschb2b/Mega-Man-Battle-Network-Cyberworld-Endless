#include "arena_words.h"

#include <stdio.h>

const char *arena_battle_option(void) { return "Battle"; }

const char *arena_sent_words(const char *name) {
	static char s[64];
	snprintf(s, sizeof s, "Let's see if\n%s\ntakes us on!", name);
	return s;
}

const char *arena_declined_words(void) { return "Aw... No NetBattle\nthis time."; }

const char *arena_gone_words(void) { return "Huh? They're gone..."; }

const char *arena_challenged_words(const char *name) {
	static char s[80];
	snprintf(s, sizeof s, "%s's\nMegaMan wants a\nPractice NetBattle!", name);
	return s;
}

const char *arena_challenged_question(void) { return "Take it,Lan?\n"; }

const char *arena_cant_words(bool souls) {
	return souls ? "Lan,our Souls can't\ngo in a NetBattle..." : "Lan,some chips in\nour Folder can't go\ninto a NetBattle...";
}

const char *arena_won_words(void) { return "Alright! We won,\nLan!!"; }

const char *arena_lost_words(void) { return "Ugh... They got us.|Next time,Lan!"; }

const char *arena_over_words(void) { return "Phew... Good battle!"; }

const char *arena_stopped_words(void) { return "Whoa! The NetBattle\nbroke off!"; }
