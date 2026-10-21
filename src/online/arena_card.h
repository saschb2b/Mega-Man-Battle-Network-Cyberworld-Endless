/* The Arena's two PET screens (issue #167, docs/ONLINE_ARENA.md, The way
 * in and the result): the way in as a Network panel (both players, each
 * MegaMan as carried, the set's score, whose PET is still on its way),
 * and the result as a card (vs whom, how long, the HP left and chips
 * used, the set's score, the rematch asked on it and the other's answer
 * as it comes). Drawn on the game's 240x160 picture from the project's
 * own drawing in the PET's colours (second_frame.h), never the ROM's
 * graphics; the words are the PET's (arena_text.h). Draws only. */
#ifndef CW_ARENA_CARD_H
#define CW_ARENA_CARD_H

#include <stdbool.h>
#include <stdint.h>

#include "arena_text.h"
#include "online_snapshot.h"

/* Whose wait a side of the way in is */
typedef enum { ARENA_SIDE_COMING, ARENA_SIDE_LINKING, ARENA_SIDE_READY } ArenaSideWait;

typedef struct {
	const char *name[2];              /* mine, the other's */
	const OnlineLoadout *loadout[2];  /* as carried, NULL the game's start's MegaMan */
	bool known[2];                    /* ... come: the other's, once his MegaMan has */
	ArenaSideWait wait[2];
	int progress;                     /* my way in, 0-100 */
	bool set;                         /* bouts with him before this one: the score shown */
	unsigned won, lost;
	uint32_t t;                       /* the panel's frames, for its blink */
} ArenaWayView;
void arena_card_way(const ArenaWayView *v);

/* The rematch on the card */
typedef enum {
	ARENA_ASK_NONE,      /* none offered: back alone */
	ARENA_ASK_OPEN,      /* asked, the cursor on `yes` or not */
	ARENA_ASK_SENT,      /* my Yes, his answer awaited */
} ArenaAsk;
/* ... and the other's answer as it comes */
typedef enum { ARENA_THEIRS_UNSAID, ARENA_THEIRS_YES, ARENA_THEIRS_GONE } ArenaTheirs;

typedef struct {
	const char *name;          /* the other's */
	unsigned result;           /* 1 won, 2 lost, 3 over otherwise, 4 broken off, 5 stopped before it began */
	const char *why;           /* a stop's line, NULL none */
	bool tally;                /* the battle read: the rows below shown */
	uint32_t frames;           /* fought */
	uint16_t hp[2], max_hp[2]; /* mine, his */
	unsigned used[2];
	bool set;                  /* a bout with him before: the score shown */
	unsigned won, lost;
	ArenaAsk ask;
	bool yes;                  /* the cursor */
	ArenaTheirs theirs;
	bool square;               /* back to the Net Square, else to the title */
	uint32_t t;
} ArenaCardView;
void arena_card_result(const ArenaCardView *v);

/* The picture faded toward black (`white` false) or white by `alpha` (0-255) */
void arena_card_fade(int alpha, bool white);

/* The screens' sounds, BN6's own, picked here: a screen opened (the panel,
 * the card), closed, the cursor moved, Yes, No, the other's answer come,
 * into the battle */
typedef enum { ARENA_CUE_OPEN, ARENA_CUE_CLOSE, ARENA_CUE_CURSOR, ARENA_CUE_YES, ARENA_CUE_NO, ARENA_CUE_ANSWER, ARENA_CUE_BATTLE } ArenaCue;
void arena_card_cue(ArenaCue cue);

#endif
