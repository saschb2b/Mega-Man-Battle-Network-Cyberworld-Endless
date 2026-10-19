/* What is said of the week's net in the Square (square_week_words.c,
 * docs/ONLINE.md, The week's net; docs/VOICE.md): the greeter Mr.Prog's
 * post and question beside the Board, and the Board's dives. Nobody says
 * seed, run or ranking: the net shifted, Navis dive it, the Board lists
 * how deep they say they got. */
#ifndef CW_SQUARE_WEEK_WORDS_H
#define CW_SQUARE_WEEK_WORDS_H

#include <stdbool.h>
#include <stdint.h>

#include "online_week.h"

/* What the greeter offers (square_week.c) */
typedef enum {
	WEEK_OFFER_NONE,       /* nothing: no week from this server */
	WEEK_OFFER_NEW,        /* this week's net, a dive to begin */
	WEEK_OFFER_AGAIN,      /* MegaMan's unfinished dive, to keep on with or begin again */
	WEEK_OFFER_ON,         /* MegaMan is on his dive now */
	WEEK_OFFER_TRADE,      /* a trade waits to go home first */
} WeekOffer;

enum { WEEK_AGAIN_OPTIONS = 3 };
extern const char *const square_week_again_options[WEEK_AGAIN_OPTIONS];

/* The greeter's post on the week's net (pages, '|' between boxes), after
 * his welcome; `posted` the layer of MegaMan's dive the Board just took
 * (0 none). */
const char *square_week_post(WeekOffer offer, uint16_t week, int posted);
/* ... his question where he asks one (its end a '\n'): Yes/No for a new
 * dive, NULL where there is none or the options ask */
const char *square_week_question(WeekOffer offer);
/* ... and his word as MegaMan goes to dive */
const char *square_week_off(void);
/* The Board's posts of the week's net: its dates, its deepest dives as
 * reported, MegaMan's own, last week's three. */
const char *square_week_board(const WeekBoard *b, uint16_t week);

#endif
