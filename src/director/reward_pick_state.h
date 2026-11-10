/* The reward pick's state, shared by its course (reward_pick.c) and its
 * picture (reward_pick_draw.c). */
#ifndef CW_REWARD_PICK_STATE_H
#define CW_REWARD_PICK_STATE_H

#include <stdbool.h>

#include "reward_pick.h"

/* Keys rest this long after it opens, its fade in among them, and A counts
 * only once it has rested PICK_REST frames on it: an A pressed through the
 * lines before it took the first of three, for good, when the pick was the
 * chat box's (session 55), and a mash through Mr.Prog's lines ran on into
 * the screen past its fade */
#define PICK_LOCK 30
#define PICK_REST 20

typedef struct {
	bool up;
	int kind;            /* PICK_* */
	PickOffer o;         /* what it offers */
	int cursor;          /* a choice's row, or o.n for the skip's */
	int t;               /* frames since it opened */
	uint32_t was;        /* the keys of the frame before */
	int rest;            /* frames since A was last held on it */
} PickState;

extern PickState RP;

/* The rows: the choices, and the skip's where it has one */
int reward_pick_rows(void);

#endif
