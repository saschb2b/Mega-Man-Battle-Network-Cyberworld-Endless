/* reconnect_text.h. The PET's own text, in its plain style (docs/VOICE.md:
 * menus and labels keep theirs), as the NetPass's and the trade screens'. */
#include "reconnect_text.h"

#include <stdio.h>

const char reconnect_text_title[] = "Net Square";
const char reconnect_text_back[] = "Back online!";
const char reconnect_text_kept[] = "YOUR SPOT IN THE SQUARE IS KEPT";
const char reconnect_text_key_home[] = "GO HOME";

const char *reconnect_text_trying(int dots) {
	static const char *const words[4] = { "Reconnecting", "Reconnecting.", "Reconnecting..", "Reconnecting..." };
	return words[dots < 0 ? 0 : dots > 3 ? 3 : dots];
}

const char *reconnect_text_left(unsigned seconds) {
	static char s[24];
	snprintf(s, sizeof s, "%uS", seconds);
	return s;
}
