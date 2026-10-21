#include "gate_words.h"

#include <stdbool.h>
#include <stdio.h>

#include "text.h"

const char *const gate_options[GATE_OPTIONS] = { "Official Square", "Another server", "My name" };

const char *gate_keeper_hello(bool official) {
	/* (where the gate leads as a place, never its address) */
	if (official) return "HELLO! I KEEP THE\nNET SQUARE GATE!|IT LEADS TO THE\nOFFICIAL SQUARE!";
	return "HELLO! I KEEP THE\nNET SQUARE GATE!|IT LEADS TO ANOTHER\nSERVER'S SQUARE!";
}

const char *gate_keeper_ask(void) { return "WHAT CAN I DO?"; }

const char *gate_keeper_official_ask(void) { return "HMM? THE OFFICIAL\nSQUARE INSTEAD?"; }

const char *gate_keeper_official_question(void) { return "SWITCH IT?\n"; }

const char *gate_keeper_official_already(void) { return "IT LEADS THERE\nALREADY!"; }

const char *gate_keeper_official(void) { return "DONE! IT LEADS TO\nTHE OFFICIAL SQUARE!"; }

const char *gate_keeper_bye(void) { return "COME BACK ANYTIME!"; }

const char *gate_keeper_set(void) { return "DONE! IT LEADS TO\nTHAT SERVER NOW!"; }

const char *gate_keeper_named(const char *name) {
	static char s[80];
	snprintf(s, sizeof s, "GOT IT! HELLO,\n%s!", name);
	return s;
}

const char *gate_keeper_name_first(void) { return "WAIT,WAIT! FIRST,\nYOUR NAME,PLEASE!"; }

const char *gate_keeper_privacy(void) {
	return "NAVIS FROM OTHER\nPETS MEET THERE!|"
		"ITS SERVER SEES YOUR\nADDRESS AND NAME,|"
		"AND MEGAMAN,TOO!";
}

const char *gate_keeper_privacy_question(void) { return "SO! GO ONLINE?\n"; }

const char *gate_question(void) { return "To the Net Square,\nLan?\n"; }

const char *gate_save_failed(void) { return "Let's stay home for\nnow,Lan."; }

/* the host's maintenance line, in capitals, after the keeper's word: the
 * sign on the gate, its words wrapped to the box's lines (two a box) */
#define BOX_LINE 19
static const char *maintenance(const char *note) {
	static char s[200];
	char line[48];
	capitals(line, sizeof line, note);
	if (!line[0]) return "SORRY! THE SQUARE'S\nUNDER MAINTENANCE!|COME BACK LATER!";
	int n = snprintf(s, sizeof s, "SORRY! THE SQUARE'S\nUNDER MAINTENANCE!|THE SIGN ON IT SAYS,");
	int lines = 1, used = 0;
	for (const char *w = line; *w && n < (int)sizeof s - BOX_LINE - 4;) {
		int len = 0;
		while (w[len] && w[len] != ' ') ++len;
		if (len > BOX_LINE) len = BOX_LINE;
		bool fits = used && used + 1 + len <= BOX_LINE;
		if (fits) s[n++] = ' ';
		else { s[n++] = lines % 2 ? '\n' : '|'; ++lines; used = 0; }
		n += snprintf(s + n, sizeof s - (size_t)n, "%.*s", len, w);
		used += len + (fits ? 1 : 0);
		w += len;
		while (*w == ' ') ++w;
	}
	return s;
}

const char *gate_end_words(VisitEnd why, int *face) {
	*face = FACE_PROG;
	switch (why) {
	case VISIT_END_LEFT: *face = FACE_MEGAMAN; return "We're home,Lan!";
	case VISIT_END_UNREACHABLE: return "HMM... NO ANSWER\nFROM THAT SERVER!|MAYBE IT'S DOWN,OR\nITS ADDRESS IS OFF!";
	case VISIT_END_VERSION: return "OH! A NEWER VERSION\nIS OUT!|UPDATE YOUR PET AT\nTHE DOWNLOAD PAGE!";
	case VISIT_END_OLD_SERVER: return "HMM! THAT SQUARE'S\nSERVER IS BEHIND!|ITS HOST HAS TO\nUPDATE IT FIRST!";
	case VISIT_END_MAINTENANCE: return maintenance(online_visit_end_note());
	case VISIT_END_ROM: return "THAT SQUARE IS SET\nFOR OTHER GAME DATA!";
	case VISIT_END_NAME: return "THE SQUARE TURNED\nYOUR NAME DOWN!|TRY ANOTHER ONE!";
	case VISIT_END_FULL: return "OH NO,OH NO! THAT\nSQUARE IS FULL UP!|TRY AGAIN LATER!";
	case VISIT_END_BARRED: return "THAT SQUARE STAYS\nSHUT FOR YOU...";
	case VISIT_END_REMOVED: return "A SQUARE MODERATOR\nSENT YOU HOME!";
	case VISIT_END_LOST: *face = FACE_MEGAMAN; return "Whoa! The link to\nthe Square broke!|We're back home,Lan.";
	case VISIT_END_BROKEN: *face = FACE_MEGAMAN; return "Huh? The Square\nwouldn't load...|We're back home,Lan.";
	case VISIT_END_MOVED: *face = FACE_MEGAMAN; return "Whoa! The whole\nSquare changed!|We're back home,Lan.";
	default: return NULL;
	}
}
