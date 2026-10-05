/* MegaMan's words on the older net's wait (guest_wait.c, docs/VOICE.md). */
#include "guest_wait_lines.h"

/* MegaMan's words, pages of up to three lines as wide as BN6's own (the
 * widest 167 of the box's 183 pixels in its chat font), each after the
 * last has been read a while: what happens, that it happens once, and,
 * once the boot is that far, that it is almost done */
const char *const guest_wait_lines[3][3] = {
	{ "Hold on,Lan!", "The older Net's", "still starting up..." },
	{ "It only starts up", "this once,Lan.", "Then it stays ready!" },
	{ "Almost there,Lan...", "Just a moment more!", "" },
};
