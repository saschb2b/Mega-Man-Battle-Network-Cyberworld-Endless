#include "square_words.h"

#include <stdio.h>
#include <string.h>

const char *const square_menu_options[2] = { "Say something", "Do an emote" };
const char *const square_keeper_options[2] = { "Next channel", "Last channel" };

const char *square_menu_question(void) { return "What'll we do,Lan?"; }

const char *square_more(void) { return "More..."; }

const char *square_back(void) { return "Back"; }

const char *square_home_question(void) { return "Head home,Lan?\n"; }

/* the names in `mask` (bit k name k), joined as MegaMan says a list */
static void names(char *out, size_t n, unsigned mask, const char *const *all, int count) {
	int total = 0, said = 0;
	for (int k = 0; k < count; ++k) total += mask >> k & 1;
	out[0] = 0;
	for (int k = 0; k < count; ++k) {
		if (!(mask >> k & 1)) continue;
		size_t used = strlen(out);
		snprintf(out + used, n - used, "%s%s", said ? (said + 1 == total ? " and " : ",") : "", all[k]);
		++said;
	}
}

const char *square_peer_words(const char *name, const OnlineProfile *card, bool pass_full) {
	static const char *const crosses[] = { "Heat", "Elec", "Slash", "Erase", "Charge" };
	static const char *const souls[] = { "Colonel", "Shadow", "Number", "Tomahawk", "Knight", "Toad" };
	static char s[400];
	int n = snprintf(s, sizeof s, "That's %s's\nMegaMan!", name);
	if (!card || !card->max_hp) return s;
	n += snprintf(s + n, sizeof s - (size_t)n, "|HP %u of %u.\nAct %u,layer %u!", card->hp, card->max_hp, card->act, card->depth);
	char list[120];
	if (card->crosses) {
		names(list, sizeof list, card->crosses, crosses, 5);
		n += snprintf(s + n, sizeof s - (size_t)n, "|Whoa! He has %s\nCross!", list);
	}
	if (card->souls) {
		names(list, sizeof list, card->souls, souls, 6);
		n += snprintf(s + n, sizeof s - (size_t)n, "|And %sSoul!", list);
	}
	/* (why Battle and trades are not offered with him: his NetPass, in stamps) */
	if (!pass_full) snprintf(s + n, sizeof s - (size_t)n, "|His NetPass has\njust the Square\nstamp.");
	return s;
}

const char *square_peer_option(int k, bool muted) {
	static const char *const options[SQUARE_PEER_OPTIONS] = { "Wave", "Mute", "Block" };
	return k == 1 && muted ? "Unmute" : options[k];
}

const char *square_blocked_words(const char *name) {
	static char s[80];
	snprintf(s, sizeof s, "OK. We won't see\n%s again.", name);
	return s;
}

const char *square_keeper_words(unsigned channel, unsigned channels, unsigned navis) {
	static char s[160];
	if (navis == 1) snprintf(s, sizeof s, "WELCOME TO THE\nNET SQUARE!|THIS IS CHANNEL %u\nOF %u. ONE NAVI HERE!", channel, channels);
	else snprintf(s, sizeof s, "WELCOME TO THE\nNET SQUARE!|THIS IS CHANNEL %u\nOF %u. %u NAVIS HERE!", channel, channels, navis);
	return s;
}

const char *square_keeper_moving(void) { return "OFF YOU GO!"; }
