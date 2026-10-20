/* What the Net Square's life says (square_life_words.c, docs/ONLINE.md,
 * The Square; docs/VOICE.md): the greeter and the referee Mr.Progs, the
 * two walking Navis' talk, the Board's posts, and what MegaMan finds
 * checking the props, each composed from the facts the client has. */
#ifndef CW_SQUARE_LIFE_WORDS_H
#define CW_SQUARE_LIFE_WORDS_H

#include <stdbool.h>

/* The facts the lines are made of. */
typedef struct {
	const char *names[3];   /* Navis here by their players' names, MegaMan's not among them */
	int nnames;
	unsigned navis;         /* here, MegaMan with them */
	unsigned channel, channels;
	int hour;               /* the local clock's, 0-23 */
	int area;               /* the area the Square wears (0 Central, 2 Sky, 3 Green) */
	int landmark;           /* its centrepiece's LOOK_* */
	const char *arrived;    /* the last Navi to log in while MegaMan was here, NULL none */
	const char *traded;     /* the last player this game traded with, NULL none */
	const char *fought;     /* ... and fought a NetBattle with, NULL none */
	int result;             /* that battle's: 1 won, 2 lost, else over */
	unsigned won, lost;     /* the bouts with him in a row, that one among them */
	bool arena;             /* this system offers NetBattles (not the New 3DS: square_arena_offered) */
	bool stamp;             /* ... and this game's NetPass holds the Arena stamp (netpass.h) */
	bool parade;            /* the walkers are on parade round the landmark */
	bool battle_on;         /* a NetBattle under way in the channel (the stands: square_watch.h) */
	struct {                /* Trade Central's board: the offers up (online_board.c) */
		const char *name;   /* the poster, NULL MegaMan's own */
		char chip[32];      /* the chip he has, "Cannon A" */
		int want;           /* what he wants for it (WANT_*, trade_want's words) */
	} posts[4];
	int nposts;
	unsigned turn;          /* which line this time */
} SquareFacts;

/* The greeter in the log-in plaza, and his word over his head as a Navi
 * logs in (`in`) or out. */
const char *square_greeter_words(const SquareFacts *f);

const char *square_greeter_bark(const char *name, bool in);
/* The referee by the Arena's ring, and his call over his head as a
 * battle of this game's ends (`result` 1 won, 2 lost, else over; `name`
 * the other's). */
const char *square_referee_words(const SquareFacts *f);
const char *square_referee_bark(int result, const char *name);
/* Walker `k` (0 a GreenNavi, 1 a GirlNavi) spoken to. */
const char *square_walker_words(int k, const SquareFacts *f);
/* A prop checked: its LOOK_* (the Board's posts, the sign's words). */
const char *square_check_words(int look, const SquareFacts *f);
/* Over the ambient Navis' heads when MegaMen emote together at the
 * landmark or on the stage, and over the first as a parade begins. */
const char *square_cheer_bark(void);
const char *square_parade_bark(void);

#endif
