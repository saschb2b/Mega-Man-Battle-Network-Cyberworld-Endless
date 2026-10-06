/* analytics_text.h. Menu words, short and plain: what is sent, what for,
 * and what never is, as README.md's Anonymous statistics says it. */
#include "analytics_text.h"

#include <stdio.h>

/* (in AnalyticsWord's order, its name beside each) */
static const char *const words[] = {
	"STATISTICS",   /* AW_TITLE */
	"Send anonymous statistics?",   /* AW_QUESTION */
	"They help a free game: the systems it runs on, the setups runs take, how far they get, which guardians win.",   /* AW_WHAT */
	"No names, no IDs, nothing about you or this device.",   /* AW_NOT */
	"Yes",   /* AW_YES */
	"No",   /* AW_NO */
	"Statistics",   /* AW_ROW */
	"On",   /* AW_ON */
	"Off",   /* AW_OFF */
	"A: what is sent, and Yes or No",   /* AW_ROW_NOTE */
	"Tap Yes or No",   /* AW_TAP */
};
_Static_assert(sizeof words / sizeof *words == AW_COUNT, "a word for each AnalyticsWord");

const char *analytics_word(AnalyticsWord w) { return (unsigned)w < AW_COUNT ? words[w] : ""; }

void analytics_later(const char *select, char *out, size_t n) { snprintf(out, n, "Change it any time: %s on the title", select); }

void analytics_keys(const char *a, const char *b, bool first, char *out, size_t n) {
	snprintf(out, n, "%s: choose   %s: %s", a, b, first ? "no" : "back");
}

const char *analytics_setting_note(void) {
	return "\n# statistics: on sends anonymous play statistics to the game's own Umami\n"
		"# (umami.saschb2b.com): the system and version, the setups runs take, how\n"
		"# far they get and which guardians win. No names, no IDs. off sends nothing.\n"
		"# The game asks at its first start (README.md, Anonymous statistics).\n";
}
