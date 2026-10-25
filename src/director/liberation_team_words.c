/* What a liberation's teammates say (liberation_team_words.h). BN5's Team
 * Colonel, each in the habit his look gives him (docs/VOICE.md, The
 * guardians): Colonel a soldier's few words, ShadowMan a ninja's fewer,
 * NumberMan's numbers, TomahawkMan's sayings, KnightMan a knight's
 * courtesy, ToadMan's croak. Each line is his alone, said with his face. */
#include "liberation_team_words.h"

#include <stdio.h>

#include "liberation_board.h"

enum { JOIN, WON, ONE_TURN, FAILED, REST, LINES };

static const char *const colonel[LINES] = {
	"Colonel,reporting.|Let's take this area back.",
	"Panel secured.",
	"Swift work. They never saw it coming.",
	"Tch... It held. We regroup.",
	"Holding position.",
};
static const char *const shadowman[LINES] = {
	"...ShadowMan. I move in the dark.",
	"...Done.",
	"Too slow.",
	"...Hmph. Next time.",
	"...I wait.",
};
static const char *const numberman[LINES] = {
	"NumberMan here!|I've run the numbers on this one!",
	"Panel cleared! Right on the numbers!",
	"Bingo!! Better odds than I figured!",
	"Huh!? That's not what I calculated...",
	"Time to recalculate...",
};
static const char *const tomahawkman[LINES] = {
	"TomahawkMan's here!|There's a saying... Many hands,light work!",
	"Hah! Cleared it!",
	"There's a saying... Strike while it's hot!",
	"Ugh! Even a good axe gets stuck...",
	"Even a warrior needs a breather.",
};
static const char *const knightman[LINES] = {
	"KnightMan,at your service.|I shall be your shield!",
	"The panel is freed!",
	"Splendid! The darkness retreats!",
	"Forgive me... It would not yield.",
	"A knight must rest his armor.",
};
static const char *const toadman[LINES] = {
	"Ribbit! ToadMan's here to keep the beat!",
	"Ribbit! Freed!",
	"Ribbit,ribbit!! What a tune!",
	"Rib... bit... Off-key...",
	"Ribbit... A little rest.",
};

static const char *const *lines_of(int navi) {
	switch (navi) {
	case MATE_COLONEL: return colonel;
	case MATE_SHADOWMAN: return shadowman;
	case MATE_NUMBERMAN: return numberman;
	case MATE_TOMAHAWKMAN: return tomahawkman;
	case MATE_KNIGHTMAN: return knightman;
	case MATE_TOADMAN: return toadman;
	default: return NULL;
	}
}

static const char *line(int navi, int which) {
	const char *const *l = lines_of(navi);
	return l ? l[which] : "";
}

const char *lib_mate_join_words(int navi) { return line(navi, JOIN); }

const char *lib_mate_battle_words(int navi, bool won, bool one_turn) { return line(navi, !won ? FAILED : one_turn ? ONE_TURN : WON); }

const char *lib_mate_rest_words(int navi) { return line(navi, REST); }

const char *lib_mate_command_words(int navi, int n) {
	static char words[80];
	switch (navi) {
	case MATE_NUMBERMAN:
		if (!n) return "NumberCheck! ...Zero? Huh.";
		snprintf(words, sizeof words, "NumberCheck! %d of 'em,no battle needed!", n);
		return words;
	case MATE_SHADOWMAN: return "...Struck from the shadows.";
	case MATE_TOADMAN: return n ? "Ribbit! This song's for you!" : "Ribbit... Nobody to sing for.";
	default: return "";
	}
}
