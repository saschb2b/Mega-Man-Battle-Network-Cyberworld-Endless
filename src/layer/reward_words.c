/* The rewards' chats (docs/voice/rules.md): a guardian's Guardian Data, its
 * NaviCust draft and its routes, a Navi gate and its SP chip, a vault, a
 * won Server's pick, an official gate, the first layer's gift. */

#include <stdio.h>
#include <string.h>

#include "data.h"
#include "navicust.h"
#include "reward_pick.h"
#include "script_kit.h"

static int pick_three(TextArchive *t, int kind, int flag, const ScriptsVault *v, const char *open, const char *empty_words, const char *leave_words);

/* The pick's screen asked for (reward_pick.h): its kind's flags, then the
 * one that asks, and the script's end, the chat box closed before it
 * opens over the game; `o` the offer it shows */
static void pick_open(TextArchive *t, int kind, const PickOffer *o) {
	reward_pick_offer(kind, o);
	for (int b = 0; b < REWARD_KIND_BITS; ++b)
		if (kind & 1 << b) ta_flag_set(t, REWARD_KIND_FLAG + b);
	ta_flag_set(t, REWARD_PICK_FLAG);
	ta_end(t);
}

/* the draft's branches: a program given, or BugFrags for none; each sets
 * `taken_flag` and ends, or goes on to the way on (`next`). What was got
 * is BN6's own box alone: the ask before the menu says to install it
 * ("Let's install it!" after the pick, or "We'll move one to make
 * room,Lan!", went with the main path's words cut, docs/game/core-loop.md;
 * BN6's NaviCust shows what moves) */
static int draft_take(TextArchive *t, int program, int color, int taken_flag, int next) {
	int s = ta_script(t);
	uint8_t give[] = { 0xEF, 0x1B, (uint8_t)program, 1, (uint8_t)color };   /* ts_item_give_navi_cust_program */
	ta_bytes(t, give, sizeof give);
	static const uint8_t no_face[] = { 0xF5, 0x01 };   /* (the options kept MegaMan's) */
	ta_bytes(t, no_face, sizeof no_face);
	ta_open(t);
	ta_text(t, "MegaMan got:\n\"");
	ta_program_name(t, program);
	ta_text(t, "\"!!");
	ta_wait(t);
	ta_flag_set(t, taken_flag);
	ta_end_or(t, next);
	return s;
}

/* The draft: MegaMan's line, then its screen (reward_pick.h): the
 * programs, a row each, and the BugFrags of none under them, each to its
 * branch (`take`, `skip`). Its script. A box each said what BN6's
 * programs do: seventeen boxes from the data to the pick, which the owner
 * found far too many (6 October 2026); then a name a line in the chat
 * box, with its colour and the copies held, until the screen (the
 * owner's call, 10 November 2026: room for each program's shape and what
 * it does). */
static int draft_open(TextArchive *t, const ScriptsDraft *draft, const int *take, int skip) {
	int s = ta_script(t);
	/* (the install said here, every draft: a returning playtester, told
	 * once runs before, left one off the board for two acts; L names one
	 * still off it after, briefing_words.c; which Spin turns it, and L and
	 * R, are BN6's own, unsaid: the voice sweep, docs/voice/sweep/net.md) */
	ta_page(t, FACE_MEGAMAN, "Program data! Let's\ninstall one!", false);
	PickOffer o = { .n = (uint8_t)draft->n, .skip = (int16_t)skip, .frags = (int16_t)draft->skip_frags };
	for (int k = 0; k < draft->n && k < PICK_MOST; ++k)
		o.c[k] = (PickChoice){ .what = PICK_PROGRAM, .id = (uint16_t)(draft->program[k] / 4), .code = draft->color[k], .held = draft->held[k], .script = (int16_t)take[k] };
	pick_open(t, PICK_DRAFT, &o);
	return s;
}

static int draft_skip(TextArchive *t, int frags, int taken_flag, int next) {
	int s = ta_script(t);
	uint8_t give[] = { 0xEF, 0x12, (uint8_t)frags, (uint8_t)(frags >> 8), 0, 0, 0xFF, 0xFF, 0xFF };   /* ts_check_give_bug_frags */
	ta_bytes(t, give, sizeof give);
	char line[96];
	snprintf(line, sizeof line, "Traveling light!\nThe data broke into\n%d BugFrags!", frags);
	/* (the box opened again: the pick's screen closed it) */
	ta_page(t, FACE_MEGAMAN, line, true);
	ta_flag_set(t, taken_flag);
	ta_end_or(t, next);
	return s;
}

/* What a Guardian Data gives, in the order its boxes say them */
enum { GOT_CROSS, GOT_HP, GOT_CHIP, GOT_EXP };

static void give(TextArchive *t, const ScriptsReward *r, int kind) {
	if (kind == GOT_HP) {
		ta_give_hp_memory(t, r->hp_memories);
		ta_bytes(t, ta_full_hp, sizeof ta_full_hp);   /* (the heal to the new max, unsaid) */
	} else if (kind == GOT_CHIP) ta_give_chip(t, r->chip, r->code, 1);   /* (to the Pack, as Battle Network gives it) */
	else if (kind == GOT_EXP) {
		/* BN6's own ExpMemry (key item 0x71): the game grows the board and
		 * runs the NaviCust again as it gives it */
		static const uint8_t b[] = { 0xF4, 0x00, SCRIPTS_EXP_MEMORY, 1 };   /* ts_item_give */
		ta_bytes(t, b, sizeof b);
	}
}

/* A guardian's Cross, HPMemory, chip and ExpMemry, and the heal, in BN6's
 * own boxes, two things a box ("MegaMan got:", each in quotes), each
 * pair given just before its box: what was given; nothing of the Pack and
 * the Folder, BN6's own (a playtester counted fourteen calls from a
 * guardian's last words to the way on, session 63), nor of the heal, which
 * the act's end and the way home ask nothing of (the main path's words
 * cut, docs/game/core-loop.md: a box a thing was four). The run's last
 * gives only his chip: no HP or heal for a walk to its exit (a playtester
 * given five HPMemory and a program after the final fight), and it stays
 * in the Library. */
static void reward_items(TextArchive *t, const ScriptsReward *r, bool *first) {
	const char *thing[4];
	int kind[4], n = 0;
	char hp[24], chip[32];
	if (r->cross) { kind[n] = GOT_CROSS; thing[n++] = r->cross; }
	if (!r->last) {
		if (r->hp_memories > 1) snprintf(hp, sizeof hp, "%d HPMemory", r->hp_memories);
		else snprintf(hp, sizeof hp, "HPMemory");
		kind[n] = GOT_HP; thing[n++] = hp;
	}
	if (r->chip > 0) {
		snprintf(chip, sizeof chip, "%s %c", r->chip_name, r->code == 26 ? '*' : 'A' + r->code);
		kind[n] = GOT_CHIP; thing[n++] = chip;
	}
	if (r->draft && r->draft->expmemry) { kind[n] = GOT_EXP; thing[n++] = "ExpMemry"; }
	for (int k = 0; k < n; k += 2) {
		char line[96];
		give(t, r, kind[k]);
		if (k + 1 < n) {
			give(t, r, kind[k + 1]);
			snprintf(line, sizeof line, "MegaMan got:\n\"%s\" and\n\"%s\"!!", thing[k], thing[k + 1]);
		} else snprintf(line, sizeof line, "MegaMan got:\n\"%s\"!!", thing[k]);
		ta_page(t, FACE_NONE, line, *first);
		*first = false;
	}
	if (r->last && r->chip > 0) ta_page(t, FACE_MEGAMAN, "It's in our Library\nfor good,Lan!", false);
}

int ta_guardian_reward(TextArchive *t, const ScriptsReward *r) {
	/* (the way on, then the draft's branches first: the choices jump to
	 * them) */
	int next = -1;
	int take[3] = { 0 }, n = r->draft ? r->draft->n : 0, menu = -1;
	for (int k = 0; k < n; ++k)
		take[k] = draft_take(t, r->draft->program[k], r->draft->color[k], r->taken_flag, next);
	if (n) menu = draft_open(t, r->draft, take, draft_skip(t, r->draft->skip_frags, r->taken_flag, next));
	int i = ta_script(t);
	bool first = true;
	/* (BN6's own "MegaMan got:" boxes lead, and its own rules go unsaid:
	 * the Cross's weakness, the Pack, ExpMemry's board, what a program
	 * does; a super boss's data has its own first line and words, and a
	 * power more to say before the items: a Cross kept, a Soul, the Nest) */
	if (r->head) {
		ta_page(t, FACE_NONE, r->head, first);
		first = false;
	}
	if (r->power) ta_pages(t, r->power, FACE_NONE, &first);
	reward_items(t, r, &first);
	if (!n) {
		ta_flag_set(t, r->taken_flag);
		ta_end_or(t, next);
		return i;
	}
	ta_jump(t, menu);
	return i;
}

int ta_navi_gate(TextArchive *t, int flag, const char *navi, int beaten, int needed) {
	char s[240];
	if (beaten < needed) {
		/* (the telegraph first: which code, and how far along) */
		snprintf(s, sizeof s, "@M It's sealed with %s's code,Lan.|@M If we delete %s %s as a guardian...|@M ...in any dive,it'd crack! %s", navi, navi,
			needed == 2 ? "twice" : "again", beaten >= 1 ? "Just once more!" : "Not even once yet...");
		return ta_say(t, FACE_MEGAMAN, s);
	}
	int quiet = ta_say(t, FACE_MEGAMAN, "The gate's open,Lan.|Nothing's left inside.");
	int no = ta_closing(t);
	int i = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)quiet, 0xFF };  /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	bool first = true;
	snprintf(s, sizeof s, "%s's code opens it!|%s SP waits inside!|The toughest fight on this layer!", navi, navi);
	ta_pages(t, s, FACE_MEGAMAN, &first);
	ta_ask_in(t, FACE_MEGAMAN, "His SP chip's the\nprize! Take him on?\n", no, true, true);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_gate_reward(TextArchive *t, const char *navi, int chip, const char *chip_name, int code) {
	int i = ta_script(t);
	bool first = true;
	char s[96];
	snprintf(s, sizeof s, "Alright! %s SP's data is ours!", navi);
	ta_page(t, FACE_MEGAMAN, s, true);
	first = false;
	ta_give_chip(t, chip, code, 1);
	ta_got_chip(t, chip_name, code, &first);
	ta_end(t);
	return i;
}

int ta_round_prize(TextArchive *t, int chip, const char *chip_name, int code) {
	int i = ta_script(t);
	bool first = true;
	ta_give_chip(t, chip, code, 1);
	ta_got_chip(t, chip_name, code, &first);
	ta_end(t);
	return i;
}

int ta_vault(TextArchive *t, int flag, int need, int have, const ScriptsVault *v) {
	char s[240];
	if (have < need) {
		/* (the telegraph first: the count it wants, and ours) */
		snprintf(s, sizeof s, "@M A vault,Lan! With a collector's lock.|@M It opens for a Library of %d chips.|@M Ours has %d.|"
			"@M Every chip we get counts,from any dive!", need, have);
		return ta_say(t, FACE_MEGAMAN, s);
	}
	snprintf(s, sizeof s, "Our Library of %d opens the lock!|Three rare chips inside!|We can take one,Lan.", have);
	return pick_three(t, PICK_VAULT, flag, v, s, "The vault's open,Lan.|We took our pick.", "Let's leave them for now.|The vault will keep.");
}

int ta_server_pick(TextArchive *t, int taken_flag, const ScriptsVault *v) {
	return pick_three(t, PICK_SERVER, taken_flag, v, "Look,Lan! The signal left chips!|We can take one!", "The signal's quiet now,Lan.",
		"Let's leave them for now.|We can come back!");
}

int ta_copy_pick(TextArchive *t, int taken_flag, const ScriptsVault *v) {
	return pick_three(t, PICK_COPY, taken_flag, v, "It's gone... The layer's back to normal.|And it left its data! We can take one!",
		"It's quiet here now,Lan.", "Let's leave them for now.|We can come back!");
}

/* Three chips, one to take (event flag `flag` set as it is, after which
 * `empty` is said instead), on the pick's screen (reward_pick.h), which
 * shows each one's card; B leaves them (`leave`). A pick on the chat box
 * asked once more ("Take TrplShot J?") on a playtester's A pressed
 * through the words before it, which took the first for good (session
 * 55): the screen's keys rest as it opens instead (reward_pick_state.h). */
static int pick_three(TextArchive *t, int kind, int flag, const ScriptsVault *v, const char *open, const char *empty_words, const char *leave_words) {
	int empty = ta_say(t, FACE_MEGAMAN, empty_words);
	PickOffer o = { .n = 3, .skip = (int16_t)ta_say(t, FACE_MEGAMAN, leave_words) };
	for (int k = 0; k < 3; ++k) {
		o.c[k] = (PickChoice){ .what = PICK_CHIP, .id = (uint16_t)v->chip[k], .code = (uint8_t)v->code[k], .script = (int16_t)ta_script(t) };
		bool first = true;
		ta_give_chip(t, v->chip[k], v->code[k], 1);
		ta_got_chip(t, v->name[k], v->code[k], &first);
		ta_flag_set(t, flag);
		ta_end(t);
	}
	int i = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)empty, 0xFF };  /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	bool first = true;
	ta_pages(t, open, FACE_MEGAMAN, &first);
	pick_open(t, kind, &o);
	return i;
}

int ta_official(TextArchive *t, int flag, int open_flag, int level, int won, bool duel_prize, const ScriptsVault *v) {
	char s[300];
	snprintf(s, sizeof s, level >= 2 ? "Chaud's full clearance opens it!|The official vault,Lan!|Three Mega chips. We can take one."
		: "Chaud's first clearance opens it!|An official Chip Order,Lan!|Chips we've held,delivered!|We can order one.");
	int open = pick_three(t, level >= 2 ? PICK_OFFICIAL : PICK_ORDER, flag, v, s, "The gate's open,Lan.|We took our pick.", "Let's leave them for now.|The gate will keep.");
	/* sealed until `open_flag`, which the director sets where Chaud's
	 * clearance reaches the gate's level, as the layer begins or as a duel
	 * on it is won (a gate beside the duel opens at once): the telegraph
	 * first, whose clearance, and how far we are */
	if (duel_prize)
		snprintf(s, sizeof s, "@M ProtoMan's official gate,Lan.|@M %s|@M It opens if we beat him here!",
			level >= 2 ? "The official vault's behind it.|@M Three Mega chips!" : "An official Chip Order's behind it!|@M Three chips we've held. We order one.");
	else if (level >= 2)
		snprintf(s, sizeof s, "@M An official gate,Lan!|@M It needs Chaud's full clearance.|@M The official vault's behind it. Three Mega chips!|"
			"@M That takes three duel wins against ProtoMan.|@M The last is a NetBattle with him. We have %d.", won);
	else
		snprintf(s, sizeof s, "@M An official gate,Lan!|@M It needs Chaud's first clearance.|@M A Chip Order's behind it. Three chips we've held!|"
			"@M We get to order one.|@M One duel win against ProtoMan opens it.|@M Not yet,though!");
	int i = ta_script(t);
	uint8_t check[] = { 0xEF, 0x00, (uint8_t)open_flag, (uint8_t)(open_flag >> 8), (uint8_t)open, 0xFF };  /* ts_check_flag */
	ta_bytes(t, check, sizeof check);
	bool first = true;
	ta_pages(t, s, FACE_MEGAMAN, &first);
	ta_end(t);
	return i;
}

int ta_gift(TextArchive *t, const ScriptsGift *g) {
	/* what each choice gives, then the flag that it was chosen */
	int thanks = ta_say(t, FACE_PROG, "GOOD LUCK,MEGAMAN!|DIVE AS DEEP AS YOU CAN!");
	int hp = ta_script(t);
	bool first = true;
	ta_give_hp_memory(t, 2);
	ta_got_hp(t, 2, &first);
	ta_flag_set(t, g->flag);
	ta_end(t);
	int chipped = ta_script(t);
	first = true;
	ta_give_chip(t, g->chip, g->code, 1);
	ta_got_chip(t, g->chip_name, g->code, &first);
	/* (a new chip goes to the pack, as in BN6: the folder is the player's,
	 * and BN6's own, unsaid) */
	ta_flag_set(t, g->flag);
	ta_end(t);
	int programmed = ta_script(t);
	uint8_t give_program[] = { 0xEF, 0x1B, (uint8_t)g->program, 1, (uint8_t)g->color };   /* ts_item_give_navi_cust_program */
	ta_bytes(t, give_program, sizeof give_program);
	static const uint8_t no_face[] = { 0xF5, 0x01 };   /* (the options kept Mr. Prog's) */
	ta_bytes(t, no_face, sizeof no_face);
	ta_open(t);
	ta_text(t, "MegaMan got:\n\"");
	ta_program_name(t, g->program);
	ta_text(t, "\"!!");
	ta_wait(t);
	ta_flag_set(t, g->flag);
	ta_end(t);

	int i = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)g->flag, (uint8_t)(g->flag >> 8), (uint8_t)thanks, 0xFF };  /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	first = true;
	/* (a returning player: one page, not three; a playtester called the
	 * chats of a restart after a long run a chore) */
	/* (the extras first, then the pick: given just before the options,
	 * a head start's two HPMemory read as the first option given twice) */
	bool extras = g->head_start || g->comfort;
	if (g->brief) ta_pages(t, extras ? "YOU'RE BACK,MEGAMAN!" : "YOU'RE BACK,MEGAMAN!|PICK YOUR GIFT!",
		FACE_PROG, &first);
	else ta_pages(t, "HELLO,MEGAMAN!|I'M DR.HIKARI'S DIVE SUPPORT PROGRAM!|SO HERE'S A GIFT!", FACE_PROG, &first);
	if (g->head_start) {
		/* the head-start helper (docs/game/meta/) */
		ta_page(t, FACE_PROG, "FIRST,A HEAD START!\nTAKE THESE!", false);
		ta_give_hp_memory(t, 2);
		ta_got_hp(t, 2, &first);
	}
	if (g->comfort) {
		/* the last dive ended early: a little more help */
		ta_page(t, FACE_PROG, "LAST DIVE ENDED\nEARLY,HUH? HERE,\nTAKE THIS,TOO!", false);
		ta_give_hp_memory(t, 1);
		ta_got_hp(t, 1, &first);
	}
	if (!g->brief) ta_page(t, FACE_PROG, "NOW PICK ONE!", false);
	else if (extras) ta_page(t, FACE_PROG, "NOW,PICK YOUR GIFT!", false);
	/* the three on the pick's screen (reward_pick.h), each one's card
	 * saying what the chip hits for and what the program does, where two
	 * of Mr.Prog's boxes said them before the chat box's options; B waits
	 * for a pick */
	PickOffer o = { .n = 3, .skip = PICK_B_NOTHING };
	o.c[0] = (PickChoice){ .what = PICK_HP, .id = 2, .script = (int16_t)hp };
	o.c[1] = (PickChoice){ .what = PICK_CHIP, .id = (uint16_t)g->chip, .code = (uint8_t)g->code, .script = (int16_t)chipped };
	o.c[2] = (PickChoice){ .what = PICK_PROGRAM, .id = (uint16_t)(g->program / 4), .code = (uint8_t)g->color, .script = (int16_t)programmed };
	pick_open(t, PICK_GIFT, &o);
	return i;
}
