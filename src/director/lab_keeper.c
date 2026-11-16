/* The Battle Lab's keeper's course at home (lab_keeper.h). */
#include "lab_keeper.h"

#include <stdio.h>

#include "bn6.h"
#include "debug.h"
#include "devtools.h"
#include "director.h"
#include "director_home.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "lab_state.h"
#include "lab_words.h"
#include "lanhp.h"
#include "save.h"
#include "script_kit.h"
#include "talk.h"
#include "text.h"

bool lab_there(void) { return dev.lab || profile.lab_sent; }

/* his words: the first meeting's boxes once a profile, his question
 * Yes or No after them (No closes the box, and the next talk is the
 * first meeting again), one box after; then the flag that opens the
 * builder */
static void words(void) {
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	ta_begin(&t);
	int talk = ta_script(&t);
	bool first = true;
	ta_pages(&t, profile.lab_met ? lab_keeper_hello() : lab_keeper_first(), FACE_LAB, &first);
	/* (the No: the script after this one, which closes the box) */
	if (!profile.lab_met) ta_ask_in(&t, FACE_LAB, lab_keeper_ask(), talk + 1, true, false);
	ta_flag_set(&t, LAB_OPEN_FLAG);
	ta_end(&t);
	if (!profile.lab_met) ta_closing(&t);
	if (!lanhp_lab_say(out, ta_build(&t, out)) && emu_debug_on()) fprintf(stderr, "lab: the keeper's words did not fit\n");
}

void home_lab_visit(void) {
	flag_clear(LAB_OPEN_FLAG);
	if (lab_there()) flag_clear(LANHP_LAB_GONE_FLAG);
	else flag_set(LANHP_LAB_GONE_FLAG);
	words();
}

void home_lab_frame(void) {
	if (!home_in_hp() || !flag_get(LAB_OPEN_FLAG) || emu_read8(BN6_CHATBOX) || talk_busy() || !on_map()) return;
	flag_clear(LAB_OPEN_FLAG);
	if (!profile.lab_met) {
		profile.lab_met = 1;
		profile_save();
		words();   /* (the one box, from the next talk on) */
	}
	/* (--dev lab: every family the engine holds, a look without a profile's history) */
	director_lab_open(dev.lab);
}

void home_lab_describe(FILE *f) {
	static const char *const pages[LAB_PAGES] = { "foes", "field", "stage", "sets" }, *const ends[] = { "none", "won", "deleted", "left" };
	if (LS.fighting || LS.editing) fprintf(f, "lab %s\n", LS.fighting ? "fighting" : "editing");
	else if (LS.up)
		fprintf(f, "lab builder page %s foes %d last %s%s\n", pages[LS.page % LAB_PAGES], LS.set.nfoes, ends[LS.last.end & 3], LS.asking ? " leaving" : "");
	else if (home_in_hp()) fprintf(f, "lab %s\n", lab_there() && !flag_get(LANHP_LAB_GONE_FLAG) ? "keeper" : "none");
}
