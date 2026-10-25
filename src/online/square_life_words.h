/* What the Net Square's life says (square_life_words.c, docs/online/square/dropped-link.md,
 * The Square; docs/voice/rules.md): the greeter and the referee Mr.Progs, the
 * two walking Navis' talk, the Board's posts, and what MegaMan finds
 * checking the props, each composed from the facts the client has. */
#ifndef CW_SQUARE_LIFE_WORDS_H
#define CW_SQUARE_LIFE_WORDS_H

#include <stdbool.h>
#include <stddef.h>

#include "square_plan.h"

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
	int result;             /* that battle's: 1 won, 2 lost, 4 broken off, else over */
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
	unsigned in_part[SQ_PARTS];      /* the Navis in each part (SQ_PART_*), MegaMan not among them */
	const char *in_name[SQ_PARTS];   /* ... one of them by name, NULL none */
	int part;                        /* the part MegaMan is in, -1 none */
} SquareFacts;

/* The greeter in the welcome plaza, and his word over his head as a Navi
 * logs in (`in`) or out. */
const char *square_greeter_words(const SquareFacts *f);

const char *square_greeter_bark(const char *name, bool in);
/* The referee by the Arena's ring, and his call over his head as a
 * battle of this game's ends (`result` 1 won, 2 lost, 4 broken off, else over; `name`
 * the other's). */
const char *square_referee_words(const SquareFacts *f);
const char *square_referee_bark(int result, const char *name);
/* ... and his call while a bout of the channel is under way, its two
 * fighters (the cable's first's first; epic #167, P20) */
const char *square_referee_bout_bark(const char *first, const char *second);
/* Walker `k` (0 a GreenNavi, 1 a GirlNavi) spoken to. */
const char *square_walker_words(int k, const SquareFacts *f);
/* A prop checked: its LOOK_* (the Board's posts, the sign's words). */
const char *square_check_words(int look, const SquareFacts *f);
/* L's Look around (square_life_look): in the plaza, which way each warp
 * lies (they stand off the plaza the same way in every Square,
 * square_plan.c: Trade Central's to its upper left, the Arena's to its
 * upper right, the stage's to its lower left); in a part, the way back;
 * then who is where. */
const char *square_look_words(const SquareFacts *f);
/* The sign by part `part`'s warp in the plaza, checked: what awaits
 * there, and who is there now. */
const char *square_sign_words(int part, const SquareFacts *f);
/* The label over a warp's gem: the part it goes to, and how many Navis are
 * there now (`navis`; `name` one of them). */
void square_warp_label(char *out, size_t n, int part, unsigned navis, const char *name);
/* Over the ambient Navis' heads when `together` MegaMen emote together at
 * the landmark or on the stage, and over the first as a parade begins;
 * the greeter's call over his head when four or more do (issue #176). */
const char *square_cheer_bark(int together);
const char *square_greeter_cheer_call(int together);
const char *square_parade_bark(void);

#endif
