#include "online_text.h"

#include <stdio.h>
#include <string.h>

const char online_title[] = "Net Square";
const char online_field_name_title[] = "YOUR NAME IN THE SQUARE";
const char online_field_address_title[] = "SERVER ADDRESS";
const char online_field_name_hint[] = "1-16 letters, digits, spaces, _ or -";
const char online_field_address_hint[] = "host or host:port, as its owner gave it";
const char online_field_bad_name[] = "Use 1-16 letters, digits, spaces, _ or -";
const char online_field_bad_address[] = "That is not a server address";
const char online_field_space[] = "SP";
const char *const online_field_buttons[3] = { "ERASE", "OK", "BACK" };
const char online_field_keys_hint[] = "A: type   L: erase   START: OK   B: back";
const char online_waiting[] = "Waiting for another Navi...";
const char online_select_help[] = "A: Choose   B: Back";
const char online_card_help[] = "A: Mute   R: Block   B: Back";
const char online_card_wait[] = "Reading Navi card...";
const char online_people_failed[] = "Could not save block/mute.";
const char online_people_title[] = "Navis in this channel";
const char online_phrases_title[] = "Quick phrases";
const char online_emotes_title[] = "Emotes";
const char online_channel_title[] = "Choose a channel";
const char online_channel_help[] = "LEFT/RIGHT: Channel   A: Join";
const char online_muted[] = "Muted";
const char online_blocked[] = "Blocked";
const char online_saved[] = "Saved on this device";

const char *const online_menu[5] = { "Navis", "Quick phrases", "Emotes", "Channel", "Leave Square" };
const char *const online_phrases[ONLINE_PHRASE_COUNT] = {
	"Hello!", "Nice build!", "Good luck!", "Thanks!", "Let's explore!", "Great job!", "One moment!", "See you!",
};
const char *const online_emotes[ONLINE_EMOTE_COUNT] = { "Wave", "Cheer", "Laugh", "Surprised", "Thinking", "Bow" };
const char *const online_emote_glyphs[ONLINE_EMOTE_COUNT] = { "o/", "* * *", "^_^", "!", "...", "(_ _)" };

void online_card_stats(char lines[5][48], unsigned hp, unsigned maximum, unsigned depth, unsigned act, unsigned best) {
	snprintf(lines[0], 48, "HP %u / %u", hp, maximum);
	snprintf(lines[1], 48, "Layer %u / Act %u", depth, act);
	snprintf(lines[2], 48, "Best layer: %u", best);
	snprintf(lines[3], 48, "%s", "Souls");
	snprintf(lines[4], 48, "%s", "Crosses");
}

void online_card_powers(char *out, size_t size, uint16_t mask, int souls) {
	static const char *const soul_names[] = { "Colonel", "Shadow", "Number", "Tomahawk", "Knight", "Toad" };
	static const char *const cross_names[] = { "Heat", "Elec", "Slash", "Erase", "Charge" };
	const char *const *names = souls ? soul_names : cross_names;
	if (!size) return;
	out[0] = '\0';
	for (unsigned i = 0; i < (souls ? 6u : 5u); ++i) {
		if (!(mask & (1u << i))) continue;
		size_t used = strlen(out);
		if (used < size) snprintf(out + used, size - used, "%s%s", used ? " " : "", names[i]);
	}
	if (!out[0]) snprintf(out, size, "%s", "None");
}

void online_card_buster(char *out, size_t size, unsigned attack, unsigned speed, unsigned charge) {
	snprintf(out, size, "ATK %u  SPD %u  CHG %u", attack, speed, charge);
}

void online_card_marks(char *out, size_t size, unsigned marks) { snprintf(out, size, "Marks: %u", marks); }
