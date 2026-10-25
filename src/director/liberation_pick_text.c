/* liberation_pick_text.h. Menu words, short and plain: what the cursor's
 * Navi is, what his command costs, the form his battles take. */
#include "liberation_pick_text.h"

#include <stdio.h>

#include "liberation_board.h"

/* (in PickWord's order, its name beside each) */
static const char *const words[] = {
	"TEAM",   /* PW_TITLE */
	"GO!",   /* PW_GO */
	"COMMAND",   /* PW_COMMAND */
	"BATTLE",   /* PW_BATTLE */
	"StoneBody",   /* PW_STONEBODY */
	"Weakness",   /* PW_WEAKNESS */
	"None",   /* PW_NONE */
};

const char *lib_pick_word(PickWord w) {
	_Static_assert(sizeof words / sizeof *words == PW_COUNT, "a word for each PickWord");
	return (unsigned)w < PW_COUNT ? words[w] : "";
}

const char *lib_pick_places(char *buf, size_t n, int picked, int places) {
	snprintf(buf, n, "%d/%d", picked, places);
	return buf;
}

const char *lib_pick_hp(char *buf, size_t n, int hp) {
	snprintf(buf, n, "HP %d", hp);
	return buf;
}

const char *lib_pick_command_note(int command) {
	switch (command) {
	case LIB_DO_NONE: return "ALWAYS ON";
	case LIB_DO_NUMBER: case LIB_DO_SNEAK: case LIB_DO_MELODY: case LIB_DO_MAGNET: case LIB_DO_BOMB: return "NO BATTLE";
	default: return "1 BATTLE";
	}
}
