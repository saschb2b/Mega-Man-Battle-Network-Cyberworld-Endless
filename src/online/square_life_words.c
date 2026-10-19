#include "square_life_words.h"

#include <stdio.h>
#include <string.h>

#include "net.h"
#include "trade_words.h"

static const char *area_name(int area) { return area == 2 ? "Sky" : area == 3 ? "Green" : "Central"; }

static const char *landmark_name(int look) {
	return look == LOOK_GIANT_TREE ? "big tree" : look == LOOK_MONUMENT ? "monument" : "statue";
}

/* the Navis here as a list: "Alpha", "Alpha and Beta", "Alpha,Beta and
 * Gamma" */
static void here(char *out, size_t n, const SquareFacts *f) {
	out[0] = 0;
	for (int k = 0; k < f->nnames; ++k) {
		size_t used = strlen(out);
		snprintf(out + used, n - used, "%s%s", k ? (k + 1 == f->nnames ? " and " : ",") : "", f->names[k]);
	}
}

const char *square_greeter_words(const SquareFacts *f) {
	static char s[200];
	int n = snprintf(s, sizeof s, "HELLO! WELCOME,\nWELCOME!");
	if (f->arrived) n += snprintf(s + n, sizeof s - (size_t)n, "|%s JUST\nLOGGED IN!", f->arrived);
	if (f->navis <= 1) snprintf(s + n, sizeof s - (size_t)n, "|IT'S QUIET NOW.\nFRIENDS LOG IN\nRIGHT HERE!");
	else snprintf(s + n, sizeof s - (size_t)n, "|%u NAVIS ARE ON\nCHANNEL %u!", f->navis, f->channel);
	return s;
}

const char *square_greeter_bark(const char *name, bool in) {
	static char s[48];
	snprintf(s, sizeof s, in ? "%s LOGGED IN!" : "%s LOGGED OUT!", name);
	for (char *c = s; *c; ++c) if (*c >= 'a' && *c <= 'z') *c = (char)(*c - 'a' + 'A');
	return s;
}

const char *square_referee_words(const SquareFacts *f) {
	static char s[240];
	if (!f->arena) return "THIS IS THE ARENA!|SORRY! NO NETBATTLES\nFROM THIS PET!";
	int n = snprintf(s, sizeof s, "THIS IS THE ARENA!|FACE A NAVI AND\nPRESS A. PICK\nBATTLE!");
	if (f->fought && f->result == 1) n += snprintf(s + n, sizeof s - (size_t)n, "|LAST TIME YOU\nBEAT %s!\nWHAT A MATCH!", f->fought);
	else if (f->fought && f->result == 2) n += snprintf(s + n, sizeof s - (size_t)n, "|LAST TIME %s\nWON! TRY AGAIN!", f->fought);
	else if (f->fought) n += snprintf(s + n, sizeof s - (size_t)n, "|YOUR MATCH WITH\n%s ENDED.\nANOTHER ONE?", f->fought);
	else n += snprintf(s + n, sizeof s - (size_t)n, "|NO NETBATTLES\nYET! WHO'S FIRST?");
	if (f->navis > 1) snprintf(s + n, sizeof s - (size_t)n, "|%u NAVIS HERE.\nPLENTY TO WATCH!", f->navis);
	return s;
}

/* GreenNavi's talk: gossip, the area, the hour */
static int green_lines(const char **out, char (*buf)[96], const SquareFacts *f) {
	int n = 0;
	if (f->nnames) { snprintf(buf[n], 96, "Hey,%s's here!\nGood crowd today!", f->names[f->turn % (unsigned)f->nnames]); out[n] = buf[n]; ++n; }
	else out[n++] = "Quiet,huh? Just\nyou and me.";
	if (f->traded) { snprintf(buf[n], 96, "Did you hear?\nYou traded with\n%s!", f->traded); out[n] = buf[n]; ++n; }
	if (f->fought) { snprintf(buf[n], 96, "That NetBattle\nwith %s? Wow!", f->fought); out[n] = buf[n]; ++n; }
	snprintf(buf[n], 96, "%s Area's Square!\nBest spot on the\nnet!", area_name(f->area)); out[n] = buf[n]; ++n;
	if (f->hour >= 22 || f->hour < 5) out[n++] = "Up late,huh?\nThe Square never\nsleeps!";
	else if (f->hour < 12) out[n++] = "Morning! The plaza's\nfresh today!";
	else if (f->hour >= 18) out[n++] = "Evening's the\nbusy time here!";
	return n;
}

/* GirlNavi's talk: she gushes, and tells one of the Square's tips as
 * gossip (docs/VOICE.md, What nobody says: a bystander's one tip) */
static int girl_lines(const char **out, char (*buf)[96], const SquareFacts *f) {
	int n = 0;
	if (f->navis > 2) { snprintf(buf[n], 96, "%u Navis! It's\nsooo lively!", f->navis); out[n] = buf[n]; ++n; }
	if (f->arrived) { snprintf(buf[n], 96, "Ooh,%s just\ngot here! Sooo cool!", f->arrived); out[n] = buf[n]; ++n; }
	snprintf(buf[n], 96, "Did you know? Navis\ncheer together by\nthe %s!", landmark_name(f->landmark)); out[n] = buf[n]; ++n;
	out[n++] = "Up on the stage,\neveryone sees you!\nSo fun!";
	if (f->navis <= 1) out[n++] = "Nobody's here...\nHow lonely!";
	return n;
}

/* on parade: the parade comes first */
static const char *parade_line(int k, const SquareFacts *f) {
	static char s[96];
	if (!k) return "Left,right! Left,right!\nCome on,join the\nparade!";
	snprintf(s, sizeof s, "Parade time! Walk\nround the %s\nwith us!", landmark_name(f->landmark));
	return s;
}

const char *square_walker_words(int k, const SquareFacts *f) {
	if (f->parade) return parade_line(k, f);
	static char buf[8][96];
	const char *lines[8];
	int n = k ? girl_lines(lines, buf, f) : green_lines(lines, buf, f);
	return lines[f->turn % (unsigned)n];
}

/* the Board: a post a box, newest first; the trade board's offers after */
static const char *board(const SquareFacts *f) {
	static char s[720];
	char list[64];
	here(list, sizeof list, f);
	int n = snprintf(s, sizeof s, "NET SQUARE BBS\nCHANNEL %u OF %u", f->channel, f->channels);
	if (f->nnames) n += snprintf(s + n, sizeof s - (size_t)n, "|>Here now:\n%s", list);
	else n += snprintf(s + n, sizeof s - (size_t)n, "|>Here now:\njust you.");
	if (f->arrived) n += snprintf(s + n, sizeof s - (size_t)n, "|>%s logged in.", f->arrived);
	if (f->traded) n += snprintf(s + n, sizeof s - (size_t)n, "|>Trade done with\n%s!", f->traded);
	if (f->fought) n += snprintf(s + n, sizeof s - (size_t)n, "|>NetBattle vs\n%s: %s", f->fought, f->result == 1 ? "won!" : f->result == 2 ? "lost." : "over.");
	for (int k = 0; k < f->nposts; ++k)
		n += f->posts[k].name ? snprintf(s + n, sizeof s - (size_t)n, "|>%s has\n%s,wants\n%s.", f->posts[k].name, f->posts[k].chip, trade_want(f->posts[k].want))
			: snprintf(s + n, sizeof s - (size_t)n, "|>Your offer:\n%s for\n%s.", f->posts[k].chip, trade_want(f->posts[k].want));
	if (!f->nposts) n += snprintf(s + n, sizeof s - (size_t)n, "|>Trade board:\nno offers up.");
	snprintf(s + n, sizeof s - (size_t)n, "|>Tip: post a chip\nat Trade Central.");
	return s;
}

const char *square_check_words(int look, const SquareFacts *f) {
	static char s[80];
	switch (look) {
	case LOOK_BBS: return board(f);
	case LOOK_SIGN:
		snprintf(s, sizeof s, "\"WELCOME TO THE\nNET SQUARE!\nCHANNEL %u\"", f->channel);
		return s;
	case LOOK_STATUE: return "A big Navi statue.\nNavis meet up\nround it.";
	case LOOK_MONUMENT: return "An old monument.\nNavis meet up\nin front of it.";
	case LOOK_GIANT_TREE: return "A giant cybertree.\nIts leaves hum\nsoftly.";
	case LOOK_BRAZIER: return "A brazier.\nIts flame flickers.";
	default: return "A cybertree.";
	}
}

const char *square_cheer_bark(void) { return "!!"; }

const char *square_parade_bark(void) { return "Parade time!"; }
