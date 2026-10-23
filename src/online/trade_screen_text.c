#include "trade_screen_text.h"

#include <stdio.h>

#include "trade_words.h"

const char *trade_text_list_title(int purpose) {
	static const char *const titles[4] = { "Offer a chip", "Offer one back", "Answer an offer", "Post an offer" };
	return purpose >= 0 && purpose < 4 ? titles[purpose] : titles[0];
}

const char *trade_text_list_for(int purpose, const char *partner) {
	static char s[48];
	if (purpose == TRADE_FOR_POST || !partner || !*partner) return "Trade board";
	snprintf(s, sizeof s, "To %s", partner);
	return s;
}

const char trade_text_spares[] = "SPARES";
const char trade_text_sort[] = "SORT";
const char *const trade_text_sorts[TRADE_SORT_KEYS] = { "ID", "ABCDE", "Code" };   /* (as the folder editor's SORT; the list's head in capitals) */

const char *trade_text_list_note(int purpose, int want, const char *partner, const char *theirs) {
	static char s[96];
	if (purpose == TRADE_FOR_ANSWER && theirs && *theirs) snprintf(s, sizeof s, "%s offers %s", partner, theirs);
	else if (want >= 0) snprintf(s, sizeof s, "Spares that fit: %s", trade_want(want));
	else s[0] = 0;
	return s;
}


const char trade_text_deal_title[] = "Chip trade";

const char *trade_text_deal_with(const char *partner) {
	static char s[48];
	snprintf(s, sizeof s, "With %s", partner);
	return s;
}

const char trade_text_navigator[] = "NAVIGATOR";

/* (MegaMan, in E-Mail's NAVIGATOR box: docs/VOICE.md) */
const char *trade_text_ask(const char *mine, const char *partner, const char *theirs) {
	static char s[96];
	snprintf(s, sizeof s, "Lan,trade our %s for %s's %s?", mine, partner, theirs);
	return s;
}

const char trade_text_yes[] = "Yes";
const char trade_text_no[] = "No";

const char *trade_text_waiting(const char *partner) {
	static char s[48];
	snprintf(s, sizeof s, "Hmm... %s's still thinking.", partner);
	return s;
}

const char trade_text_swapping[] = "Here it comes!";

const char *trade_text_traded(const char *theirs) {
	static char s[64];
	snprintf(s, sizeof s, "Yeah! We got %s!", theirs);
	return s;
}
