/* The Glitch layer's words (glitch_words.h; docs/voice/rules.md, docs/game/glitch-layer.md):
 * MegaMan senses that something is wrong and never names what he has not
 * seen; Lan reacts. The copy speaks in cut words with no face of its own:
 * a few Navis' data written over each other. */
#include "glitch_words.h"

#include <stdio.h>

#include "script_kit.h"

int ta_glitch_warp(TextArchive *t, int flag) {
	int no = ta_closing(t);
	int i = ta_script(t);
	bool first = true;
	/* (what it is, as he senses it, and where its exit leads: the fork's
	 * two ways told apart, as the Undernet's warp tells its own) */
	ta_pages(t, "W-Whoa... Lan,this warp's wrong.|It keeps changing! Like it can't decide...|Its exit leads to the next layer,though.",
		FACE_MEGAMAN, &first);
	ta_ask_in(t, FACE_MEGAMAN, "Something's off in\nthere... Go in?\n", no, true, true);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

/* (ts_check_flag: on to script `to` while `flag` is set) */
static void check_flag(TextArchive *t, int flag, int to) {
	uint8_t check[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)to, 0xFF };
	ta_bytes(t, check, sizeof check);
}

int ta_copy(TextArchive *t, int flag, int won_flag, int taken_flag, int pick, const ScriptsVault *v, bool met) {
	int quiet = ta_say(t, FACE_MEGAMAN, "It's quiet here now,Lan.");
	int no = ta_closing(t);
	int i = ta_script(t);
	check_flag(t, taken_flag, quiet);
	check_flag(t, won_flag, pick);
	bool first = true;
	ta_pages(t, met ? "@M Lan! It's that copy again!|@N ...y-you... a-again..." :
		"@M Lan! That Navi... It keeps changing!|@N ...d-de...lete... ...us...|@M It's like a few Navis at once!", FACE_MEGAMAN, &first);
	/* (what it pays, the three by name, as a Server's are) */
	char held[96];
	snprintf(held, sizeof held, "@M Its data... %s,%s and %s!", v->name[0], v->name[1], v->name[2]);
	ta_pages(t, held, FACE_MEGAMAN, &first);
	ta_ask_in(t, FACE_MEGAMAN, "Win,and one's ours!\nTake it on?\n", no, true, true);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

const char *glitch_stage_words(int stage) {
	switch (stage) {
	case 2: return "@M Whoa! The floor turned into another area!|@L Just for a second... What IS this place?";
	case 3: return "@M The viruses here... They're getting stronger!|@L The deeper we go,the worse it gets...";
	case 4: return "@M Lan! Something's at the middle of all this!";
	default: return NULL;
	}
}
