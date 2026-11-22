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

const char *square_week_post(WeekOffer offer, uint16_t week, int posted, bool first) {
	static char s[240];
	char when[32];
	dates(when, sizeof when, week, true);
	int n = 0;
	if (posted) n += snprintf(s + n, sizeof s - (size_t)n, "YOUR DIVE TO\nLAYER %d IS UP ON\nTHE BOARD!|", posted);
	switch (offer) {
	case WEEK_OFFER_NEW:
		/* (the week's first dive alone: the dates and the one thing a new diver needs, said once; the question says the
		 * rest, issue #779) */
		if (first) snprintf(s + n, sizeof s - (size_t)n, "%s.\nEVERY NAVI DIVES\nTHE SAME NET!", when);
		else if (n) s[n - 1] = 0;
		else s[0] = 0;
		break;
	case WEEK_OFFER_AGAIN:
		/* (the options say it: Keep diving, Start over, Not now; issue #779) */
		if (n) s[n - 1] = 0;
		else s[0] = 0;
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

const char *square_week_question(bool known, unsigned diving) {
	static char s[64];
	/* (two lines, BN6's Yes and No on the third; sixteen letters a line at most, as the portal's others) */
	if (!known) snprintf(s, sizeof s, "DIVE INTO THIS\nWEEK'S DEEP NET?\n");
	else if (!diving) snprintf(s, sizeof s, "NO NAVI DIVING\nYET. DIVE IN?\n");
	else if (diving == 1) snprintf(s, sizeof s, "1 NAVI DIVING!\nDIVE IN?\n");
	else snprintf(s, sizeof s, "%u NAVIS DIVING!\nDIVE IN?\n", diving);
	return s;
}
const char *square_week_closed(void) { return "THE PORTAL'S\nSHUT. NO DEEP NET\nTHIS WEEK."; }

static int line(char *s, size_t n, int place, const WeekEntry *e) {
	return snprintf(s, n, "%d.%s:%u%s%s%s", place, e->name, e->depth, e->won ? "!" : "", e->party ? "+" : "", e->pulled ? "*" : "");
}

/* who dives now: a box of the count, then the divers three to a box */
static int pack(char *s, size_t n, const DiveDivers *d) {
	if (!d || !d->total) return 0;
	int k = snprintf(s, n, "|>Diving now:%u", d->total);
	for (int i = 0; i < d->count && (size_t)k < n; ++i) {
		k += snprintf(s + k, n - (size_t)k, i % 3 ? "\n" : "|");
		k += snprintf(s + k, n - (size_t)k, "%s:layer %u%s", d->e[i].name, d->e[i].depth, d->e[i].flags & ONLINE_DIVERS_FIGHTING ? "!" : "");
	}
	return k;
}

/* one split record: the guardian's layer, the party's size (1 alone), the time and who */
static int record(char *s, size_t n, int act, int size, const WeekSplit *b) {
	int layer = act < 3 ? 3 * act + 3 : 10;
	if (size == 1) return snprintf(s, n, "L%d solo:%u.%us %s", layer, b->tenths / 10u, b->tenths % 10u, b->name);
	return snprintf(s, n, "L%d x%d:%u.%us %s", layer, size, b->tenths / 10u, b->tenths % 10u, b->name);
}

/* the week's fastest bust of each guardian (layers 3, 6, 9 and the tenth's),
 * alone and by party size, three to a box */
static int records(char *s, size_t n, const WeekSplits *w) {
	if (!w) return 0;
	int k = 0, shown = 0;
	for (int act = 0; act < 4; ++act)
		for (int size = 0; size < ONLINE_PARTY_MOST && (size_t)k < n; ++size) {
			const WeekSplit *b = &w->best[act][size];
			if (!b->tenths) continue;
			if (!shown) k += snprintf(s + k, n - (size_t)k, "|>Fastest guardian\nbusts this week");
			k += snprintf(s + k, n - (size_t)k, shown++ % 3 ? "\n" : "|");
			k += record(s + k, n - (size_t)k, act, size + 1, b);
		}
	return k;
}

/* the Board's boxes: a heading, then the dives three to a box */
const char *square_week_board(const WeekBoard *b, uint16_t week, const DiveDivers *d, const WeekSplits *w) {
	static char s[1200];
	char when[32];
	dates(when, sizeof when, week, false);
	int n = snprintf(s, sizeof s, ">DEEP NET,%s\nDeepest layers,as\nNavis report them.", when);
	if (!b->ntop) n += snprintf(s + n, sizeof s - (size_t)n, "|>No dives yet.\nBe the first!");
	for (int k = 0; k < b->ntop; ++k) {
		n += snprintf(s + n, sizeof s - (size_t)n, k % 3 ? "\n" : "|");
		n += line(s + n, sizeof s - (size_t)n, k + 1, &b->top[k]);
	}
	for (int k = 0; k < b->ntop; ++k)
		if (b->top[k].party) { n += snprintf(s + n, sizeof s - (size_t)n, "|>+:a guardian beaten\nwith a party."); break; }
	for (int k = 0; k < b->ntop; ++k)
		if (b->top[k].pulled) { n += snprintf(s + n, sizeof s - (size_t)n, "|>*:pulled back once\nby the crew."); break; }
	if (b->place) n += snprintf(s + n, sizeof s - (size_t)n, "|>Your best is\nlayer %u,#%d!", b->mine.depth, b->place);
	n += pack(s + n, sizeof s - (size_t)n, d);
	n += records(s + n, sizeof s - (size_t)n, w);
	if (b->nlast) {
		n += snprintf(s + n, sizeof s - (size_t)n, "|>Last week's best");
		for (int k = 0; k < b->nlast; ++k) {
			n += snprintf(s + n, sizeof s - (size_t)n, "\n");
			n += line(s + n, sizeof s - (size_t)n, k + 1, &b->last[k]);
		}
	}
	return s;
}
