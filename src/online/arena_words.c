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
	snprintf(s, sizeof s, "Huh? %s\nisn't here\nanymore...", name);
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

const char *arena_team_option(bool partner) { return partner ? "Split up" : "Team up"; }

const char *arena_team_sent_words(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "Let's see if\n%s\nteams up with us!", name);
	return s;
}

const char *arena_team_asked_words(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "%s wants\nto team up\nwith us!", name);
	return s;
}

const char *arena_team_asked_question(void) { return "Team up,Lan?\n"; }

const char *arena_team_formed_words(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "Alright! We're a\nteam with\n%s!", name);
	return s;
}

const char *arena_team_declined_words(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "Aw... %s\nwon't team up\nthis time.", name);
	return s;
}

const char *arena_team_split_words(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "OK... Our team\nwith %s\nsplit up.", name);
	return s;
}

const char *arena_team_have_words(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "Lan,we're with\n%s\nalready!", name);
	return s;
}

const char *arena_team_lone_words(const char *name, bool we_teamed) {
	static char s[96];
	if (we_teamed) snprintf(s, sizeof s, "Huh? %s\nhas no partner...", name);
	else snprintf(s, sizeof s, "Huh? %s's\non a team...|We'd need a\npartner too,Lan.", name);
	return s;
}

const char *arena_team_partner_words(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "Huh? %s's\non our team,Lan!", name);
	return s;
}

const char *arena_team_challenged_words(const char *name) {
	static char s[160];
	snprintf(s, sizeof s, "%s's team\nwants a Team\nNetBattle!", name);
	return s;
}

const char *arena_team_order_words(void) { return "Who goes first,\nLan?"; }

const char *arena_team_order_us(void) { return "We do"; }

const char *arena_team_first_words(bool we_lead, const char *ours, const char *theirs) {
	static char s[128];
	if (we_lead) snprintf(s, sizeof s, "We're up first,\nLan!|Against\n%s!", theirs);
	else snprintf(s, sizeof s, "%s goes\nfirst!|Go get 'em,\n%s!", ours, ours);
	return s;
}

const char *arena_team_over_words(unsigned result) {
	switch (result) {
	case 1: return "Alright!! Our team\nwon,Lan!!";
	case 2: return "Aw... Our team\nlost.|We'll get 'em\nnext time!";
	case 4: return "Phew... Both teams\nwent down at once!";
	default: return "Whoa! The Team\nNetBattle broke off!";
	}
}

const char *arena_installs_apart_words(void) {
	return "Huh? Our PETs don't\nmatch...|Lan,let's reboot\nthe PET and try\nagain!";
}
