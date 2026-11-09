/* What is said on the way by Oran Isle (docs/world/bn5-town.md; docs/voice/rules.md):
 * MegaMan at the portal, Lan and MegaMan at the dock, and the island's
 * three: the word of the guardian (hearsay, never his moves), the stall,
 * the job. */
#include "oran.h"

#include <stdio.h>

const char *oran_word_words(const char *navi, const char *area, const char *rumor) {
	static char s[240];
	snprintf(s, sizeof s, "Diving in from here?|Folks say %s guards %s.|Word is,%s", navi, area, rumor);
	return s;
}

const char *oran_stall_lead(void) { return "Chips from the older Net!|One per customer!"; }

const char *oran_stall_ask(const char *chip, char code, int price) {
	static char s[64];
	snprintf(s, sizeof s, "%s %c for %d zenny?\n", chip, code, price);
	return s;
}

const char *oran_stall_broke(void) { return "Not enough zenny? Aww..."; }
const char *oran_stall_sold(void) { return "Sold out for you!|Good luck down there!"; }
const char *oran_stall_leave(void) { return "Suit yourself!"; }

const char *oran_job_offer(int battles) {
	static char s[160];
	snprintf(s, sizeof s, "Hey,you're diving? Got a job.|Win %d battles without a scratch.|Your PET's courier gets the pay.", battles);
	return s;
}

const char *oran_job_question(void) { return "Take the job?\n"; }
const char *oran_job_taken(void) { return "Thanks! Pay's waiting after the guardian."; }
const char *oran_job_held(void) { return "You've got a job going?|One at a time,then!"; }
const char *oran_job_no(void) { return "No worries. Ask anytime!"; }

const char *oran_portal_words(void) { return "Lan,the older Net runs from Oran Isle too!|We could go in from there."; }
const char *oran_portal_question(void) { return "Go by Oran Isle?\n"; }
const char *oran_arrival_words(void) { return "@L Oran Isle! It's been a while!|@M The port's by the signal post,Lan!"; }
