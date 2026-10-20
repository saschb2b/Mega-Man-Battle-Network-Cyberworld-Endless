#include "trade_words.h"

#include <stdio.h>
#include <string.h>

const char *const trade_options[3] = { "Offer a chip", "Trade board", "How it works" };
const char *const trade_offer_options[2] = { "Offer one back", "No thanks" };
const char *const trade_wait_options[2] = { "Keep waiting", "Call it off" };

const char *trade_hello(void) { return "WELCOME TO\nTRADE CENTRAL!|ONE CHIP FOR ONE CHIP!\nWHAT'LL IT BE?"; }

const char *trade_how(void) {
	return "PICK A NAVI HERE,\nAND A SPARE CHIP!|THEY PICK ONE BACK.\nYOU BOTH SAY DEAL...|AND THE CHIPS SWAP\nWHEN YOU'RE HOME!|OR PUT A SPARE UP ON\nTHE TRADE BOARD,AND\nSAY WHAT YOU WANT!|ONE TRADE A VISIT!";
}

const char *trade_bye(void) { return "COME AGAIN!"; }

const char *trade_who(void) { return "WHO'LL YOU TRADE\nWITH?"; }

const char *trade_none_here(void) { return "HMM... NO OTHER\nNAVIS HERE RIGHT NOW!"; }

const char *trade_none_stamped(void) { return "HMM... NO NAVI HERE\nHAS A TRADE STAMP\nRIGHT NOW!"; }

const char *trade_pick(void) { return "PICK THE CHIP YOU'LL\nOFFER! SPARES ONLY!"; }

const char *trade_no_stamp(void) {
	return "WELCOME TO\nTRADE CENTRAL!|HMM... NO TRADE\nSTAMP ON YOUR\nNETPASS!|SORRY! I CAN'T SWAP\nCHIPS WITHOUT ONE!";
}

const char *trade_no_spares(void) { return "OH NO! NO SPARE CHIPS!\nYOUR FOLDERS HOLD\nTHEM ALL!"; }

const char *trade_sent(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "SENT! LET'S SEE WHAT\n%s SAYS!", name);
	return s;
}

const char *trade_offered(const char *name, const char *chip) {
	static char s[128];
	snprintf(s, sizeof s, "%s OFFERS\n%s!", name, chip);
	return s;
}

const char *trade_waiting(const char *name) {
	static char s[96];
	snprintf(s, sizeof s, "STILL WAITING ON\n%s!", name);
	return s;
}

const char *trade_heard(const char *name, const char *chip) {
	static char s[160];
	snprintf(s, sizeof s, "Lan! %s offers\nus %s!|Let's answer at\nTrade Central!", name, chip);
	return s;
}

const char *trade_review(const char *name, const char *mine, const char *theirs) {
	static char s[200];
	snprintf(s, sizeof s, "%s gives us\n%s...|for our %s!", name, theirs, mine);
	return s;
}

const char *trade_deal_question(void) { return "Deal,Lan?\n"; }

const char *trade_done(const char *chip) {
	static char s[96];
	snprintf(s, sizeof s, "Deal! %s\nis ours once we're home!", chip);
	return s;
}

const char *trade_off(void) { return "Aw... The trade's off."; }

const char *trade_settled(const char *gave, const char *got) {
	static char s[160];
	snprintf(s, sizeof s, "Lan! %s is in\nour Pack now!|%s went to its\nnew home.", got, gave);
	return s;
}

const char *trade_order_prompt(void) { return "WHICH CHIP WILL YOU\nOFFER?\nB BUTTON:BACK"; }

const char *trade_order_back(void) { return "BACK TO THE DESK!"; }

const char *trade_order_ready(void) { return "GOT IT! YOUR OFFER\nIS ON ITS WAY!"; }

const char *trade_order_after(void) { return "THAT'S YOUR OFFER!\nB BUTTON:BACK"; }

const char *trade_order_all(void) { return "THAT'S ALL YOUR\nSPARES!"; }

const char *trade_order_once(void) { return "THAT ONE'S PICKED\nALREADY!"; }

const char *trade_order_leave(void) { return "Back to the desk,\nLan?"; }

const char *trade_order_offer(void) { return "YOU'LL OFFER\n"; }

const char *trade_order_said(void) { return "We'll offer\n"; }

/* ---- the board ---- */

static const char *const wants[10] = {
	"anything", "a Fire chip", "an Aqua chip", "an Elec chip", "a Wood chip",
	"a Sword chip", "a Wind chip", "a Break chip", "a Cursor chip", "a Mega chip",
};

/* (the list a post's want is picked from) */
const char *const trade_want_options[10] = {
	"Anything", "Fire chip", "Aqua chip", "Elec chip", "Wood chip",
	"Sword chip", "Wind chip", "Break chip", "Cursor chip", "Mega chip",
};

const char *trade_want(int want) { return want >= 0 && want < 10 ? wants[want] : wants[0]; }

/* a want in the clerk's capitals */
static const char *want_caps(int want) {
	static char s[24];
	snprintf(s, sizeof s, "%s", trade_want(want));
	for (char *c = s; *c; ++c) if (*c >= 'a' && *c <= 'z') *c = (char)(*c - 'a' + 'A');
	return s;
}

const char *trade_board_intro(int offers) {
	static char s[64];
	if (!offers) return "THE TRADE BOARD!\nNO OFFERS UP YET!";
	snprintf(s, sizeof s, "THE TRADE BOARD!\n%d OFFER%s UP!", offers, offers == 1 ? "" : "S");
	return s;
}

const char *trade_board_offer(const char *name, const char *chip, int want) {
	static char s[128];
	snprintf(s, sizeof s, "%s HAS\n%s!\nWANTS %s!", name, chip, want_caps(want));
	return s;
}

const char *const trade_board_options[2] = { "Answer it", "Next offer" };

const char *trade_board_mine(bool posted) { return posted ? "Take mine down" : "Post an offer"; }

const char *trade_board_leave(void) { return "Never mind"; }

const char *trade_take_pick(int want) {
	static char s[80];
	snprintf(s, sizeof s, "PICK A SPARE! THEY\nWANT %s!", want_caps(want));
	return s;
}

const char *trade_take_none(const char *name, int want) {
	static char s[128];
	snprintf(s, sizeof s, "HMM... %s WANTS\n%s!|NO SPARE OF YOURS\nFITS THAT!", name, want_caps(want));
	return s;
}

const char *trade_post_pick(void) { return "PICK THE SPARE TO\nPUT UP!"; }

const char *trade_post_want(void) { return "AND WHAT DO YOU\nWANT FOR IT?"; }

const char *trade_posted(const char *chip, int want) {
	static char s[96];
	snprintf(s, sizeof s, "IT'S UP! %s\nFOR %s!", chip, want_caps(want));
	return s;
}

const char *trade_post_down(void) { return "OKAY! YOUR OFFER'S\nDOWN!"; }

const char *trade_traded(void) { return "YOU TRADED ALREADY!|ONE TRADE A VISIT!\nTAKE IT HOME FIRST!"; }

const char *trade_diving(void) { return "YOU'RE ON THIS\nWEEK'S DIVE!|DIVERS DON'T TRADE!\nSAME NET,SAME\nCHIPS FOR ALL!"; }

const char *trade_board_review(const char *name, const char *mine, const char *theirs) {
	static char s[200];
	snprintf(s, sizeof s, "Lan! %s took\nour offer!|%s for our\n%s!", name, theirs, mine);
	return s;
}
