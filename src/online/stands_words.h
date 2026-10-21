/* What the stands say (stands_words.c, issue #128, docs/ONLINE.md, The
 * stands; docs/VOICE.md): the referee Mr.Prog calling the channel's
 * NetBattle as it goes (who fights whom, their HP, the last chip each
 * used, the winner, the cheers) and asking whether to watch, and the
 * Board's posts on it. Composed from the server's report. */
#ifndef CW_STANDS_WORDS_H
#define CW_STANDS_WORDS_H

#include <stdbool.h>

/* A battle as the lines tell it */
typedef struct {
	const char *name[2];   /* the cable's first and second players' */
	bool on;               /* under way (else over) */
	bool report;           /* its HP known */
	unsigned hp[2];
	const char *chip[2];   /* the last chip each used, NULL none (or one this game cannot name) */
	unsigned watchers, cheers;
	int winner;            /* 0 none (broke off, or not over), 1 or 2 the side */
	bool mine;             /* this game fought it */
	unsigned bout;         /* a Team NetBattle's bout, its number; 0 a lone battle */
} StandsBout;

/* The referee on the battle on (`live`) and the last one over (`last`),
 * either NULL; "" where neither */
const char *stands_referee_words(const StandsBout *live, const StandsBout *last);
/* ... his question where this game can watch it, and his word on a Yes */
const char *stands_watch_question(void);
const char *stands_watch_off(void);
/* The Board's posts on them ("" none) */
const char *stands_board_words(const StandsBout *live, const StandsBout *last);

#endif
