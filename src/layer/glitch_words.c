/* The Glitch layer's words (glitch_words.h; docs/voice/rules.md, docs/game/glitch-layer.md):
 * MegaMan senses that something is wrong and never names what he has not
 * seen; Lan reacts. The copy speaks in cut words with no face of its own:
 * a few Navis' data written over each other. */
#include "glitch_words.h"

#include <stdio.h>

#include "script_kit.h"

/* A choice of the Glitch layer's, MegaMan's `question`, Yes setting its flag */
static void glitch_ask(TextArchive *t, ScriptsChoice c, const char *question, int no) {
	ta_ask_in(t, FACE_MEGAMAN, question, no, true, true);
	ta_flag_set(t, c.flag);
	ta_end(t);
}

int ta_glitch_warp(TextArchive *t, ScriptsChoice c) {
	static const char ask[] = "Something's off in\nthere... Go in?\n";
	int no = ta_closing(t);
	/* (said on the layer, a later talk is a box and the question, as a
	 * choice's are: session 78) */
	int later = ta_script(t);
	bool first = true;
	ta_pages(t, "It's still changing,Lan...", FACE_MEGAMAN, &first);
	glitch_ask(t, c, ask, no);
	int i = ta_script(t);
	ta_if_told(t, c, later);
	first = true;
	/* (what it is, as he senses it, and where its exit leads: the fork's
	 * two ways told apart, as the Undernet's warp tells its own) */
	ta_pages(t, "W-Whoa... Lan,this warp's wrong.|It keeps changing! Like it can't decide...|Its exit leads to the next layer,though.",
		FACE_MEGAMAN, &first);
	ta_told(t, c);
	glitch_ask(t, c, ask, no);
	return i;
}

int ta_copy(TextArchive *t, ScriptsChoice c, int won_flag, int taken_flag, int pick, const ScriptsVault *v, bool met) {
	static const char ask[] = "Win,and one's ours!\nTake it on?\n";
	int quiet = ta_say(t, FACE_MEGAMAN, "It's quiet here now,Lan.");
	int no = ta_closing(t);
	/* (what it pays, the three by name, as a Server's are) */
	char held[96];
	snprintf(held, sizeof held, "@M Its data... %s,%s and %s!", v->name[0], v->name[1], v->name[2]);
	/* (met on the layer, a later talk, after a No or a run from its
	 * battle, is its data and the question: session 78) */
	int later = ta_script(t);
	bool first = true;
	ta_pages(t, held, FACE_MEGAMAN, &first);
	glitch_ask(t, c, ask, no);
	int i = ta_script(t);
	ta_if_flag(t, taken_flag, quiet);
	ta_if_flag(t, won_flag, pick);
	ta_if_told(t, c, later);
	first = true;
	ta_pages(t, met ? "@M Lan! It's that copy again!|@N ...y-you... a-again..." :
		"@M Lan! That Navi... It keeps changing!|@N ...d-de...lete... ...us...|@M It's like a few Navis at once!", FACE_MEGAMAN, &first);
	ta_pages(t, held, FACE_MEGAMAN, &first);
	ta_told(t, c);
	glitch_ask(t, c, ask, no);
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
