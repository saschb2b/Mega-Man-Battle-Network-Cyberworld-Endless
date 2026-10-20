#include "square_week_words.h"

#include <stdio.h>
#include <string.h>

const char *const square_week_again_options[WEEK_AGAIN_OPTIONS] = { "Keep diving", "Start over", "Not now" };

/* a day's month (0-11) and day of the month from days since 1970 (the
 * proleptic Gregorian calendar's, Howard Hinnant's civil_from_days) */
static void civil(int64_t days, int *month, int *day) {
	int64_t z = days + 719468, era = (z >= 0 ? z : z - 146096) / 146097;
	int64_t doe = z - era * 146097, yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100), mp = (5 * doy + 2) / 153;
	*day = (int)(doy - (153 * mp + 2) / 5 + 1);
	*month = (int)(mp < 10 ? mp + 2 : mp - 10);
}

/* the week's Monday to Sunday, "OCT 19-OCT 25" (UTC: the server's) */
static void dates(char *out, size_t n, uint16_t week, bool caps) {
	static const char *const months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	int64_t start = online_week_start(week) / (24 * 3600);
	char part[2][12];
	for (int k = 0; k < 2; ++k) {
		int month, day;
		civil(start + k * 6, &month, &day);
		snprintf(part[k], sizeof part[k], "%s %d", months[month], day);
	}
	snprintf(out, n, "%s-%s", part[0], part[1]);
	if (caps) for (char *c = out; *c; ++c) if (*c >= 'a' && *c <= 'z') *c = (char)(*c - 'a' + 'A');
}

const char *square_week_post(WeekOffer offer, uint16_t week, int posted) {
	static char s[240];
	char when[32];
	dates(when, sizeof when, week, true);
	int n = 0;
	if (posted) n += snprintf(s + n, sizeof s - (size_t)n, "YOUR DIVE TO\nLAYER %d IS UP ON\nTHE BOARD!|", posted);
	switch (offer) {
	case WEEK_OFFER_NEW:
		snprintf(s + n, sizeof s - (size_t)n, "THE NET SHIFTED!\nTHIS WEEK'S DEEP\nNET IS OPEN!|%s.\nEVERY NAVI DIVES\nTHE SAME NET!", when);
		break;
	case WEEK_OFFER_AGAIN:
		snprintf(s + n, sizeof s - (size_t)n, "YOUR DIVE INTO\nTHE DEEP NET\nISN'T OVER!");
		break;
	case WEEK_OFFER_ON:
		snprintf(s + n, sizeof s - (size_t)n, "YOU'RE ON THIS\nWEEK'S DIVE!\nGO DEEP!");
		break;
	case WEEK_OFFER_TRADE:
		snprintf(s + n, sizeof s - (size_t)n, "THE DEEP NET'S\nOPEN! TAKE YOUR\nTRADE HOME FIRST!");
		break;
	case WEEK_OFFER_STAMP:
		snprintf(s + n, sizeof s - (size_t)n, "THIS WEEK'S DEEP\nNET IS OPEN!|BUT YOUR NETPASS\nHAS NO DEEPNET\nSTAMP! SORRY!");
		break;
	default:
		if (n) s[n - 1] = 0; else s[0] = 0;
		break;
	}
	return s;
}

const char *square_week_question(WeekOffer offer) { return offer == WEEK_OFFER_NEW ? "DIVE IN NOW?\n" : NULL; }

const char *square_week_off(void) { return "OFF YOU GO!\nHEAD HOME AND\nJACK IN!"; }

static int line(char *s, size_t n, int place, const WeekEntry *e) {
	return snprintf(s, n, "%d.%s:%u%s", place, e->name, e->depth, e->won ? "!" : "");
}

/* the Board's boxes: a heading, then the dives three to a box */
const char *square_week_board(const WeekBoard *b, uint16_t week) {
	static char s[720];
	char when[32];
	dates(when, sizeof when, week, false);
	int n = snprintf(s, sizeof s, ">DEEP NET,%s\nDeepest layers,as\nNavis report them.", when);
	if (!b->ntop) n += snprintf(s + n, sizeof s - (size_t)n, "|>No dives yet.\nBe the first!");
	for (int k = 0; k < b->ntop; ++k) {
		n += snprintf(s + n, sizeof s - (size_t)n, k % 3 ? "\n" : "|");
		n += line(s + n, sizeof s - (size_t)n, k + 1, &b->top[k]);
	}
	if (b->place) n += snprintf(s + n, sizeof s - (size_t)n, "|>Your best is\nlayer %u,#%d!", b->mine.depth, b->place);
	if (b->nlast) {
		n += snprintf(s + n, sizeof s - (size_t)n, "|>Last week's best");
		for (int k = 0; k < b->nlast; ++k) {
			n += snprintf(s + n, sizeof s - (size_t)n, "\n");
			n += line(s + n, sizeof s - (size_t)n, k + 1, &b->last[k]);
		}
	}
	return s;
}
