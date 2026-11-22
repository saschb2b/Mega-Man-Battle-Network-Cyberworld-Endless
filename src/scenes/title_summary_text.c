/* title_summary_text.h. Menu words, short and plain: the screens' names
 * and the rows, as BN6's RESULT names DeleteTime and Busting LV. */
#include "title_summary_text.h"

#include <stdio.h>

/* (in SummaryText's order, its name beside each) */
static const char *const words[] = {
	"RESULT",   /* SM_RESULT */
	"RECORDS",   /* SM_RECORDS */
	"NAVIGATOR",   /* SM_NAVIGATOR */
	"INFORMATION",   /* SM_INFORMATION */
	"MegaMan was deleted",   /* SM_DELETED */
	"The Nest has fallen!",   /* SM_FALLEN */
	"Reached",   /* SM_REACHED */
	"Viruses deleted",   /* SM_VIRUSES */
	"Navis deleted",   /* SM_NAVIS */
	"Library",   /* SM_LIBRARY */
	"Best",   /* SM_BEST */
	"This week",   /* SM_WEEK */
	"NEW",   /* SM_NEW */
	"DEEP NET",   /* SM_DIVE */
	"The dive is over",   /* SM_DIVE_OVER */
	"Crew battles",   /* SM_CREW */
	"Board this week",   /* SM_PLACE */
};
const char *summary_text(SummaryText w) {
	_Static_assert(sizeof words / sizeof *words == SM_COUNT, "a word for each SummaryText");
	return (unsigned)w < SM_COUNT ? words[w] : "";
}

const char *summary_crew(char *buf, size_t n, int fought, int won) {
	if (!fought) snprintf(buf, n, "None");
	else snprintf(buf, n, "%d of %d won", won, fought);
	return buf;
}

/* an English place's ending: 1st, 2nd, 3rd, 4th, 11th, 21st (the PET's font has no "#") */
static const char *ordinal(int n) {
	if (n % 100 >= 11 && n % 100 <= 13) return "th";
	return n % 10 == 1 ? "st" : n % 10 == 2 ? "nd" : n % 10 == 3 ? "rd" : "th";
}

const char *summary_place(char *buf, size_t n, int place) {
	if (place > 0) snprintf(buf, n, "%d%s", place, ordinal(place));
	else snprintf(buf, n, "%s", place ? "No link" : "Asking...");
	return buf;
}

const char *summary_layer(char *buf, size_t n, int layer) {
	snprintf(buf, n, "Layer %d", layer);
	return buf;
}

const char *summary_library(char *buf, size_t n, int count, int added) {
	if (added) snprintf(buf, n, "%d (+%d)", count, added);
	else snprintf(buf, n, "%d", count);
	return buf;
}

const char *summary_band(char *buf, size_t n, const char *helpers, const char *off) {
	if (helpers && off) snprintf(buf, n, "HELP: %s  OFF: %s", helpers, off);
	else if (helpers) snprintf(buf, n, "HELP: %s", helpers);
	else snprintf(buf, n, "OFF: %s", off ? off : "");
	return buf;
}

const char *summary_band_count(char *buf, size_t n, int on, int off) {
	if (on && off) snprintf(buf, n, "HELP: %d ON  OFF: %d", on, off);
	else if (on) snprintf(buf, n, "HELP: %d ON", on);
	else snprintf(buf, n, "OFF: %d", off);
	return buf;
}
