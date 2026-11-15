/* dive_words.h */
#include "dive_words.h"

#include <stdio.h>

#include "protocol.h"

const char *dive_feed_words(unsigned kind, const char *name, unsigned depth, unsigned party) {
	static char s[64];
	switch (kind) {
	case ONLINE_FEED_DELETED: snprintf(s, sizeof s, "%s deleted on layer %u", name, depth); break;
	case ONLINE_FEED_WON: snprintf(s, sizeof s, "%s cleared the Deep Net!", name); break;
	case ONLINE_FEED_GUARDIAN:
		if (party > 1) snprintf(s, sizeof s, "%s's party of %u beat layer %u", name, party, depth);
		else snprintf(s, sizeof s, "%s beat layer %u's guardian", name, depth);
		break;
	case ONLINE_FEED_FLARE: snprintf(s, sizeof s, "%s waits at layer %u's door", name, depth); break;
	default: s[0] = 0; break;
	}
	return s;
}

const char *dive_find_words(const char *name, bool mine) {
	static char s[64];
	if (mine) snprintf(s, sizeof s, "First to open this one this week!");
	else snprintf(s, sizeof s, "%s opened this one first", name);
	return s;
}

/* an English place: 1st, 2nd, 3rd, 4th, 11th, 21st */
static const char *suffix(unsigned n) {
	if (n % 100 >= 11 && n % 100 <= 13) return "th";
	return n % 10 == 1 ? "st" : n % 10 == 2 ? "nd" : n % 10 == 3 ? "rd" : "th";
}

const char *dive_place_words(unsigned tenths, unsigned place, unsigned count, unsigned best, const char *best_name) {
	static char s[64];
	if (place == 1) snprintf(s, sizeof s, "%u.%u sec! Fastest of %u this week!", tenths / 10, tenths % 10, count);
	else snprintf(s, sizeof s, "%u.%u sec,%u%s of %u. %s:%u.%u", tenths / 10, tenths % 10, place, suffix(place), count,
		best_name, best / 10, best % 10);
	return s;
}

const char *dive_mark_words(const char *name, const char *cause, bool won) {
	static char s[160];
	if (won) snprintf(s, sizeof s, "Lan,look! %s made it all the way down!|Wow...", name);
	else if (*cause) snprintf(s, sizeof s, "Lan... %s got deleted here.|By a %s...|We'd better be careful.", name, cause);
	else snprintf(s, sizeof s, "Lan... %s got deleted here.|We'd better be careful.", name);
	return s;
}

const char *dive_phrase_words(const char *name, const char *phrase, unsigned waves) {
	static char s[160];
	int n = snprintf(s, sizeof s, "%s left a message here.|\"%s\"", name, phrase);
	if (waves == 1) n += snprintf(s + n, sizeof s - (size_t)n, "|One Navi waved back.");
	else if (waves > 1) n += snprintf(s + n, sizeof s - (size_t)n, "|%u Navis waved back.", waves);
	snprintf(s + n, sizeof s - (size_t)n, "|*wave*");
	return s;
}
