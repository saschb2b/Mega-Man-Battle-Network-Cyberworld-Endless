/* The wait for a dropped link drawn (issue #153, docs/online/square/dropped-link.md, A dropped
 * link): a PET panel over the Square held still, in the NetPass's colours
 * and parts (netpass_draw.h): the link's state with a searching signal, what
 * the wait keeps, the time left to try (past a few seconds) and B to go
 * home; then, the link back, the heading alone for a moment. Drawing only:
 * it reads the view it is given and changes nothing. */
#ifndef CW_RECONNECT_DRAW_H
#define CW_RECONNECT_DRAW_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	bool back;          /* the link back: the heading alone, a moment */
	uint32_t elapsed;   /* ms since the link went quiet or dropped */
	uint32_t window;    /* ms it is tried for, from then */
	uint32_t t;         /* ms, for what moves */
} ReconnectView;

/* Over the game's 240 x 160 picture from (x, y) */
void reconnect_draw(int x, int y, const ReconnectView *v);

#endif
