/* What a liberation's teammates say (liberation_team_words.h). BN5's Team
 * Colonel, each in the habit his look gives him (docs/voice/rules.md, The
 * guardians): Colonel a soldier's few words, ShadowMan a ninja's fewer,
 * NumberMan's numbers, TomahawkMan's sayings, KnightMan a knight's
 * courtesy, ToadMan's croak; Gregar's Cross Navis in the habits BN6 gives
 * them: HeatMan hot-blooded, ElecMan's calm crackle, SlashMan's "Slash!",
 * EraseMan's cackle, ChargeMan a train; and ProtoMan, the real one, Chaud's
 * (ProtoSoul held: the rival's netbattle won, docs/game/rival.md), a stern
 * swordsman of few words as BN6 gives him. Team ProtoMan's other five, the
 * older net's copies, take a habit from their look as Team Colonel's do:
 * GyroMan a pilot, SearchMan a sniper's few words, NapalmMan a blaster's
 * bang, MagnetMan's poles, Meddy a nurse's care. Each line is his alone (hers,
 * Meddy's), said with his face (the five have none yet: a HeelNavi's body,
 * no face). */
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

/* (Chaud's Navi, who keeps score against MegaMan and lent him his Soul:
 * here on Chaud's word, curt with his rival) */
static const char *const protoman[LINES] = {
	"...ProtoMan. Chaud sent me.|Don't slow me down,MegaMan.",
	"Hmph. The panel's clear.",
	"One stroke. That's all it took.",
	"...Tch. My blade was slow.",
	"Rest. Then we move.",
};

static const char *const gyroman[LINES] = {
	"GyroMan,taking off!|I'll fly right over this dark!",
	"Touchdown! Panel's clear!",
	"Hah! Barely left the ground!",
	"Mayday... Lost altitude...",
	"Landing for a breather.",
};
static const char *const searchman[LINES] = {
	"SearchMan. Scanning the area.|...Targets locked.",
	"Target down.",
	"Clean hit. Mission complete.",
	"...Target lost.",
	"Recalibrating my scope.",
};
static const char *const napalmman[LINES] = {
	"NapalmMan,locked and loaded!|Let's light this place up!",
	"KA-BOOM! Blown wide open!",
	"Hahaha! One blast did it!",
	"Tch... A dud...",
	"Cooling down my launchers...",
};
static const char *const magnetman[LINES] = {
	"MagnetMan here!|This dark won't stick around!",
	"Pulled it clean!",
	"Hah! Repelled in one go!",
	"Ngh... Wrong polarity...",
	"Recharging my poles...",
};
static const char *const meddy[LINES] = {
	"Meddy here!|I'll look after everyone!",
	"Phew! All better now!",
	"Quick and painless!",
	"Oh no... I couldn't fix it...",
	"Let me catch my breath...",
};

static const char *const heatman[LINES] = {
	"HeatMan's here!|I'm all fired up! C'mon!",
	"Burned clean through!",
	"Hah! Didn't even break a sweat!",
	"Grr... My fire went out...",
	"Gotta stoke the flames...",
};
static const char *const elecman[LINES] = {
	"...tzz. ElecMan.|I shall keep this team from harm.",
	"The panel is clear.",
	"Swift as current. ...tzz.",
	"...tzz. The circuit failed.",
	"Recharging.",
};
static const char *const slashman[LINES] = {
	"Slaaash! SlashMan's in!|I'll cut this dark to ribbons!",
	"Slashed it!",
	"Slaaash! Too easy!",
	"Tch! It didn't cut...",
	"Sharpening my claws...",
};
static const char *const eraseman[LINES] = {
	"Hyahaha! EraseMan...|My scythe wants this darkness.",
	"Erased.",
	"Hyahaha! Gone in one swing!",
	"Hrr... It would not fade...",
	"My scythe rests... for now.",
};
static const char *const chargeman[LINES] = {
	"Choo,choo! ChargeMan,all aboard!|Next stop,the Darkloid!",
	"Track's clear!",
	"Choo,choo! Right on schedule!",
	"Derailed...",
	"Taking on coal...",
};

static const char *const *lines_of(int navi) {
	switch (navi) {
	case MATE_COLONEL: return colonel;
	case MATE_SHADOWMAN: return shadowman;
	case MATE_NUMBERMAN: return numberman;
	case MATE_TOMAHAWKMAN: return tomahawkman;
	case MATE_KNIGHTMAN: return knightman;
	case MATE_TOADMAN: return toadman;
	case MATE_PROTOMAN: return protoman;
	case MATE_GYROMAN: return gyroman;
	case MATE_SEARCHMAN: return searchman;
	case MATE_NAPALMMAN: return napalmman;
	case MATE_MAGNETMAN: return magnetman;
	case MATE_MEDDY: return meddy;
	case MATE_HEATMAN: return heatman;
	case MATE_ELECMAN: return elecman;
	case MATE_SLASHMAN: return slashman;
	case MATE_ERASEMAN: return eraseman;
	case MATE_CHARGEMAN: return chargeman;
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
	case MATE_ELECMAN: return "...tzz. MagnetBarrier. None shall touch us.";
	case MATE_MAGNETMAN: return "MagnetBarrier! Nothing gets through!";
	case MATE_GYROMAN: return "Bombs away! Panel's clear!";
	case MATE_CHARGEMAN: return "Choo,choo! Clearing the track!";
	default: return "";
	}
}

const char *lib_soul_form_name(int navi) {
	static const char *const forms[] = { "ProtoSoul", "GyroSoul", "SearchSoul", "NapalmSoul", "MagnetSoul", "MeddySoul",
		"ColonelSoul", "ShadowSoul", "NumberSoul", "TomahawkSoul", "KnightSoul", "ToadSoul" };
	return navi >= MATE_PROTOMAN && navi <= MATE_TOADMAN ? forms[navi - MATE_PROTOMAN] : "";
}
