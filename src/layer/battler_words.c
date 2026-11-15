/* A Virus Battler Mr.Prog's words (battler_words.h): BN6's minigame in his
 * voice, ALL CAPITALS (docs/voice/rules.md, Mr.Prog). His rules are said once a
 * profile, with the BtlrCard; after that he names the pair and the price,
 * and a later talk on his layer is the pair and the question. */
#include "battler_words.h"

#include <stdio.h>

#include "bn6.h"
#include "script_kit.h"

/* What his answers lead to: not enough zenny, No, and the stake paid */
typedef struct { int broke, no, paid; } Answers;

static Answers answers(TextArchive *t, const BattlerOffer *b) {
	char s[80];
	snprintf(s, sizeof s, "NOT ENOUGH ZENNY!|COME BACK WITH %d!", b->stake);
	Answers a;
	a.broke = ta_say(t, b->face, s);
	a.no = ta_say(t, b->face, "OK! ANYTIME!");
	a.paid = ta_script(t);
	ta_flag_set(t, b->flag);
	bool first = false;
	ta_pages(t, "ALL RIGHT! LET'S VIRUS BATTLE!", b->face, &first);
	ta_end(t);
	return a;
}

/* The pair after `lead` (its boxes, or its line in the pair's box), the
 * stake and the question, then the stake taken, into the script under
 * way: the box opened (`first`) */
static void offer(TextArchive *t, const BattlerOffer *b, const Answers *a, bool first, const char *lead) {
	char s[200];
	snprintf(s, sizeof s, "%sMY FIRST PAIR...\n%s!", lead, b->pair);
	ta_pages(t, s, b->face, &first);
	snprintf(s, sizeof s, "ENTER FOR\n%d ZENNY?\n", b->stake);
	ta_ask_in(t, b->face, s, a->no, !first, true);
	ta_take_zenny(t, b->stake, a->paid, a->broke);
	ta_end(t);
}

/* The BtlrCard, BN6's own: its key item and the flags BN6's HeelNavi sets
 * as he sells it (the card, Mettaur's and Gunner's data), and his rules */
static void card(TextArchive *t, int stake, int face) {
	bool first = true;
	ta_pages(t, "HI THERE! I'M A VIRUS BATTLER!|YOUR VIRUSES BATTLE MINE!|BUT FIRST,TAKE THIS!", face, &first);
	uint8_t give[] = { 0xF4, 0x00, BN6_ITEM_BTLRCARD, 1 };   /* ts_item_give */
	ta_bytes(t, give, sizeof give);
	ta_flag_set(t, BN6_FLAG_BTLRCARD);
	ta_flag_set(t, BN6_FLAG_VIRUS_DATA + 1);
	ta_flag_set(t, BN6_FLAG_VIRUS_DATA + 23);
	ta_flag_set(t, BATTLER_TOLD_FLAG);
	ta_got(t, "BtlrCard", &first);
	char s[240];
	snprintf(s, sizeof s, "METTAUR AND GUNNER ARE ON IT!|DELETE A RARE VIRUS,AND IT JOINS THEM!|@M A card of viruses,Lan!|"
		"ENTRY'S %d ZENNY.|EACH WIN ADDS %d TO THE POT!|STOP AFTER ANY WIN. IT'S YOURS!|@L And if we lose?|THEN IT'S ALL MINE!",
		stake, stake);
	ta_pages(t, s, face, &first);
}

int ta_battler(TextArchive *t, const BattlerOffer *b) {
	char lead[80];
	int done = ta_say(t, b->face, "THAT'S MY SERIES FOR TODAY!|SEE YOU AROUND THE NET!");
	Answers a = answers(t, b);
	/* (his words said: a later talk on his layer is a box and the
	 * question, as a shop's "Back for more?"; a playtester heard his
	 * rounds and rule again after a No, session 76) */
	int again = ta_script(t);
	offer(t, b, &a, true, "OH! YOU'RE BACK!\n");
	/* (the card once a profile: a later Battler's first talk his rounds
	 * and the pot's rule) */
	int ask = ta_script(t);
	ta_flag_set(t, BATTLER_TOLD_FLAG);
	snprintf(lead, sizeof lead, "VIRUS BATTLE! %d ROUNDS!|EACH WIN ADDS %d ZENNY!|", b->rounds, b->stake);
	offer(t, b, &a, true, lead);
	int i = ta_script(t);
	ta_if_flag(t, BATTLER_DONE_FLAG, done);
	ta_if_flag(t, BATTLER_TOLD_FLAG, again);
	if (b->card) {
		ta_jump(t, ask);
		return i;
	}
	card(t, b->stake, b->face);
	snprintf(lead, sizeof lead, "%d ROUNDS!|", b->rounds);
	offer(t, b, &a, false, lead);
	return i;
}

/* The pot given, as BN6 gives zenny */
static void pot(TextArchive *t, int zenny, bool *first) {
	char what[24];
	ta_give_zenny(t, zenny);
	snprintf(what, sizeof what, "%d Zenny", zenny);
	ta_got(t, what, first);
}

/* A win short of the series' end: the pot, the next pair, and the choice */
static int won(TextArchive *t, const BattlerAfter *a) {
	char s[200];
	int take = ta_script(t);
	bool first = false;
	pot(t, a->pot, &first);
	ta_flag_set(t, BATTLER_DONE_FLAG);
	ta_pages(t, "SMART! SEE YOU AROUND THE NET!", a->face, &first);
	ta_end(t);
	int on = ta_script(t);
	ta_flag_set(t, BATTLER_ON_FLAG);
	ta_page(t, a->face, "THAT'S THE SPIRIT! HERE THEY COME!", false);
	ta_end(t);
	int i = ta_script(t);
	first = true;
	snprintf(s, sizeof s, "YOU WIN!! THE POT'S\n%d ZENNY!|ROUND %d OF %d!\nNEXT PAIR...\n%s!", a->pot, a->round + 1, a->rounds, a->next);
	ta_pages(t, s, a->face, &first);
	ta_mugshot(t, a->face);
	ta_clear(t);
	static const char *const labels[2] = { "TAKE THE POT", "FIGHT ON" };
	ta_options(t, labels, 2);
	int to[2] = { take, on };
	ta_choose(t, to, 2, take);
	return i;
}

/* The series won: the pot, and a first win's prize */
static int series_won(TextArchive *t, const BattlerAfter *a) {
	int i = ta_script(t);
	bool first = true;
	ta_pages(t, "YOU BEAT MY WHOLE SERIES!!|WOW,WOW!", a->face, &first);
	pot(t, a->pot, &first);
	const BattlerPrize *p = a->prize;
	if (p && p->id) {
		ta_pages(t, "AND FOR YOUR FIRST WIN...|TAKE THIS!", a->face, &first);
		if (p->chip) {
			ta_give_chip(t, p->id, p->code, 1);
			ta_got_chip(t, p->name, p->code, &first);
		} else {
			uint8_t give[] = { 0xEF, 0x1B, (uint8_t)p->id, 1, (uint8_t)p->code };   /* ts_item_give_navi_cust_program */
			ta_bytes(t, give, sizeof give);
			ta_got(t, p->name, &first);
		}
		ta_flag_set(t, a->prize_flag);
	}
	ta_flag_set(t, BATTLER_DONE_FLAG);
	ta_end(t);
	return i;
}

int ta_battler_after(TextArchive *t, const BattlerAfter *a) {
	if (a->result == BN6_RESULT_WON) return a->round < a->rounds ? won(t, a) : series_won(t, a);
	int i = ta_script(t);
	bool first = true;
	char s[160];
	if (a->result == BN6_RESULT_DRAW) snprintf(s, sizeof s, "TIME UP! A DRAW!|A DRAW GOES TO ME!|THE %d ZENNY'S MINE!", a->pot);
	else snprintf(s, sizeof s, "MY VIRUSES WIN!|THE %d ZENNY'S MINE! THANKS!", a->pot);
	ta_pages(t, s, a->face, &first);
	ta_flag_set(t, BATTLER_DONE_FLAG);
	ta_end(t);
	return i;
}
