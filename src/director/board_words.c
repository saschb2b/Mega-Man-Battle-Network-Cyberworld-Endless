/* MegaMan on the NaviCust (docs/NAVICUST.md, docs/VOICE.md): a program
 * off the board, one with no room, compression codes. */
#include "board_words.h"

#include <stdio.h>

#include "director_board.h"
#include "director_folder.h"
#include "director_state.h"
#include "save.h"

static const char *code_words(int v, bool as_it_stands);

const char *no_room_words(const char *name, int v) {
	static char words[360];
	int board = key_item(SCRIPTS_EXP_MEMORY);
	if (no_room_told == board) return NULL;
	no_room_told = board;
	if (board < 2)
		snprintf(words, sizeof words, "@M Hmm... %s won't fit beside the others yet.|@M It'll keep in the PET till the board grows,Lan.%s", name,
			code_words(v, false));
	else snprintf(words, sizeof words, "@M Hmm... %s won't fit beside the others,Lan.|@M To use it,we'd have to take one off the board.%s", name,
		code_words(v, false));
	return words;
}

/* MegaMan's words for a program left off the board, once a layer (NULL:
 * none to say): where to place it, and whether it turns. */
const char *off_board_words(void) {
	int offv = 0;
	const char *off = D.off_told || run_won_here() ? NULL : program_off_board(&offv);
	static char words[480];
	if (!off || !*off) return NULL;
	if (!fits_beside_placed(offv)) return no_room_words(off, offv);
	if (bit_of(off_explained, offv)) return NULL;
	bit_set(off_explained, offv);
	/* (and whether it turns: a playtester pressed L and R on his gift's
	 * SuperArmor with no Spin, and nothing said why; and whether it takes
	 * moving others first, or its code) */
	bool stands = fits_as_it_stands(offv);
	snprintf(words, sizeof words, "@M Lan,%s isn't on our board!|@M It does nothing till it's placed.|@M In the PET,go to MegaMan,then NaviCust!|@M %s%s%s",
		off, stands ? "" : "There's no room for it as the board stands.|@M We'll have to move a program or two.|@M ", navicust_turn_words(offv),
		stands ? "" : code_words(offv, true));
	return words;
}

/* MegaMan's word on a program's compression code where it is the way to fit
 * the program (issue #50): a code entered in an earlier run, not in this
 * one, whose shape fits beside the board's programs (`as_it_stands`: in its
 * free cells as they stand); "" otherwise. A box of its own, the code on a
 * line of its own, as Dad's Compression mail spells it. */
static const char *code_words(int v, bool as_it_stands) {
	static char words[160];
	char code[12];
	if (!profile_code_entered(v / 4) || flag_get(BN6_FLAG_COMPRESSED + v) || !navicust_code(v / 4, code)) return "";
	if (as_it_stands ? !fits_free_as(v, true) : !fits_beside_as(v, true)) return "";
	snprintf(words, sizeof words, "|@M Or we compress it!|@M In the NaviCust,hold RIGHT on it,|@M then press\n%s", code);
	return words;
}

const char *cramped_words(void) {
	static char words[360];
	int v = 0;
	const char *off = program_off_board(&v);
	/* (a Guardian Data's program: its draft has said so, before the pick) */
	if (!off || !*off || !fits_beside_placed(v) || fits_as_it_stands(v) || in_draft(v)) return NULL;
	snprintf(words, sizeof words, "@M Hmm... %s won't fit in our free space,Lan.|@M We'll have to move a program or two to make room.%s", off,
		code_words(v, true));
	return words;
}
