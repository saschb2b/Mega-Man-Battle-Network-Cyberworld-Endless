/* The stands' words on the watching screen (stands_text.h) */
#include "stands_text.h"

#include <stdio.h>

const char stands_keys[] = "R: other side  L: cheer  B: leave";
const char stands_setting_up[] = "Setting up the two PETs...";
const char stands_catching_up[] = "Catching up...";
const char stands_stopped[] = "Stopped watching:";
const char stands_over[] = "The NetBattle is over.";
const char stands_back[] = "A: back to the Square";

const char *stands_heading(const char *first, const char *second) {
	static char s[64];
	snprintf(s, sizeof s, "WATCHING %s VS %s", first, second);
	return s;
}

const char *stands_won(const char *winner) {
	static char s[64];
	snprintf(s, sizeof s, "%s won the NetBattle.", winner);
	return s;
}

const char *stands_team_won(const char *lead) {
	static char s[64];
	snprintf(s, sizeof s, "%s's team won the NetBattle.", lead);
	return s;
}
