#include "gate_words.h"

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

const char *gate_end_words(VisitEnd why, int *face) {
	*face = FACE_PROG;
	switch (why) {
	case VISIT_END_LEFT: *face = FACE_MEGAMAN; return "We're home,Lan!";
	case VISIT_END_UNREACHABLE: return "HMM... NO ANSWER\nFROM THAT SERVER!|MAYBE IT'S DOWN,OR\nITS ADDRESS IS OFF!";
	case VISIT_END_VERSION: return "OH NO! THAT SQUARE\nIS ANOTHER VERSION!|BOTH SIDES NEED THE\nSAME ONE!";
	case VISIT_END_ROM: return "THAT SQUARE IS SET\nFOR OTHER GAME DATA!";
	case VISIT_END_NAME: return "THE SQUARE TURNED\nYOUR NAME DOWN!|TRY ANOTHER ONE!";
	case VISIT_END_FULL: return "OH NO,OH NO! THAT\nSQUARE IS FULL UP!|TRY AGAIN LATER!";
	case VISIT_END_BARRED: return "THAT SQUARE STAYS\nSHUT FOR YOU...";
	case VISIT_END_REMOVED: return "A SQUARE MODERATOR\nSENT YOU HOME!";
	case VISIT_END_LOST: *face = FACE_MEGAMAN; return "Whoa! The link to\nthe Square broke!|We're back home,Lan.";
	case VISIT_END_BROKEN: *face = FACE_MEGAMAN; return "Huh? The Square\nwouldn't load...|We're back home,Lan.";
	default: return NULL;
	}
}
