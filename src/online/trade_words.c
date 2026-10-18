#include "trade_words.h"

#include <stdio.h>

const char *const trade_options[2] = { "Offer a chip", "How it works" };
const char *const trade_offer_options[2] = { "Offer one back", "No thanks" };
const char *const trade_wait_options[2] = { "Keep waiting", "Call it off" };

const char *trade_hello(void) { return "WELCOME TO\nTRADE CENTRAL!|ONE CHIP FOR ONE CHIP!\nWHAT'LL IT BE?"; }

const char *trade_how(void) {
	return "PICK A NAVI HERE,\nAND A SPARE CHIP!|THEY PICK ONE BACK.\nYOU BOTH SAY DEAL...|AND THE CHIPS SWAP\nWHEN YOU'RE HOME!";
}

const char *trade_bye(void) { return "COME AGAIN!"; }

const char *trade_who(void) { return "WHO'LL YOU TRADE\nWITH?"; }

const char *trade_none_here(void) { return "HMM... NO OTHER\nNAVIS HERE RIGHT NOW!"; }

const char *trade_pick(bool bn6_only) {
	return bn6_only ? "PICK THE CHIP YOU'LL\nOFFER! THAT NAVI\nTRADES BN6 CHIPS!" : "PICK THE CHIP YOU'LL\nOFFER! SPARES ONLY!";
}

const char *trade_bn6_only(const char *name) {
	static char s[128];
	snprintf(s, sizeof s, "HMM... %s\nTRADES BN6 CHIPS\nONLY!|AND YOUR SPARES ARE\nALL BN5 CHIPS!", name);
	return s;
}

const char *trade_bn5_spares(void) {
	return "HMM... YOUR SPARES\nARE ALL BN5 CHIPS!|NO NAVI HERE\nTRADES THOSE!";
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
