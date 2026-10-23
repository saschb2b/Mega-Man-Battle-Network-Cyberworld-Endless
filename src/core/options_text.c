/* options_text.h. Menu words, short and plain: what each line opens or
 * sets, and the sound's two choices as docs/GUIDE.md's Sound says them. */
#include "options_text.h"

#include <stdio.h>

/* (in OptionsWord's order, its name beside each) */
static const char *const words[] = {
	"Options",   /* OW_TITLE */
	"Controls",   /* OW_CONTROLS */
	"Which buttons and keys are A, B, L, R, START and SELECT",   /* OW_CONTROLS_NOTE */
	"Audio",   /* OW_AUDIO */
	"How the game sounds",   /* OW_AUDIO_NOTE */
	"Done",   /* OW_DONE */
	"Close Options",   /* OW_DONE_NOTE */
	"Back to Options",   /* OW_BACK_NOTE */
	"Sound",   /* OW_SOUND */
	"GBA",   /* OW_GBA */
	"As a GBA plays it: BN6's own mix, crisp and bright",   /* OW_GBA_NOTE */
	"Soft",   /* OW_SOFT */
	"The Legacy Collection's balance: softer highs, less deep bass",   /* OW_SOFT_NOTE */
	"Tap a line to choose it",   /* OW_TAP */
};
_Static_assert(sizeof words / sizeof *words == OW_COUNT, "a word for each OptionsWord");

const char *options_word(OptionsWord w) { return (unsigned)w < OW_COUNT ? words[w] : ""; }

void options_keys(const char *a, const char *b, bool change, char *out, size_t n) {
	snprintf(out, n, "%s: %s  %s: back", a, change ? "change" : "choose", b);
}

const char *options_sound_note(void) {
	return "\n# sound: gba plays the game's sound as a GBA makes it, a bright hiss above\n"
		"# the music and all; soft gives it the Legacy Collection's balance, as\n"
		"# measured from it: the highs and the deepest bass taken down. Options on\n"
		"# the title (SELECT) changes it too.\n";
}
