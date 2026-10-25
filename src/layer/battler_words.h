/* What a Virus Battler Mr.Prog says (battler_words.c; docs/VOICE.md):
 * his offer on the layer, and his word after each round of his series. */
#ifndef CW_BATTLER_WORDS_H
#define CW_BATTLER_WORDS_H

#include <stdbool.h>

#include "battler.h"
#include "text.h"

/* His offer: the BtlrCard first where the profile holds none (BN6's own
 * key item, Mettaur's and Gunner's data on it, BATTLER_CARD_FLAG set),
 * his rules then said in full; his first pair, the stake, a question that
 * starts on No, and on Yes the stake taken and `flag` set (not enough
 * zenny: he says so); once BATTLER_DONE_FLAG is set, his word that the
 * series is over. */
typedef struct {
	int flag;
	int stake, rounds;
	const char *pair;    /* his first pair (battler_pair) */
	bool card;           /* the profile holds the BtlrCard */
	int face;            /* his own Mr.Prog's mugshot (battler_sprite: BN6's face for a Navi of list 6 is his sprite's) */
} BattlerOffer;
int ta_battler(TextArchive *t, const BattlerOffer *b);

/* His word after round `round` (1-based) of `rounds`, its BN6_RESULT_*:
 * a win short of the last, the pot and the next pair, then TAKE THE POT
 * (B too: the pot given, BATTLER_DONE_FLAG) or FIGHT ON (BATTLER_ON_FLAG);
 * the last won, the pot given, and the first prize where `prize` names
 * one (its flag `prize_flag` set); a loss or a draw, the pot gone
 * (BATTLER_DONE_FLAG). Its script in `t`. */
typedef struct {
	int result, round, rounds, pot;
	const char *next;
	const BattlerPrize *prize;   /* NULL for none */
	int prize_flag;
	int face;
} BattlerAfter;
int ta_battler_after(TextArchive *t, const BattlerAfter *a);

#endif
