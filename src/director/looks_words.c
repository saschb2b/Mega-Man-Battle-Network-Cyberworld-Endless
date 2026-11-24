/* looks_words.h. MegaMan's word on each palette names where it came from,
 * one beat a box, then BN6's own "MegaMan got:" box; Dad's mail says
 * where the colours are changed. */
#include "looks_words.h"

#include <stdio.h>

#include "looks.h"
#include "looks_got_words.h"

/* (his word where it came from: a reaction and the place, six words or so) */
static const char *const came[LOOK_PALETTES] = {
	[LOOK_UNDERNET] = "Lan,the Undernet... I kept its colors!",
	[LOOK_GREGAR] = "We beat the beast,Lan! I kept its colors!",
	[LOOK_PROTOMAN] = "ProtoMan's colors! Chaud won't like this! Hehe.",
	[LOOK_NEBULA] = "A Darkloid's colors... Spooky,huh,Lan?",
	[LOOK_TOURNAMENT] = "Champion colors,Lan! We won it all!",
	[LOOK_HAZARD] = "We made it through the worst,Lan! Look!",
	[LOOK_CENTRAL] = "Colors from home! Thanks,Lan!",
	[LOOK_SEASIDE] = "Colors from home! Thanks,Lan!",
	[LOOK_SKY] = "Colors from home! Thanks,Lan!",
	[LOOK_MYSTERY] = "Lan,look! The Mystery colors! For me!",
	[LOOK_GLITCH] = "Whoa! The Glitch colors! They flicker!",
};

const char *looks_got_words(unsigned palettes, unsigned found) {
	static char words[1024];
	int k = 0, n = 0;
	words[0] = 0;
	for (int p = LOOK_CLASSIC + 1; p < LOOK_PALETTES; ++p)
		n += (int)(palettes >> p & 1);
	/* (several at once, a profile's from before palettes: one word, a box each) */
	if (n > 2) k += snprintf(words + k, sizeof words - (size_t)k, "@M Lan! I've got new colors!");
	for (int p = LOOK_CLASSIC + 1; p < LOOK_PALETTES && k < (int)sizeof words - 80; ++p) {
		if (!(palettes >> p & 1)) continue;
		if (n <= 2 && came[p]) k += snprintf(words + k, sizeof words - (size_t)k, "%s@M %s", k ? "|" : "", came[p]);
		if (!(found >> p & 1)) k += snprintf(words + k, sizeof words - (size_t)k, "%s@N %s", k ? "|" : "", looks_got_box(p));
	}
	return words;
}

const char *looks_mail_subject(void) { return "Colors"; }

const char *looks_mail_text(void) {
	return "Lan,MegaMan showed\nme his new colors!\f"
		"He can change them\nin the PET,under\nCOLOURS.\f"
		"AsterLand sells some\ncolors too. Take a\nlook,OK?";
}
