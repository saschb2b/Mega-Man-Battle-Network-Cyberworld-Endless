/* The Arena from the Net Square's state, shared by its two parts
 * (square_arena.h): the challenge, the way in and the battle
 * (square_arena.c), and the result card's course with the two PET
 * screens drawn (square_card.c). */
#ifndef CW_SQUARE_ARENA_STATE_H
#define CW_SQUARE_ARENA_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "arena.h"
#include "arena_card.h"
#include "arena_link_net.h"
#include "online_snapshot.h"
#include "online_types.h"
#include "square_arena.h"

#define OVER_TICKS      600      /* a stop's card shown before the Square */
#define OVER_HELD       30       /* ... at least, before a key counts */
#define FADE            12       /* frames of a fade: the panel and the battle from white, the card from black and to it */
#define STALL_NOTE      60       /* ticks waiting on the other's keys before the note names whose */
#define STALL_OFFER     600      /* ... before B leaves (a press meant for the Buster before) */
#define STALL_END       1800     /* ... before the battle stops by itself, broken off */
#define AGAIN_MS        45000    /* a rematch's answer kept, from the battle's end: the card's time */

typedef enum {
	SA_IDLE,
	SA_FOLDER,      /* the folder to carry asked, before my challenge or my Yes */
	SA_SENT,        /* my challenge sent: the server's word awaited */
	SA_WAITING,     /* ... the other's answer */
	SA_ASKING,      /* challenged: the question to ask once no chat is open */
	SA_ASKED,       /* ... open */
	SA_ACCEPTED,    /* Yes said: START awaited */
	SA_LOADOUT,     /* START: the other's MegaMan awaited (from here the Square holds) */
	SA_CONNECTING,  /* the way in */
	SA_READY,       /* the other's ready awaited */
	SA_BATTLE,
	SA_OVER,        /* the result card, or a stop's, on the picture */
} SquareArenaStage;

typedef struct {
	SquareArenaStage stage;
	uint32_t match, other, since, latency;
	char name[ONLINE_NAME_BYTES + 1];
	bool ended;               /* the server has said this battle is over */
	bool challenger;          /* this game challenged (the folder asked before the challenge) */
	int folder;               /* the folder carried, 0-2 */
	OnlineSnapshot s;
	ArenaPings pings;         /* the round trips measured, for the delay asked for (arena_sync.h) */
	uint32_t notice_serial;   /* the notices' serial as my challenge went */
	uint32_t quiet_notice;    /* a refusal's come while the Square was held: never shown in the corner */
	ArenaNetLink link;
	Arena *arena;
	OnlineLoadout mine, theirs;
	bool mine_carried, theirs_carried, theirs_in;
	uint32_t t;               /* frames of the screen shown: the panel, the battle, the card */
	bool counted;             /* the battle's end told to the statistics */
	const char *said;         /* MegaMan's word, once back in the Square */
	char why[160];            /* a stop's cause, for the log */
	ArenaStop stop;           /* ... its PET line on the card */
	/* the result card, up from the battle's end (or a stop) until it
	 * closes: the rematch asked on it, Lan's cursor, the other's answer */
	struct {
		bool up;
		int closing;          /* frames of its fade out, 0 none */
		unsigned result;      /* SquareBout's, 5 stopped before the battle */
		ArenaTally tally;
		ArenaAsk ask;
		bool yes;
		ArenaTheirs theirs;
	} card;
} SquareArenaState;
extern SquareArenaState SA;

/* the last battle this game fought from the Square, for its referee */
typedef struct { SquareBout bout; uint32_t other; char name[ONLINE_NAME_BYTES + 1]; } SquareArenaLast;
extern SquareArenaLast SA_LAST;

/* the rematch: the last battle's other player and folder, Lan's answer,
 * and a challenge of his come before it */
enum { AGAIN_UNSAID, AGAIN_YES, AGAIN_NO };
typedef struct {
	uint32_t other, since, held;
	char name[ONLINE_NAME_BYTES + 1];
	int folder, answer;
} SquareArenaAgain;
extern SquareArenaAgain SA_AGAIN;

/* square_arena.c's, for the card: a player's name by his session (NULL
 * gone from the Square); back on the Square's map, the card closed; the
 * rematch's challenge of his taken, or ours sent */
const char *square_arena_name_of(uint32_t id);
void square_arena_back(void);
void square_arena_again_take(uint32_t match);
void square_arena_again_send(void);

/* square_card.c's: the card up from a battle's end or a stop (`result`
 * SquareBout's, 5 stopped before the battle), from black, the rematch
 * asked on it or not; the other gone or his No; its frame (keys, time,
 * the fade out back to the Square) */
void square_card_up(unsigned result, ArenaAsk ask);
void square_card_gone(void);
void square_card_frame(void);
/* ... and the stall note's way out: B, once the note offers it */
bool square_card_leave(void);

#endif
