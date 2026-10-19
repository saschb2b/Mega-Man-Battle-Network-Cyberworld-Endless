/* MegaMan on the NaviCust (docs/NAVICUST.md, docs/VOICE.md): a program
 * off the board, Reg memory, compression codes, a board run clean. */
#include "board_words.h"

#include <stdio.h>
#include <string.h>

#include "director_board.h"
#include "director_folder.h"
#include "director_state.h"
#include "save.h"

/* MegaMan's word for a program left off the board, once a layer (NULL:
 * none to say), where it fits beside those on it; how to place it, turn
 * it or make room is the player's, as in BN6 (the voice sweep, 19 October
 * 2026: docs/voice/sweep/net.md). */
const char *off_board_words(void) {
	int offv = 0;
	const char *off = D.off_told || run_won_here() ? NULL : program_off_board(&offv);
	static char words[96];
	if (!off || !*off || !fits_beside_placed(offv) || off_board_explained(offv)) return NULL;
	off_board_explain(offv);
	snprintf(words, sizeof words, "@M Lan,%s isn't on our board!", off);
	return words;
}

/* MegaMan's words on Reg memory (issue #51), `reg` MB of it: the first
 * RegUp in any run; a RegUp after that; and the TagChip system, for a
 * profile whose Chaud's clearance came before it did. What a Regular Chip
 * and a TagChip are, and their buttons, are BN6's own, unsaid (the voice
 * sweep, docs/voice/sweep/net.md). */
const char *reg_words(int kind, int reg) {
	static char words[96];
	if (kind == REG_LESSON) snprintf(words, sizeof words, "@M A RegUp,Lan! Our Reg memory's %d MB now!", reg);
	else if (kind == REG_UP) snprintf(words, sizeof words, "@M Reg memory up! It's %d MB now,Lan!", reg);
	else snprintf(words, sizeof words, "@M Lan! Chaud's clearance comes with the TagChip system!");
	return words;
}

/* A program's compression code entered the first time in any run (issue
 * #50; `about` its NaviCust words, "Name: ..."): where Dad keeps it */
const char *compressed_words(const char *about) {
	static char words[200];
	snprintf(words, sizeof words, "@M %.*s's compressed,Lan!|@M Dad's lab keeps its code for every dive now.|@M It's in his Compression mail!",
		(int)strcspn(about, ":"), about);
	return words;
}

/* The NaviCust's bugs gone */
const char *clean_words(void) { return "@M Phew! Our NaviCust runs clean now,Lan!"; }
