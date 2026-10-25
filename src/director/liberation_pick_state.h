/* The team pick's state, shared by its course (liberation_pick.c) and its
 * picture (liberation_pick_draw.c). */
#ifndef CW_LIBERATION_PICK_STATE_H
#define CW_LIBERATION_PICK_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "liberation_board.h"

/* The list's rows shown at once (the window's field), the held Navis' one
 * row each, then GO! under the window: the row after the last Navi's */
#define PICK_ROWS_SHOWN 8

typedef struct {
	int places;               /* beside MegaMan: the board's */
	int n;                    /* the Navis held */
	uint8_t navi[LIB_HELD];   /* ... MATE_*, in the run's order (lib_team_held's) */
	uint8_t order[LIB_HELD];  /* ... each one's place in the pick, 1 on (0 not picked) */
	int cursor;               /* its row: a Navi's, or n for GO! */
	int first;                /* the list's first row shown */
	int t;                    /* frames since it opened: its fade */
} LibPickState;

extern LibPickState LP;

/* How many are picked */
int lib_pick_count(void);

#endif
