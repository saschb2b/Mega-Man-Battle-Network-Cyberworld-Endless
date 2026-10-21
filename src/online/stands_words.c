#include "stands_words.h"

#include <stdio.h>
#include <string.h>

/* a name as a Mr.Prog says it, in capitals */
static const char *caps(const char *s, char *out, size_t n) {
	size_t i = 0;
	for (; s && s[i] && i + 1 < n; ++i) out[i] = (char)(s[i] >= 'a' && s[i] <= 'z' ? s[i] - 'a' + 'A' : s[i]);
	out[i] = 0;
	return out;
}

/* the battle on: who, the HP, the last chips, who watches */
static int live_words(char *s, size_t n, const StandsBout *b) {
	char a[20], z[20], chip[24];
	caps(b->name[0], a, sizeof a);
	caps(b->name[1], z, sizeof z);
	int k = b->bout ? snprintf(s, n, "|TEAM NETBATTLE!\nBOUT %u!|%s VS\n%s!", b->bout, a, z) : snprintf(s, n, "|A NETBATTLE'S ON!\n%s VS\n%s!", a, z);
	if (b->report) k += snprintf(s + k, n - (size_t)k, "|%s HAS %u HP!\n%s HAS %u!", a, b->hp[0], z, b->hp[1]);
	for (int side = 0; side < 2; ++side)
		if (b->chip[side]) k += snprintf(s + k, n - (size_t)k, "|%s USED\n%s!", side ? z : a, caps(b->chip[side], chip, sizeof chip));
	if (b->watchers) k += snprintf(s + k, n - (size_t)k, b->watchers == 1 ? "|ONE NAVI'S\nWATCHING!" : "|%u NAVIS\nWATCHING!", b->watchers);
	return k;
}

/* the last battle over: who won, the cheers (this game's own: the cheers
 * alone, the referee's own words having called the bout) */
static int last_words(char *s, size_t n, const StandsBout *b) {
	if (b->mine) return b->cheers ? snprintf(s, n, "|THE STANDS CHEERED\nFOR YOU! %u CHEERS!", b->cheers) : 0;
	char a[20], z[20];
	caps(b->name[b->winner == 2], a, sizeof a);
	caps(b->name[b->winner != 2], z, sizeof z);
	int k = b->winner ? snprintf(s, n, "|%s BEAT\n%s!\nWHAT A BOUT!", a, z) : snprintf(s, n, "|%s VS\n%s...\nIT BROKE OFF!", a, z);
	if (b->cheers) k += snprintf(s + k, n - (size_t)k, "|%u CHEERS FROM\nTHE STANDS!", b->cheers);
	return k;
}

const char *stands_referee_words(const StandsBout *live, const StandsBout *last) {
	static char s[400];
	int k = 0;
	s[0] = 0;
	if (live) k += live_words(s + k, sizeof s - (size_t)k, live);
	if (last && !live) last_words(s + k, sizeof s - (size_t)k, last);
	return s;
}

const char *stands_watch_question(void) { return "WANNA WATCH?\n"; }

const char *stands_watch_off(void) { return "TO THE STANDS!\nENJOY THE BOUT!"; }

const char *stands_board_words(const StandsBout *live, const StandsBout *last) {
	static char s[320];
	int k = 0;
	s[0] = 0;
	if (live) {
		k += live->bout ? snprintf(s + k, sizeof s - (size_t)k, ">Team NetBattle,\nbout %u!\n%s vs %s", live->bout, live->name[0], live->name[1]) :
			snprintf(s + k, sizeof s - (size_t)k, ">NetBattle now!\n%s vs %s", live->name[0], live->name[1]);
		if (live->report)
			k += snprintf(s + k, sizeof s - (size_t)k, "|>%s %u HP,\n%s %u HP.", live->name[0], live->hp[0], live->name[1], live->hp[1]);
	}
	if (!last || !last->winner) return s;
	/* (a box of its own after the battle on's) */
	if (k && (size_t)k + 1 < sizeof s) s[k++] = '|';
	snprintf(s + k, sizeof s - (size_t)k, ">%s beat\n%s!\n%u cheers.", last->name[last->winner == 2], last->name[last->winner != 2],
		last->cheers);
	return s;
}
