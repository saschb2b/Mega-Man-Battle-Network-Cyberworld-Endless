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
	static char s[240];
	int n = snprintf(s, sizeof s, "HELLO! WELCOME,\nWELCOME!");
	if (f->arrived) n += snprintf(s + n, sizeof s - (size_t)n, "|%s JUST\nLOGGED IN!", f->arrived);
	if (f->navis <= 1) n += snprintf(s + n, sizeof s - (size_t)n, "|IT'S QUIET NOW.\nFRIENDS LOG IN\nRIGHT HERE!");
	else n += snprintf(s + n, sizeof s - (size_t)n, "|%u NAVIS ARE ON\nCHANNEL %u!", f->navis, f->channel);
	/* (the parts, each a warp off the plaza: session 74 found neither wing) */
	snprintf(s + n, sizeof s - (size_t)n, "|THE WARPS GO TO\nTRADE CENTRAL,THE\nARENA,THE STAGE!");
	return s;
}


const char *square_greeter_bark(const char *name, bool in) {
	static char s[48];
	snprintf(s, sizeof s, in ? "%s LOGGED IN!" : "%s LOGGED OUT!", name);
	for (char *c = s; *c; ++c) if (*c >= 'a' && *c <= 'z') *c = (char)(*c - 'a' + 'A');
	return s;
}

/* the bouts with the last one in a row, as the referee keeps count */
static int tally(char *s, size_t n, const SquareFacts *f) {
	if (f->won + f->lost < 2) return 0;
	if (f->won == f->lost) return snprintf(s, n, "|%u TO %u! ALL EVEN,\nALL EVEN!", f->won, f->lost);
	if (f->won > f->lost) return snprintf(s, n, "|THAT'S %u TO %u.\nYOU LEAD!", f->won, f->lost);
	return snprintf(s, n, "|THAT'S %u TO %u.\n%s\nLEADS!", f->lost, f->won, f->fought);
}

const char *square_referee_words(const SquareFacts *f) {
	static char s[240];
	if (!f->stamp) return "THIS IS THE ARENA!|NO ARENA STAMP ON\nYOUR NETPASS!|SORRY! THE RING'S\nCLOSED TO YOU!";
	if (!f->arena) return "THIS IS THE ARENA!|SORRY! NO NETBATTLES\nFROM THIS PET!";
	int n = snprintf(s, sizeof s, "THIS IS THE ARENA!");
	/* (the ring's one tip until a bout; after one, the last bout and the count) */
	if (!f->fought) {
		n += snprintf(s + n, sizeof s - (size_t)n, "|FACE A NAVI AND\nPRESS A. PICK\nBATTLE!");
		/* (none yet where the channel's first is on: the stands' words say it) */
		if (!f->battle_on) n += snprintf(s + n, sizeof s - (size_t)n, "|NO NETBATTLES\nYET! WHO'S FIRST?");
	} else if (f->result == 1) n += snprintf(s + n, sizeof s - (size_t)n, "|LAST BOUT: YOU\nBEAT %s!\nWHAT A MATCH!", f->fought);
	else if (f->result == 2) n += snprintf(s + n, sizeof s - (size_t)n, "|LAST BOUT:\n%s BEAT\nYOU! NEXT TIME!", f->fought);
	else if (f->result == 4) n += snprintf(s + n, sizeof s - (size_t)n, "|LAST BOUT WITH\n%s BROKE OFF!\nNO CALL ON THAT ONE!", f->fought);
	else n += snprintf(s + n, sizeof s - (size_t)n, "|LAST BOUT WITH\n%s!\nWHAT A BOUT!", f->fought);
	if (f->fought) n += tally(s + n, sizeof s - (size_t)n, f);
	if (f->navis > 1) snprintf(s + n, sizeof s - (size_t)n, "|%u NAVIS HERE.\nPLENTY TO WATCH!", f->navis);
	return s;
}

const char *square_referee_bark(int result, const char *name) {
	static char s[48];
	if (result == 1) return "YOU WIN!";
	if (result == 4) return "BROKE OFF! NO CALL!";
	if (result != 2) return "WHAT A BOUT!";
	snprintf(s, sizeof s, "%s WINS!", name);
	for (char *c = s; *c; ++c) if (*c >= 'a' && *c <= 'z') *c = (char)(*c - 'a' + 'A');
	return s;
}

const char *square_referee_bout_bark(const char *first, const char *second) {
	static char s[64];
	snprintf(s, sizeof s, "%s VS %s!", first, second);
	for (char *c = s; *c; ++c) if (*c >= 'a' && *c <= 'z') *c = (char)(*c - 'a' + 'A');
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

/* the trade board's offers, a box each, from `n` of `s` */
static int offers(char *s, size_t size, int n, const SquareFacts *f) {
	for (int k = 0; k < f->nposts; ++k)
		n += f->posts[k].name ? snprintf(s + n, size - (size_t)n, "|>%s has\n%s,wants\n%s.", f->posts[k].name, f->posts[k].chip, trade_want(f->posts[k].want))
			: snprintf(s + n, size - (size_t)n, "|>Your offer:\n%s for\n%s.", f->posts[k].chip, trade_want(f->posts[k].want));
	if (!f->nposts) n += snprintf(s + n, size - (size_t)n, "|>Trade board:\nno offers up.");
	return n;
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
	if (f->fought) n += snprintf(s + n, sizeof s - (size_t)n, "|>NetBattle vs\n%s: %s", f->fought, f->result == 1 ? "won!" : f->result == 2 ? "lost." : f->result == 4 ? "broke off." : "over.");
	n = offers(s, sizeof s, n, f);
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
	/* (the Undernet's statue is a beast's, not a Navi's, as a player
	 * saw: named as MegaMan can tell it, session 74) */
	case LOOK_STATUE: return "Whoa... A statue of\nsome huge beast!|Navis meet up\nround it.";
	case LOOK_MONUMENT: return "An old monument.\nNavis meet up\nin front of it.";
	case LOOK_GIANT_TREE: return "A giant cybertree.\nIts leaves hum\nsoftly.";
	case LOOK_BRAZIER: return "A brazier.\nIts flame flickers.";
	default: return "A cybertree.";
	}
}

/* where a Navi is, part by part, as MegaMan says it */
static const char *const part_in[SQ_PARTS] = { "in the plaza", "at Trade Central", "in the Arena", "on the stage" };

/* who is in each part but MegaMan's, a box each: "Alpha's in the Arena!",
 * "Alpha and 2 more are on the stage!" */
static int who_where(char *s, size_t n, const SquareFacts *f) {
	int used = 0;
	for (int k = 0; k < SQ_PARTS; ++k) {
		if (k == f->part || !f->in_part[k] || !f->in_name[k]) continue;
		if (f->in_part[k] == 1) used += snprintf(s + used, n - (size_t)used, "|%s's %s!", f->in_name[k], part_in[k]);
		else used += snprintf(s + used, n - (size_t)used, "|%s and %u more are %s!", f->in_name[k], f->in_part[k] - 1, part_in[k]);
		if ((size_t)used >= n) return (int)n - 1;
	}
	return used;
}

/* (a player ran round an empty Square for ten calls and found neither
 * Trade Central's clerk nor the referee, session 74: L names the way to
 * each part, and who is there, so friends find each other) */
const char *square_look_words(const SquareFacts *f) {
	static char s[360];
	int n;
	if (f->part <= SQ_PART_PLAZA)
		n = snprintf(s, sizeof s, "Lan,Trade Central's warp is to the upper left!|The Arena's is to the upper right.|"
			"And the stage's is to the lower left!");
	else n = snprintf(s, sizeof s, "The warp we came in on goes back to the plaza,Lan!");
	who_where(s + n, sizeof s - (size_t)n, f);
	return s;
}

/* (the Arena's braziers and the stage's cybertrees by their warps say it
 * as MegaMan reads the place; Trade Central's board in its own posts) */
const char *square_sign_words(int part, const SquareFacts *f) {
	static char s[720];
	static const char *const sign[SQ_PARTS] = { "", "TRADE CENTRAL BBS\nSpare chips,swapped\none for one.",
		"Lan,look! The\nArena's over there!|Navis NetBattle\nin its ring!", "Lan,that's the\nstage over there!|Up there,everyone\nsees you!" };
	if (part <= SQ_PART_PLAZA || part >= SQ_PARTS) return "";
	int n = snprintf(s, sizeof s, "%s", sign[part]);
	bool post = part == SQ_PART_TRADE;
	if (!f->in_part[part] || !f->in_name[part]) n += snprintf(s + n, sizeof s - (size_t)n, post ? "|>There now:\nnobody yet." : "|Nobody's there yet.");
	else if (f->in_part[part] == 1) n += snprintf(s + n, sizeof s - (size_t)n, post ? "|>There now:\n%s." : "|%s's over there!", f->in_name[part]);
	else n += snprintf(s + n, sizeof s - (size_t)n, post ? "|>There now:\n%s\nand %u more." : "|%s and %u more are there!",
		f->in_name[part], f->in_part[part] - 1);
	if (post) offers(s, sizeof s, n, f);
	return s;
}

void square_warp_label(char *out, size_t n, int part, unsigned navis, const char *name) {
	static const char *const label[SQ_PARTS] = { "PLAZA", "TRADE CENTRAL", "ARENA", "STAGE" };
	if (part < 0 || part >= SQ_PARTS) snprintf(out, n, "%s", "");
	else if (!navis || !name) snprintf(out, n, "%s", label[part]);
	else if (navis == 1) snprintf(out, n, "%s: %s", label[part], name);
	else snprintf(out, n, "%s: %u NAVIS", label[part], navis);
}

const char *square_cheer_bark(int together) { return together >= 3 ? "Yeah!! Go,go!" : "!!"; }

const char *square_greeter_cheer_call(int together) { return together >= 5 ? "WHOA! EVERYBODY!!" : "ALL TOGETHER NOW!"; }

const char *square_parade_bark(void) { return "Parade time!"; }
