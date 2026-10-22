#include "emote_strip_text.h"

#include <stdio.h>

const char *const strip_text_pages[2] = { "EMOTES", "PHRASES" };
const char strip_text_netpass[] = "NETPASS";
const char strip_text_keys[] = "A SEND  B CLOSE";
const char strip_text_favourites[] = "HOLD L +";
const char strip_text_keep[] = "SELECT+ARROW KEEPS";
const char strip_text_tap[] = "TAP TO SEND";
const char strip_text_on_l[] = "HOLD L + THAT WAY";

const char *strip_text_aim(const char *name) {
	static char s[32];
	if (name && *name) snprintf(s, sizeof s, "AT %.16s", name);
	else snprintf(s, sizeof s, "TO EVERYONE");
	return s;
}

const char *strip_text_kept(int way) {
	static const char *const ways[4] = { "UP", "RIGHT", "DOWN", "LEFT" };
	static char s[24];
	snprintf(s, sizeof s, "ON L+%s", ways[way & 3]);
	return s;
}
