#include "online_text.h"

const char *const online_phrases[ONLINE_PHRASE_COUNT] = {
	"Hello!", "Nice build!", "Good luck!", "Thanks!", "Let's explore!", "Great job!", "One moment!", "See you!",
};
const char *const online_emotes[ONLINE_EMOTE_COUNT] = { "Wave", "Cheer", "Laugh", "Surprised", "Thinking", "Bow" };
const char *const online_emote_glyphs[ONLINE_EMOTE_COUNT] = { "o/", "* * *", "^_^", "!", "...", "(_ _)" };

const char online_field_name_title[] = "YOUR NAME IN THE SQUARE";
const char online_field_address_title[] = "SERVER ADDRESS";
const char online_field_name_hint[] = "1-16 letters, digits, spaces, _ or -";
const char online_field_address_hint[] = "host or host:port, as its owner gave it";
const char online_field_bad_name[] = "Use 1-16 letters, digits, spaces, _ or -";
const char online_field_bad_address[] = "That is not a server address";
const char online_field_space[] = "SP";
const char *const online_field_buttons[3] = { "ERASE", "OK", "BACK" };
const char online_field_keys_hint[] = "A: type   L: erase   START: OK   B: back";
